/* A small C API over cRSID (ccs_player.h): one player per SID file. The API and this file are the owner's own code
 * (written for the ChipChill project, now part of this repository under its LGPL-2.1-or-later); cRSID itself is by Hermit (WTF licence, docs/licenses/cRSID-WTF.txt).
 * cRSID is a "unity build" (libcRSID.c includes the rest of its sources), so it is included here and the whole library is
 * one translation unit with its static inline functions visible.
 * Memory: the player struct and the file copy are never below 1100 bytes (CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL=1024 would put
 * smaller blocks in internal RAM); the cRSID machine (calloc of cRSID_C64instance, 70 KB) is far above it. */

#include "crsid/libcRSID/libcRSID.c"

#include <string.h>

#include "ccs_player.h"

/* Header offsets (PSID/RSID, all big-endian). */
enum {
	H_VERSION = 0x04, H_DATA_OFFSET = 0x06, H_LOAD = 0x08, H_INIT = 0x0A, H_PLAY = 0x0C, H_SONGS = 0x0E,
	H_START = 0x10, H_TITLE = 0x16, H_AUTHOR = 0x36, H_RELEASED = 0x56, H_FLAGS = 0x77,
	V1_HEADER = 0x76, V2_HEADER = 0x7C,
	/* cRSID reads its header struct (up to $7F) straight from the data, and the first two data bytes when the
	 * load address is in them; this much zero padding keeps every such read inside the copy. */
	PADDING = 0x100
};

struct ccs_player {
	unsigned char* data; /* the file, normalised (see prepare()), then PADDING zero bytes */
	int size;            /* without the padding */
	int rate;
	int subtunes, default_subtune, rsid, pal;
	char title[33], author[33], released[33];
	cRSID_C64instance* c64; /* NULL until ccs_start() */
	int sids[CCS_MAX_SIDS]; /* cRSID's SID numbers (1..4) of the SIDs in use */
	int sid_count;
	unsigned char mute[CCS_MAX_SIDS];
	short* tap;
	int tap_capacity, tap_frames;
};

static int be16(const unsigned char* p) { return (p[0] << 8) | p[1]; }

static void put_be16(unsigned char* p, int v) { p[0] = (unsigned char)(v >> 8); p[1] = (unsigned char)v; }

static void header_string(char* out, const unsigned char* in) {
	memcpy(out, in, 32);
	out[32] = 0;
}

/* Checks the header against the file and copies the file into p->data, fixed up where cRSID would otherwise
 * misread it:
 *  - A version 1 header is widened to version 2 with its flags zero. cRSID reads the version 2 fields
 *    (SID model, clock, SID addresses) regardless of the version, which in a version 1 file are music data.
 *  - More than 255 tunes: cRSID reads only the low byte of the count. Capped at 255.
 *  - A PSID's init address 0 means "the load address" (the format's rule, which cRSID does not apply).
 * Returns 0 or a CCS_ERR_*. */
static int prepare(ccs_player* p, const unsigned char* in, unsigned long size) {
	int version, header, load, offset, length, songs, start, init;
	unsigned char* out;

	if (size < V1_HEADER) return CCS_ERR_SHORT;
	if ((in[0] != 'P' && in[0] != 'R') || memcmp(in + 1, "SID", 3) != 0) return CCS_ERR_NOT_SID;
	p->rsid = in[0] == 'R';
	version = be16(in + H_VERSION);
	if (!((version >= 1 && version <= 4) || version == CRSID_FILEVERSION_WEBSID)) return CCS_ERR_VERSION;
	if (p->rsid && version == 1) return CCS_ERR_VERSION;
	header = be16(in + H_DATA_OFFSET);
	if (version == 1 ? header != V1_HEADER : (header < V2_HEADER || header > 0xFF)) return CCS_ERR_VERSION;
	if (size > 0x10000 + 0x100 + 2) return CCS_ERR_TOO_BIG; /* longer than any 64 KB image with a header */
	if ((unsigned long)header >= size) return CCS_ERR_NO_DATA;

	load = be16(in + H_LOAD);
	offset = header;
	if (load == 0) {
		if ((unsigned long)header + 2 >= size) return CCS_ERR_NO_DATA;
		load = in[header] | (in[header + 1] << 8);
		offset = header + 2;
	}
	length = (int)size - offset;
	if (load + length > 0x10000) return CCS_ERR_TOO_BIG;

	songs = be16(in + H_SONGS);
	if (songs == 0) return CCS_ERR_NO_TUNES;
	if (songs > 255) songs = 255;
	start = be16(in + H_START);
	if (start < 1 || start > songs) start = 1;
	init = be16(in + H_INIT);
	if (p->rsid && ((version >= 2 && (in[H_FLAGS] & 0x02)) || init == 0)) return CCS_ERR_BASIC;

	p->size = version == 1 ? (int)size + (V2_HEADER - V1_HEADER) : (int)size;
	out = calloc(1, (size_t)p->size + PADDING < 1100 ? 1100 : (size_t)p->size + PADDING);
	if (out == NULL) return CCS_ERR_MEMORY;
	if (version == 1) {
		memcpy(out, in, V1_HEADER);
		memcpy(out + V2_HEADER, in + V1_HEADER, size - V1_HEADER);
		put_be16(out + H_VERSION, 2);
		put_be16(out + H_DATA_OFFSET, V2_HEADER);
	} else {
		memcpy(out, in, size);
	}
	put_be16(out + H_SONGS, songs);
	put_be16(out + H_START, start);
	if (init == 0) put_be16(out + H_INIT, load);
	p->data = out;

	p->subtunes = songs;
	p->default_subtune = start - 1;
	/* cRSID_setC64(): NTSC only when the two clock bits say exactly "NTSC". */
	p->pal = ((out[H_FLAGS] & 0x0C) >> 2) != 2;
	header_string(p->title, out + H_TITLE);
	header_string(p->author, out + H_AUTHOR);
	header_string(p->released, out + H_RELEASED);
	return 0;
}

