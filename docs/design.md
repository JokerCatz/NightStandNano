# NightStand Nano — design notes

Headless, single-board descendant of `~/self_remote/nightstand` (CM4 bedside speaker) on an ESP32-S31 dev board.
Port of the headless parts of `../ESP32S31_round` (JukeDial); owner: JC (zh-TW).

## Intent
Bedside audio box. Only peripherals: TF card, a USB-C DAC dongle ("小尾巴") on the OTG port, one plain LED.
Output: A2DP (SBC/LDAC) to headphones/speaker, or the USB DAC. Control: Bluetooth only (BLE GATT) from a
GitHub-Pages Web Bluetooth page (also shows the spectrum) and a native app. Wi-Fi: radio + time (+ maybe OTA).
Wi-Fi is never a control channel (https page cannot call LAN http).

## Decisions (owner, 2026-11-24)
- Name NightStandNano; directory `ESP32S31_single`; control = Bluetooth only; Wi-Fi yes.
- Wi-Fi credentials stay on the TF card (as in round); never read, printed or committed.
- User-adjustable TF pins; the on-board red LED (plain, not RGB) may be used as status if it is a GPIO.
- Test board: the peer devkit (its UART bridge appears under `/dev/serial/by-id`; the helper scripts take the port from `NSN_PORT`, see `tools/nsn_port.py`), not backed up, free to flash.
  Always flash with an explicit port; never touch the round board.

## Facts found
- S31: SOC_USB_OTG_SUPPORTED=1, one UTMI PHY. IDF 6.1 host examples do not list S31 -> USB host UAC is the first thing to prove.
- Reusable from round: wifi_svc, radio_svc, bt_svc*, sleep_timer, noise_engine/color_noise, player core/decoders.
- Not in round: USB host UAC (ext_uac.c is device side, never on a board), generic BLE GATT control + spectrum.

## Plan
1. Spike (throwaway): TF mount with configurable pins, USB host enumerates the DAC, LED.
2. Then the real design: approaches, GATT protocol, repo layout (share components with round vs copy).

## Spike results (2026-11-24, peer devkit)
- TF: SDMMC 4-bit with round's default pins (CLK24 CMD25 D0-3 20-23) mounts a 16 GB card (JukeDial layout). Pins are Kconfig `NSN_SD_*` for now; the user-facing way (NVS/console/BLE) is still to design.
- USB host on S31 works with IDF 6.1 + `espressif/usb` ^1.0.0: enumerates the DAC at high speed. Needs `CONFIG_USB_HOST_CONTROL_TRANSFER_MAX_SIZE=2048` (UAC config descriptors are ~422 B; the default 256 fails enumeration).
- DAC: Synaptics CX31993 "hifi audio" 06cb:1594, UAC2 (protocol 0x20), HS, 1 ISO OUT endpoint, 3 alt settings (192/288/384 B per microframe), ISO IN too (mic). IDF has no UAC host class driver: we write our own (class-specific AS descriptors, clock source, set-rate, ISO OUT streaming).
- LED: pin unknown. Red LED may be a fixed power LED. Need the devkit name/silkscreen.

## exFAT + storage (2026-11-24)
- exFAT: patched FatFs copy via `tools/fatfs_exfat` (docs/exfat.md). `exfat_selftest.c` formats an 8 MiB PSRAM RAM disk as exFAT, mounts it through the VFS and round-trips a file with a long UTF-8 name: PASS on the peer board. Not yet tried on a real exFAT card (the 16GB test card is FAT32).
- TF pins live in NVS (`nsn`/`sdpins`), Kconfig `NSN_SD_*` are defaults; serial console `sdpins`, `sdpins clk cmd d0 d1 d2 d3 width`, `sdpins reset`. Whether S31's SDMMC accepts arbitrary GPIOs (matrix) or only IOMUX pins is unverified.
- Helper: `tools/board.sh flash|log` (port always explicit).

## USB DAC playback works (2026-11-24, `app/main/uac_host.c`)
- Own UAC2 host: walks the config descriptor (clock id from the USB-streaming input terminal, best playback alt = 2ch/16-bit/2-byte slot,
  falls back to any OUT alt; 24/32-bit slots are converted from S16), GET_RANGE on the clock, SET_CUR sample rate, claims the AS interface, ISO OUT.
