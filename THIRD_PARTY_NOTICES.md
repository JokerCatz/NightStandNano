# Third-party notices

NightStand Nano's own code is **LGPL-2.1-or-later** (`LICENSE`; every own source file starts with an `SPDX-License-Identifier` line).
This file lists everything else that is in the repository, is fetched when the firmware is built, or ends up in a release file, with its licence
and where the licence text is. It states what the files say; it is not legal advice. The origin and pruning history of `components/` is in
`components/ORIGIN.md`, each component's own `SOURCE.md` has the details of its upstream.

## 1. Firmware components in `components/` (copied into the repository)

<!-- release-notes:begin -->
| component | what | licence |
|---|---|---|
| player_core | player framework, FX chain, container and stream code (own code, from JukeDial) | LGPL-2.1-or-later |
| player_core/third_party/dr_flac | FLAC decoder | public domain or MIT-0 |
| player_core/third_party/minimp3 | MP3 decoder | CC0 1.0 |
| player_core/third_party/miniz | zip / inflate | MIT (the header also says public domain) |
| player_core/third_party/tomlc99 | TOML parser | MIT |
| player_core/third_party/zepto8 | no code, a reference for the PICO-8 synth port (`SOURCE.md` only) | WTFPL v2 |
| libldac | Sony LDAC encoder (AOSP `external/libldac`) | Apache-2.0; its `NOTICE` mentions Sony's certification |
| libxmp | tracker module player | MIT / Expat |
| libmpcdec | Musepack decoder | BSD-3-Clause |
| libtta | TTA decoder | "LGPL" in the headers, LGPL v3 text in `COPYING` (see below) |
| libhvl | HivelyTracker replayer | BSD-3-Clause |
| libcrsid | cRSID SID player | author's "WTF" statement (do what you want, mention the author) |
| libadplug | AdPlug and libbinio (OPL players) | LGPL-2.1-or-later |
| emu2413 | YM2413 emulator (fork) | MIT |
| gme | Game_Music_Emu 0.6.5 with patches | LGPL-2.1-or-later |
| fmvgm | FM-chip player (own code, LGPL-2.1-or-later) around ymfm (`ymfm/`, commit 81aec25, three local patches in `patches/`; `ymfm_misc` = the AY8910 / YM2149 class), and two MAME-derived cores from libvgm (commit c5c03a2: `pcm/emu/cores/sn76496.c`, `okim6295.c`, `okiadpcm.[ch]`, unchanged, headers say `license:BSD-3-Clause`) behind own adapters and a compatibility layer (`pcm/`) | ymfm: BSD-3-Clause; MAME cores: BSD-3-Clause |
<!-- release-notes:end -->

Where the texts are:

| component | licence text in the repository |
|---|---|
| player_core (own code) | `LICENSE`, per-file SPDX headers |
| dr_flac | end of `components/player_core/third_party/dr_flac/dr_flac.h` |
| minimp3 | `components/player_core/third_party/minimp3/LICENSE` |
| miniz | `components/player_core/third_party/miniz/LICENSE` |
| tomlc99 | `components/player_core/third_party/tomlc99/LICENSE` |
| zepto8 | `docs/licenses/zepto8-WTFPL.txt` |
| libldac | `components/libldac/LICENSE`, `components/libldac/NOTICE` |
| libxmp | `components/libxmp/libxmp/docs/COPYING`, `docs/CREDITS`, `src/loaders/prowizard/LICENSE.txt` |
| libmpcdec | `components/libmpcdec/libmpcdec/COPYING`, `docs/licenses/musepack-libmpcdec-BSD3.txt` |
| libtta | `components/libtta/libtta/COPYING`, `docs/licenses/libtta-LGPL-COPYING.txt` |
| libhvl | `components/libhvl/hvl/LICENCE`, `docs/licenses/hivelytracker-BSD3.txt` |
| libcrsid | `docs/licenses/cRSID-WTF.txt`, the READMEs under `components/libcrsid/crsid/` |
| libadplug | `docs/licenses/adplug-LGPL-2.1.txt`, `docs/licenses/libbinio-LGPL-2.1.txt`, file headers |
| emu2413 | `components/emu2413/LICENSE`, `docs/licenses/emu2413-MIT.txt` |
| gme | `components/gme/license.txt` (LGPL-2.1), `components/gme/license.gpl2.txt`, `readme.txt` |
| fmvgm (own code) / ymfm / MAME cores | `LICENSE`, per-file SPDX headers / `docs/licenses/ymfm-BSD3.txt`, `components/fmvgm/ymfm/LICENSE` / `docs/licenses/mame-sound-cores-BSD3.txt` |