ccs_player* ccs_open(const unsigned char* data, unsigned long size, int sample_rate, int* error) {
	ccs_player* p;
	int err;

	if (error) *error = 0;
	if (sample_rate < CCS_MIN_RATE || sample_rate > CCS_MAX_RATE) {
		if (error) *error = CCS_ERR_RATE;
		return NULL;
	}
	if (data == NULL) size = 0;
	p = calloc(1, sizeof *p < 1100 ? 1100 : sizeof *p);
	if (p == NULL) {
		if (error) *error = CCS_ERR_MEMORY;
		return NULL;
	}
	p->rate = sample_rate;
	p->sid_count = -1;
	err = prepare(p, data, size);
	if (err) {
		if (error) *error = err;
		free(p->data);
		free(p);
		return NULL;
	}
	return p;
}

void ccs_close(ccs_player* p) {
	if (p == NULL) return;
	if (p->c64) cRSID_close(p->c64);
	free(p->data);
	free(p);
}

int ccs_subtunes(const ccs_player* p) { return p ? p->subtunes : 0; }
int ccs_default_subtune(const ccs_player* p) { return p ? p->default_subtune : 0; }
int ccs_is_rsid(const ccs_player* p) { return p ? p->rsid : 0; }
int ccs_is_pal(const ccs_player* p) { return p ? p->pal : 0; }
const char* ccs_title(const ccs_player* p) { return p ? p->title : NULL; }
const char* ccs_author(const ccs_player* p) { return p ? p->author : NULL; }
const char* ccs_released(const ccs_player* p) { return p ? p->released : NULL; }

static void apply_voice_state(ccs_player* p) {
	int i;
	for (i = 0; i < p->sid_count; ++i) p->c64->SID[p->sids[i]].MuteMask = p->mute[i];
	p->c64->VoiceTap = p->tap != NULL;
}

int ccs_start(ccs_player* p, int subtune) {
	cRSID_C64instance* c64;
	cRSID_SIDheader* header;
	int i;

	if (p == NULL || subtune < 0 || subtune >= p->subtunes) return -1;
	if (p->c64) cRSID_close(p->c64);
	p->c64 = NULL;
	p->sid_count = -1;
	p->tap_frames = 0;

	/* A fresh machine every time, rather than cRSID_initSIDtune() again on the old one: a few counters (the
	 * VIC's raster position, the PSID TOD clock, the dither generator...) are only set when the machine is
	 * created, and seeking restarts the tune and must land on exactly the samples a straight play gives. */
	c64 = cRSID_init((unsigned short)p->rate, 0);
	if (c64 == NULL) return -1;
	/* cRSID's "stereo" (narrowed) panning for 2SID/3SID tunes; one SID plays mono whatever this says. */
	c64->Interface.Stereo = CRSID_CHANNELMODE_NARROW;
	header = cRSID_processSIDfileData(c64, p->data, p->size);
	if (header == NULL) {
		cRSID_close(c64);
		return -1;
	}
	cRSID_initSIDtune(c64, header, subtune + 1);
	/* The "audio thread" copies of the settings, which cRSID_generateSound() refreshes before every buffer;
	 * nothing changes them between our calls, so once is enough. */
	c64->RealSIDmode = c64->Interface.RealSIDmode;
	c64->AudioThread_SIDchipCount = c64->SIDchipCount;
	c64->Stereo = c64->AudioThread_SIDchipCount > 1 ? c64->Interface.Stereo : CRSID_CHANNELMODE_MONO;
	c64->HighQualitySID = c64->Interface.HighQualitySID;
	c64->HighQualityResampler = c64->Interface.HighQualityResampler;

	p->c64 = c64;
	p->sid_count = 0;
	for (i = 1; i <= CRSID_SIDCOUNT_MAX; ++i) {
		if (i == 1 || c64->SID[i].BaseAddress != 0) p->sids[p->sid_count++] = i;
	}
	apply_voice_state(p);
	return 0;
}

