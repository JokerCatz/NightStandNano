// SPDX-License-Identifier: LGPL-2.1-or-later
/* fx apps: the shared helpers of the contract (spec §2, §5): parameter stepping and text, ramps, the mode rule. */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "pc/pc_fxapp.h"
#include "pc/pc_i18n.h"

const pc_fxparam_t pc_fxapp_mix_param = { "mix", "濃度", "Mix", 0, 1, 1, 0.05f, PC_FXU_PCT, 0, NULL, NULL };

/* ---------- stepping ---------- */
static float clampf(float v, float lo, float hi) { return !(v >= lo) ? lo : v > hi ? hi : v; }   /* NaN -> lo */

float pc_fxparam_step(const pc_fxparam_t *p, float v, int detents, bool from_tap)
{
    if (!isfinite(v)) v = p->def;                              /* an imported "nan" / "inf" is the default */
    double lo = p->lo, hi = p->hi, x = clampf(v, p->lo, p->hi);
    const double eps = 1e-6 * (hi - lo);
    if (p->unit == PC_FXU_BOOL) {
        if (from_tap) return x > 0.5 * (lo + hi) ? p->lo : p->hi;
        return detents > 0 ? p->hi : detents < 0 ? p->lo : (float)x;
    }
    if (from_tap) {
        if (x >= hi - eps) return p->lo;                       /* tap at hi wraps to lo */
        detents = 1;
    }
    if (p->flags & PC_FXP_LOG) {
        double y = x * pow((double)p->step, detents);
        return clampf((float)y, p->lo, p->hi);
    }
    double step = p->unit == PC_FXU_CHOICE ? 1.0 : p->step;
    long k = lround((x - lo) / step) + detents;                /* the grid lo + k * step: no drift */
    double kmax = floor((hi - lo) / step + 1e-6);
    if (k < 0) return p->lo;
    if (k > kmax) return p->hi;
    return clampf((float)(lo + (double)k * step), p->lo, p->hi);
}

/* ---------- text ---------- */
static int step_decimals(float step)
{
    int d = 0;
    double s = step;
    while (d < 4 && fabs(s - round(s)) > 1e-6 * (s > 1 ? s : 1)) { s *= 10; d++; }
    return d;
}

/* v rounded to 3 significant digits */
static double round3(double v)
{
    double a = fabs(v);
    if (!isfinite(v) || a == 0) return isfinite(v) ? v : 0;
    double m = pow(10, 2 - floor(log10(a)));
    return round(v * m) / m;
}

/* 3 significant digits, trailing zeros dropped */
static void sig3(char *b, size_t n, double v)
{
    if (!isfinite(v)) v = 0;
    double a = fabs(v);
    int d = a > 0 ? 2 - (int)fmax(-300, fmin(300, floor(log10(a)))) : 0;
    if (d < 0) d = 0;
    snprintf(b, n, "%.*f", d, v);
    if (strchr(b, '.')) {
        char *e = b + strlen(b) - 1;
        while (*e == '0') *e-- = 0;
        if (*e == '.') *e = 0;
    }
}

static void lin(char *b, size_t n, double v, int dec)
{
    double m = pow(10, dec), r = round(v * m) / m;
    if (r == 0) r = 0;                                         /* no "-0" */
    snprintf(b, n, "%.*f", dec, r);
}

int pc_fxparam_text(const pc_fxparam_t *p, float v, bool zh, char *b, size_t n)
{
    if (!isfinite(v)) v = p->def;
    char num[24];
    const char *unit = "";
    switch (p->unit) {
    case PC_FXU_BOOL:
        return snprintf(b, n, "%s", v > 0.5f * (p->lo + p->hi) ? (zh ? TZ("開") : "on") : (zh ? TZ("關") : "off"));
    case PC_FXU_CHOICE: {
        int i = (int)lroundf(clampf(v, p->lo, p->hi) - p->lo);
        const char *const *ch = zh ? p->ch_zh : p->ch_en;
        const char *c = ch && ch[i] ? ch[i] : "?";
        return snprintf(b, n, "%s", zh ? TZ(c) : c);
    }
    case PC_FXU_PCT:
        lin(num, sizeof num, (double)v * 100, 0);
        return snprintf(b, n, "%s %%", num);
    default:
        break;
    }
    double x = v;
    double m = pow(10, step_decimals(p->step)), shown = (p->flags & PC_FXP_LOG) ? round3(x) : round(x * m) / m;   /* the unit follows the rounded value: 999.7 is "1 kHz" */
    if (p->unit == PC_FXU_HZ && fabs(shown) >= 1000) { x = shown / 1000; unit = " kHz"; sig3(num, sizeof num, x); }
    else {
        if (p->flags & PC_FXP_LOG) sig3(num, sizeof num, x);
        else lin(num, sizeof num, x, step_decimals(p->step));
        unit = p->unit == PC_FXU_DB ? " dB" : p->unit == PC_FXU_HZ ? " Hz" : p->unit == PC_FXU_MS ? " ms"
             : p->unit == PC_FXU_S ? " s" : p->unit == PC_FXU_RATIO ? ":1" : "";
    }
    return snprintf(b, n, "%s%s", num, unit);
}

