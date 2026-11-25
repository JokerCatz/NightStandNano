# zepto8 (reference for pc_backend_p8.c)

- Upstream: Sam Hocevar, https://github.com/samhocevar/zepto8 at commit b2929a4fe31e5765ef66d7794edd11086b9638b4 (fetched 2026-11-17 into `.local/downloads/zepto8/`, outside git; read only, never built or run).
- Files read: `src/synth.cpp` (ee304e72397554250ebfed0136f1c711855363756982d69d1c593c04e0da6831), `src/synth.h` (9eeb994c8e59cf75053a0baeb3171b4658501e62fe71fb8ada71742ed9ebe399), `src/pico8/sfx.cpp` (01183c7279d285e2d48b366ee2753d3a0e9b67282d6c52f5267f878f3aecb752).
- Licence: WTFPL v2 (stated in each file header). Text and notice: `docs/licenses/zepto8-WTFPL.txt`. fake-08 was not used (its licence is incomplete); `compress.cpp` was not used (the cart's code is never read).
- Nothing is vendored: `components/player_core/src/pc_backend_p8.c` is our C code, ported from the C++ synth and sequencer. What changed is listed in the file's header comment (fixed-point phases, note state every 8 samples, no dampen filter, no hardware registers).
