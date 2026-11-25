// SPDX-License-Identifier: LGPL-2.1-or-later
/* Tracker modules (MOD XM S3M IT and ~50 more: pc_xmp_ext) from the library, through libxmp 4.7.2 (components/libxmp).
 * libxmp reads the whole module into its own structures while loading (through our 16 KB read cache,
 * freed right after), so nothing touches the card during playback. On the device every libxmp
 * allocation goes to PSRAM (components/libxmp/port). The song plays once: rendering stops when
 * libxmp's loop counter moves (the song came back to its loop point) so the library moves on.
 * Resume token (byte_pos): the order (song position) being played; open() starts at that order. */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <xmp.h>
#include "pc/pc_backend.h"
#include "pc/pc_fmtreason.h"
#include "pc/pc_voice.h"
#include "pc/pc_vmute.h"
#if defined(__has_include)
#if __has_include("esp_attr.h")
#include "esp_attr.h"
#endif
#endif
#ifndef EXT_RAM_BSS_ATTR
#define EXT_RAM_BSS_ATTR
#endif

#define RATE 44100                   /* the chiptunes and most MP3s run at 44.1 kHz: no DAC rate switch */
#define RBUF 16384

typedef struct {
    pc_reader_t r;
    uint32_t off;
    uint8_t *buf;                    /* load-time read cache: RBUF bytes from buf_off */
    uint32_t buf_off, buf_len;
} xio_t;

typedef struct {
    xmp_context ctx;
    bool playing, ended;
    const int16_t *frame;            /* the current libxmp frame (stereo int16), `avail` frames, `used` consumed */
    int avail, used;
    int pos;                         /* order of the current frame */
    int32_t time_ms, length_ms;
    struct xmp_frame_info fi;        /* ~1.7 KB: kept in the handle, not on the audio task's stack */
    int nch;                         /* module channels shown as voices: at most PC_VOICES_MAX */
    int chn;                         /* all module channels (up to XMP_MAX_CHANNELS); those beyond nch have no voice row: muted by a solo, never by the mask */
    uint32_t mask;                   /* the mute mask given to libxmp (pc_vmute): bit i = channel i */
    bool extra;                      /* channels nch..chn-1 are muted (a solo is set) */
    int amiga;                       /* the Amiga mode applied to this module (PC_AMIGA_*), -1 = not yet */
    bool amod;                       /* an Amiga MOD (libxmp's Paula mixer type is available for it) */
} xmp_t;

/* Per-voice levels: the classic tracker trigger meter, plus the note (period table of the module: note + pitchbend), a key-on counter and the channel pan. One snapshot (PSRAM), written by the audio task, read by
 * the UI task through pc_backend_xmp_voices(); no lock (a torn read is one stale level for one frame), `n` is written last when it
 * opens and first (to 0) when it closes. A pre-opened next module (gapless) leaves it alone: the first block a handle renders takes
 * it over (s_owner), and closing a handle clears it only if that handle owns it. */
EXT_RAM_BSS_ATTR static pc_voices_t s_voices;   /* 322 bytes, PSRAM (voice snapshot v2) */
static const xmp_t *s_owner;

EXT_RAM_BSS_ATTR static volatile int s_amiga;         /* setting amiga_mix, set by the app (pc_backend_xmp_set_amiga); read once per render block */
EXT_RAM_BSS_ATTR static volatile bool s_amiga_on;     /* the owner handle's state, for the UI */
void pc_backend_xmp_set_amiga(int mode) { s_amiga = mode < 0 || mode >= PC_AMIGA_N ? 0 : mode; }
bool pc_backend_xmp_amiga_active(void) { return s_amiga_on; }

/* put the Amiga mode into the player flags of a module (cheap: libxmp picks the mixer set at every frame; a module that is not an Amiga MOD ignores the flags) */
static void amiga_apply(xmp_t *x)
{
    const int want = s_amiga;
    if (want == x->amiga) return;
    int fl = xmp_get_player(x->ctx, XMP_PLAYER_CFLAGS);
    if (fl < 0) return;
    fl &= ~(XMP_FLAGS_A500 | XMP_FLAGS_LEDON | XMP_FLAGS_A1200);
    if (want != PC_AMIGA_OFF) fl |= XMP_FLAGS_A500;
    if (want == PC_AMIGA_A500LED) fl |= XMP_FLAGS_LEDON;
    if (want == PC_AMIGA_A1200) fl |= XMP_FLAGS_A1200;
    xmp_set_player(x->ctx, XMP_PLAYER_CFLAGS, fl);
    x->amiga = want;
    x->amod = want != PC_AMIGA_OFF && xmp_get_player(x->ctx, XMP_PLAYER_MIXER_TYPE) != XMP_MIXER_STANDARD;   /* only an Amiga MOD reports an A500 mixer type */
    if (s_owner == x) s_amiga_on = x->amod;
}

