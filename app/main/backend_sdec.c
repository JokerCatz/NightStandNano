// SPDX-License-Identifier: LGPL-2.1-or-later
// NIGHTSTAND NANO: this copy differs from ESP32S31_round/app/main/backend_sdec.c in one place (search "NANO CHANGE": end-of-file drain). Re-apply after re-copying.
#include "backend_sdec.h"
#include <stdlib.h>
#include <string.h>
#include "esp_heap_caps.h"
#include "pc/pc_mp4.h"
#include "decoder/impl/esp_aac_dec.h"
#include "decoder/impl/esp_alac_dec.h"
#include "decoder/impl/esp_opus_dec.h"
#include "decoder/impl/esp_vorbis_dec.h"
#include "simple_dec/esp_audio_simple_dec.h"
#include "simple_dec/esp_audio_simple_dec_default.h"

#define IN_SIZE 4096
typedef struct {
    pc_reader_t r;
    uint32_t off;
    esp_audio_simple_dec_handle_t d;
    uint8_t in[IN_SIZE];
    uint32_t in_len, in_pos;
    int16_t *pcm;
    uint32_t pcm_cap;
    int pcm_frames, pcm_pos, ch, bad, bits;
    uint32_t rate;
    uint64_t played;
    int drain_miss; bool drain;                          /* NANO CHANGE: the file is read to its end; only the decoder's own queue is left */
    pc_mp4_fs_t *fs;                     /* M4A with moov after mdat: the rearranging reader (freed on close) */
} aac_t;

/* 1 a frame is ready, 0 no data, -1 undecodable. Bytes the decoder did not take stay in the buffer: it wants more
 * of the same frame appended, not a fresh start (dropping them cut frames: ~3 decode errors a second). */
static int next_frame(aac_t *a)
{
    bool need_more = !a->drain && a->in_pos >= a->in_len;
    for (;;) {
        if (need_more) {
            if (a->in_pos) { memmove(a->in, a->in + a->in_pos, a->in_len - a->in_pos); a->in_len -= a->in_pos; a->in_pos = 0; }
            if (a->in_len == IN_SIZE) { a->in_len = 0; if (++a->bad > 200) return -1; }   /* a whole buffer without a frame */
            int got = a->r.read(a->r.u, a->off, a->in + a->in_len, IN_SIZE - a->in_len);
            if (got <= 0) {                                  /* NANO CHANGE (differs from round): end of file. The Ogg demuxer swallows whole pages and hands out one */
                a->drain = true;                             /* packet per call, so what is still queued inside it is collected by feeding it 32 zero bytes per call */
                a->in_len = a->in_pos = 0;                   /* until nothing comes out (without this a 6 s Ogg lost its last 1.2 s; an Opus file up to a third) */
            } else {
                a->off += (uint32_t)got;
                a->in_len += (uint32_t)got;
            }
            need_more = false;
        }
        if (a->drain) { memset(a->in, 0, 32); a->in_pos = 0; a->in_len = 0; }
        esp_audio_simple_dec_raw_t raw = { .buffer = a->in + a->in_pos, .len = a->drain ? 32 : a->in_len - a->in_pos };
        esp_audio_simple_dec_out_t out = { .buffer = (uint8_t *)a->pcm, .len = a->pcm_cap };
        esp_audio_err_t rc = esp_audio_simple_dec_process(a->d, &raw, &out);
        if (rc == ESP_AUDIO_ERR_BUFF_NOT_ENOUGH) {
            int16_t *p = heap_caps_realloc(a->pcm, out.needed_size, MALLOC_CAP_SPIRAM);
            if (!p) return -1;
            a->pcm = p;
            a->pcm_cap = out.needed_size;
            continue;
        }
        a->in_pos += raw.consumed;
        if (a->drain) a->in_pos = a->in_len = 0;                   /* NANO CHANGE: the 32 fed bytes are not buffered input (in_pos past in_len would corrupt the next memmove) */
        if (a->drain && (rc != ESP_AUDIO_ERR_OK || !out.decoded_size) && ++a->drain_miss > 3) return 0;        /* NANO CHANGE: drained */
        if (rc != ESP_AUDIO_ERR_OK) {
            if (!raw.consumed) a->in_pos++;                  /* step past what it cannot use */
            if (++a->bad > 200) return -1;
            if (a->in_pos >= a->in_len) need_more = true;
            continue;
        }
        if (!out.decoded_size) {
            if (!raw.consumed || a->in_pos >= a->in_len) need_more = true;
            continue;
        }
        if (!a->rate) {
            esp_audio_simple_dec_info_t info;
            if (esp_audio_simple_dec_get_info(a->d, &info) != ESP_AUDIO_ERR_OK || !info.sample_rate || !info.channel) return -1;
            a->rate = info.sample_rate;
            a->ch = info.channel;
            a->bits = info.bits_per_sample ? info.bits_per_sample : 16;
            if (a->bits != 16) return -1;                     /* the decoders here give 16-bit PCM */
        }
        a->bad = 0;
        a->pcm_frames = (int)(out.decoded_size / 2 / (uint32_t)a->ch);
        a->pcm_pos = 0;
        return 1;
    }
}

