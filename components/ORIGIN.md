# Where components/ comes from

NightStandNano is an independent repository. The decoders, `player_core` and the other libraries below were **copied** from
JukeDial (`ESP32S31_round`) and then **pruned** to what this firmware links. From the copy date on the two repositories are
maintained separately: each may look at the other, neither has to follow it. Nothing in the build refers to a sibling checkout.

- Copied from: JukeDial `ESP32S31_round`, commit `6fdce406c3532130338d71ce34b1e723b1fa00f1` (clean working tree), on 2026-11-25.
- Only files tracked by git in that commit were taken (`git ls-files components/<name>`); round's untracked build directories were not.
- Same sources as published in the owner's public repository `JokerCatz/JukeDial` under LGPL-2.1 (owner's statement, not
  re-checked against the published tree), so copying them raised no new publication question. Every licence file, copyright header and
  third-party notice that travels with a component was kept (list below).
- `tools/bt_ldac/` (`ldac.patch` byte-identical, `make_bt.sh` adapted: runs from this repo, no-op while current, `--force` regenerates)
  was copied from JukeDial's `tools/bt_ldac` at the same commit. `tools/fatfs_exfat/` had been copied earlier (see `docs/exfat.md`).
- `docs/licenses/` holds the eight licence texts that the components' `SOURCE.md` files refer to (copied from JukeDial's `docs/licenses/`).

## What was copied, and what is left

Files / lines / compiled sources (`.c`, `.cpp`) per component, copy -> after pruning (2026-11-25):

| component | files | lines | .c/.cpp files |
|---|---|---|---|
| emu2413 | 6 -> 6 | 2267 -> 2267 | 1 -> 1 |
| gme | 131 -> 128 | 41442 -> 35945 | 51 -> 48 |
| libadplug | 151 -> 148 | 40303 -> 40187 | 65 -> 65 |
| libcrsid | 30 -> 30 | 5347 -> 5347 | 14 -> 14 |
| libfreeaptx | 6 -> removed | 2020 -> removed | 1 -> removed |
| libhvl | 10 -> 10 | 3650 -> 3650 | 2 -> 2 |
| libldac | 34 -> 34 | 8376 -> 8376 | 22 -> 22 |
| libmpcdec | 31 -> 29 | 4794 -> 4601 | 9 -> 9 |
| libtta | 12 -> 12 | 2191 -> 2191 | 1 -> 1 |
| libxmp | 214 -> 206 | 72848 -> 70864 | 153 -> 145 |
| player_core | 610 -> 125 | 101093 -> 32817 | 376 -> 62 |
| **total** | 1235 -> 728 | 284331 -> 206245 | 695 -> 369 |

`libfreeaptx` (aptX encoder) was removed entirely: nothing includes its header and its object contributes 0 bytes (aptX is not wired
into Nano). `espressif/cjson` is no longer a dependency of `player_core` (only the removed `pc_scenes.c` used it).

How the pruning was decided (not by guessing): `tools/prune_report.py` reads the linker map of the baseline build. A source file is
unused when none of its object's input sections is placed in the ELF (`-ffunction-sections -fdata-sections --gc-sections`: an object can be
pulled in and still contribute 0 bytes). Headers and other files were removed when no remaining source reaches them through
`#include` (text-level closure, conservative: conditional includes count). Run `tools/prune_report.py --unused --files` after a
build to see whether anything dead has crept back in; `--summary` prints the table above.

Removed: 129 `player_core` sources that contribute 0 bytes (UI, games, clock, power, theme, thumbnails, JukeDial-only services) with their
headers, the `player_core/host_test` tree (224 files; its CMake needs JukeDial's `tools/gen`, `tools/host_test.sh`, network FetchContent
of cJSON and untracked fixtures, so it could not stand alone), `libxmp` `misc.c` `md5.c` `win32.c` and the five unbuilt ProWizard
`pm*.c` / `pp30.c`, `gme` `M3u_Playlist.cpp` and the unbuilt `Ym2612_MAME.cpp` / `Ym2612_Nuked.cpp`, three unreferenced `libadplug`
headers, two unreferenced `libmpcdec` headers.
`player_core` now lists its sources explicitly instead of globbing `src/*.c` (a new file must be added to `components/player_core/CMakeLists.txt`).

Kept on purpose although the build does not read them: every `patches/` directory (the record of what was changed against upstream, which the
`SOURCE.md` files and the LGPL modification notices refer to), `SOURCE.md` files, licence / notice / credit / readme files, `Kconfig`
(`FX_VELVET_ROOM`, read by `pc_fxapp_reverb.c` and `pc_fxchain.c`), `libxmp/xmp_sources.cmake`.
Function-level pruning inside kept files (for example the unused part of `pc_i18n_data.c`) was out of scope.

Image check: the firmware image built after the copy is byte-identical to the one built from JukeDial's tree; after pruning the loadable
content, every section and every symbol are identical (`idf.py size`, `size-components`, `size-files` unchanged). The `.bin` differs in the
ELF checksum fields only, because deleting `md5.c` changes the symbol-table type / size of the three ROM symbols `MD5Init`, `MD5Update`,
`MD5Final` in the ELF (debug / symbol metadata, no code or data). With `md5.c` put back, the pruned tree builds the identical `.bin`.

