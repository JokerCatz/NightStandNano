# libtta (vendored)

- Upstream: Aleksander Djuric / True Audio Software, `libtta-c-2.3.tar.gz` (2015-02-20), SourceForge project "tta", folder tta/libtta.
- sha256 of the tarball: `68ba6db2d494dcd4aa6832a40952b88a3d50ca1174ab0ef1e091db16992303c7`. Downloaded into its own folder
  (`.local/downloads/libtta/`, outside git), unpacked there (data only, nothing built or run from it), then `libtta.c`, `libtta.h`, `filter.h`,
  `COPYING`, `README` copied.
- Licence: GNU Lesser General Public License. **The source headers say "LGPL" with no version, and `COPYING` is the LGPL version 3 text.** Without a
  version number the recipient may choose any LGPL version, so we use it under LGPL-2.1 (the firmware is LGPL-2.1-or-later). If the owner prefers the
  stricter reading (LGPL-3 only) the firmware's combined licence would have to be LGPL-3 for this component's users: flagged in FMT-report.md.
  Text in `docs/licenses/libtta-LGPL-COPYING.txt`.
- Kept: the decoder. The encoder stays in the file behind `#ifndef TTA_NO_ENCODER` so the host tests can build TTA files in memory; the firmware
  builds with `TTA_NO_ENCODER`. Left out: the console tool, the build system, `filter_arm.S`, SSE paths (`config.h` is ours, in `port/`).
- Patches (`patches/0001-*.patch`; the vendored file carries them, CRLF line ends kept): initialised pointers become macros (no `.data`),
  the seek table is allocated from PSRAM on the target, 24-bit samples are stored (decoder) and read (encoder) byte by byte (the 32-bit access was misaligned: it traps on RISC-V),
  `tta_decoder_claim` / `tta_decoder_release` (the state is global, one decoder at a time), the encoder out of the firmware build; `libtta.h` takes `<stdbool.h>` (0002: C23 compilers make `bool` a keyword).
- The `.bss` of the library goes to PSRAM through `linker.lf`. Built with `-fwrapv`: the adaptive filter relies on wrap-around of signed sums (UBSan flags it otherwise). The backend hands libtta a 4-byte-aligned output buffer (its 16-bit samples are `int16` stores).
- Patch `patches/0003-hostile-header-rice-bounds.patch` (FIX6, 2026-11-18, wave-6 review; the vendored file carries it, CRLF kept): both `init` functions refuse a sample rate outside 8..384 kHz (rate 0 divided by zero, a tiny rate made the frame count wrap `dec_frames * 8` in a 32-bit `size_t`, `256 * rate` wrapped); the seek-table size is computed in 64 bits; the Rice parameters k0/k1 stop at 31 (the global tables were read past their end).
