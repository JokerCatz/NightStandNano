// SPDX-License-Identifier: LGPL-2.1-or-later
/* Hardware settings: TF card wiring and status LED. Stored in NVS through the settings helpers (needed at init, so not on the card), read once at boot,
 * changed only through the `hw` command on the wired console (the flashing page). Over BLE `hw` only reads: a radio link must never move pins.
 *
 * NVS keys (namespace "nsn"), absent = the board default (board/board.h):
 *   hw_sd   "m <clk> <cmd> <d0> <d1> <d2> <d3> <width>"   SDMMC       or   "s <clk> <mosi> <miso> <cs>"   SPI mode
 *   hw_led  "<gpio> <order>"                                gpio -1 = LED off
 *   sdpins  (older builds) a 7-byte blob; still read when hw_sd is absent, erased by every write of hw_sd and by `hw reset`
 *
 * Refusals (`hw sd: refused <reason>`, same words for led):
 *   syntax                    the arguments are not numbers / the wrong count
 *   range                     a GPIO number the chip does not have (SOC_GPIO_VALID_GPIO_MASK)
 *   reserved <n>              GPIO n is in the board's reserved table (board_default.c; `hw reserved`)
 *   duplicate <n>             GPIO n given twice inside the SD set
 *   width                     SDMMC width other than 1 or 4, or d1..d3 not -1 at width 1
 *   order                     colour order not one of rgb grb brg rbg gbr bgr
 *   sd/led conflict <n>       the LED pin and an SD pin of the stored settings are the same GPIO n
 *   not supported <why>       the chip cannot do it: sdmmc-pins-fixed (the SDMMC host reaches only slot 0's pins, use `hw sd spi ...`)
 *   storage                   NVS did not take the value */
#include "hw.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "nvs.h"
#include "soc/sdmmc_pins.h"
#include "board.h"
#include "settings.h"

static const char *TAG = "hw";
#define K_SD  "hw_sd"
#define K_LED "hw_led"
#define K_OLD "sdpins"

static nsn_sdpins_t s_sd_run;          /* what this boot runs with */
static nsn_hw_led_t s_led_run;

const nsn_hw_led_t *nsn_hw_led(void) { return &s_led_run; }
bool nsn_hw_order_ok(const char *o)
{
    static const char *const ok[] = { "rgb", "grb", "brg", "rbg", "gbr", "bgr" };
    for (unsigned i = 0; i < sizeof ok / sizeof ok[0]; i++) if (o && !strcmp(o, ok[i])) return true;
    return false;
}

/* ---- the pins a setting uses, and the checks ---- */

static int sd_used(const nsn_sdpins_t *p, int pins[6])
{
    if (p->mode == NSN_SD_SPI) { pins[0] = p->clk; pins[1] = p->mosi; pins[2] = p->miso; pins[3] = p->cs; return 4; }
    pins[0] = p->clk; pins[1] = p->cmd; pins[2] = p->d0; pins[3] = p->d1; pins[4] = p->d2; pins[5] = p->d3;
    return p->width == 4 ? 6 : 3;
}

static bool sd_same(const nsn_sdpins_t *a, const nsn_sdpins_t *b)
{
    int pa[6], pb[6];
    int na = sd_used(a, pa), nb = sd_used(b, pb);
    if (a->mode != b->mode || na != nb) return false;
    return !memcmp(pa, pb, (size_t)na * sizeof pa[0]) && (a->mode == NSN_SD_SPI || a->width == b->width);
}

/* the board's own pins are always accepted for their role, even when the table reserves them (GPIO60: strapping pin, stock LED) */
static bool is_default_sd_pin(int g)
{
    nsn_sdpins_t d; board_default_sd(&d);
    int p[6], n = sd_used(&d, p);
    for (int i = 0; i < n; i++) if (p[i] == g) return true;
    return false;
}
static bool is_default_led_pin(int g) { nsn_hw_led_t d; board_default_led(&d); return d.gpio == g; }

