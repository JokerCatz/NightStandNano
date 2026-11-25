# emu2413 (vendored)

- Upstream: https://github.com/digital-sound-antiques/emu2413 (Mitsutaka Okazaki, MIT)
- Source of these files: our fork https://github.com/JokerCatz/emu2413, branch `nodouble`
- Commit: d8ed4d2a1ce8e02ec355013b97868295462cf1df (on top of upstream develop a1d5120, v1.6.4)
- Files copied unmodified: emu2413.c, emu2413.h, emu2413_tables.h (generated const tables), LICENSE.
- The fork adds: no double arithmetic in the hot path, per-slot caches, optional EMU2413_CONST_TABLES (set by this component's CMakeLists.txt).
- Licence: MIT (LICENSE here, copy in docs/licenses/emu2413-MIT.txt). The project is LGPL-2.1-or-later; MIT is compatible, keep the copyright notice.
- Users: components/gme (Ym2413_Emu for YM2413 VGM, Nes_Vrc7_Apu for VRC7 NSF, Kss_Emu MSX-MUSIC through Ym2413_Emu), the host builds (tools/bench/chip_bench, components/player_core/host_test).
sha256 e1e906a22452a5ba101c840f6b7af6aecf612fdd2956a6733997d042cd34ae6e  emu2413.c
sha256 e1258fee355c6d9d0d06576c1404a200312cc7f4f86299c6b79902625b4e786c  emu2413.h
sha256 e43f78811a8bf717d5ae3bc030c6c3b972dca66428c7b2b46001227a32b8dfd1  emu2413_tables.h