- 6 transfers x 16 microframes (2 ms each, ~12 ms in flight), pull callback `nsn_pcm_pull_t` (stereo S16) runs in the USB client task.
- CX31993 supports 8/16/32/44.1/48/96/192/384 kHz. 1 kHz tone at 48 kHz ran 22 s: 9999 transfers, 0 bad.
- NOT verified by ear (no way to listen from here) - owner to confirm the tone is audible and clean.
- Left to do: hot-unplug / replug test, rates other than 48k (44.1k uses the 5/6-frame accumulator), 24-bit sources, hardware volume (feature unit), mic alt ignored.

## Sharing code with round (decision 2026-11-24, reversible)
`app/CMakeLists.txt` adds JukeDial's `player_core`, `libxmp`, `libmpcdec`, `libtta`, `libhvl`, `libcrsid`, `libadplug`, `emu2413`, `gme` straight from
`../ESP32S31_round/components` (single source of truth, no 29k-line copy). Cost: this repo is not self-contained; to publish it, vendor those
components (or round's export whitelist). libadplug needs `esp_timer` linked in by our CMake.

## File -> DAC path works (2026-11-24, `player.c`)
decode task (core 1, `pc_backend_t` float stereo) -> S16 ring (96 KB internal) -> UAC pull. Backends wired: mp3 flac wav aiff mpc tta ahx/hvl sid.
Console: `ls [dir]`, `play PATH`, `stop`, `status`, `mkwav PATH` (5 s 44.1 kHz test file), `rm PATH`, `sdpins ...`, `reboot`.
Test: generated WAV 44.1 kHz played to the end through the DAC; 7 underruns at the start (to investigate; mp3/flac not yet tried: the test card has no music).
`/sdcard/nsn_test.wav` was left on the owner's test card.
Gotcha: stdin on UART needs `uart_driver_install` + `uart_vfs_dev_use_driver`, otherwise `fgets` returns EOF immediately.
Gotcha: the CH343 bridge does not reset the board on port open; `tools/board.sh log` pulses RTS itself (set NSN_NORESET=1 to skip).

## Bluetooth A2DP: not working yet against the PC (2026-11-24)
`bt_audio.c` (Bluedroid dual mode, A2DP source SBC, inquiry, SSP auto-accept, `bt scan|list|conn|tone|unpair`). On the dev PC (BlueZ, no WirePlumber
bluez monitor, so no media endpoints) SSP pairing works but AVDTP fails: we send AVDT_DISCOVER, the peer answers, we close L2CAP,
`BTA_AV_OPEN_EVT FAILED status 3` (FAIL_STREAM). Same with a temporary SBC sink endpoint (`tools/a2dp_sink_probe.py`) registered on BlueZ.
Not proven to be our bug (the PC may simply not offer a usable SEP) and not proven fine. Needs a real headphone / speaker. JukeDial's A2DP works with
a Sony WH-1000XM5 and uses the 6.1 multi-endpoint API + external codec (`esp_a2d_source_register_stream_endpoint`); that is the fallback if the legacy path is the problem.
Debug overlay: `app/sdkconfig.btdbg` (own build dir `build-dbg`, `SDKCONFIG=sdkconfig.dbg`); AVDT traces do not print even at VERBOSE.
PC state was restored (not discoverable; the owner's JukeDial pairing untouched).

## BLE control + spectrum work (2026-11-24, `bt_ctl.c`, `cmd.c`, `spectrum.c`)
- One text command interpreter (`cmd.c`) serves the UART console and the BLE CMD characteristic: `st` (JSON status), `play PATH`, `stop`, `pause`, `resume`, `vol N`, `ls [DIR]`, `spec`, `bt ...`, `sdpins ...`, `mkwav`, `rm`, `reboot`.
- GATT service 7f2a0001-a1c4-4e53-9a31-6e73616e6f00; CMD write (…0002), EVT notify (…0003, reply text split to the MTU, '\n' per line), SPEC notify/read (…0004, `[seq, 32, 32 bands 0..255]`).
- Spectrum: pull callbacks push the played samples (so it follows the audio, not the decoder) into a 1024 mono window; a core-1 task does a Hann FFT every 33 ms, 32 log bands 40 Hz..16 kHz, -70..0 dBFS -> 0..255, fast attack / slow release.
- Legacy (4.2) advertising: S31 defaults to BLE 5.0 extended-adv API only; `CONFIG_BT_BLE_42_FEATURES_SUPPORTED=y`.
- Tested from the PC with `tools/ble_probe.py` (bleak in `.local/venv`): found in 0.2 s, commands + replies fine, 33.4 spectrum frames/s, 34-byte frames (MTU negotiated by BlueZ).
- SECURITY OPEN: `CONFIG_NSN_BLE_OPEN=y` (default for bring-up) = no encryption, anyone in range can control the box. Decide before release: bonding with a pairing window (boot button / first N minutes / console), allow-list of bonded addresses.
- Internal RAM: 231 KB free idle, 53 KB while playing (96 KB ring + 24 KB decode stack are internal). Wi-Fi will need more: move the ring to PSRAM.

## Status 2026-11-24 night (owner away, autonomous run)
Done and tested on the peer devkit: TF (FAT + exFAT RAM-disk proof, NVS pins), USB DAC playback (UAC2 host), file/folder playback with repeat,
next/prev, sleep timer with 30 s fade, BLE control + spectrum (driven from the PC), Wi-Fi from the card + SNTP (connected, time correct),
2.5 min soak: internal RAM flat at ~100 KB free while playing with Wi-Fi + BT + BLE up, no underruns.
Not done / not verified: A2DP against real headphones, internet radio, white noise, LED (pin unknown; owner: not required), real exFAT card,
web page in a real browser (Web Bluetooth chooser cannot be automated), UAC hot-unplug, BLE security (OPEN flag), resampler for 22.05/88.2 kHz sources.
Mistakes made on the way (for the next person): console stdin needed the UART driver; port open does not reset the board (RTS pulse does);
a byte ring returns short at its wrap point; `sleep` in board.sh collides with a device command of the same name (device command is `timer`).
Next steps in order: (1) A2DP with a real headphone and, if the legacy API is the problem, port JukeDial's multi-endpoint a2dp source;
(2) BLE bonding + pairing window; (3) internet radio (`pc_backend_stream` + esp_http_client); (4) white noise from `noise_engine`; (5) settings persistence
(volume, last output, last folder) and auto-resume at boot; (6) iOS: native app or Bluefy.

## 2026-11-24 later (owner away again, autonomous)
- A2DP proven against JukeDial in speaker mode (round `btmode 1`, pairing answered with round's `pair yes`; round was restored to `btmode 0`; it now has a bond for "NightStand Nano").
  The earlier PC failure was the PC (no media endpoints), not our source. SBC only; `short=11` pulls at stream start are harmless.
- Output routing (`out auto|dac|bt`), resampling with JukeDial's `pc_rsq` (22.05->48 kHz on the DAC and 48->44.1 kHz on A2DP both played through), reconnect to the last A2DP device.
- RGB LED on GPIO60 via `espressif/led_strip` (WS2812 assumed; the owner saw the blue idle breath, so it works; R/G/B order unchecked).
- Settings in NVS (`settings.c`), alarm (`alarm.c`), noise sources, radio (`radio.c`: esp_http_client + pc_stream + pc_backend_mp3, MP3 only, TLS buffers in PSRAM).
- From the owner's earlier ESP32-S3 project (`~/self/RP2350_player`, JukeChip): decisions 0019 (BLE for control: Web Bluetooth page on a static host, pairing offered not required,
  open control tier, credentials only via a one-time-code provisioning window), 0027 (storage state must be visible and re-mountable), 0006 (Wi-Fi gated by wifi.txt),
  alarm.c (crossing logic), and the noise generators (already in round as pc_noisegen). Adopted: 0019's security posture, 0027 (LED + quiet remount), alarm, noise.
  Not adopted: provisioning window (needs a panel or another way to show the one-time code; with no screen it could blink on the LED).
  Also in there: `web/index.html` + `web/provision.js` (their Web Bluetooth page: a UI style the owner may prefer), `docs/reference/ble-gatt.md`.
- Soak (2.5 min folder playback with Wi-Fi + BT + BLE) and radio (SomaFM, 45 s) ran clean. Radio internal RAM: ~70 KB free while streaming.

## 2026-11-24 later still: A2DP rewrite, roles, LDAC, chip music, memory
- `bt_audio.c` rewritten on external codecs (esp_audio_codec): SBC/AAC in both directions + LDAC out. Bluedroid allows one A2DP role at a time (`esp_a2d_sink_init` after `source_init` = ESP_ERR_INVALID_STATE, measured),
  so there are two modes: speaker (sink: phone -> Nano -> DAC via the source "bt:in") and headphones (source). `bt mode speaker|headphones`; `out bt` implies headphones mode. Controller, Bluedroid and BLE stay up in both.
- Speaker mode: decode task (core 1) -> PCM ring in PSRAM -> `bt:in` backend (100 ms cushion, drift trimmed by dropping beyond 250 ms) -> same pipeline as any track. Round as the "phone" (AAC): 92 s, 0 underruns.
- LDAC: uses JukeDial's patched Bluedroid (`../ESP32S31_round/.local/bt_ldac`, build it with round's `tools/bt_ldac/make_bt.sh`) and libldac; the build fails with a clear message if it is missing.
  Offline `bt ldactest` encodes 1 s of tone per quality. NOT tested against a real LDAC sink.
- Chip music: GME wrapper copied from round (`backend_gme.cpp`), whole file in PSRAM (<= 8 MB, .vgz gunzipped), trackers/OPL through `pc_backend_chain`; multi-tune files advance tune by tune, `next`/`prev` move inside the file first.
- Internal RAM: tasks that never write flash have PSRAM stacks (`...WithCaps`), so a task must not delete itself: decode is a persistent worker woken by a notification. ~57 KB -> ~100 KB free while playing.
- Harness lessons (cost hours): (1) opening the Nano serial port from `board.sh` RESETS the board (also with NSN_NORESET), so every extra call drops BT links and the pairing window: do a whole scenario in ONE `board.sh log` session;
  (2) after a few mode changes JukeDial's headphones mode stops connecting until it is rebooted (`ctl.py {"op":"reboot"}`); (3) `btmode` on round survives reboots: always restore `btmode 0`.

## Architecture as of 2026-11-25
The sections above are the bring-up log, kept as history. This is how the firmware is put together now. Requirements and their status: `requirements.md`; numbers: `measurements.md`; ideas not taken up: `ideas.md`.

**Audio path.** One decode task (core 1, stack in PSRAM, persistent) renders float stereo blocks from a `pc_backend_t` (JukeDial's decoders, referenced by path), runs the fx chain on them, resamples to the output's rate when needed (`pc_rsq`), applies volume / ReplayGain / sleep fade and writes 16-bit PCM into a PSRAM ring. The output pulls from the ring through one callback type. Outputs: USB DAC (`uac_host.c`, own UAC2 host), Bluetooth headphones (`bt_audio.c`, A2DP source with SBC / AAC / LDAC encoders), Auracast broadcast (`cast.c`, LC3, two BIS). A phone streaming in is a source (`bt:in`), not an output. Sources are path strings: a card file or folder, `http(s)://…` (radio, `radio.c`), `noise:<colour>`, `scene:<n>` (`scene.c`), `bt:in`.

**Who may start and stop playback.** `nsn_player_play / stop / resume` are called from the console task, the BLE command task, the phone follower, the keys task, night mode and the main loop; one recursive lock serialises them, a new session never starts while the previous one is active, and `stop` cancels a station that is still connecting. The decode task owns everything else of a session. It must not write flash (PSRAM stack), so anything persistent it produces (the resume bookmark) is handed to the main task.

**Control.** `cmd.c` is one command table shared by the UART console and the BLE GATT command characteristic; entries flagged UART-only are refused over BLE in one place (file writes, deletes, pin changes, reboot). The one way a BLE client changes a card file is `radio add / del`, limited to `radio.txt`, http(s) addresses and bounded sizes. Physical controls: Bluetooth headphone buttons (AVRCP target in `bt_audio.c`) and wired-headset line-control buttons read from the DAC dongle's HID interface (`uac_host.c` → `keys.c`, a configurable button × gesture → action map).

**Bluetooth.** Bluedroid, dual mode, BLE 5.0 feature set (needed by LE Audio; the 4.2 advertising API cannot be built next to it, so the control service advertises a legacy PDU through the extended-advertising API on set 1; the broadcaster uses set 0). Classic A2DP has one role at a time (sink or source) and switches at run time. LE Audio is started on the first `out cast`; it must come up with no GATT client connected (IDF asserts otherwise), so the control link is dropped for a moment and the client reconnects; it cannot be unloaded again without a reboot. `bt_ctl.c` owns the single GAP callback and forwards events that are not its own to the broadcaster.

**Memory.** Internal RAM is the limit (Bluetooth and Wi-Fi stacks live there); PSRAM has about 15 MB to spare. Rules: buffers, rings, file data, decoder state and task stacks go to PSRAM unless the task writes flash; JukeDial's Wi-Fi trims are applied (6 static TX buffers, Wi-Fi hot paths in flash). `heap_caps_get_minimum_free_size` is not trustworthy on this board; use `st`'s `mem` / `big` and the UART `memwatch` command.

**CPU.** Core 0: protocol stacks, USB, console, small services. Core 1: decode, fx, encoders (LC3 needs the vector unit, which only core 1 has). Effects are optional and are shed in two ways: a fixed rule pauses the heavy ones under LDAC and Auracast, and a load rule pauses them when core 1 stays at 90 % or more for two seconds.

**Settings.** One NVS key per setting (namespace `nsn`), written only when changed; the old single blob is migrated once. Modules keep their own small values through `nsn_settings_get_i32 / set_i32`.

**Dependency on JukeDial.** Built against the sibling checkout by relative path; `round.lock` pins the commit and the hashes of the three files that were copied and adapted (`fx.c`, `backend_gme.cpp`, `backend_sdec.c`); `tools/check_round.sh` reports drift at configure time. The web page copies tunetap's visualizer (`web/viz/`) and station finder (`web/radio/`) unmodified, with Nano's glue in separate files.

**Known limits.** A2DP sink and source cannot run together. Auracast and headphone output are exclusive. Scenes use about 26 KB more internal RAM than other sources (the Ogg decoders' small allocations; the codec library is binary, and moving them would mean lowering the global PSRAM-malloc threshold, which was judged not worth the risk while the worst case, broadcast + scene, still leaves about 50 KB). m4a / ogg / aac files cannot resume mid-file. HLS radio: TS / packed-audio segments only (see "HLS radio" below).

## Playback refinements (2026-11-25): gapless, seamless scene loops, volume rules
**Gapless (`gapless on|off`, default on, NVS `gapless`, `st` `"gapless"`).** `play_one()` now keeps one `trk_t` (decoder + reader + file) and, when a track ends by itself, `try_chain()` closes it, opens the playlist's next file into the same struct and carries on in the same loop: same sink, same ring, same resampler state, `s_gap` stays false (so an under-run there is counted). It chains only when the next file has the same source rate and the same output; otherwise it hands the already-advanced index back (`s_adv`) and `run_session()` takes the old retune path (drain, stop sink, reopen). `advance()` is the one place that moves the playlist (shuffle history, repeat, once). Manual skips are untouched (drain and jump). Radio, phone, noise and scene sources never chain. MP3 encoder delay / padding are trimmed inside `pc_backend_mp3` (LAME/Xing tag), always, whatever the setting; AAC / M4A / Ogg / Opus are not trimmed (no module in JukeDial does it).
What the setting really changes: the decoder runs one ring (0.5 s at 48 kHz) ahead of the output, so a next file that opens in less than the ring's content was already gap-free before (measured: FLAC / MP3 joins identical with the setting on and off). The setting adds (1) no resampler re-init at a join (a 22.05 kHz -> 48 kHz join used to insert 34 frames of silence plus two jumps, now clean), (2) a real under-run count over a join. An open that takes longer than the ring holds (`dbg opendelay 800`) still gaps, on or off.
**Scene loops.** `scene.c` runs each layer through JukeDial's `pc_loop` (50 ms equal-power crossfade, output one crossfade behind the decoder). The "rewind" callback closes and reopens the layer's decoder from the PSRAM copy, so one decoder per layer exists at any time; `pc_loop`'s own second decoder (`dbg spare 1`) costs about 26 KB more internal RAM for two layers and is not used.
**Volume.** `s_ovol[]` holds one volume per output (DAC = `g_set.vol`, Bluetooth `vol_bt`, Auracast `vol_cast`), `s_vmax` the limit (`vollimit`, NVS), both read through `pc_volsel_clamp`. The output is chosen in `play_one()` (`apply_out_volume`), `vol` / keys / AVRCP change the playing output's value (idle: the output `pick_sink` / the preference would give). An output that never had a volume of its own starts at min(current, 40) (`SAFE_START`) and stores it. The soft start is a 400 ms linear ramp applied to the PCM the sink pulls (`pull()`), armed at session start and on pause -> resume (not for `bt:in`). Persistence: setters mark `s_dirty`; `nsn_player_persist()` writes from the command task (`vol`, `vollimit`, `gapless`) and the main loop (keys / AVRCP changes, within 5 s).
**Debug output check (`outcheck.c`, UART `dbg ...`).** Runs on the PCM the sink pulls: `J` = a sample that breaks the sine prediction x[n] = c x[n-1] - x[n-2] (c fitted on the fly) by more than max(24, 3 % of the recent peak); `Z` = at least 24 frames of exact digital silence inside a signal; frames the sink asked for and did not get count as zeros. Joins of the playlist are listed by output frame so events can be matched to them. Only meaningful for steady tones.

## Standalone repository (2026-11-25)
Supersedes "Sharing code with round" and the architecture note "Dependency on JukeDial". Owner decision: this is an independent public repo. The decoder components
and `player_core` were copied from JukeDial (commit `6fdce40`) into `components/`, pruned to the sources whose objects put bytes into the linked image (1235 -> 728 files,
284k -> 206k lines; `libfreeaptx` and `player_core/host_test` gone; table and licences in `components/ORIGIN.md`), and `app/CMakeLists.txt` now uses `../components` and
`../.local/bt_ldac`. The patched Bluedroid is generated at configure time by `tools/bt_ldac/make_bt.sh` (same mechanism as the exFAT FatFs; the failure message names the script).
`round.lock`, `tools/check_round.sh` and the configure-time drift check are removed; the two repositories are maintained separately. `tools/round_cmd.py` stays as a test helper for the peer board only.
Reproducible builds: `CONFIG_APP_REPRODUCIBLE_BUILD=y` plus `-fmacro-prefix-map=<repo root>=/NSN` (the project directory is mapped to "." by IDF), so the image does not depend on where the repo is checked out; pass the same `-DPROJECT_VER` to compare two builds (the default is `git describe`, which carries `-dirty`).
Check used for the pruning: the image built from the copy is byte-identical to the one built from JukeDial's tree; after pruning, sections, symbols and `idf.py size*` are identical and the `.bin` differs only in the ELF checksum fields (deleted `md5.c` changes the symbol type of three ROM symbols; see ORIGIN.md).
Not pruned: code inside kept files (for example `pc_i18n_data.c`); a Kconfig option can still switch on pruned-in-spirit code paths inside kept files (`FX_VELVET_ROOM`).

## Hardware configuration and extension points (2026-11-25)
**`hw` (hw.c).** TF card wiring and the status LED (GPIO, colour order) are NVS settings (`hw_sd`, `hw_led`, through `nsn_settings_*`), read once at boot by `nsn_hw_load()` (before storage and led), changed only by the UART console; over BLE `hw` reads and every setter answers `?`. Validation uses the board's reserved-pin table (`app/main/board/board_default.c`, sources in `docs/porting.md`); the board's own default pins are always accepted for their role. `pending` compares the stored settings with what this boot runs with. A bad stored value falls back to the default at load, and a card that does not answer leaves BLE / UART / `hw` untouched (`state=none`).
**SDMMC routing (measured from the headers, not hardware).** `SOC_SDMMC_USE_IOMUX=1`, no GPIO matrix (`SDMMC_LL_SLOT_SUPPORT_GPIO_MATRIX=0`): only slot 0 (24 25 20 21 22 23) is usable here (slot 1 is made of strapping pins). Other pins: SD in SPI mode on a matrix SPI bus (`hw sd spi`, storage.c, SPI2, 20 MHz), 1-bit.
**Outputs as a table (sink.h).** The player talks to `nsn_sink_t` (present, link_rate, start, stop, auto_rank, name); dac / bt / cast are adapters in player.c with ids 1..3 (the `out` preference value, `st`'s `out`, `nsn_player_sink()`); the board adds more in `board_sinks_register()` (id 4 = `i2s`, `board/sink_i2s.c`, a template compiled only with `NSN_SINK_I2S`, not implemented). AUTO = present sink with the lowest `auto_rank` (dac 10, i2s 20, bt 30; cast 0 = never).
**Board hooks (board/board.h).** `board_display_init()`, `board_buttons_init()` (empty), called from app_main after the BLE service starts; `nsn_spec_observe()` gives a display a second spectrum consumer. `ver` prints version / IDF / ELF SHA prefix; the version comes from -DPROJECT_VER, else `git describe --tags --dirty`, else `version.txt` + commit.

## HLS radio (2026-11-25)
`radio.c` plays `.m3u8` playlists (URL ending in `.m3u8`, a playlist content type, or a body that starts with `#EXTM3U` and carries `#EXT-X-` tags). The one network task, the 256 KB PSRAM ring, `pc_stream` and `backend_aac` / `pc_backend_mp3` are unchanged; HLS is one more way to fill the ring.
`pc_hls.c` / `pc_ts.c` had been pruned from `components/` (they put no bytes into the image while nothing called them); they were copied again from JukeDial and extended (below). They are in `player_core/CMakeLists.txt`.

**Flow** (`hls_run`, after JukeDial's `hls_session`): playlist text -> `pc_hls_parse_from` -> a master list picks one variant and the loop fetches it -> a media list: `pc_hls_next(last)` gives the first segment not played (3 back from live the first time) -> each segment is read in 4 KB pieces, demuxed (TS: `pc_ts_feed` -> ADTS AAC or MP3; packed audio: optional ID3 tag skipped, format sniffed) and written to the ring -> when the list is used up it is fetched again; nothing new for 6 x half a target duration = "reconnecting" (new session, normal backoff). `#EXT-X-ENDLIST` ends the stream when the last segment is in the ring (the player then ends the track as for a file).
**Variant rule** (`pc_hls_parse_from`): variants with E-AC-3 / AC-3 / FLAC / Opus / ALAC codecs never; audio-only variants (no `RESOLUTION`, no video codec) first; else an `#EXT-X-MEDIA TYPE=AUDIO` rendition with a URI; else any other variant (a TS with video still demuxes: the audio PID is picked). Within the tier: the highest `BANDWIDTH` at or under 200 kbit/s (JukeDial's limit), else the lowest. No separate floor.
**Refused** (state text stays in `rstate` until the next source starts; the session ends): `#EXT-X-KEY` with a METHOD other than NONE -> `HLS: encrypted`; `#EXT-X-MAP` or ftyp/styp/moof/mdat segment -> `HLS: fMP4 not supported`; AC-3 sync word -> `HLS: AC-3 not supported`; a TS without AAC/MP3 audio (stream types 0x81, 0x87 ...) or all variants of an unplayable codec -> `HLS: unsupported audio`; format changing mid-stream -> `HLS: format changed`; https while the largest internal block is under 24 KB (JukeDial's `tls_ok`) -> `https: low memory`.
**Departures from JukeDial.** (1) One `esp_http_client` handle for the playlists and all segments (JukeDial opens one per segment): `esp_http_client_set_url` keeps the connection while host and port are the same, a stale kept-alive connection is retried once on a fresh one. (2) Redirects are followed by hand on every request (JukeDial's radio had them for playlists only; the ICY path here did not follow redirects at all before). (3) Reads use a 1 s timeout looped up to 8 times with a cancel check, so `stop` is seen within a second; connect and headers are bounded by 5 s + 6 x 1 s. (4) Pacing: the next segment is not requested until the ring has room for the previous segment's size (+8 KB); JukeDial reads into a full ring and lets the TCP window fill. (5) An ended list longer than the 16-segment window is played from its first segment (`pc_hls_parse_from` with a start sequence); JukeDial's parser keeps the newest 16 and would play the end. (6) A reconnect goes on after the last played segment instead of replaying the window. (7) 404 / 410 on a segment skips it and reloads the list. (8) Variant choice by CODECS / RESOLUTION / audio renditions, refusal of encrypted / fMP4 / AC-3, `#EXTINF` titles ("no desc" and empty ignored), URLs up to 1023 characters (signed CDN addresses are 700+). (9) JukeDial plays https HLS through its http-twin policy; Nano has no twin logic and uses the https address directly.
**Memory.** All HLS state is in one PSRAM block (`work_t`: 4 KB read buffer, TS output buffer, parser result and scratch, 128 KB playlist text, URL buffers), allocated once with the ring; it replaced two static internal-RAM buffers of the old code (4 KB + 8 KB). The seen-segment bookkeeping is one `uint64` (`last`). The only per-segment allocations are the HTTP client's own. TLS buffers are in PSRAM (`CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC`).

## Chip and tracker formats after the sweep (2026-11-25)
What was exercised and the numbers: `requirements.md` (A4 and the table at its end), `measurements.md` (CPU). Decisions taken while fixing what the sweep found:
**VGM gate.** `components/player_core/src/pc_vgmhdr.c` (pure C, no allocation, no Nano dependency; host test `tools/hosttest/test_pc_vgmhdr.c`) parses the header by the VGM 1.71 layout, version-gated and bounded by the data offset (a v1.50 file's bytes after its 0x40 header are never read as clocks), and scans the command stream for the chips it really writes. `app/main/vgmgate.c` is Nano's policy: GME plays SN76489 (dual, T6W28), YM2612, YM2413 (YM2612 + YM2413 together: GME keeps only the YM2612, so the YM2413 counts as unsupported). A declared chip whose commands never occur is ignored; a file with an unsupported chip that is written is refused whole (no partial mixes: owner's "only what truly cannot be played is excluded" is applied per file, and a mix that drops a chip silently is what the sweep found wrong). A scan that cannot be trusted (unknown opcode, cut-off command) refuses every declared unsupported chip. The module will later be the router of an FM backend: its `unsupported` call takes the supported mask as a parameter.
**Where it runs.** In `open_track()` (decode task) for every `.vgm` / `.vgz`, card or zip entry, so playlists skip without stalling (`skip <file>: chip YM2151` in the log and in `events`). Single-file `play` also checks before it touches the running session, in a small worker task with a PSRAM stack: un-gzipping needs far more stack than the console / BLE tasks have (the first version crashed with a stack protection fault there). Reply: `play: ESP_ERR_NOT_SUPPORTED unsupported chip YM2151, OKIM6295` (the first word stays an esp error name: the page maps it, shows "can't be played", and can later show the rest). gzip is detected by its magic, not the extension.
**Bookmarks of chip files.** Chip formats have no byte position, and `resume` used to go back to a stale bookmark of an unrelated file. The token is now the tune number for GME and SID (restored with `start_track`), 0 for the formats that open from memory at their start (trackers, AdLib, YM, P8); `trk_t.card` is set for those files too, so "heard to the end" clears the bookmark as for any card file.
**Tags.** GME tune info (`backend_gme_tags`: song or game as title, author, game as album; refreshed at each tune change) and the headers of SID and MOD / XM / S3M / IT (`chiptags.c`). Strings are cleaned to valid UTF-8 (Latin-1 bytes become `?`). YM (LHA-packed), HES, KSS and the AdLib formats have no tags.
**Experimental tier.** `pc_chipseq`'s experimental readers (a2m a2t d00 sa2 and others) were claimed by `backend_for()` but refused at open because Nano never registered the setting. Registered now, default on (`expfmt on|off`, NVS `expfmt`). Reason: owner rule "support everything that can be played"; those readers were gated in JukeDial for fuzz findings, so a box that must not risk them can switch it off. The sweep played d00 / sa2 / a2m files without a crash; that is a few files, not a fuzz run.
**Failed opens.** The GME reason is logged and goes to `events` as `skip <file>: <reason>` ("Digimusic not supported" for SAP TYPE D). A single `play` of a file that then fails to open still answers `play: ESP_OK`: the open happens in the decode task after the reply (the session ends with `nothing playable` in the log). The page learns it only from `st` going idle.
**Smaller fixes.** A manual `next` inside a multi-tune file empties the ring: that refill is a gap, not an underrun (`s_gap`). A gapless chain that hits an unopenable file used to open it twice. `try_chain` marks the gap for the same reason.
**Not fixed, for the owner.** HES and KSS report 256 tunes (neither format stores a count; GME's default), so `next` walks into empty tune numbers and an album does not move to the next file until they are used up; options: a sidecar `.m3u`, a per-format cap, or "stop at the first tune that ends silent". SAP TYPE D, A2M v9–11 (aPlib), RSID tunes that need a real C64 (Stereophonik is silent on the PC too) are decoder limits. VGM files that write to the YM2612 DAC-stream commands (0x90–0x95) desync GME's command walker (`command_len` of 0x9x is 1); the inspector reports `dac_stream` but Nano does not act on it yet. KSS plays with the MSX-MUSIC FM on (41–48 % of core 1); `backend_gme.h` said "off by default", which was stale and is corrected.
