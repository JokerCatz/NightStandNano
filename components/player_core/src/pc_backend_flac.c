// SPDX-License-Identifier: LGPL-2.1-or-later
/* FLAC files from the library (dr_flac). The reader is unbuffered on the device (mp3_ctl), so reads go
 * through our own 16 KB buffer. Every rate is delivered native (hires T13): the audio task halves what the output cannot take
 * (pc_halve, the one place for halving); this backend never resamples. */
#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_STDIO
#define DR_FLAC_NO_OGG
#include "dr_flac.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pc/pc_backend.h"
#include "pc/pc_rate.h"

#define RBUF 16384
#define CHUNK 512                    /* input frames decoded per step */

typedef struct {
    pc_reader_t r;
    uint32_t off;                    /* reader position dr_flac sees */
    uint8_t *buf;                    /* RBUF bytes cached from buf_off */
    uint32_t buf_off, buf_len;
    drflac *fl;
    uint32_t in_rate;
    uint64_t in_pos;                 /* input frames decoded (resume token, position) */
    float *in;                       /* CHUNK * ch input frames */
    int in_n, in_i;                  /* decoded / consumed in `in` */
    struct ra *ra;                   /* HISPEED: card read-ahead task (above 192 kHz, device only) */
} flac_t;

#ifdef ESP_PLATFORM
/* HISPEED card read-ahead. The card gives about 3.5 MB/s on the FakePod and a read of the next 16 KB blocks the decoder for its whole time (4.4 s of every 10 s at 384 kHz); a
 * 384 kHz file wants 1.3-1.8 MB/s, so the card is idle half the time. A task on core 0 (the SD host and the screen are there; its waiting costs no CPU) keeps RA_SLOTS reads
 * of RBUF ahead of the decoder, which then copies from RAM. Sequential access only: a jump (seek, resume) restarts the task at the new place. The task owns r.read while it runs
 * (the decoder never calls it itself), and flac_close waits for it before the file may be closed. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_heap_caps.h"
#define RA_ABOVE_RATE 192000         /* the read-ahead runs for files above this rate (352.8 / 384 kHz) */
#define RA_SLOTS 6
#define RA_PRIO 6
#define RA_WAIT_MS 1500
typedef struct ra {
    const pc_reader_t *r;
    uint8_t *mem;                    /* RA_SLOTS * RBUF, PSRAM, 128-byte aligned: the card DMAs straight into it */
    uint32_t start;                  /* file offset of slot 0 of this run (a multiple of 512) */
    volatile uint32_t wcnt, rcnt;    /* slots filled / released; slot k lives at (k % RA_SLOTS), file offset start + k * RBUF */
    volatile int32_t len[RA_SLOTS];
    volatile bool eof, stop, alive;
    SemaphoreHandle_t to_cons, to_prod;
} ra_t;

static void ra_task(void *arg)
{
    ra_t *a = arg;
    while (!a->stop) {
        const uint32_t w = __atomic_load_n(&a->wcnt, __ATOMIC_ACQUIRE), r = __atomic_load_n(&a->rcnt, __ATOMIC_ACQUIRE);
        if (a->eof || w - r >= RA_SLOTS) { xSemaphoreTake(a->to_prod, pdMS_TO_TICKS(100)); continue; }
        const uint32_t idx = w % RA_SLOTS;
        int got = a->r->read(a->r->u, a->start + w * RBUF, a->mem + (size_t)idx * RBUF, RBUF);
        a->len[idx] = got > 0 ? got : 0;
        __atomic_store_n(&a->wcnt, w + 1, __ATOMIC_RELEASE);
        if (got < RBUF) __atomic_store_n(&a->eof, true, __ATOMIC_RELEASE);   /* after the slot is published: eof with no slot ahead means no more data */
        xSemaphoreGive(a->to_cons);
    }
    a->alive = false;
    vTaskDeleteWithCaps(NULL);
}

static void ra_free(ra_t *a)
{
    if (!a) return;
    a->stop = true;
    xSemaphoreGive(a->to_prod);
    for (int i = 0; i < 500 && a->alive; i++) vTaskDelay(1);
    if (a->alive) { printf("APP flac read-ahead task did not stop: memory kept\n"); return; }   /* never free what the task may still write */
    vSemaphoreDelete(a->to_cons);
    vSemaphoreDelete(a->to_prod);
    heap_caps_free(a->mem);
    free(a);
}

