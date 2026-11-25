// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdbool.h>
#include <stdint.h>
/* Bluetooth A2DP codec helpers for headphones mode (host-tested): which codec to prefer, LDAC quality levels and
 * the adaptive bit rate, aptX packet sizes, an LDAC packet check (used by the test peer), a test-tone analyser. */

/* ---- codecs, in order of preference (LDAC > aptX HD > aptX > AAC > SBC) ---- */
typedef enum { PC_BTC_LDAC, PC_BTC_APTX_HD, PC_BTC_APTX, PC_BTC_AAC, PC_BTC_SBC, PC_BTC_N, PC_BTC_NONE = -1 } pc_btc_t;
const char *pc_btc_name(pc_btc_t c);
/* the preferred codec out of a mask of (1 << pc_btc_t) both sides support; PC_BTC_NONE if the mask is empty */
pc_btc_t pc_btc_pick(unsigned mask);
/* the order to register source endpoints in (the stack offers them by seid): codecs in `built`, best first;
 * returns how many were written */
int pc_btc_order(unsigned built, pc_btc_t out[PC_BTC_N]);
/* which encoder task owns a codec in headphones mode: the vendor one (LDAC, aptX HD, aptX) or the standard one
 * (AAC, SBC). Only the owner may read the audio ring. */
typedef enum { PC_ENC_NONE, PC_ENC_STD, PC_ENC_VENDOR } pc_btc_enc_t;
pc_btc_enc_t pc_btc_encoder(pc_btc_t c);
/* whether a codec can stream over a media channel of this MTU. libldac always fills one 2-DH5 packet (679 octets,
 * ldacBT_assert_mtu) at every bit rate, so a lower LDAC rate does not help; aptX packets shrink to the MTU. */
#define PC_LDAC_MTU_MIN 679
bool pc_btc_fits_mtu(pc_btc_t c, int mtu);

/* ---- LDAC ---- */
/* quality levels the player uses, best first: 990 / 660 / 492 / 330 kbps at 48 kHz (909 / 606 / 452 / 303 at
 * 44.1). The frame is 128 samples per channel; frame bytes include LDAC's 3-octet frame header. */
#define PC_LDAC_LEVELS 4
#define PC_LDAC_FRAME_SAMPLES 128
typedef struct { int kbps; int frame_bytes; int frames_per_packet; } pc_ldac_level_t;
const pc_ldac_level_t *pc_ldac_level(int i);              /* NULL out of range */
int pc_ldac_level_for_kbps(int kbps);                     /* best level not above kbps (330 and less: the last) */
int pc_ldac_kbps(int frame_bytes, uint32_t rate);         /* on-air payload bit rate of one frame size */
/* frames libldac puts in one packet: its 2-DH5 budget (679 - 18 octets), and no more than the link MTU allows
 * after the RTP (12) and the media payload header (1) */
int pc_ldac_frames_per_packet(int frame_bytes, int mtu);

/* adaptive bit rate. Inputs per call: the source queue depth (packets waiting), packets the stack or the send
 * call lost since the last call, whether Wi-Fi is on, and the user's top rate. Steps down one level at once on a
 * loss or a very deep queue, or after a deep queue that lasts; steps up one level after a quiet spell, which
 * doubles each time an up-step had to be undone soon and halves each time one holds. Never above 660 kbps while
 * Wi-Fi is on. */
typedef struct {
    int level;                  /* index into pc_ldac_level() */
    uint32_t last_down_ms, deep_since_ms, quiet_since_ms, up_wait_ms, last_up_ms;
    bool deep, started, held;   /* held: the last up-step is settled (lasted PC_ABR_UP_FAIL_MS, or was undone) */
    uint32_t downs, ups;
} pc_ldac_abr_t;
#define PC_ABR_QUEUE_CRITICAL 12   /* packets: step down now */
#define PC_ABR_QUEUE_DEEP 6        /* ... for PC_ABR_DEEP_MS: step down */
#define PC_ABR_QUEUE_QUIET 2       /* at most this for up_wait: step up */
#define PC_ABR_DEEP_MS 300
#define PC_ABR_DOWN_GAP_MS 1000    /* between two down-steps (the queue needs time to drain) */
#define PC_ABR_UP_MS 10000         /* first quiet spell before an up-step; doubles (to 80 s) after a failed one */
#define PC_ABR_UP_MAX_MS 80000
#define PC_ABR_UP_FAIL_MS 5000     /* a down-step this soon after an up-step: the up-step failed; one that
                                      lasts this long halves the wait again (to PC_ABR_UP_MS at least) */
int pc_ldac_abr_cap(bool wifi_on, int max_kbps);          /* the best level allowed */
void pc_ldac_abr_init(pc_ldac_abr_t *a, bool wifi_on, int max_kbps);
/* returns the level to use from now on */
int pc_ldac_abr_update(pc_ldac_abr_t *a, uint32_t now_ms, int queued, int lost, bool wifi_on, int max_kbps);

