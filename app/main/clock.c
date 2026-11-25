// SPDX-License-Identifier: LGPL-2.1-or-later
#include "clock.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "esp_log.h"
#include "settings.h"

static const char *TAG = "clock";
#define TZ_DEFAULT "CST-8"
#define TZ_KEY     "tz"
#define T_MIN      1735689600LL       /* 2025-01-01 */
#define T_MAX      4102444800LL       /* 2100-01-01 */

static volatile bool s_ntp, s_client;
static char s_tz[48] = TZ_DEFAULT;

static void apply_tz(const char *tz)
{
    snprintf(s_tz, sizeof s_tz, "%s", tz);
    setenv("TZ", s_tz, 1);
    tzset();
}

void nsn_clock_boot(void)
{
    char tz[48];
    apply_tz(nsn_settings_get_str(TZ_KEY, tz, sizeof tz) && *tz ? tz : TZ_DEFAULT);
    ESP_LOGI(TAG, "zone %s", s_tz);
}

nsn_clock_src_t nsn_clock_src(void) { return s_ntp ? NSN_CLOCK_NTP : s_client ? NSN_CLOCK_CLIENT : NSN_CLOCK_NONE; }
bool nsn_clock_valid(void) { return nsn_clock_src() != NSN_CLOCK_NONE; }
void nsn_clock_ntp_synced(void) { s_ntp = true; }

/* days since 1970-01-01 of a civil date (Howard Hinnant's algorithm) */
static long long days_from_civil(int y, int m, int d)
{
    y -= m <= 2;
    long long era = (y >= 0 ? y : y - 399) / 400;
    int yoe = (int)(y - era * 400), doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1, doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

void nsn_clock_iso(time_t t, char *out, size_t n)
{
    struct tm tm; localtime_r(&t, &tm);
    /* the offset is local fields read as UTC minus the real UTC time (does not depend on libc's %z / tm_gmtoff support) */
    long long as_utc = days_from_civil(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday) * 86400LL + tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;
    int off = (int)((as_utc - (long long)t) / 60), a = off < 0 ? -off : off;
    snprintf(out, n, "%04d-%02d-%02dT%02d:%02d:%02d%c%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
             off < 0 ? '-' : '+', a / 60, a % 60);
}

void nsn_clock_status(char *out, size_t n)
{
    time_t t = time(NULL); char iso[40];
    nsn_clock_iso(t, iso, sizeof iso);
    snprintf(out, n, "time %lld %s src=%s tz=\"%s\"", (long long)t, iso, (const char *[]){ "none", "client", "ntp" }[nsn_clock_src()], s_tz);
}

static bool tz_ok(const char *tz)
{
    size_t l = strlen(tz);
    if (l < 1 || l > 47) return false;
    for (; *tz; tz++) if ((unsigned char)*tz <= ' ' || (unsigned char)*tz > '~') return false;
    return true;
}

const char *nsn_clock_cmd_tz(const char *tz, char *out, size_t n)
{
    if (!tz_ok(tz)) { snprintf(out, n, "time tz: refused zone must be 1-47 printable characters without spaces"); return out; }
    apply_tz(tz);
    return nsn_settings_set_str(TZ_KEY, tz) ? "time tz: ok" : "time tz: ok, not stored";
}

/* args: "<unix seconds> [<tz>]" */
const char *nsn_clock_cmd_set(const char *args, char *out, size_t n)
{
    char *end; long long sec = strtoll(args, &end, 10);
    if (end == args || (*end && *end != ' ')) return "time set: refused seconds must be a number";
    if (sec < T_MIN || sec >= T_MAX) return "time set: refused seconds out of range (2025..2099)";
    while (*end == ' ') end++;
    if (*end && !tz_ok(end)) return "time set: refused zone must be 1-47 printable characters without spaces";
    if (*end) { apply_tz(end); nsn_settings_set_str(TZ_KEY, end); }
    if (s_ntp) return "time set: zone only, clock is from ntp";
    struct timeval tv = { .tv_sec = (time_t)sec, .tv_usec = 0 };
    settimeofday(&tv, NULL);
    s_client = true;
    ESP_LOGI(TAG, "clock set by a client");
    return "time set: ok";
}
