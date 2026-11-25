# NightStand Nano flasher and hardware setup page

A browser page (Web Serial, desktop Chrome or Edge only) that flashes the NightStand Nano firmware over the board's UART bridge and sets the
SD card and LED pins over the same cable. Published at `https://jokercatz.github.io/NightStandNano/flash/`, next to the Bluetooth control page
(`../`, sources in `web/`). The two pages share no runtime code; the language choice carries over because both use the localStorage key `nsn.lang`.
Hardware pin settings are made here over the wired UART, never over Bluetooth.

The page hosts no firmware: download the release file(s), pick them in the page. The file never leaves the browser.

## Release files (for the release script)

Attach these to the GitHub release (`<version>` is free text such as `1.2.3` or `2026.11.25`; the page reads the real version from the image):

| File | What | Flashed at |
|---|---|---|
| `NightStandNano-<version>-full.bin` | ONE merged image made by `esptool merge-bin` with `--flash-mode keep --flash-size keep --flash-freq keep`: starts at flash offset 0x0, 0xFF up to 0x2000, bootloader at 0x2000, partition table at 0x8000, app at 0x10000, 0xFF between, nothing after the end of the app | 0x0 by esptool; the page writes only the three regions |
| `NightStandNano-<version>-bootloader.bin` | `bootloader/bootloader.bin` | 0x2000 |
| `NightStandNano-<version>-partition-table.bin` | `partition_table/partition-table.bin` (with its MD5 line, as IDF writes it) | 0x8000 |
| `NightStandNano-<version>-app.bin` | `nightstand_nano.bin` | 0x10000 |

The page accepts either the one merged file, or all three separate files. The raw build names `bootloader.bin`, `partition-table.bin` and
`nightstand_nano.bin` are accepted too. Detection is by content (the name is only a hint): a merged image is 0xFF up to 0x2000 and an ESP image
(magic byte 0xE9, chip id 32 = ESP32-S31, SHA-256 appended) at 0x2000; the table starts `AA 50` and ends with an MD5 line; the app carries an
`esp_app_desc_t` (magic 0xABCD5432 at 0x20). The gaps in a merged image, including the settings area 0x9000..0x10000, must be 0xFF, and nothing
may follow the app: a flash read-back is refused. Keep the build free of host paths (`CONFIG_APP_REPRODUCIBLE_BUILD=y`).

Flashing runs at 460800 baud; if the speed change or the first write fails the page retries once at 115200 and says so ("Advanced" lets you pick 460800 / 230400 / 115200).

Manual command line (what the page does, for the release notes): `python -m esptool --chip esp32s31 -b 460800 --before default-reset --after hard-reset --no-stub write-flash --flash-mode dio --flash-size 16MB --flash-freq 80m 0x0 NightStandNano-<version>-full.bin`
(this erases the settings area, unlike the page) or `... 0x2000 bootloader.bin 0x8000 partition-table.bin 0x10000 nightstand_nano.bin`.

A flash keeps settings: only bootloader, table and app are written. "Also erase the settings (NVS)" (off by default, with a warning) additionally
blanks the `nvs` partition of the image's own table (0x9000, 0x6000 per `app/partitions.csv`): Wi-Fi settings, Bluetooth pairings, volume and the
hardware pin settings are lost. `phy_init` (0xF000) is never touched.

## Hardware tab: console protocol (115200 baud, text lines)

`ver`, `hw`, `hw sd <clk> <cmd> <d0> <d1> <d2> <d3> <width>` (SDMMC: only the fixed pins of the `hw sdmmc` line, width 4 or 1), `hw sd spi <clk> <mosi> <miso> <cs>` (any free GPIOs), `hw led <gpio> [order] | off`, `hw reset`, `reboot`; replies are matched by prefix out of
the noisy log (`src/js/core/proto.js` has the grammar and the validation, which mirrors `app/main/hw.c`; `src/js/demo/mock.js` ports `hw.c` for `?demo` and the tests). Lines starting with `wifi set` are redacted in the console pane.

## Build

```
npm install        # esbuild 0.28.2 (same as web/); no runtime dependencies
npm run build      # src/ -> dist/ (committed; deterministic; relative URLs only; the footer's build stamp is a hash of src/)
npm test           # node --test test/
npm run check      # strings, syntax, dist up to date, vendor SHA256, CSP, privacy scan (no home paths, MACs, USB serials, key markers, private IPs)
npm run dev        # http://localhost:8001/ with rebuild on change
```

`?demo` runs a pretend board and a fake flashing run (see the comment at the top of `src/index.html` for the extra parameters).

## Tree

`src/index.html`, `src/css/app.css`, `src/js/main.js`, `src/js/core/` (`proto` console protocol, `release` file detection and write plan, `conn` the one
serial connection, `flasher` the esptool-js session, `md5`, `i18n`, `strings`), `src/js/ui/` (shell, flash tab, hardware tab), `src/js/demo/mock.js`,
`src/vendor/esptool-js/0.7.1/` (vendored, with licence), `test/`, `testlib/` (image builders, privacy patterns), `build.mjs`, `check.mjs`, `dist/`.

## Derived from JukeDial

From the owner's JukeDial web tool (`ESP32S31_round/tools/web/`, LGPL-2.1-or-later), adapted:

- `src/js/core/conn.js` from `conn.js` (state machine, pause/resume for esptool-js; text console instead of JSON, bridge port filters, reset on open)
- `src/js/core/flasher.js` from `flasher.js` (`begin`/`write`/retry; ROM-only, own MD5 check, no backup, no board choice, no watchdog reset)
- `src/js/core/release.js` from `flasher.js` (`espImage`, `partitionTable`, `planWrite`, `chipCheck`; merged or three files)
- `src/js/core/md5.js` from `flasher.js` `md5Hex`
- `src/js/ui/flashtab.js` from `flashtab.js`
- `src/js/core/i18n.js` is copied from the control page's `web/src/js/core/i18n.js`
- `check.mjs` privacy patterns from `tools/web_publish.py`
- `src/vendor/esptool-js/0.7.1/` is the same vendored copy (Apache-2.0, pako MIT/Zlib notices in `NOTICE`)

Left out: the fonts tabs, the CTL JSON protocol and settings tabs, board selection and the FakePod watchdog reset, the backup step.
