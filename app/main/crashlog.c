// SPDX-License-Identifier: LGPL-2.1-or-later
#include "crashlog.h"
#include "clock.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "esp_app_desc.h"
#include "pc/pc_crash.h"

static const char *TAG = "crash";
#define LOGDIR "/sdcard/_player/logs"
#define PATH LOGDIR "/crash.txt"
static RTC_NOINIT_ATTR pc_crash_t s_live;
static RTC_NOINIT_ATTR uint32_t s_streak, s_streak_inv;
static pc_crash_t s_pending; static int s_pending_reason; static bool s_have, s_safe;

static bool valid(const pc_crash_t *c) { return c->magic == PC_CRASH_MAGIC && c->mode <= PC_CRASH_MODE_CAST; }

void nsn_crash_boot(void)
{
    esp_reset_reason_t r = esp_reset_reason();
    bool crash = r == ESP_RST_PANIC || r == ESP_RST_INT_WDT || r == ESP_RST_TASK_WDT || r == ESP_RST_WDT || r == ESP_RST_BROWNOUT;
    uint32_t streak = (s_streak_inv == ~s_streak) ? s_streak : 0;
    if (crash && valid(&s_live)) {
        s_pending = s_live; s_pending_reason = (int)r; s_have = true;
        streak = s_live.uptime_s < 60 ? streak + 1 : 1;
    } else streak = 0;
    s_streak = streak; s_streak_inv = ~streak;
    s_safe = streak >= 3;
    s_live = (pc_crash_t){ .magic = PC_CRASH_MAGIC, .mode = PC_CRASH_MODE_NORMAL };
    if (s_safe) ESP_LOGE(TAG, "boot loop (%u crashes in a row): safe mode", (unsigned)streak);
    else if (s_have) ESP_LOGW(TAG, "last run ended in reset reason %d after %u s (%s)", s_pending_reason, (unsigned)s_pending.uptime_s, s_pending.screen);
}

void nsn_crash_crumb(const char *what)
{
    size_t i = 0;
    for (; what && what[i] && i < sizeof s_live.screen; i++) s_live.screen[i] = (what[i] >= 32 && what[i] < 127) ? what[i] : '?';
    for (; i < sizeof s_live.screen; i++) s_live.screen[i] = 0;
    s_live.uptime_s = (uint32_t)(esp_timer_get_time() / 1000000);
    s_live.internal_min = (uint32_t)heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
    time_t now = time(NULL);
    s_live.wall = nsn_clock_valid() ? (uint32_t)now : 0;
}

void nsn_crash_flush(void)
{
    if (!s_have) return;
    char line[200];
    int n = pc_crash_format(&s_pending, s_pending_reason, esp_app_get_description()->version, line, sizeof line);
    if (n <= 0) return;
    mkdir("/sdcard/_player", 0777); mkdir(LOGDIR, 0777);
    FILE *f = fopen(PATH, "a");
    if (!f) return;
    fwrite(line, 1, (size_t)n, f); fclose(f);
    s_have = false;
    ESP_LOGW(TAG, "crash line written to " PATH);
}
bool nsn_crash_safe_mode(void) { return s_safe; }
