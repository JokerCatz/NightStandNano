# Our changes to cRSID

`vendor/cRSID/` is the library part of cRSID 1.58 (see `../README.md` for which files) with the patches in this
directory applied, committed in its patched state. The patches are the record of what changed and why, and what
gets re-applied to a newer release. The hand-made changes of patches 0002-0004 carry a comment naming them
(`grep -rn ChipChill vendor/cRSID`); 0001 only removes code, and most of 0002 is mechanical renaming.

## The rule: a patch never changes the sound

A patch may restructure cRSID, make its state readable, or leave a voice out of the mix when asked. It may not
change a single rendered sample of what cRSID plays by default. `tests/equivalence.rs` holds hashes of the first
four seconds of every SID fixture, at 44.1 and 48 kHz, rendered by the **pristine** 1.58 source (one tune per
process: pristine cRSID keeps its state in globals) before any patch was applied. The patched build reproduces
each one alone, with the per-voice tap and voice reads on, with every case playing at once on its own thread, and
with two players interleaved on one thread.

Checked by hand as well, 2026-09-28: the first 10 s of all 74 tunes in the owner's 6 HVSC files, at 44.1 and
48 kHz, hash the same from the pristine source and from the patched one (with the tap off and on):
`CRSID_LOCAL_CORPUS=<dir> cargo test --release -p libcrsid-sys --test equivalence -- --ignored --nocapture`, compared
with `diff` against the same list from a pristine build. Those files cannot be committed.

The patched library is also about 14% faster than the pristine one (300 s of a tune in 1.40 s instead of 1.63 s,
both with GCC -O2): the state is reached through one pointer instead of through absolute addresses.

## The patches

| Patch | Files | What and why |
|---|---|---|
| `0001-library-only` | `libcRSID.c/.h`, `host/*` | Removes what only the desktop player uses: the SDL and ALSA audio back ends, loading files from disk, playlists, and the song-length database lookup with its MD5 (`host/md5.c`, which is not vendored). None of it is compiled in a non-PC build except the MD5 `#include` and the database lookup, which never ran without a database. What the lookup did first, zeroing the subtune durations, stays. |
| `0002-instance` | all | **One machine per instance.** Pristine cRSID keeps everything in two globals (`cRSID`, the settings and tune info, and `cRSID_C64`, the machine) and in function-local `static` variables, so a process can play one tune at a time, and a second thread corrupts the first. Now all of it lives in one `cRSID_C64instance`, allocated zeroed by `cRSID_init()` (as the globals were) and freed by `cRSID_close()`, and every function takes it as its first parameter. Details below. |
| `0003-subtune-int` | `libcRSID.c/.h` | `cRSID_initSIDtune()` took the tune number as `char`, which is signed on x86 and unsigned on ARM: tunes 128-255 went negative on x86 (the tune number reached the 6502's A register as a negative 16-bit value, and the timing-source lookup read outside its 4-byte field). Now `int`. |
| `0004-voice-view` | `C64/C64.h`, `SID_ADSR.c`, `SID_OscWaves.c`, `C64_SIDrouting.c` | For level meters, oscilloscopes and mutes, per voice: **key-on counter**, counted in the envelope generator where it sees the gate rise; **mute**, a mask that sets the voice's envelope output to 0 before it is mixed or filtered; **voice tap**, each voice's waveform times envelope, summed over the oversampled steps and averaged per output sample exactly as the mix is. With the tap off the cost is one well-predicted branch per voice per oversampled step for the tap and one for the mute. Only the high-quality path (`cRSID_emulateHQwaves`) has them: it is the one cRSID uses by default and the only one ChipChill runs. |

### 0002 in detail

Mechanical, by a script, then checked by the compiler: every `cRSID_C64.x` became `C64->x`, every `cRSID.x`
became `C64->Interface.x`, and each function that touched either got `cRSID_C64instance* C64` as its first
parameter (its callers pass it on). With the globals gone, a function that still reached for one would not
compile; with every prototype now complete (`-Wstrict-prototypes` is clean), a call with the wrong arguments would
not compile either. (A few pristine calls already passed a `C64` argument to functions declared with `()`, which C
silently accepted.)

Then by hand, the function-local `static` variables:

- Temporaries that are assigned before every use (in `cRSID_emulateADSRs`, `cRSID_emulateHQwaves`,
  `cRSID_emulateSID_light`, `cRSID_initSIDchip`, `cRSID_initMem`, `cRSID_processSIDfileData`,
  `cRSID_generateSound/Sample`, `cRSID_emulateC64`) became ordinary locals. As statics they were data races
  between threads.
- State kept from one call to the next became fields of the instance, with the initial values the statics had:
  the dither generator (`DitherLFSR`, 0x0055A5AA), the Nyquist filters of the oversampler (`NonFiltSample1`...),
  the Sinc resampler's buffers (`ResampleBufPos`..., `NextResampleBufPos` 4096), the PSID sample player
  (`PSIDdigi*`) and the VU meter counter.
- The three SID mixers returned a pointer to a `static cRSID_Output`; they return a pointer to
  `C64->MixedOutput` instead (they fill all of it on every call, so nothing carries over).

Afterwards the library's object file has no writable global data left (`nm` lists none, only the unused
enum-typed `static`s that name constants and are never read or written).