## Licences that travel with the components

| component | licence | file(s) kept |
|---|---|---|
| player_core (own code) | LGPL-2.1-or-later (SPDX headers) | per-file headers |
| player_core/third_party/dr_flac | public domain or MIT-0 | end of `dr_flac.h`, `SOURCE.md` |
| player_core/third_party/minimp3 | CC0 | `LICENSE`, `README.md`, `SOURCE.md` |
| player_core/third_party/miniz | public domain (unlicense) / MIT | `LICENSE`, `README.md`, `SOURCE.md` |
| player_core/third_party/tomlc99 | MIT | `LICENSE`, `README.md`, `SOURCE.md` |
| player_core/third_party/zepto8 | WTFPL v2 (reference only, no code) | `SOURCE.md`, `docs/licenses/zepto8-WTFPL.txt` |
| libldac | Apache-2.0 (LDAC in a product needs Sony's certification) | `LICENSE`, `NOTICE`, `SOURCE.md` |
| libxmp | MIT / Expat | `libxmp/docs/COPYING`, `libxmp/docs/CREDITS`, `libxmp/README`, `loaders/prowizard/LICENSE.txt` |
| libmpcdec | BSD-3-Clause | `libmpcdec/COPYING`, `libmpcdec/AUTHORS`, `docs/licenses/musepack-libmpcdec-BSD3.txt` |
| libtta | LGPL (used under 2.1, see `SOURCE.md`) | `libtta/COPYING`, `libtta/README`, `docs/licenses/libtta-LGPL-COPYING.txt` |
| libhvl | BSD-3-Clause | `hvl/LICENCE`, `docs/licenses/hivelytracker-BSD3.txt` |
| libcrsid | author's "WTF" licence (credit the author) | `crsid/README.txt`, `crsid/libcRSID/README.txt`, `docs/licenses/cRSID-WTF.txt` |
| libadplug | LGPL-2.1-or-later (AdPlug, libbinio, fmopl) | file headers, `docs/licenses/adplug-LGPL-2.1.txt`, `docs/licenses/libbinio-LGPL-2.1.txt` |
| emu2413 | MIT | `LICENSE`, `docs/licenses/emu2413-MIT.txt` |
| gme | LGPL-2.1+ | `license.txt`, `license.gpl2.txt`, `readme.txt`, `README.md` |

JukeDial's top-level `LICENSE`, `COPYING.*` and `THIRD_PARTY_NOTICES.md` were not copied: this repository has its own `LICENSE` (LGPL-2.1-or-later)
and `THIRD_PARTY_NOTICES.md` at the root, and the table above is what the kept components need.

## Copies in app/main (adapted, stay where they are)

| file | from JukeDial | state |
|---|---|---|
| `backend_sdec.c/.h` | `app/main/backend_sdec.*` | copy with one change ("NANO CHANGE": end-of-file drain), no longer identical |
| `backend_gme.cpp/.h` | `app/main/backend_gme.*` | adapted |
| `fx.c/.h` | `app/main/fx_state.*` | LVGL removed |
| `card_test.c/.h` | `card_test.*` | taken from JukeDial, Nano glue in `card_svc.c` |
| `cardfmt.c/.h` | `cardfmt` (`sd_format.c`) | port |
| `cast.c/.h` | `bt_svc_cast_tx.c`, `bt_svc.c` (broadcast_on / off) | ported without UI and services |

The comments in those files still say "JukeDial" or "round"; they describe where the code came from.

## Added after the copy: HLS (2026-11-25)
`player_core/src/pc_hls.c`, `pc_ts.c` and their headers had been pruned (no caller, 0 bytes in the image). They were copied again from JukeDial's working tree
(`ESP32S31_round/components/player_core`, same files as in the public JukeDial) and added to `player_core/CMakeLists.txt`. `pc_ts.*` is unchanged;
`pc_hls.*` was extended in this repository (variant choice by CODECS / RESOLUTION / audio renditions, encryption / fMP4 flags, `#EXTINF` titles, a window that
starts at a given sequence number, 1023-character addresses, heap work space in `pc_url_resolve`); see `docs/design.md`, "HLS radio". These two files now differ from JukeDial's.

## fmvgm (added 2026-11-25, not from JukeDial)

`components/fmvgm/` is new in this repository: an FM-chip player (YM2151, YM2203; VGM and S98) around **ymfm** (BSD-3-Clause, Aaron Giles, commit `81aec25ccbb98f4873a255f7551ac4dadac59b4a`, vendored in `fmvgm/ymfm/`, three local patches in `fmvgm/patches/`, details and the bit-exactness proof in `fmvgm/SOURCE.md`). The component depends only on `player_core`'s headers (`pc_backend.h`, `pc_io.h`, `pc_vgmhdr.[ch]`) and a small porting table (`fmvgm_port_t`; the ESP-IDF side is `fmvgm_port_idf.c`), so JukeDial can copy the directory as it is; Nano's glue (`app/main/fmglue.c`, the router in `vgmgate.c`, the skip rules in `player.c`) stays here. Study and design: `docs/research/fm-chips-ymfm.md`, `docs/design.md`.