/* ---------- ramps ---------- */
void pc_fx_ramp_set(pc_fx_ramp_t *r, float tgt) { r->tgt = tgt; }
float pc_fx_ramp_at(const pc_fx_ramp_t *r, int i, int frames) { return r->cur + (r->tgt - r->cur) * (float)(i + 1) / (float)frames; }
void pc_fx_ramp_end(pc_fx_ramp_t *r) { r->cur = r->tgt; }

/* ---------- the mode rule (§5): light runs in every mode but HIRES2; heavy pauses on battery, aptX, LDAC, broadcast and hi-res (T12: the LDAC bench with every heavy app on failed; hires H23, H27) ---------- */
#define HEAVY_OFF (PC_FXM_BATT | PC_FXM_APTX | PC_FXM_LDAC | PC_FXM_CAST | PC_FXM_HIRES | PC_FXM_HIRES2)

bool pc_fx_allowed(pc_fxclass_t cls, uint8_t mode_bits)
{
    if (mode_bits & PC_FXM_HIRES2) return false;   /* above 96 kHz no class runs (the per-app re-allow below lifts the limiter) */
    return cls == PC_FXC_LIGHT || !(mode_bits & HEAVY_OFF);
}

/* H27, hires T8b: the per-app re-allow on top of the class rule. The limiter keeps running above 96 kHz (Batch B fxbench: 3.85 % of a core at
 * 96k, 7.69 % at 192k, p07); under HIRES2 it is judged by its class with the HIRES bit only. Every other app stays paused there. */
bool pc_fx_app_allowed(const pc_fxapp_t *ap, uint8_t mode_bits)
{
    if (!ap) return false;
    if (ap->flags & PC_FXA_WIRED_ONLY) return mode_bits == 0;   /* T17: any bit (battery, a Bluetooth sink of any codec, broadcast, hi-res) suspends it */
    if (ap->fam == PC_FXFAM_LIMIT) mode_bits &= (uint8_t)~PC_FXM_HIRES2;
    return pc_fx_allowed((pc_fxclass_t)ap->cls, mode_bits);
}

int pc_fx_bench_block(uint32_t rate)
{
    uint32_t b = rate / 100;
    return (int)(b > PC_FX_BLOCK ? PC_FX_BLOCK : b);
}

uint8_t pc_fx_rate_mode(uint32_t rate)
{
    return rate > 96000 ? (uint8_t)(PC_FXM_HIRES | PC_FXM_HIRES2) : rate > 48000 ? (uint8_t)PC_FXM_HIRES : 0;
}

const char *pc_fx_mode_why(uint8_t mode_bits, bool zh)
{
    if (mode_bits & PC_FXM_HIRES2) return zh ? TZ("超高取樣率需要 CPU") : "very high rates need the CPU";
    if (mode_bits & PC_FXM_BATT) return zh ? TZ("用電池時省電") : "On battery";
    if (mode_bits & PC_FXM_APTX) return zh ? TZ("aptX 編碼需要 CPU") : "aptX encoding needs the CPU";
    if (mode_bits & PC_FXM_CAST) return zh ? TZ("廣播時需要 CPU") : "Broadcasting needs the CPU";
    if (mode_bits & PC_FXM_LDAC) return zh ? TZ("LDAC 編碼需要 CPU") : "LDAC encoding needs the CPU";
    if (mode_bits & PC_FXM_HIRES) return zh ? TZ("高取樣率需要 CPU") : "hi-res needs the CPU";
    return NULL;
}

/* the one reason text of an app: what the list row and the page show (fx_look); the class reason unless the app's own rule is the stricter one */
const char *pc_fx_app_why(const pc_fxapp_t *ap, uint8_t mode_bits, bool zh)
{
    if (!ap) return NULL;
    if ((ap->flags & PC_FXA_WIRED_ONLY) && mode_bits) return zh ? TZ("只在有線播放時可用") : "wired playback only";
    return pc_fx_mode_why(mode_bits, zh);
}