/* pins[0..n) must exist, not be reserved (unless exempt), and be distinct. why gets the reason on failure. */
static bool check_pins(const int *pins, int n, bool (*exempt)(int), char *why, size_t wn)
{
    for (int i = 0; i < n; i++) if (!board_gpio_exists(pins[i])) { snprintf(why, wn, "range"); return false; }
    for (int i = 0; i < n; i++) if (board_gpio_reserved(pins[i]) && !(exempt && exempt(pins[i]))) { snprintf(why, wn, "reserved %d", pins[i]); return false; }
    for (int i = 0; i < n; i++) for (int j = 0; j < i; j++) if (pins[i] == pins[j]) { snprintf(why, wn, "duplicate %d", pins[i]); return false; }
    return true;
}

static bool check_sd(const nsn_sdpins_t *p, int led_gpio, char *why, size_t wn)
{
    if (p->mode == NSN_SD_SDMMC) {
        if (p->width != 1 && p->width != 4) { snprintf(why, wn, "width"); return false; }
        if (p->width == 1 && (p->d1 != -1 || p->d2 != -1 || p->d3 != -1)) { snprintf(why, wn, "width"); return false; }
    }
    int pins[6], n = sd_used(p, pins);
    if (!check_pins(pins, n, is_default_sd_pin, why, wn)) return false;
    if (led_gpio >= 0) for (int i = 0; i < n; i++) if (pins[i] == led_gpio) { snprintf(why, wn, "sd/led conflict %d", led_gpio); return false; }
    if (p->mode == NSN_SD_SDMMC) {
        /* SOC_SDMMC_USE_IOMUX, no GPIO matrix (SDMMC_LL_SLOT_SUPPORT_GPIO_MATRIX = 0): the host reaches slot 0's pins only. Slot 1 (39 40 35..38) is made of
         * reserved strapping pins here, so it was refused above. */
        bool fixed = p->clk == SDMMC_SLOT0_IOMUX_PIN_NUM_CLK && p->cmd == SDMMC_SLOT0_IOMUX_PIN_NUM_CMD && p->d0 == SDMMC_SLOT0_IOMUX_PIN_NUM_D0 &&
                     (p->width == 1 || (p->d1 == SDMMC_SLOT0_IOMUX_PIN_NUM_D1 && p->d2 == SDMMC_SLOT0_IOMUX_PIN_NUM_D2 && p->d3 == SDMMC_SLOT0_IOMUX_PIN_NUM_D3));
        if (!fixed) { snprintf(why, wn, "not supported sdmmc-pins-fixed"); return false; }
    }
    return true;
}

static bool check_led(const nsn_hw_led_t *l, const nsn_sdpins_t *sd, char *why, size_t wn)
{
    if (!nsn_hw_order_ok(l->order)) { snprintf(why, wn, "order"); return false; }
    if (l->gpio < 0) return true;                                           /* off: no pin involved */
    int gpio = l->gpio;
    if (!check_pins(&gpio, 1, is_default_led_pin, why, wn)) return false;
    int pins[6], n = sd_used(sd, pins);
    for (int i = 0; i < n; i++) if (pins[i] == l->gpio) { snprintf(why, wn, "sd/led conflict %d", l->gpio); return false; }
    return true;
}

/* ---- storage ---- */

static bool legacy_sd(nsn_sdpins_t *p)      /* the 7-byte blob of older builds: clk cmd d0 d1 d2 d3 width */
{
    nvs_handle_t h; uint8_t b[7]; size_t len = sizeof b; bool ok = false;
    if (nvs_open("nsn", NVS_READONLY, &h) != ESP_OK) return false;
    if (nvs_get_blob(h, K_OLD, b, &len) == ESP_OK && len == sizeof b) {
        *p = (nsn_sdpins_t){ (int8_t)b[0], (int8_t)b[1], (int8_t)b[2], (int8_t)b[3], (int8_t)b[4], (int8_t)b[5], b[6], NSN_SD_SDMMC, -1, -1, -1 };
        ok = true;
    }
    nvs_close(h);
    return ok;
}

