# libxmp patches (record of what differs from the pristine 4.7.2 tarball)
`patches/NNNN-*.patch` are `diff -u` of each touched file (pristine tarball `src/` vs this tree); the tree under `libxmp/` is already patched.
Also differing from the tarball without a patch file: files left out (see SOURCE.md), `src/tempfile.c` + `src/mkstemp.c` removed (0001), `src/loaders/vorbis.*` removed (0006).
| patch | what | why |
|---|---|---|
| 0001 | only the in-memory Amiga depackers (PP20, XPK-SQSH, ARC, ArcFS, MMCMP, Pack-Ice, LZX, S404) run, also for `xmp_load_module_from_callbacks` | the card reader is a callback source; no temp files, no helper programs on the device |
| 0002 | ProWizard output goes to a memory buffer (cap 5 MiB) | no temp file; a damaged packer cannot grow it without bound |
| 0003 | `LIBXMP_NO_PAULA` (not defined since AMIGA / 0008: the Paula mixer is built, off until the cflag A500 is set) | project decision: no Amiga Paula mixer; today's MOD / XM / S3M / IT sound unchanged |
| 0004 | MFP refuses without a file name; no environment lookup | callback sources have no folder |
| 0005 | `LIBXMP_LITE_COMPAT` (OFF since 2026-11-18: not in `XMP_DEFS`; the code stays so it can be switched back) | when defined, keeps MOD / IT output bit-identical to libxmp-lite (the full loaders otherwise set per-tracker IT flow modes and the PT full-repeat flag, and the player adds the invert-loop effect) |
| 0006 | no Ogg Vorbis samples in XM | stb_vorbis: 26 KB flash, 1 KB internal RAM, a decoder that trusts its input |
| 0007 | PowerPacker bit buffer shifted as unsigned | UBSan finding of the fuzz (host and target behave the same; made defined) |
| 0008 | Paula tones: `XMP_FLAGS_LEDON` (A500 LED filter always on), `XMP_FLAGS_A1200` (own band-limit table `precomp_blep_a1200.h`, tools/gen/amiga_blep.py; mixer set `libxmp_a1200_mixers`); `output_sample` takes the table | the Amiga tone setting; libxmp 4.7.2 ships only the two A500 tables |
| 0009 | `rng.c`: fixed seed instead of `time(NULL)`; `filter.c`: the 256-entry cutoff frequency table instead of `powf` (here a soft-double `pow`) per filter setup | CHIPFIX: a render of a random-LFO module was different on every run, and the board and host differed; the filter setup cost a double pow per tick and channel |
Fuzz findings after 0007: none open (see XMPFULL-report.md for the runs).