Points that the component notes themselves raise:

- **libldac.** `components/libldac/NOTICE` reads, in full: "Certification. Taking the certification process is required to use LDAC in your
  products. For the detail of certification process, see the following URL: https://www.sony.net/Products/LDAC/aosp/". `LICENSE` is the Apache License 2.0.
  The firmware carries the LDAC encoder; whether and how the certification applies to a given use is the user's question, not answered here.
- **libtta.** The source headers say "LGPL" without a version and `COPYING` is the LGPL version 3 text (`components/libtta/SOURCE.md`).
- **libcrsid.** The licence is the author's informal statement, quoted in `docs/licenses/cRSID-WTF.txt`.
- **libadplug.** Players whose files are GPL-only or have no licence (`jbm.cpp`, `adl.cpp`, `depack.c`) were left out (`components/libadplug/SOURCE.md`).

## 2. Fetched when the firmware is built (not in the repository)

`app/main/idf_component.yml` asks the ESP-IDF component manager for these; `app/dependencies.lock` pins the exact versions and hashes.

| component | version (lock) | licence | text |
|---|---|---|---|
| `espressif/usb` | 1.6.0 | Apache-2.0 | `LICENSE` inside the component (`app/managed_components/espressif__usb/`, after a build) |
| `espressif/led_strip` | 3.1.1 | Apache-2.0 | `app/managed_components/espressif__led_strip/LICENSE` |
| `espressif/esp_audio_codec` | 2.6.2 | "Espressif Modified MIT" (`LicenseRef-Espressif-Modified-MIT`) | `app/managed_components/espressif__esp_audio_codec/LICENSE` |

The `esp_audio_codec` licence text says: "Permission is hereby granted for use EXCLUSIVELY with Espressif Systems products", requires that the
software "MUST BE USED IN CONJUNCTION WITH ESPRESSIF SYSTEMS PRODUCTS", and that "Redistribution of the Software in source or binary form FOR USE WITH
NON-ESPRESSIF PRODUCTS is strictly prohibited". It provides the SBC and AAC codecs used for Bluetooth audio. Its source is not in this repository
(the build downloads it); the release `.bin` files contain its compiled code and run on an ESP32-S31 only.

## 3. ESP-IDF and the patched copies generated from it

- **ESP-IDF v6.1** (preview, target esp32s31), Apache-2.0, https://github.com/espressif/esp-idf. It is a build prerequisite, not part of the repository.
  Its precompiled radio and Bluetooth controller libraries carry Espressif's own terms (see the ESP-IDF licence notes).
- `tools/bt_ldac/` holds `ldac.patch` and `make_bt.sh`, which write a patched copy of IDF's Bluedroid (`bt` component, Apache-2.0) to `.local/bt_ldac`
  at configure time. The patch is distributed under the same terms as the code it changes (Apache-2.0).
- `tools/fatfs_exfat/` holds `make_fatfs.sh`, which writes a copy of IDF's `fatfs` component with exFAT switched on (four one-line edits) to `.local/fatfs_exfat`.
  FatFs is by ChaN, with its own licence: the BSD-style text at the top of `ff.c` ("Redistributions of source code must retain the above copyright notice,
  this condition and the following disclaimer"; binary redistribution has no condition). It is not copied into this repository either; the firmware
  binary contains the compiled FatFs.

## 4. Web pages

| piece | where | licence |
|---|---|---|
| control page (own code) | `web/src/` | LGPL-2.1-or-later |
| visualizers and radio search from tunetap (same author) | `web/src/vendor/tunetap/` (`README.md` explains the copy) | LGPL-2.1-or-later |
| flasher page (own code, derived from JukeDial's flasher) | `flash/src/` | LGPL-2.1-or-later |
| esptool-js 0.7.1 (Espressif), with pako (MIT / Zlib notices) | `flash/src/vendor/esptool-js/0.7.1/` (`LICENSE`, `NOTICE`, `VENDOR.md`) | Apache-2.0 |
| esbuild 0.28.2 | build tool only (`devDependency`); its output is the page, nothing of it is shipped | MIT |

The built pages (`web/dist/`, `flash/dist/`) are committed and served by GitHub Pages; they contain the code above.

## 5. Data

No music, scene library or sample is in the repository or in a release. `tools/make_scenes.py` reads the owner's own sleepmixer scene library
(a file that is **not** in the repository) and writes scene files for a TF card; running it needs that library. Radio stations come from the public
radio-browser service at run time, in the user's browser; the repository ships no station list.
