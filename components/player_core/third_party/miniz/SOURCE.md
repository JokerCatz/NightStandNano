# miniz (vendored)

- Upstream: https://github.com/richgel999/miniz, tag 3.0.2 (README.md here).
- Version inside the files: `miniz.h` line 1 says "miniz.c 3.0.0"; `MZ_VERSION "11.0.2"` (line 275). Verified 2026-11-19 against the upstream tag 3.0.2 tarball: `miniz.h`, `miniz_common.h`, `miniz_tdef.h`, `miniz_tinfl.c`, `miniz_tinfl.h`, `miniz_zip.h` and `LICENSE` are byte-identical; upstream's own header still says "miniz.c 3.0.0" at that tag, so the two numbers do not contradict each other.
- Licence as stated: `miniz.h` says public domain ("unlicense" at the end of the file); the LICENSE file here is MIT (RAD Game Tools / Valve / Rich Geldreich).
- Taken into the repo: 2026-11-08, commit 6f9aa506.
- Local changes: none recorded (README.md says unmodified). `miniz_export.h` is a one-line stand-in for the file miniz's CMake generates (ours). Only `miniz_tinfl.c` is compiled, through `src/pc_inflate.c`, which renames every `tinfl_*` symbol to `pc_tinfl_*` (README.md).