static bool sd_parse(const char *s, nsn_sdpins_t *p)
{
    int v[7];
    board_default_sd(p);
    if (s[0] == 'm' && sscanf(s, "m %d %d %d %d %d %d %d", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6]) == 7) {
        *p = (nsn_sdpins_t){ (int8_t)v[0], (int8_t)v[1], (int8_t)v[2], (int8_t)v[3], (int8_t)v[4], (int8_t)v[5], (uint8_t)v[6], NSN_SD_SDMMC, -1, -1, -1 };
        return true;
    }
    if (s[0] == 's' && sscanf(s, "s %d %d %d %d", &v[0], &v[1], &v[2], &v[3]) == 4) {
        *p = (nsn_sdpins_t){ (int8_t)v[0], -1, -1, -1, -1, -1, 1, NSN_SD_SPI, (int8_t)v[1], (int8_t)v[2], (int8_t)v[3] };
        return true;
    }
    return false;
}

/* the stored value, or the board default when nothing is stored (or what is stored does not pass the checks: a bad value must never keep the box from booting) */
static bool load_led(nsn_hw_led_t *l)
{
    char s[24]; int g; char o[8];
    board_default_led(l);
    if (!nsn_settings_get_str(K_LED, s, sizeof s)) return false;
    if (sscanf(s, "%d %7s", &g, o) != 2 || g < -1 || g > 127 || !nsn_hw_order_ok(o)) return false;
    l->gpio = (int8_t)g; strlcpy(l->order, o, sizeof l->order);
    return true;
}

static bool load_sd(nsn_sdpins_t *p)
{
    char s[48];
    board_default_sd(p);
    if (nsn_settings_get_str(K_SD, s, sizeof s)) return sd_parse(s, p);
    return legacy_sd(p);
}

static void load_both(nsn_sdpins_t *sd, nsn_hw_led_t *led, bool *sd_custom, bool *led_custom)
{
    nsn_sdpins_t dsd; nsn_hw_led_t dled; char why[48];
    board_default_sd(&dsd); board_default_led(&dled);
    *sd_custom = load_sd(sd); *led_custom = load_led(led);
    if (*led_custom && !check_led(led, *sd_custom ? sd : &dsd, why, sizeof why)) { ESP_LOGW(TAG, "stored led setting refused (%s): default used", why); *led = dled; *led_custom = false; }
    if (*sd_custom && !check_sd(sd, led->gpio, why, sizeof why)) { ESP_LOGW(TAG, "stored sd setting refused (%s): default used", why); *sd = dsd; *sd_custom = false; }
    if (*sd_custom && sd_same(sd, &dsd)) *sd_custom = false;
    if (*led_custom && led->gpio == dled.gpio && !strcmp(led->order, dled.order)) *led_custom = false;
}

void nsn_hw_load(void)
{
    bool a, b;
    load_both(&s_sd_run, &s_led_run, &a, &b);
    nsn_storage_set_pins(&s_sd_run);
    int p[6], n = sd_used(&s_sd_run, p);
    char pins[40]; size_t k = 0;
    for (int i = 0; i < n; i++) k += (size_t)snprintf(pins + k, sizeof pins - k, "%s%d", i ? " " : "", p[i]);
    ESP_LOGI(TAG, "sd %s %s: %s; led %s gpio %d %s", s_sd_run.mode == NSN_SD_SPI ? "spi" : "sdmmc", a ? "custom" : "default", pins,
             b ? "custom" : "default", s_led_run.gpio, s_led_run.order);
}

static bool save_sd(const nsn_sdpins_t *p)
{
    nsn_sdpins_t d; board_default_sd(&d);
    nsn_settings_erase(K_OLD);
    if (sd_same(p, &d)) { nsn_settings_erase(K_SD); return true; }
    char s[48];
    if (p->mode == NSN_SD_SPI) snprintf(s, sizeof s, "s %d %d %d %d", p->clk, p->mosi, p->miso, p->cs);
    else snprintf(s, sizeof s, "m %d %d %d %d %d %d %d", p->clk, p->cmd, p->d0, p->d1, p->d2, p->d3, p->width);
    return nsn_settings_set_str(K_SD, s);
}

static bool save_led(const nsn_hw_led_t *l)
{
    nsn_hw_led_t d; board_default_led(&d);
    if (l->gpio == d.gpio && !strcmp(l->order, d.order)) { nsn_settings_erase(K_LED); return true; }
    char s[24];
    snprintf(s, sizeof s, "%d %s", l->gpio, l->order);
    return nsn_settings_set_str(K_LED, s);
}

/* ---- the command ---- */