int pc_backend_xmp_voices(pc_voices_t *out) { return pc_voice_read(&s_voices, out); }

/* one libxmp frame into the levels: a channel with a new note on the first frame of a row jumps to its volume (0..64 -> 0..255),
 * otherwise it falls */
static void voices_frame(const xmp_t *x)     /* decay: once per libxmp frame (20 ms at 125 BPM), not per render call */
{
    for (int i = 0; i < x->nch; i++) {
        const struct xmp_channel_info *c = &x->fi.channel_info[i];
        uint8_t now = 0;
        if (x->fi.frame == 0 && c->event.note > 0 && c->event.note < XMP_KEY_OFF) {
            unsigned v = (unsigned)c->volume * 255u / 64u;
            now = v > 255u ? 255 : (uint8_t)v;
        }
        if (x->mask >> i & 1) now = 0;                         /* a muted channel keeps its volume register: it reads silent */
        s_voices.level[i] = pc_voice_decay(s_voices.level[i], now);
        const bool strike = x->fi.frame == 0 && c->event.note > 0 && c->event.note < XMP_KEY_OFF;
        if (strike && !(x->mask >> i & 1)) s_voices.strikes[i]++;
        /* xmp note 1 = C-0 = MIDI 12; pitchbend is the linear bend from the base note, 100 per semitone */
        if (c->note > 0 && c->note < XMP_KEY_OFF) {
            int bend = c->pitchbend < -600 ? -600 : c->pitchbend > 600 ? 600 : c->pitchbend;
            int cents = ((int)c->note + 11) * 100 + bend;
            s_voices.cents[i] = (int16_t)(cents < 0 ? 0 : cents > 12700 ? 12700 : cents);
            s_voices.flags[i] = PC_VF_PITCH;
        } else { s_voices.cents[i] = PC_VOICE_NOPITCH; s_voices.flags[i] = 0; }
        { int pn = (int)c->pan - 128; s_voices.pan[i] = (int8_t)(pn < -127 ? -127 : pn > 127 ? 127 : pn); }
    }
}

/* before the block is rendered: the mute mask the UI posted reaches libxmp (only the handle that owns the voices) */
static void mute_apply(xmp_t *x)
{
    if (s_owner != x) return;
    const uint32_t want = pc_vmute_mask();
    const bool extra = pc_vmute_extra_muted();
    if (want != x->mask) {
        for (int i = 0; i < x->nch; i++) if (((want ^ x->mask) >> i) & 1) xmp_channel_mute(x->ctx, i, (int)(want >> i & 1));
        x->mask = want;
    }
    if (extra != x->extra) {                         /* wave-6 #21: a solo silences the channels the 32 mask bits cannot reach */
        for (int i = x->nch; i < x->chn; i++) xmp_channel_mute(x->ctx, i, extra);
        x->extra = extra;
    }
}

static void voices_take(xmp_t *x)
{
    pc_voice_publish_n(&s_voices, 0);
    s_owner = x;
    memset(s_voices.level, 0, sizeof s_voices.level);
    for (int i = 0; i < PC_VOICES_MAX; i++) pc_voice_clear_extra(&s_voices, i);
    pc_vmute_reset();                /* a new module: every channel on */
    for (int i = 0; i < x->chn; i++) xmp_channel_mute(x->ctx, i, 0);
    x->mask = 0;
    x->extra = false;
    for (int i = 0; i < x->nch; i++) pc_voice_short(NULL, i, s_voices.name[i]);   /* "1".."32" */
    voices_frame(x);                 /* the frame open() primed: its first row's notes count */
    pc_voice_publish_n(&s_voices, (uint8_t)x->nch);
    s_amiga_on = x->amod;
}

static unsigned long io_read(void *dest, unsigned long size, unsigned long nmemb, void *priv)
{
    xio_t *io = priv;
    if (!size || !nmemb) return 0;
    unsigned long want = size * nmemb, done = 0;
    while (done < want) {
        if (io->off < io->buf_off || io->off >= io->buf_off + io->buf_len) {
            if (io->off >= io->r.size) break;
            int got = io->r.read(io->r.u, io->off, io->buf, RBUF);
            if (got <= 0) break;
            io->buf_off = io->off;
            io->buf_len = (uint32_t)got;
        }
        uint32_t at = io->off - io->buf_off, k = io->buf_len - at;
        if (k > want - done) k = (uint32_t)(want - done);
        memcpy((uint8_t *)dest + done, io->buf + at, k);
        done += k;
        io->off += k;
    }
    return done / size;              /* fread: whole items */
}

static int io_seek(void *priv, long offset, int whence)
{
    xio_t *io = priv;
    long to = whence == SEEK_SET ? offset : whence == SEEK_CUR ? (long)io->off + offset : (long)io->r.size + offset;
    if (to < 0) return -1;
    io->off = (uint32_t)to;
    return 0;
}

static long io_tell(void *priv) { return (long)((xio_t *)priv)->off; }

