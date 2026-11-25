// SPDX-License-Identifier: LGPL-2.1-or-later
#include "fmvgm.h"
#include "fmvgm_port_idf.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static unsigned s_reserve = 24 * 1024;      /* the largest internal block that must stay free after a FAST allocation (the HTTPS check of the radio wants 24 KB; with Auracast loaded the whole chip state still fits: measured, docs/measurements.md) */
void fmvgm_port_set_internal_reserve(unsigned bytes) { s_reserve = bytes; }

static void *p_alloc(size_t n, fmvgm_mem_t kind)
{
    if (kind == FMVGM_MEM_FAST && n <= 32 * 1024 && heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT) > n + s_reserve) {
        void *p = heap_caps_malloc(n, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (p) return p;
    }
    void *p = heap_caps_malloc(n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    return p ? p : heap_caps_malloc(n, MALLOC_CAP_8BIT);
}
static void p_free(void *p) { heap_caps_free(p); }
static void *p_task_start(void (*fn)(void *), void *arg, int core, int prio, uint32_t stack)
{
    TaskHandle_t t = NULL;
    if (xTaskCreatePinnedToCoreWithCaps(fn, "fmvgm", stack, arg, prio, &t, core, MALLOC_CAP_SPIRAM) != pdPASS) return NULL;
    return t;
}
static void p_task_wake(void *t) { if (t) xTaskNotifyGive((TaskHandle_t)t); }
static void p_task_wait(uint32_t ms) { TickType_t k = pdMS_TO_TICKS(ms); ulTaskNotifyTake(pdTRUE, k ? k : 1); }
static void p_task_prio(void *t, int prio) { if (t) vTaskPrioritySet((TaskHandle_t)t, prio); }
static void p_sleep(uint32_t ms) { TickType_t k = pdMS_TO_TICKS(ms); vTaskDelay(k ? k : 1); }
static uint32_t p_now(void) { return (uint32_t)esp_timer_get_time(); }

void fmvgm_port_install_idf(void)
{
    static const fmvgm_port_t port = { p_alloc, p_free, p_task_start, p_task_wake, p_task_wait, p_task_prio, p_sleep, p_now };
    fmvgm_set_port(&port);
}