The shim (`../shim/ccs_player.c`) creates a fresh instance for every `ccs_start()`: a few counters (the VIC's
raster position, the PSID TOD clock) are only set when a machine is created, so a tune restarted on the same
instance would not repeat its first run exactly, and seeking relies on that.

## The SID as an instrument

`ccs_sid_*` in `../shim/ccs_player.h` plays one SID from MIDI (chipchill-core's `instrument`): a cRSID machine
whose CPU, CIAs and VIC never run; each sample the envelope generators are clocked for the sample's cycles and the
oscillators through cRSID's oversampling averager, the calls `cRSID_emulateC64()` makes, and each voice's tap
(0004) is its output. The envelope is opened and closed directly (a note sounds from its first sample; the caller
shapes it), and the filter, which works on the mix, is not in the path. Nothing in `vendor/` changed for it.
`tests/instrument.rs` checks each voice's pitch and that it starts on its first sample, and makes, plays and
deletes instances at random (run it under AddressSanitizer too, as below: clean on 2026-09-29).

## AddressSanitizer

Rust's own sanitizer needs nightly, so the C alone is instrumented: `LIBCRSID_SYS_ASAN=1` makes `build.rs` build
it with `-fsanitize=address -fno-omit-frame-pointer`, and the test binaries link the runtime:

```sh
LIBCRSID_SYS_ASAN=1 RUSTFLAGS="-C link-args=-Wl,--push-state,--no-as-needed,-lasan,--pop-state" \
ASAN_OPTIONS=verify_asan_link_order=0 CARGO_TARGET_DIR=target/asan \
CARGO_TARGET_X86_64_UNKNOWN_LINUX_GNU_RUNNER="env LD_PRELOAD=$(gcc -print-file-name=libasan.so)" \
cargo test --target x86_64-unknown-linux-gnu -p libcrsid-sys -p chipchill-core --test equivalence --test decode_sid
```

The runner loads the AddressSanitizer runtime ahead of everything else: linked by `-lasan` alone it can come
after libc (or libstdc++ and libm), whose `memcpy`, `memset` and the like are then the ones called, and a read
past the end of a heap block goes unseen (`crates/libgme-sys/patches/README.md`, "AddressSanitizer").

The runs recorded below were made without the runner. With it, 2026-09-29: this crate's tests and all of
chipchill-core's, clean; and the mutation run of `chipchill-core/tests/damage.rs`
(`docs/design/damaged-files.md`) over the seven SID fixtures, 39,555 damaged copies (35,575 opened and played),
and 4,000 noise files behind the PSID and RSID signatures: clean.

(`--target` keeps the flags off build scripts.) Clean on 2026-09-28, and over the owner's 74 tunes with
`local_corpus_hashes` (add `CRSID_LOCAL_CORPUS=<dir>` and `-- --ignored`). Also clean: 3000 randomly mutated copies
of the fixtures and those tunes (header and data bytes changed, files cut short) opened, started, rendered and read
through the shim under AddressSanitizer and UndefinedBehaviorSanitizer together.

## Re-creating the patched tree

From the release archive (see `../README.md` for the URL and the sha256):

```sh
cd crates/libcrsid-sys
curl -fsSL -o crsid.tar.gz https://csdb.dk/getinternalfile.php/279277/cRSID-1.58-with-source-code.tar.gz
echo "98bc2da927b65ab8781b46dc5341b1113967b0b61aa05406877c0e5dcffa8b9d  crsid.tar.gz" | sha256sum -c
tar xzf crsid.tar.gz
rm -rf vendor/cRSID && mkdir -p vendor/cRSID/libcRSID/C64 vendor/cRSID/libcRSID/host
cp cRSID-1.58/README.txt vendor/cRSID/
(cd cRSID-1.58/libcRSID && cp README.txt libcRSID.c libcRSID.h Config.h Optimize.h ../../vendor/cRSID/libcRSID/ \
  && cp C64/*.c C64/*.h ../../vendor/cRSID/libcRSID/C64/ \
  && cp host/file.c host/audio.c host/host.h ../../vendor/cRSID/libcRSID/host/)
for p in patches/0*.patch; do patch -d vendor/cRSID -p1 --no-backup-if-mismatch < "$p"; done
rm -rf cRSID-1.58 crsid.tar.gz
git status vendor   # nothing: the committed tree is exactly this
```

The patches are `git diff` output against the previous step (each assumes the ones before it), in file-name
order. To move to a newer release, derive the equivalence hashes from that release's pristine source first (a
small C program that calls `cRSID_init`, `cRSID_processSIDfileData`, `cRSID_initSIDtune` and
`cRSID_generateSound`, one tune per process, with `cRSID.Stereo = CRSID_CHANNELMODE_NARROW`), then patch.

## Licence

cRSID's terms (see `../README.md`) ask for credit, nothing else: publishing these changes is not required.