static void x_close(void *h)
{
    xmp_t *x = h;
    if (!x) return;
    if (s_owner == x) { pc_voice_publish_n(&s_voices, 0); s_owner = NULL; s_amiga_on = false; pc_vmute_reset(); }
    if (x->ctx) {
        if (x->playing) xmp_end_player(x->ctx);
        xmp_free_context(x->ctx);    /* also releases a loaded module */
    }
    free(x);
}

/* next libxmp frame into x->frame; false at the end of the song */
static bool next_frame(xmp_t *x)
{
    if (x->ended) return false;
    if (xmp_play_frame(x->ctx) != 0) { x->ended = true; return false; }
    xmp_get_frame_info(x->ctx, &x->fi);
    if (x->fi.loop_count > 0) { x->ended = true; return false; }   /* back at the loop point: played once */
    x->frame = x->fi.buffer;
    x->avail = x->fi.buffer_size / (2 * (int)sizeof(int16_t));
    x->used = 0;
    x->pos = x->fi.pos;
    x->time_ms = x->fi.time - x->fi.frame_time / 1000;   /* fi.time is already past this frame */
    if (s_owner == x) voices_frame(x);
    return true;
}

static int x_open(void **h, const pc_reader_t *r, uint32_t start_off)
{
    if (!r->read || !r->size) return -1;
    /* an S3M with AdLib (OPL) instruments: libxmp loads it but plays those voices silent, so it is refused (pc_fmtreason says why) as it always was */
    if (pc_refuse_reason(r, ".s3m", NULL, 0) == PC_REFUSE_OPL_MODULE) return -1;
    xmp_t *x = calloc(1, sizeof *x);
    xio_t io = { .r = *r, .buf = malloc(RBUF) };
    if (io.r.u == r) io.r.u = &io.r;         /* memory readers point at themselves */
    if (!x || !io.buf || !(x->ctx = xmp_create_context())) goto fail;
    /* libxmp's callback loader rejects a NULL priv; it reads only during this call */
    struct xmp_callbacks cb = { .read_func = io_read, .seek_func = io_seek, .tell_func = io_tell, .close_func = NULL };
    int rc = xmp_load_module_from_callbacks(x->ctx, &io, cb);
    free(io.buf);
    io.buf = NULL;
    if (rc != 0) goto fail;
    struct xmp_module_info mi;
    xmp_get_module_info(x->ctx, &mi);
    x->length_ms = mi.num_sequences > 0 && mi.seq_data[0].duration > 0 ? mi.seq_data[0].duration : -1;
    if (xmp_start_player(x->ctx, RATE, 0) != 0) goto fail;   /* 16-bit signed, stereo, interleaved */
    x->playing = true;
    x->amiga = -1;
    amiga_apply(x);                          /* before the first frame: the mixer set is picked per frame */
    x->chn = mi.mod && mi.mod->chn > 0 ? (mi.mod->chn < XMP_MAX_CHANNELS ? mi.mod->chn : XMP_MAX_CHANNELS) : 0;
    x->nch = x->chn < PC_VOICES_MAX ? x->chn : PC_VOICES_MAX;
    if (start_off && mi.mod && (int)start_off < mi.mod->len) xmp_set_position(x->ctx, (int)start_off);
    next_frame(x);                           /* primes position_ms / byte_pos; an empty song just ends */
    *h = x;
    return 0;
fail:
    free(io.buf);
    x_close(x);
    return -1;
}

static int x_render(void *h, float *out, int frames)
{
    xmp_t *x = h;
    int done = 0;
    if (s_owner != x && !x->ended) voices_take(x);
    mute_apply(x);
    amiga_apply(x);                                  /* the setting changed while playing: from this block on */
    while (done < frames) {
        if (x->used >= x->avail && !next_frame(x)) break;
        int k = x->avail - x->used;
        if (k > frames - done) k = frames - done;
        const int16_t *s = x->frame + 2 * x->used;
        for (int i = 0; i < 2 * k; i++) out[2 * done + i] = s[i] * (1.0f / 32768.0f);
        x->used += k;
        done += k;
    }
    return done;
}

static uint32_t x_rate(void *h) { (void)h; return RATE; }
static int32_t x_length(void *h) { return ((xmp_t *)h)->length_ms; }
static int32_t x_position(void *h)
{
    xmp_t *x = h;
    return x->ended && x->length_ms > 0 ? x->length_ms : x->time_ms + (int32_t)((int64_t)x->used * 1000 / RATE);
}
static uint32_t x_byte_pos(void *h) { xmp_t *x = h; return x->ended ? 0 : (uint32_t)x->pos; }

const pc_backend_t pc_backend_xmp = {
    .open = x_open, .render = x_render, .rate = x_rate,
    .length_ms = x_length, .position_ms = x_position, .close = x_close, .byte_pos = x_byte_pos,
    .name = "MOD",
};