static ra_t *ra_new(const pc_reader_t *r, uint32_t from)
{
    ra_t *a = calloc(1, sizeof *a);
    if (!a) return NULL;
    a->r = r;
    a->start = from & ~(uint32_t)511;
    a->mem = heap_caps_aligned_alloc(128, (size_t)RA_SLOTS * RBUF, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    a->to_cons = xSemaphoreCreateBinary();
    a->to_prod = xSemaphoreCreateBinary();
    a->alive = true;
    if (!a->mem || !a->to_cons || !a->to_prod ||
        xTaskCreatePinnedToCoreWithCaps(ra_task, "flac_ra", 6144, a, RA_PRIO, NULL, 0, MALLOC_CAP_SPIRAM) != pdPASS) {
        a->alive = false;
        ra_free(a);
        return NULL;
    }
    return a;
}
#endif

static size_t on_read(void *u, void *out, size_t n)
{
    flac_t *f = u;
    size_t done = 0;
#ifdef ESP_PLATFORM
    while (f->ra && done < n) {
        ra_t *a = f->ra;
        const uint32_t cur = a->rcnt, soff = a->start + cur * RBUF;
        if (f->off < soff || f->off >= soff + 2 * (uint32_t)RBUF) {          /* not the slot in use or the next one: a jump, restart there */
            ra_free(a);
            f->ra = ra_new(&f->r, f->off);
            if (!f->ra) break;                                               /* no task: the direct reads below */
            continue;
        }
        if (f->off >= soff + RBUF) {                                         /* the next slot: release this one */
            __atomic_store_n(&a->rcnt, cur + 1, __ATOMIC_RELEASE);
            xSemaphoreGive(a->to_prod);
            continue;
        }
        if (__atomic_load_n(&a->wcnt, __ATOMIC_ACQUIRE) <= cur) {            /* not read yet: wait for the task */
            const bool e = __atomic_load_n(&a->eof, __ATOMIC_ACQUIRE);       /* eof first, then the count: a slot published before eof shows here */
            if (__atomic_load_n(&a->wcnt, __ATOMIC_ACQUIRE) <= cur) {
                if (e) return done;                                          /* the task stopped at the end of the file or on an error */
                const BaseType_t got = xSemaphoreTake(a->to_cons, pdMS_TO_TICKS(RA_WAIT_MS));
                if (got != pdTRUE) return done;                              /* the card did not answer in 1.5 s: a short read, the decoder reports the error */
                continue;                                                    /* look again */
            }
        }
        const int32_t len = a->len[cur % RA_SLOTS];
        const uint32_t at = f->off - soff;
        if (at >= (uint32_t)len) return done;                                /* the end of the file */
        uint32_t k = (uint32_t)len - at;
        if (k > n - done) k = (uint32_t)(n - done);
        memcpy((uint8_t *)out + done, a->mem + (size_t)(cur % RA_SLOTS) * RBUF + at, k);
        done += k;
        f->off += k;
    }
    if (f->ra) return done;
#endif
    while (done < n) {
        if (f->off < f->buf_off || f->off >= f->buf_off + f->buf_len) {
            int got = f->r.read(f->r.u, f->off, f->buf, RBUF);
            if (got <= 0) break;
            f->buf_off = f->off;
            f->buf_len = (uint32_t)got;
        }
        uint32_t at = f->off - f->buf_off, k = f->buf_len - at;
        if (k > n - done) k = (uint32_t)(n - done);
        memcpy((uint8_t *)out + done, f->buf + at, k);
        done += k;
        f->off += k;
    }
    return done;
}

static drflac_bool32 on_seek(void *u, int offset, drflac_seek_origin origin)
{
    flac_t *f = u;
    int64_t to = origin == DRFLAC_SEEK_SET ? offset : origin == DRFLAC_SEEK_CUR ? (int64_t)f->off + offset : (int64_t)f->r.size + offset;
    if (to < 0 || (f->r.size && to > f->r.size)) return DRFLAC_FALSE;
    f->off = (uint32_t)to;
    return DRFLAC_TRUE;
}

static drflac_bool32 on_tell(void *u, drflac_int64 *pos) { *pos = ((flac_t *)u)->off; return DRFLAC_TRUE; }

static void free_all(flac_t *f)
{
#ifdef ESP_PLATFORM
    ra_free(f->ra);
#endif
    if (f->fl) drflac_close(f->fl);
    free(f->in);
    free(f->buf);
    free(f);
}

static int flac_open(void **h, const pc_reader_t *r, uint32_t start_off)
{
    if (!r->read) return -1;
    flac_t *f = calloc(1, sizeof *f);
    if (!f) return -1;
    f->r = *r;
    if (f->r.u == r) f->r.u = &f->r;          /* memory readers point at themselves */
    f->buf = malloc(RBUF);
    if (!f->buf) { free(f); return -1; }
    f->fl = drflac_open(on_read, on_seek, on_tell, f, NULL);
    if (!f->fl || !f->fl->channels || !f->fl->sampleRate) { free_all(f); return -1; }
    f->in_rate = f->fl->sampleRate;
    f->in = malloc((size_t)CHUNK * f->fl->channels * sizeof(float));
    if (!f->in) { free_all(f); return -1; }
    if (start_off) {                          /* resume: the token is the input frame index */
        if (!drflac_seek_to_pcm_frame(f->fl, start_off)) { free_all(f); return -1; }
        f->in_pos = start_off;
    }
#ifdef ESP_PLATFORM
    if (f->in_rate > RA_ABOVE_RATE) {
        f->ra = ra_new(&f->r, f->off);
        if (!f->ra) printf("APP flac read-ahead FAILED rate=%lu: direct reads, the card blocks the decoder (underruns)\n", (unsigned long)f->in_rate);   /* only the failure, at an open (a user action) */
    }
#endif
    *h = f;
    return 0;
}

static inline float clip(float v) { return v > 1.0f ? 1.0f : v < -1.0f ? -1.0f : v; }

static int flac_render(void *h, float *out, int frames)
{
    flac_t *f = h;
    int ch = (int)f->fl->channels, done = 0;
    while (done < frames) {
        if (f->in_i >= f->in_n) {
            drflac_uint64 n = drflac_read_pcm_frames_f32(f->fl, CHUNK, f->in);
            if (!n) break;
            f->in_n = (int)n;
            f->in_i = 0;
        }
        int k = f->in_n - f->in_i;
        if (k > frames - done) k = frames - done;
        const float *s = f->in + (size_t)f->in_i * ch;
        float *o = out + 2 * done;
        if (ch == 2) {
            for (int i = 0; i < 2 * k; i++) o[i] = clip(s[i]);
        } else {
            for (int i = 0; i < k; i++) {
                float l = s[(size_t)i * ch], rr = ch > 1 ? s[(size_t)i * ch + 1] : l;
                o[2 * i] = clip(l);
                o[2 * i + 1] = clip(rr);
            }
        }
        f->in_i += k;
        f->in_pos += (uint64_t)k;
        done += k;
    }
    return done;
}

static uint32_t flac_rate(void *h) { flac_t *f = h; return f->in_rate; }
static int32_t flac_length(void *h)
{
    flac_t *f = h;
    return f->fl->totalPCMFrameCount ? (int32_t)(f->fl->totalPCMFrameCount * 1000 / f->in_rate) : -1;
}
static int32_t flac_pos(void *h) { flac_t *f = h; return (int32_t)(f->in_pos * 1000 / f->in_rate); }
static void flac_close(void *h) { free_all(h); }
static uint32_t flac_byte_pos(void *h) { return (uint32_t)((flac_t *)h)->in_pos; }
static uint8_t flac_bits(void *h) { return ((flac_t *)h)->fl->bitsPerSample; }
static uint32_t flac_burst(void *h) { return ((flac_t *)h)->fl->maxBlockSizeInPCMFrames; }

const pc_backend_t pc_backend_flac = {
    .open = flac_open, .render = flac_render, .rate = flac_rate,
    .length_ms = flac_length, .position_ms = flac_pos, .close = flac_close, .byte_pos = flac_byte_pos,
    .name = "FLAC", .bits = flac_bits, .burst = flac_burst,
};
