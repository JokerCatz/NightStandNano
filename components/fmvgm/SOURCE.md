# fmvgm (FM-chip music: YM2151 and YM2203 from VGM / VGZ / S98)

The player is written for this repository (LGPL-2.1-or-later, SPDX headers in `src/` and `include/`). The chip cores are **ymfm**, vendored in `ymfm/`.

## ymfm

- Upstream: Aaron Giles, https://github.com/aaronsgiles/ymfm , commit `81aec25ccbb98f4873a255f7551ac4dadac59b4a` (2026-07-27), fetched 2026-11-25.
- Licence: BSD 3-Clause, `ymfm/LICENSE` (copy in `docs/licenses/ymfm-BSD3.txt`).
- Taken, unchanged except for the patches below: `ymfm.h ymfm_fm.h ymfm_fm.ipp ymfm_opm.h ymfm_opm.cpp ymfm_opn.h ymfm_opn.cpp ymfm_ssg.h ymfm_ssg.cpp ymfm_adpcm.h ymfm_adpcm.cpp`.
  `ymfm_adpcm.*` is needed because `ymfm_opn.cpp` also holds the YM2608 / YM2610 classes (the linker drops what is not used). Not taken: OPL (`ymfm_opl.*`), OPQ / OPZ / OPX, `ymfm_misc.*`, `ymfm_pcm.*`, the examples.
- Built with `-O2` (set in `CMakeLists.txt` whatever the project default is), `-DNDEBUG` (ymfm's index assertions are compiled out; the parsers validate their input instead), `-w` (GCC 15 warns inside the OPL part of `ymfm_fm.ipp`), `-ffp-contract=off` (only matters for the resampler, which is in `src/`).
- ymfm is pure integer: the same register writes give the same output samples on any platform. `tools/hosttest/` checks this against the study's reference benchmark (identical `sumL` / `sumR`) and the board prints the same checksums as the PC.

## Local changes (`patches/`, applied in order to the upstream files; the three patched files are reproduced byte for byte by `patch -p1` on a copy of upstream)

| patch | file | what | why | proof it changes no output sample |
|---|---|---|---|---|
| 0001 `allocation-hook` | `ymfm.h`, `ymfm_fm.h` | `fm_operator` and `fm_channel` get class-level `operator new / delete` that call `ymfm::ymfm_alloc / ymfm_free`, which the host defines (fmvgm: a per-chip arena filled while the chip is constructed, freed as one block) | the firmware chooses where chip state lives (internal RAM or PSRAM) through the port's allocator, and nothing is allocated afterwards | no arithmetic involved |
| 0002 `skip-released-operators` | `ymfm_fm.ipp` (`fm_operator::clock`) | an operator in the release state at the full attenuation 0x3ff, with no SSG-EG state, returns at once | such an operator changes nothing but its phase until its next key-on, and a key-on resets the phase (`start_attack`) and starts from the same attenuation; its output is 0 (`compute_volume` returns 0 above `EG_QUIET`). The study measured 47 % of a dense YM2151's cost for a chip that plays nothing | `tools/hosttest/run_fm.sh` against a build without 0002 / 0003 (`YMFM_DIR`): 26 test files, every checksum identical, among them four random-register streams per chip family that switch SSG-EG, LFO, CSM and channel-3 modes at random. A negative control (the condition loosened to 0x3c0) is flagged different on every file |
| 0003 `phase-step-cache` | `ymfm_fm.h`, `ymfm_fm.ipp` (`fm_operator::clock_phase`, `prepare`) | for an operator whose phase step depends on the LFO (`PHASE_STEP_DYNAMIC`: YM2151 with PM) the step computed for the last LFO value is reused while the LFO value is unchanged; `prepare()` (which rebuilds the operator's cache after any register write) drops it | the study: `compute_phase_step` is 28 % of a dense YM2151. The LFO value holds for many samples | same test set as 0002 |

Measured gain on the board (ESP32-S31, producer only, 20 s of the synthetic streams, native rate, core 1): YM2151 dense 937 -> 713 ms per second of audio, typical 900 -> 662, a chip that plays nothing 462 -> 164; YM2203 unchanged (no PM, few idle operators). Numbers and method: `docs/measurements.md`.

## What is ours (`src/`, `include/`)

`fmvgm_src.cpp` VGM and S98 command sources, GD3 / S98 tags, routing; `fmvgm_core.cpp` chips, song clock, rate groups, producer, ring, worker, the `pc_backend_t`; `fmvgm_resamp.[ch]` the integer polyphase resampler; `fmvgm_port_idf.c` the ESP-IDF port (the only file that includes FreeRTOS or heap_caps headers); `include/fmvgm.h` the public surface (documented in the header). Host build and tests: `tools/hosttest/run_fm.sh`, `test_fmvgm.cpp`, `gen_fm_vgm.py`, `alias_test.c`.
Dependencies: `player_core` for `pc/pc_backend.h`, `pc/pc_io.h`, `pc/pc_vgmhdr.[ch]` (JukeDial has the same files) and nothing else.
