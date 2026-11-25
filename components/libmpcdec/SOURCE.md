# libmpcdec (vendored)

- Upstream: Musepack Development Team, `musepack_src_r475.tar.gz` (SVN r475, 2011-08-10), https://files.musepack.net/source/musepack_src_r475.tar.gz
- sha256 of the tarball: `a4b1742f997f83e1056142d556a8c20845ba764b70365ff9ccf2e3f81c427b2b`. It was downloaded into its own folder
  (`.local/downloads/musepack_r475/`, outside git), unpacked there (data only, nothing built or run from it) and the files below copied.
- Licence: BSD-3-Clause (`libmpcdec/COPYING`, also `docs/licenses/musepack-libmpcdec-BSD3.txt`). Compatible with the LGPL-2.1-or-later firmware.
- Kept: `libmpcdec/` (huffman, mpc_bits_reader, mpc_decoder, mpc_demux, requant, streaminfo, synth_filter and their headers), `include/mpc/`,
  `common/crc32.c`. Left out: `mpc_reader.c` (the stdio reader; our backend supplies a reader), the encoder, the psychoacoustic model, the command-line tools.
- Patches (`patches/`, the vendored files carry them): 0001 Huffman tables: the initialised globals (5.3 KB of internal RAM) become const templates
  copied into one malloc'd PSRAM block by `huff_init_lut`; 0002 `common/crc32.c` without a static table. `port/` routes every malloc to PSRAM on the target.
- Float decoder (`MPC_FIXED_POINT` off); the output is float in [-1, 1) scaled by 32768 in libmpcdec's convention (see pc_backend_mpc.c).
- Patch `patches/0003-hostile-seek-table-requant-bounds.patch` (FIX6, 2026-11-18, wave-6 review): `mpc_demux_ST` parses the SV8 seek table from a padded private copy of its block (size and Golomb zero-run bounded; a table that does not fit its block is dropped and the generic one is built), `seek_table_cap` bounds the appends while decoding (SV8 with 0 samples), `mpc_demux_seek_sample` stops at the end block / a bad key (it spun forever past the data), SV7 band resolutions outside -1..17 become 0 (`Cc[]` and `Dc[]` have 19 entries).