static int sdec_open(void **h, const pc_reader_t *r, uint32_t start_off, esp_audio_simple_dec_type_t type)
{
    static bool reg;
    if (!reg) {   /* only these: the default set brings esp_audio_codec's own minimp3, which clashes with ours */
        esp_aac_dec_register(); esp_vorbis_dec_register(); esp_opus_dec_register();
        esp_alac_dec_register();   /* library .m4a (AAC/ALAC); .wav has its own backend (pc_backend_wav) */
        esp_audio_simple_dec_register_default();
        reg = true;
    }
    aac_t *a = heap_caps_calloc(1, sizeof *a, MALLOC_CAP_SPIRAM);
    if (!a) return -1;
    a->r = *r;
    a->off = start_off;
    a->pcm_cap = 4096 * 2 * 2;                       /* HE-AAC 2048, Vorbis up to 4096 samples (it grows if needed) */
    a->pcm = heap_caps_malloc(a->pcm_cap, MALLOC_CAP_SPIRAM);
    esp_aac_dec_cfg_t c = ESP_AAC_DEC_CONFIG_DEFAULT();
    c.aac_plus_enable = true;
    esp_audio_simple_dec_cfg_t cfg = { .dec_type = type };
    if (type == ESP_AUDIO_SIMPLE_DEC_TYPE_AAC) { cfg.dec_cfg = &c; cfg.cfg_size = sizeof c; }
    if (!a->pcm || esp_audio_simple_dec_open(&cfg, &a->d) != ESP_AUDIO_ERR_OK || next_frame(a) != 1) {
        if (a->d) esp_audio_simple_dec_close(a->d);
        free(a->pcm);
        free(a);
        return -1;
    }
    *h = a;
    return 0;
}

static int aac_open(void **h, const pc_reader_t *r, uint32_t o) { return sdec_open(h, r, o, ESP_AUDIO_SIMPLE_DEC_TYPE_AAC); }
static int ogg_open(void **h, const pc_reader_t *r, uint32_t o) { return sdec_open(h, r, o, ESP_AUDIO_SIMPLE_DEC_TYPE_OGG); }
static int m4a_open(void **h, const pc_reader_t *r, uint32_t o)   /* always from the start */
{
    (void)o;
    pc_reader_t v;
    pc_mp4_fs_t *fs = NULL;
    int k = pc_mp4_faststart(r, &v, &fs);   /* the parser refuses moov after mdat (ffmpeg's default): reorder on the fly */
    if (k < 0) return -1;
    if (sdec_open(h, k ? &v : r, 0, ESP_AUDIO_SIMPLE_DEC_TYPE_M4A)) { pc_mp4_faststart_free(fs); return -1; }
    ((aac_t *)*h)->fs = fs;
    return 0;
}

static int aac_render(void *h, float *out, int frames)
{
    aac_t *a = h;
    int done = 0;
    while (done < frames) {
        if (a->pcm_pos >= a->pcm_frames) {
            int rc = next_frame(a);
            if (rc < 0) return done ? done : -1;
            if (rc == 0) break;
        }
        int n = a->pcm_frames - a->pcm_pos;
        if (n > frames - done) n = frames - done;
        const int16_t *s = a->pcm + a->pcm_pos * a->ch;
        for (int i = 0; i < n; i++) {
            float l = s[i * a->ch] * (1.0f / 32768), rr = a->ch > 1 ? s[i * a->ch + 1] * (1.0f / 32768) : l;
            out[2 * (done + i)] = l;
            out[2 * (done + i) + 1] = rr;
        }
        a->pcm_pos += n;
        done += n;
    }
    a->played += (uint64_t)done;
    return done;
}

static uint32_t aac_rate(void *h) { return ((aac_t *)h)->rate; }
static int32_t aac_len(void *h) { (void)h; return -1; }
static int32_t aac_pos(void *h) { aac_t *a = h; return a->rate ? (int32_t)(a->played * 1000 / a->rate) : 0; }
static void aac_close(void *h)
{
    aac_t *a = h;
    esp_audio_simple_dec_close(a->d);
    pc_mp4_faststart_free(a->fs);
    free(a->pcm);
    free(a);
}

#define OPS .render = aac_render, .rate = aac_rate, .length_ms = aac_len, .position_ms = aac_pos, .close = aac_close
const pc_backend_t backend_aac = { .open = aac_open, OPS, .name = "AAC" };
const pc_backend_t backend_ogg = { .open = ogg_open, OPS, .name = "OGG" };
const pc_backend_t backend_m4a = { .open = m4a_open, OPS, .name = "M4A" };