int ccs_render(ccs_player* p, short* out, int frames) {
	int i, v, voices;
	cRSID_Output o;

	if (p == NULL || p->c64 == NULL || out == NULL || frames <= 0) return 0;
	voices = 3 * p->sid_count;
	for (i = 0; i < frames; ++i) {
		o = cRSID_generateSample(p->c64);
		/* cRSID_generateSample() has already saturated both to 16 bits. */
		out[2 * i] = (short)o.L;
		out[2 * i + 1] = (short)o.R;
		if (p->tap != NULL && p->tap_frames < p->tap_capacity) {
			short* t = p->tap + (long)p->tap_frames * voices;
			for (v = 0; v < voices; ++v) {
				int s = p->c64->SID[p->sids[v / 3]].TapOut[v % 3];
				t[v] = (short)(s > 32767 ? 32767 : s < -32768 ? -32768 : s);
			}
			++p->tap_frames;
		}
	}
	return frames;
}

int ccs_sid_count(const ccs_player* p) { return p ? p->sid_count : -1; }

int ccs_sid_model(const ccs_player* p, int sid) {
	if (p == NULL || p->c64 == NULL || sid < 0 || sid >= p->sid_count) return -1;
	return p->c64->SID[p->sids[sid]].ChipModel;
}

int ccs_voice_count(const ccs_player* p) {
	return p && p->c64 ? 3 * p->sid_count : 0;
}

int ccs_voice(const ccs_player* p, int voice, ccs_voice_t* out) {
	const cRSID_SIDinstance* sid;
	const unsigned char* r;
	int n;

	if (p == NULL || p->c64 == NULL || out == NULL || voice < 0 || voice >= 3 * p->sid_count) return -1;
	sid = &p->c64->SID[p->sids[voice / 3]];
	n = voice % 3;
	r = sid->BasePtr + 7 * n;
	out->key_on = r[4] & 1;
	out->freq_reg = r[0] | (r[1] << 8);
	out->pulse_width = (r[2] | (r[3] << 8)) & 0xFFF;
	out->waveform = r[4] >> 4;
	out->envelope = sid->EnvelopeCounter[7 * n];
	out->key_on_count = (int)(sid->KeyOnCount[7 * n] & 0x7FFFFFFF);
	out->filtered = (sid->BasePtr[0x17] >> n) & 1;
	out->ring_mod = (r[4] >> 2) & 1;
	out->sync = (r[4] >> 1) & 1;
	out->test = (r[4] >> 3) & 1;
	out->muted = (sid->MuteMask >> n) & 1;
	out->off3 = n == 2 && !out->filtered && (sid->BasePtr[0x18] & 0x80) != 0;
	return 0;
}

int ccs_mute_voice(ccs_player* p, int voice, int mute) {
	int slot, bit;

	if (p == NULL || voice < 0 || voice >= CCS_MAX_VOICES) return -1;
	if (p->sid_count >= 0 && voice >= 3 * p->sid_count) return -1;
	slot = voice / 3;
	bit = 1 << (voice % 3);
	p->mute[slot] = (unsigned char)(mute ? p->mute[slot] | bit : p->mute[slot] & ~bit);
	if (p->c64) apply_voice_state(p);
	return 0;
}

int ccs_set_voice_tap(ccs_player* p, short* buf, int frames_capacity) {
	if (p == NULL) return -1;
	if (buf == NULL || frames_capacity <= 0) {
		p->tap = NULL;
		p->tap_capacity = 0;
	} else {
		p->tap = buf;
		p->tap_capacity = frames_capacity;
	}
	p->tap_frames = 0;
	if (p->c64) apply_voice_state(p);
	return 0;
}

int ccs_take_voice_tap(ccs_player* p) {
	int n;
	if (p == NULL) return -1;
	n = p->tap_frames;
	p->tap_frames = 0;
	return n;
}
