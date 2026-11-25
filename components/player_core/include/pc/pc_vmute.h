// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* Per-voice mute and solo of the playing chip or tracker file (visualiser page, owner 2026-11-17).
 * The UI task owns the per-voice STATE (on / muted / solo) and posts the resulting MUTE MASK; the audio task's backends read
 * the mask before they decode a block (gme_mute_voices, xmp_channel_mute) and clear it when a new source takes the voice
 * snapshot over (a track change resets every voice to on). The mix code (pc_rv_block, the sink snapshot, the toy-sound mix points) is not involved:
 * muting only removes signal inside the decoder, before any gain.
 * Rules: solo = hear exactly one voice (soloing a voice ends an earlier solo); mute is independent per voice while no solo is set;
 * cycling a voice goes on -> muted -> solo -> on. Thread rules: the state functions may be called from the UI task and the console task (locked); pc_vmute_mask() and
 * pc_vmute_epoch() are read from the audio task; pc_vmute_reset() is called by the audio task (a backend taking a source over). */
#include <stdbool.h>
#include <stdint.h>
#include "pc/pc_voice.h"

typedef enum { PC_VM_ON, PC_VM_MUTED, PC_VM_SOLO } pc_vm_state_t;

#ifdef __cplusplus
extern "C" {
#endif
/* audio task: a new source owns the voices: every voice on, the posted mask 0, the epoch moves (the UI drops its copy when it sees that) */
void pc_vmute_reset(void);
uint32_t pc_vmute_epoch(void);                       /* any task */
uint32_t pc_vmute_mask(void);                        /* any task: bit i set = voice i muted (a solo already folded in: every other voice's bit) */
bool pc_vmute_extra_muted(void);                     /* any task: a solo is set, so voices beyond the 32 mask bits (xmp channels 33+) are muted too */
/* UI task or console task (a spin lock serialises them): the state of voice i (0 for out of range) and the setters; each posts the new mask. n = the voice count (the solo mask covers voices 0..n-1). */
pc_vm_state_t pc_vmute_get(int i);
void pc_vmute_cycle(int i, int n);                   /* on -> muted -> solo -> on */
void pc_vmute_set(int i, pc_vm_state_t s, int n);
void pc_vmute_clear(int n);                          /* every voice on */
int  pc_vmute_solo_voice(void);                      /* the soloed voice, -1 none */
/* UI task: call every frame with the epoch seen: when the audio side reset the state (a track change) the UI copy is dropped. Returns true when it was. */
bool pc_vmute_sync(void);
#ifdef __cplusplus
}
#endif
