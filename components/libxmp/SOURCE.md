# libxmp 4.7.2 (vendored, full library)

- Upstream: https://github.com/libxmp/libxmp, release `libxmp-4.7.2` (`XMP_VERSION "4.7.2"`, upstream Changelog "4.7.2 (20260726)").
- Same version as the libxmp-lite copy this replaces (components/libxmp_lite, removed 2026-11-18; the lite tag commit was
  `a13276d27feabcf9ee4f982913f718ee05a65cb7`; the sources of every file both trees have are byte-identical). Why 4.7.2 and not a newer release: it is the
  version the player already ran, so the swap changes the loader set and nothing else (a newer release was not looked for).
- Fetched as the release tarball into its own folder `.local/downloads/libxmp-4.7.2/` (never executed, only read and compiled),
  `libxmp-4.7.2.tar.gz` (already fetched by CHIP2B on 2026-11-18, re-checked here, not downloaded again), sha256 `510a96eefd79e4558fb1fa41fb5494870328776b3f77563f94f61f241f64bde1` (CHIP2B 2026-11-18, same file).
- Licence: MIT/Expat, `libxmp/docs/COPYING` (also at the end of `libxmp/README`); `libxmp/docs/CREDITS` lists the third-party parts we build
  (PowerPacker depacker: public domain; Stonecracker S404: public domain; MD5: public domain; miniz tinfl: MIT; ProWizard: MIT,
  `loaders/prowizard/LICENSE.txt`). Compatible with the project's LGPL-2.1-or-later.
- Our tree = the tarball's `include/` and `src/` minus what is not built: `src/lite/`, the zip / lha (lhasa) / gzip / bzip2 / xz / compress /
  xfd / popen depackers (generic archives, not Amiga music; they need temp files or helper programs), `stb_vorbis` (`loaders/vorbis.*`, patch 0007),
  `tempfile.c` + `mkstemp.c` (patch 0001), the Makefiles, tests, examples and the docs except COPYING / CREDITS.
- Built (`xmp_sources.cmake`, the one source list shared with the host tests, the bench and the fuzz harness): the core player, ALL module
  loaders of `format_loaders[]` (MOD XM S3M IT STM 669 FAR MTM PTM OKT ULT MDL STX PT3 SFX FLT ST EMOD IMF DIGI FNK ICE LIQ IMS PSM AMF STIM
  OctaMED MED2/3/4 MMD1/3 RTM DT NO ARCH SYM DBM UMX GDM GAL4/5 MFP ASYLUM MUSE HMN MGT CHIP ABK COCO XMF), ProWizard (~40 Amiga packers) and the in-memory depackers PowerPacker,
  XPK-SQSH, MMCMP, Stonecracker S404, Pack-Ice (ICE!/Ice!), ARC, ArcFS, LZX.
- NOT built: the Amiga Paula (BLEP) mixer (`LIBXMP_NO_PAULA`, patch 0003, `mix_paula.c` left out): `xmp_start_player(..., flags=0)` never selected it, so
  the sound of MOD / XM / S3M / IT is unchanged.
- Our additions: `CMakeLists.txt`, `xmp_sources.cmake`, `port/` (PSRAM allocator, force-included; libxmp itself is changed only by the patches).

## Patches (`patches/NNNN-*.patch`, `patch -p1` inside `libxmp/`, applied on the pristine tarball tree; `patches/README.md` has the findings behind each)
The tree under `libxmp/` is already patched; the patch files are the record and apply to the pristine tarball (minus the pruned files).

Known upstream quirks (worked around in `components/player_core/src/pc_backend_xmp.c`, not patched here):
- `xmp_load_module_from_callbacks` fails when `priv` is NULL.
- `xmp_channel_mute`: the docs say 0 mutes; the code mutes on 1 (not used here).

Upstream's CONTRIBUTING.md asks for no AI-generated contributions to libxmp; nothing here goes upstream.
