// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_vmute.h"
#include <string.h>
#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#else
#include <sched.h>
#endif
#if defined(__has_include)
#if __has_include("esp_attr.h")
#include "esp_attr.h"
#endif
#endif
#ifndef EXT_RAM_BSS_ATTR
#define EXT_RAM_BSS_ATTR
#endif

EXT_RAM_BSS_ATTR static uint8_t s_state[PC_VOICES_MAX];   /* the UI-side state, under s_lock (UI task + console task); PSRAM (the internal RAM budget is at its ceiling) */
EXT_RAM_BSS_ATTR static int s_solo1;                                    /* the soloed voice + 1, 0 = none (a .bss variable in PSRAM: no initial value other than zero) */
EXT_RAM_BSS_ATTR static uint32_t s_mask_req;                          /* the posted mask; written by the UI side, cleared by the audio task's reset */
EXT_RAM_BSS_ATTR static uint32_t s_extra_req;                         /* 1 = a solo is set: voices beyond the 32 mask bits (xmp modules with more channels) are muted too */
EXT_RAM_BSS_ATTR static uint32_t s_epoch;
EXT_RAM_BSS_ATTR static uint32_t s_seen;                              /* the epoch the UI copy belongs to (under s_lock) */
EXT_RAM_BSS_ATTR static bool s_reported;                              /* an internal sync dropped the copy: the next pc_vmute_sync() still says so (under s_lock) */
EXT_RAM_BSS_ATTR static uint32_t s_lock;

static uint32_t rd(const uint32_t *p) { return __atomic_load_n(p, __ATOMIC_ACQUIRE); }
static void wr(uint32_t *p, uint32_t v) { __atomic_store_n(p, v, __ATOMIC_RELEASE); }

/* The setters used to be "UI task only"; the console task calls them too (vmute command). A spin lock with a yield: the sections are a few stores, no allocation. */
static void lock(void)
{
    while (__atomic_exchange_n(&s_lock, 1, __ATOMIC_ACQUIRE)) {
#ifdef ESP_PLATFORM
        vTaskDelay(1);
#else
        sched_yield();
#endif
    }
}
static void unlock(void) { __atomic_store_n(&s_lock, 0, __ATOMIC_RELEASE); }

void pc_vmute_reset(void)
{
    __atomic_fetch_add(&s_epoch, 1, __ATOMIC_ACQ_REL);   /* the epoch first, then the mask: a post that lands after the mask clear has seen the new epoch (post() re-checks it) */
    wr(&s_mask_req, 0);
    wr(&s_extra_req, 0);
}
uint32_t pc_vmute_epoch(void) { return rd(&s_epoch); }
uint32_t pc_vmute_mask(void) { return rd(&s_mask_req); }
bool pc_vmute_extra_muted(void) { return rd(&s_extra_req) != 0; }

/* the audio side reset the source since this copy was made: the copy goes (every voice on) */
static void sync_locked(void)
{
    const uint32_t e = rd(&s_epoch);
    if (e == s_seen) return;
    s_seen = e;
    memset(s_state, 0, sizeof s_state);
    s_solo1 = 0;
    s_reported = true;
    wr(&s_mask_req, 0);                              /* a mask posted between the audio reset and this sync belonged to the old source */
    wr(&s_extra_req, 0);
}

static void post(int n)
{
    if (n > PC_VOICES_MAX) n = PC_VOICES_MAX;
    uint32_t m = 0;
    for (int i = 0; i < n; i++) {
        const bool mute = s_solo1 ? i != s_solo1 - 1 : s_state[i] == PC_VM_MUTED;
        if (mute) m |= 1u << i;
    }
    wr(&s_mask_req, m);
    wr(&s_extra_req, s_solo1 != 0);
    if (rd(&s_epoch) != s_seen) {                    /* a reset slipped in between the sync and the write: what was written belongs to the old source */
        wr(&s_mask_req, 0);
        wr(&s_extra_req, 0);
    }
}

static pc_vm_state_t get_locked(int i) { return i >= 0 && i < PC_VOICES_MAX ? (pc_vm_state_t)s_state[i] : PC_VM_ON; }

static void set_locked(int i, pc_vm_state_t s, int n)
{
    if (i < 0 || i >= PC_VOICES_MAX || (unsigned)s > PC_VM_SOLO) return;
    sync_locked();                                   /* the source changed since the UI copy was made: the copy goes first, then this change applies to the new source (every voice on + this one) */
    if (s == PC_VM_SOLO) {
        if (s_solo1 && s_solo1 - 1 != i) s_state[s_solo1 - 1] = PC_VM_ON;      /* one voice only */
        s_solo1 = i + 1;
    } else if (s_solo1 == i + 1) s_solo1 = 0;
    s_state[i] = (uint8_t)s;
    post(n);
}

pc_vm_state_t pc_vmute_get(int i) { lock(); pc_vm_state_t r = get_locked(i); unlock(); return r; }
int pc_vmute_solo_voice(void) { lock(); int r = s_solo1 - 1; unlock(); return r; }

void pc_vmute_set(int i, pc_vm_state_t s, int n) { lock(); set_locked(i, s, n); unlock(); }

void pc_vmute_cycle(int i, int n)
{
    if (i < 0 || i >= PC_VOICES_MAX) return;
    lock();
    sync_locked();
    const pc_vm_state_t s = get_locked(i);
    set_locked(i, s == PC_VM_ON ? PC_VM_MUTED : s == PC_VM_MUTED ? PC_VM_SOLO : PC_VM_ON, n);
    unlock();
}

void pc_vmute_clear(int n)
{
    lock();
    sync_locked();
    memset(s_state, 0, sizeof s_state);
    s_solo1 = 0;
    post(n);
    unlock();
}

bool pc_vmute_sync(void)
{
    lock();
    sync_locked();
    const bool r = s_reported;
    s_reported = false;
    unlock();
    return r;
}