typedef struct { nsn_out_t out; void *ctx; } sink_t;
static void say(const sink_t *o, const char *fmt, ...)
{
    char b[320]; va_list ap;
    va_start(ap, fmt); vsnprintf(b, sizeof b, fmt, ap); va_end(ap);
    o->out(o->ctx, b);
}

static const char *state_word(void)
{
    switch (nsn_storage_state()) { case NSN_CARD_OK: return "mounted"; case NSN_CARD_BAD: return "unreadable"; default: return "none"; }
}

static void show(const sink_t *o)
{
    nsn_sdpins_t sd; nsn_hw_led_t led; bool sd_c, led_c;
    load_both(&sd, &led, &sd_c, &led_c);
    const char *src = sd_c ? "custom" : "default";
    if (sd.mode == NSN_SD_SPI) say(o, "hw sd clk=%d mosi=%d miso=%d cs=%d src=%s state=%s mode=spi", sd.clk, sd.mosi, sd.miso, sd.cs, src, state_word());
    else say(o, "hw sd clk=%d cmd=%d d0=%d d1=%d d2=%d d3=%d width=%d src=%s state=%s mode=sdmmc", sd.clk, sd.cmd, sd.d0, sd.d1, sd.d2, sd.d3, sd.width, src, state_word());
    say(o, "hw sdmmc routing=iomux clk=%d cmd=%d d0=%d d1=%d d2=%d d3=%d spi=any", SDMMC_SLOT0_IOMUX_PIN_NUM_CLK, SDMMC_SLOT0_IOMUX_PIN_NUM_CMD,
        SDMMC_SLOT0_IOMUX_PIN_NUM_D0, SDMMC_SLOT0_IOMUX_PIN_NUM_D1, SDMMC_SLOT0_IOMUX_PIN_NUM_D2, SDMMC_SLOT0_IOMUX_PIN_NUM_D3);
    say(o, "hw led gpio=%d order=%s on=%d src=%s", led.gpio, led.order, led.gpio >= 0, led_c ? "custom" : "default");
    char list[160]; size_t k = 0;
    for (int g = 0; g <= board_gpio_max() && k + 6 < sizeof list; g++) if (board_gpio_reserved(g)) k += (size_t)snprintf(list + k, sizeof list - k, "%s%d", k ? "," : "", g);
    list[k] = 0;
    say(o, "hw reserved %s", list);
    say(o, "hw gpios 0-%d", board_gpio_max());
    say(o, "hw pending %d", !sd_same(&sd, &s_sd_run) || strcmp(led.order, s_led_run.order) || led.gpio != s_led_run.gpio);
}

static int split(char *s, char **t, int max)       /* words of s into t; returns the count, max + 1 when there are more */
{
    int n = 0;
    for (char *w = strtok(s, " "); w; w = strtok(NULL, " ")) { if (n == max) return max + 1; t[n++] = w; }
    return n;
}

static bool num(const char *s, int *v)             /* a whole token as an integer */
{
    char *e;
    long n = strtol(s, &e, 10);
    if (!*s || *e || n < -1000 || n > 1000) return false;
    *v = (int)n;
    return true;
}

/* `<clk> <cmd> <d0> <d1> <d2> <d3> <width>` or `spi <clk> <mosi> <miso> <cs>` (the words after `hw sd`); why = the refusal when false */
static bool parse_sd(char *args, nsn_sdpins_t *p, const char **why)
{
    char *t[9]; int v[7], k = split(args, t, 8);
    bool spi = k > 0 && !strcmp(t[0], "spi");
    int n = spi ? 4 : 7;
    *why = "syntax";
    if (k != n + (spi ? 1 : 0)) return false;
    for (int i = 0; i < n; i++) if (!num(t[i + (spi ? 1 : 0)], &v[i])) return false;
    for (int i = 0; i < n; i++) if (v[i] < -1 || v[i] > 127) { *why = "range"; return false; }        /* must fit the stored int8 */
    if (spi) *p = (nsn_sdpins_t){ (int8_t)v[0], -1, -1, -1, -1, -1, 1, NSN_SD_SPI, (int8_t)v[1], (int8_t)v[2], (int8_t)v[3] };
    else *p = (nsn_sdpins_t){ (int8_t)v[0], (int8_t)v[1], (int8_t)v[2], (int8_t)v[3], (int8_t)v[4], (int8_t)v[5], (uint8_t)v[6], NSN_SD_SDMMC, -1, -1, -1 };
    return true;
}

