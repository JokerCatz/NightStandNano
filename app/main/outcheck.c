// SPDX-License-Identifier: LGPL-2.1-or-later
#include "outcheck.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAX_EV 24
#define MAX_JOIN 16
#define WARM 96                        /* samples after a (re)start in which the fit is not trusted yet */

typedef struct { char kind; uint32_t at; int32_t val; } ev_t;
static struct {
    volatile bool on;
    uint32_t frames, starved, nj, nz, maxz, zrun, zrun_at, quiet_after_z;
    bool seen_signal;
    float x1, x2, num, den, peak, maxerr, maxpeak;
    int warm;
    int n_ev; ev_t ev[MAX_EV];
    int n_join; uint32_t join[MAX_JOIN];
} s;

void nsn_oc_arm(bool on)
{
    if (on) { memset(&s, 0, sizeof s); s.warm = WARM; }
    s.on = on;
}
bool nsn_oc_armed(void) { return s.on; }
void nsn_oc_idle(void) { if (s.on) { s.seen_signal = false; s.zrun = 0; s.warm = WARM; s.x1 = s.x2 = 0; s.num = s.den = 0; } }

static void event(char kind, uint32_t at, int32_t val)
{
    if (s.n_ev < MAX_EV) s.ev[s.n_ev++] = (ev_t){ kind, at, val };
}

static void one(int16_t l, int16_t r)
{
    uint32_t pos = s.frames++;
    if (l == 0 && r == 0) {
        if (s.seen_signal) { if (!s.zrun) s.zrun_at = pos; s.zrun++; }
        s.x2 = s.x1; s.x1 = 0;
        return;
    }
    if (s.zrun >= NSN_OC_ZMIN) {                         /* the signal is back after a silence */
        s.nz++; if (s.zrun > s.maxz) s.maxz = s.zrun;
        event('Z', s.zrun_at, (int32_t)s.zrun);
        s.warm = 3; s.quiet_after_z = 3;                 /* the first samples after a dropout are the dropout's own edge */
    }
    s.zrun = 0;
    float x0 = l;
    float a = fabsf(x0);
    s.peak = a > s.peak ? a : s.peak * 0.9995f;
    if (a > s.maxpeak) s.maxpeak = a;
    if (!s.seen_signal) { s.seen_signal = true; s.warm = WARM; s.x1 = s.x2 = 0; s.num = s.den = 0; }
    if (s.warm > 0) {
        s.warm--;
    } else {
        float c = s.den > 1e3f ? s.num / s.den : 2.0f;
        float e = x0 + s.x2 - c * s.x1;
        float thr = 0.03f * s.peak; if (thr < 24.0f) thr = 24.0f;
        float ae = fabsf(e);
        if (ae > s.maxerr) s.maxerr = ae;
        if (ae > thr) {
            if (!s.quiet_after_z) { s.nj++; event('J', pos, (int32_t)e); }
            s.warm = 8;                                   /* do not count one break several times */
        }
    }
    if (s.quiet_after_z) s.quiet_after_z--;
    if (s.warm <= 0 || s.warm > 8) { s.num = s.num * 0.999f + s.x1 * (x0 + s.x2); s.den = s.den * 0.999f + s.x1 * s.x1; }
    s.x2 = s.x1; s.x1 = x0;
}

void nsn_oc_feed(const int16_t *p, size_t frames)
{
    if (!s.on) return;
    for (size_t i = 0; i < frames; i++) one(p[2 * i], p[2 * i + 1]);
}

void nsn_oc_starve(size_t frames)
{
    if (!s.on) return;
    s.starved += (uint32_t)frames;
    for (size_t i = 0; i < frames; i++) one(0, 0);
}

void nsn_oc_join(uint32_t frame) { if (s.on && s.n_join < MAX_JOIN) s.join[s.n_join++] = frame; }

void nsn_oc_report(void (*line)(void *, const char *), void *ctx)
{
    char b[200];
    snprintf(b, sizeof b, "oc %s frames=%lu jumps=%lu zeros=%lu (longest %lu) starved=%lu maxerr=%.1f maxpeak=%.0f", s.on ? "armed" : "off", (unsigned long)s.frames,
             (unsigned long)s.nj, (unsigned long)s.nz, (unsigned long)s.maxz, (unsigned long)s.starved, (double)s.maxerr, (double)s.maxpeak);
    line(ctx, b);
    int k = 0; b[0] = 0;
    for (int i = 0; i < s.n_join; i++) k += snprintf(b + k, sizeof b - k, "%s%lu", i ? "," : "oc joins at frames: ", (unsigned long)s.join[i]);
    if (s.n_join) line(ctx, b);
    for (int i = 0; i < s.n_ev; i++) {
        snprintf(b, sizeof b, "oc %c at frame %lu %s %ld", s.ev[i].kind, (unsigned long)s.ev[i].at, s.ev[i].kind == 'Z' ? "len" : "err", (long)s.ev[i].val);
        line(ctx, b);
    }
}