/* ---- fixed-rate codecs: step down a codec when the link cannot carry it ---- */
/* aptX HD and aptX have one bit rate each, so on a congested link (Wi-Fi traffic) packets are lost in bursts where
 * LDAC's ABR would just lower the rate. The ladder is the negotiated codec, then cheaper ones the sink also has:
 * aptX HD (576 kbps) > aptX (384) > SBC at bitpool PC_SBC_STEP_BITPOOL (249). The stream switches codec in place
 * (suspend, AVDTP reconfiguration, start: a short gap), so a step is only taken on sustained losses. A sink mask of
 * 0 (capabilities not reported) means only the negotiated codec and SBC, which every sink has. LDAC (ABR), AAC
 * and SBC get a ladder of one. Returns the number of rungs. */
#define PC_SBC_STEP_BITPOOL 35
int pc_btc_ladder(pc_btc_t c, unsigned sink_mask, pc_btc_t out[PC_BTC_N]);
int pc_btc_step_kbps(pc_btc_t c, uint32_t rate);          /* nominal rate of a rung (SBC: the step bitpool) */

/* the step policy. Inputs per call: packets lost since the last call and whether Wi-Fi is on. A step is itself a
 * pause (measured 0.7-0.9 s on the board, 2026-11-10), so only sustained losses count: one rung down when at least
 * PC_CS_DOWN_LOST packets were lost in the last PC_CS_WIN_S seconds, spread over at least PC_CS_DOWN_SECS of those
 * seconds (one burst, however big, is not enough); at once after an up-step, else at least PC_CS_MIN_GAP_MS after
 * the last down-step. One rung up after a quiet spell (no losses) of up_wait, which doubles each time an up-step is
 * undone within PC_CS_UP_FAIL_MS and halves each time one holds; with Wi-Fi off the spell is PC_CS_UP_NOWIFI_MS at
 * most, counted from when Wi-Fi went off. */
#define PC_CS_WIN_S 10
#define PC_CS_DOWN_LOST 30          /* packets in the window: about 0.27 s of aptX HD */
#define PC_CS_DOWN_SECS 3
#define PC_CS_MIN_GAP_MS 20000
#define PC_CS_UP_MS 120000
#define PC_CS_UP_MAX_MS 600000
#define PC_CS_UP_FAIL_MS 30000
#define PC_CS_UP_NOWIFI_MS 10000
typedef struct {
    int level, n;                   /* rung in use (0 = the negotiated codec), number of rungs */
    uint16_t lost[PC_CS_WIN_S];     /* per-second buckets */
    uint32_t sec;                   /* the second of the newest bucket */
    uint32_t last_down_ms, last_up_ms, quiet_since_ms, up_wait_ms;
    bool started, held, wifi, off;  /* off: a switch failed, no more stepping on this stream */
    uint32_t downs, ups;
} pc_cstep_t;
void pc_cstep_init(pc_cstep_t *s, int n);
int pc_cstep_update(pc_cstep_t *s, uint32_t now_ms, int lost, bool wifi_on);   /* the rung to use from now on */
void pc_cstep_failed(pc_cstep_t *s, int level_in_use);    /* the switch did not happen: stay, stop stepping */

/* LDAC packet check (the peer cannot decode LDAC): every frame starts with the sync byte 0xAA; its header holds
 * the sampling-rate id (0 44.1, 1 48, 2 88.2, 3 96 kHz), the channel config (0 mono, 1 dual, 2 stereo) and the
 * frame length. The frames must fill the payload exactly and match the frame count of the media header. */
typedef struct { int frames, bad_frames; int frame_bytes; int rate_id, ch_id; bool ok; } pc_ldac_check_t;
void pc_ldac_check(const uint8_t *p, int len, int hdr_frames, int want_rate_id, int want_ch_id, pc_ldac_check_t *out);
int pc_ldac_rate_id(uint32_t rate);                       /* -1 if LDAC has none */

/* ---- aptX / aptX HD (AOSP a2dp_vendor_aptx*_encoder framing) ---- */
/* 4 samples per channel make one codeword: 4 octets (aptX) or 6 (aptX HD). Samples per packet as AOSP's 2-DH5
 * sizes: aptX 672 at 48 kHz (14 ms), 660 at 44.1; aptX HD 432 (9 ms) / 396. Classic aptX has no RTP header;
 * aptX HD has one (12 octets). Fewer samples if the MTU is smaller. */
int pc_aptx_codeword_bytes(bool hd);
int pc_aptx_samples_per_packet(bool hd, uint32_t rate, int mtu);
int pc_aptx_packet_bytes(bool hd, int samples);

/* ---- test tone (the peer checks real decoded audio) ---- */
/* least-squares sine fit to n samples: frequency (searched near a zero-crossing estimate), level (dBFS of the
 * amplitude, 0 = full scale 1.0) and SNR (the fitted sine against everything else, DC removed). */
typedef struct { float freq_hz, level_db, snr_db; } pc_tone_t;
bool pc_tone_analyse(const float *x, int n, float rate, pc_tone_t *out);
/* headphones that dropped: wait before reconnect attempt n (0-based) - 3 s doubling, at most 60 s */
uint32_t pc_bt_reconnect_ms(int attempt);