static void set_sd(const sink_t *o, char *args)
{
    nsn_sdpins_t p, cur_sd; nsn_hw_led_t cur_led; bool a, b; const char *why = "";
    char reason[48];
    if (!parse_sd(args, &p, &why)) { say(o, "hw sd: refused %s", why); return; }
    load_both(&cur_sd, &cur_led, &a, &b);
    if (!check_sd(&p, cur_led.gpio, reason, sizeof reason)) { say(o, "hw sd: refused %s", reason); return; }
    say(o, save_sd(&p) ? "hw sd: ok, reboot to apply" : "hw sd: refused storage");
}

/* `hw led <gpio> [order]` or `hw led off` */
static void set_led(const sink_t *o, char *args)
{
    char *t[3]; int k = split(args, t, 2), g = -1;
    nsn_sdpins_t sd; nsn_hw_led_t l, cur; bool a, b; char why[48];
    board_default_led(&l);
    if (k == 1 && !strcmp(t[0], "off")) l.gpio = -1;
    else {
        if (k < 1 || k > 2 || !num(t[0], &g)) { say(o, "hw led: refused syntax"); return; }
        if (g < 0 || g > 127) { say(o, "hw led: refused range"); return; }
        l.gpio = (int8_t)g;
        if (k == 2) { if (strlen(t[1]) >= sizeof l.order) { say(o, "hw led: refused order"); return; } strlcpy(l.order, t[1], sizeof l.order); }
    }
    load_both(&sd, &cur, &a, &b);
    if (!check_led(&l, &sd, why, sizeof why)) { say(o, "hw led: refused %s", why); return; }
    say(o, save_led(&l) ? "hw led: ok, reboot to apply" : "hw led: refused storage");
}

void nsn_hw_exec(const char *args, bool local, nsn_out_t out, void *ctx)
{
    sink_t o = { out, ctx };
    if (!args || !*args) { show(&o); return; }
    if (!local) { say(&o, "?"); return; }                      /* the setters are for the wired console, like the file writes */
    char buf[96];
    strlcpy(buf, args, sizeof buf);
    char *rest = strchr(buf, ' ');
    if (rest) *rest++ = 0;
    if (!strcmp(buf, "sd") && rest) set_sd(&o, rest);
    else if (!strcmp(buf, "led") && rest) set_led(&o, rest);
    else if (!strcmp(buf, "reset") && !rest) {
        nsn_settings_erase(K_SD); nsn_settings_erase(K_OLD); nsn_settings_erase(K_LED);
        say(&o, "hw reset: ok, reboot to apply");
    } else say(&o, "?");
}

/* the older `sdpins`: reading for everyone, "reset" and the 7-number setter on the wired console only; the same checks as `hw sd`, its own wording */
void nsn_hw_sdpins_legacy(const char *args, bool local, nsn_out_t out, void *ctx)
{
    sink_t o = { out, ctx };
    if (args && !local) { say(&o, "?"); return; }
    nsn_sdpins_t sd, p; nsn_hw_led_t led; bool a, b;
    char buf[96], why[48]; const char *w = "";
    strlcpy(buf, args ? args : "", sizeof buf);
    load_both(&sd, &led, &a, &b);
    if (strstr(buf, "reset")) {
        nsn_settings_erase(K_SD); nsn_settings_erase(K_OLD);
        say(&o, "sdpins reset; reboot to apply");
    } else if (buf[0]) {
        if (!parse_sd(buf, &p, &w)) say(&o, "sdpins rejected: %s", w);
        else if (p.mode != NSN_SD_SDMMC) say(&o, "sdpins rejected: syntax");
        else if (!check_sd(&p, led.gpio, why, sizeof why)) say(&o, "sdpins rejected: %s", why);
        else say(&o, save_sd(&p) ? "sdpins saved; reboot to apply" : "sdpins rejected: storage");
    } else {
        say(&o, "sdpins clk=%d cmd=%d d0=%d d1=%d d2=%d d3=%d width=%d", sd.clk, sd.cmd, sd.d0, sd.d1, sd.d2, sd.d3, sd.width);
    }
}
