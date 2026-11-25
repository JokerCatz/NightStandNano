// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_voice.h"
#include <ctype.h>
#include <string.h>

void pc_voice_short(const char *full, int index, char out[4])
{
    const char *p = full ? full : "";
    while (*p == ' ') p++;
    if (!*p) {
        int v = (index < 0 ? 0 : index > 98 ? 98 : index) + 1;   /* 1..99: two digits fit */
        if (v >= 10) { out[0] = (char)('0' + v / 10); out[1] = (char)('0' + v % 10); out[2] = 0; }
        else { out[0] = (char)('0' + v); out[1] = 0; }
        return;
    }
    if (isdigit((unsigned char)*p)) {              /* nothing to abbreviate: its first digits */
        int k = 0;
        while (k < 3 && isdigit((unsigned char)p[k])) { out[k] = p[k]; k++; }
        out[k] = 0;
        return;
    }
    size_t len = strlen(p);
    while (len && p[len - 1] == ' ') len--;
    size_t d = 0;                                  /* trailing digits */
    while (d < len && isdigit((unsigned char)p[len - 1 - d])) d++;
    if (d > 2) d = 2;
    out[0] = (char)toupper((unsigned char)*p);
    memcpy(out + 1, p + len - d, d);
    out[1 + d] = 0;
}

uint8_t pc_voice_decay(uint8_t held, uint8_t now)
{
    int fall = held - held / 4 - 1;
    if (fall < 0) fall = 0;
    return now > fall ? now : (uint8_t)fall;
}

int pc_voice_read(const pc_voices_t *src, pc_voices_t *out)
{
    uint8_t n = __atomic_load_n(&src->n, __ATOMIC_ACQUIRE);
    if (n > PC_VOICES_MAX) n = 0;
    memcpy(out, src, sizeof *out);               /* a torn copy can show the old count with the new names or levels for one frame (the count is clamped, the rest is plain bytes) */
    out->n = n;
    if (!n) memset(out, 0, sizeof *out);
    return n;
}

void pc_voice_clear_extra(pc_voices_t *v, int i)
{
    if (!v || i < 0 || i >= PC_VOICES_MAX) return;
    v->cents[i] = PC_VOICE_NOPITCH;
    v->strikes[i] = 0;
    v->pan[i] = PC_PAN_UNKNOWN;
    v->flags[i] = 0;
}

/* round(1200 * log2(hz / 440)) + 6900 from an integer Hz x 100, by octave halving and a short ln series (no libm). */
int pc_voice_cents_of_hz_x100(long hz_x100)
{
    if (hz_x100 <= 0) return PC_VOICE_NOPITCH;
    /* octave: bring hz into [440, 880) by doublings; each doubling is 1200 cents */
    long h = hz_x100;
    int oct = 0;
    while (h >= 88000 && oct < 20) { h /= 2; oct++; }          /* the halving loses < 1 unit of 0.01 Hz: far below one cent */
    while (h < 44000 && oct > -20) { h *= 2; oct--; }
    /* ln(r) = 2 atanh((r-1)/(r+1)) for r in [1,2): the series converges fast */
    const float r = (float)h / 44000.0f;
    float y = (r - 1.0f) / (r + 1.0f), y2 = y * y, ln = 2.0f * y * (1.0f + y2 / 3.0f + y2 * y2 / 5.0f + y2 * y2 * y2 / 7.0f + y2 * y2 * y2 * y2 / 9.0f);
    float cents = ln * (1200.0f / 0.69314718f);
    int m = 6900 + oct * 1200 + (int)(cents + 0.5f);
    return m < 0 || m > 12799 ? PC_VOICE_NOPITCH : m;
}

void pc_voice_note_name(int cents, char out[4])
{
    static const char *const N[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    if (cents < 0) { out[0] = '-'; out[1] = '-'; out[2] = 0; out[3] = 0; return; }
    int m = (cents + 50) / 100;
    if (m > 127) m = 127;
    const char *nm = N[m % 12];
    int o = m / 12 - 1, k = 0;
    out[k++] = nm[0];
    if (nm[1]) out[k++] = nm[1];
    if (o < 0) o = 0;                                           /* C-1 .. B-1 (MIDI 0..11) show as 0 */
    out[k++] = (char)('0' + (o > 9 ? 9 : o));
    out[k] = 0;
    if (k < 3) out[3] = 0;
}

bool pc_voice_level_strike(uint8_t level, uint8_t *armed)
{
    if (level < 24) { *armed = 1; return false; }
    if (level >= 64 && *armed) { *armed = 0; return true; }
    return false;
}
