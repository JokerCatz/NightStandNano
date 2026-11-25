/* A small C API over cRSID: one player per SID file, any number of them at once, each used by one thread at a time.
 * The owner's own code, compiled in one translation unit with the (patched) cRSID sources.
 *
 * Every function accepts a NULL player, a voice or SID index out of range and a NULL output pointer, and then
 * does nothing and returns -1 (or 0 / NULL where the return is a count or a string).
 *
 * Voices are numbered 0..3*ccs_sid_count()-1: voice v is voice v%3 of the (v/3)th SID the tune uses. */

#ifndef CCS_PLAYER_H
#define CCS_PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ccs_player ccs_player;

/* ccs_open() failures. */
enum {
	CCS_ERR_SHORT = 1,     /* shorter than a header, or than the header says it is */
	CCS_ERR_NOT_SID = 2,   /* neither "PSID" nor "RSID" */
	CCS_ERR_VERSION = 3,   /* a header version cRSID does not know */
	CCS_ERR_NO_DATA = 4,   /* no music data after the header */
	CCS_ERR_TOO_BIG = 5,   /* the data does not fit in the C64's 64 KB from its load address */
	CCS_ERR_BASIC = 6,     /* an RSID that is a BASIC program: needs the C64's BASIC ROM, which cRSID has not got */
	CCS_ERR_NO_TUNES = 7,  /* the header says 0 tunes */
	CCS_ERR_RATE = 8,      /* sample rate outside CCS_MIN_RATE..CCS_MAX_RATE */
	CCS_ERR_MEMORY = 9
};

/* cRSID keeps the output rate in 16 bits. */
#define CCS_MIN_RATE 8000
#define CCS_MAX_RATE 65535

/* Up to four SIDs (cRSID supports the "WebSID" 4SID format); 3 voices each. */
#define CCS_MAX_SIDS 4
#define CCS_MAX_VOICES 12

/* Copies `data`, so it need not outlive the call. Nothing plays until ccs_start(). On failure returns NULL and,
 * if `error` is not NULL, stores a CCS_ERR_* there. */
ccs_player* ccs_open(const unsigned char* data, unsigned long size, int sample_rate, int* error);
void ccs_close(ccs_player*);

/* From the header. Tunes are 0-based here (the file counts from 1). */
int ccs_subtunes(const ccs_player*);
int ccs_default_subtune(const ccs_player*);
int ccs_is_rsid(const ccs_player*);
/* 1 if cRSID runs the tune at the PAL clock (985248 Hz), 0 for NTSC (1022727 Hz). Only a header that says
 * "NTSC" and nothing else gets NTSC; "unknown" and "both" run as PAL, as in cRSID. */
int ccs_is_pal(const ccs_player*);
/* The header's strings, NUL-terminated, as they are in the file: Latin-1, at most 32 bytes. Never NULL for a
 * valid player. */
const char* ccs_title(const ccs_player*);
const char* ccs_author(const ccs_player*);
const char* ccs_released(const ccs_player*);

/* (Re)starts a tune from its beginning, on a fresh emulated C64: the same tune started twice renders the same
 * samples. A PSID's init routine runs here, to completion (cRSID allows it 10 million instructions). Returns
 * 0, or -1 for a tune out of range or no memory (the player then has no tune until the next ccs_start()). */
int ccs_start(ccs_player*, int subtune);

/* Renders `frames` frames of interleaved stereo into `out`. Returns frames written: `frames`, or 0 when no
 * tune has been started. A SID tune never ends by itself. */
int ccs_render(ccs_player*, short* out, int frames);

/* After ccs_start(): SIDs the tune uses (1..4), and each one's model, 6581 or 8580. -1 before. */
int ccs_sid_count(const ccs_player*);
int ccs_sid_model(const ccs_player*, int sid);

/* 3 x ccs_sid_count(), or 0 before ccs_start(). */
int ccs_voice_count(const ccs_player*);

/* One voice's registers and envelope, as of the last sample rendered. */
typedef struct ccs_voice_t {
	int key_on;         /* the gate bit, as last written */
	int freq_reg;       /* 16-bit frequency register; Hz = freq_reg * clock / 2^24 */
	int pulse_width;    /* 12-bit pulse width register; the duty cycle is pulse_width / 4096 */
	int waveform;       /* the waveform bits, control register >> 4: 1 triangle, 2 sawtooth, 4 pulse, 8 noise
	                       (combinations are the SID's combined waveforms; 0 is no waveform) */
	int envelope;       /* envelope generator, 0..255 */
	int key_on_count;   /* rising edges of the gate since ccs_start(), re-triggers included; wraps */
	int filtered;       /* 1 if the voice is routed through the filter */
	int ring_mod;       /* the control register's ring-modulation bit */
	int sync;           /* its hard-sync bit */
	int test;           /* its test bit (holds the oscillator at zero) */
	int muted;          /* 1 if ccs_mute_voice() muted it */
	int off3;           /* voice 3 of a SID only: the volume register's "3 off" bit cuts it from the mix (it still
	                       plays through the filter, and players read its oscillator as a modulation source) */
} ccs_voice_t;

int ccs_voice(const ccs_player*, int voice, ccs_voice_t* out);

/* Leaves a voice out of the mix (and out of the filter): its envelope counts as zero. Kept across
 * ccs_start(). */
int ccs_mute_voice(ccs_player*, int voice, int mute);

/* From the next ccs_render() on, writes each voice's share of the mix for every output frame into buf: frames
 * of ccs_voice_count() samples, voice 0 first, at the output rate, in step with the output. A sample is the
 * voice's waveform times its envelope (a muted voice's is 0), before the filter and the SID's volume,
 * averaged over the oversampled frame like the mix; full scale is about +-32640. When buf holds
 * `frames_capacity` frames, later frames are dropped until ccs_take_voice_tap(). buf NULL or frames_capacity
 * <= 0 turns the tap off. buf must stay valid while set. Costs one well-predicted branch per voice per
 * oversampled step while off. */
int ccs_set_voice_tap(ccs_player*, short* buf, int frames_capacity);

/* Frames written to the tap buffer since it was set or last taken; the next ones go to its start again. */
int ccs_take_voice_tap(ccs_player*);

#ifdef __cplusplus
}
#endif

#endif
