// SPDX-License-Identifier: LGPL-2.1-or-later
// One flashing session on top of esptool-js (vendored, pinned, loaded only when the user starts flashing). Derived from the owner's JukeDial web
// tool (tools/web/flasher.js: Flasher.begin / write / end, retry, error codes) and changed for Nano:
//  * ROM loader only. The firmware is flashed with `esptool --no-stub`; esptool-js 0.7.1 would upload its ESP32-S31 stub inside main(), so
//    runStub() is replaced by a no-op (nothing is uploaded, IS_STUB stays false, the baud change and the writes use the ROM commands).
//  * Port: a USB-serial bridge (CH340 ...) wired to UART0 with the DTR/RTS auto-reset circuit. esptool-js picks its classic DTR/RTS reset
//    for every port that is not the chip's own USB-Serial-JTAG (VID 0x303a, PID 0x1001), which is this case.
//  * Verification: esptool-js compares the flash MD5 with the file's only when asked, and in ROM mode it hex-encodes the chip's ASCII answer
//    (64 characters against 32), which can never match; so the page asks the chip itself (romMd5) and compares after the write.
//  * After the write the board is reset with the classic sequence by Conn.resume() (it resets on every open), not by esptool-js's hard reset.
//
//   const s = await beginSession({ conn, log })    pauses the console, syncs at 115200, switches to 460800, reads chip and flash size
//   s.info                                         { chip, flashSize, flashBytes, baud }
//   await s.write(parts, { onProgress })           deflate write of [{name, offset, data}], then flash-MD5 check of every part
//   await s.end()                                  release the port, resume the console (resets the board); returns true when the port did not come back
import { md5Hex } from "./md5.js";

export const ESPTOOL_VERSION = "0.7.1";
export const ESPTOOL_PATH = "vendor/esptool-js/" + ESPTOOL_VERSION + "/bundle.js";
export const FLASH_BAUD = 460800, ROM_BAUD = 115200;
// where the page's own script lives, captured while it runs (the dynamic import below is relative to the page, so a sub-path works)
const BASE = (globalThis.document && globalThis.document.currentScript && globalThis.document.currentScript.src) || (globalThis.location && globalThis.location.href) || "";

const err = (code, extra) => { const e = new Error(code + (extra ? ": " + extra : "")); e.code = code; e.extra = extra; return e; };
let lib = null;
async function loadEsptool() {
  if (lib) return lib;
  const url = new URL(ESPTOOL_PATH, BASE).href;
  try { lib = await import(/* webpackIgnore: true */ url); } catch (e) { throw err("lib", e && e.message); }
  return lib;
}
export const setEsptool = (m) => { lib = m; };

// MD5 of flash[offset .. offset+len) as the chip computes it (SPI_FLASH_MD5). In ROM mode the chip answers 32 ASCII characters, which
// esptool-js's flashMd5sum() hex-encodes (64 characters); with a stub it answers 16 raw bytes (32 characters). Both come out as 32 hex digits here.
export async function romMd5(loader, offset, len) {
  const r = String(await loader.flashMd5sum(offset, len)).toLowerCase();
  if (r.length === 64 && /^[0-9a-f]+$/.test(r)) {
    let s = "";
    for (let i = 0; i < 64; i += 2) s += String.fromCharCode(parseInt(r.slice(i, i + 2), 16));
    return s.toLowerCase();
  }
  return r;
}

const flashBytesOf = (fs) => { if (!fs) return 0; const n = parseInt(fs, 10); return /KB/i.test(fs) ? n * 1024 : n * 1048576; };

export const BAUDS = [460800, 230400, 115200];

// baud: the speed to switch to after syncing at romBaud. When switching to it fails, or the first write at it fails, the session is set up once more at romBaud
// (115200: slower, but more forgiving of cables and USB-serial bridges) and onFallback({ from, to, stage: "connect" | "write", message }) says so.
export async function beginSession({ conn, esp, log = () => {}, baud = FLASH_BAUD, romBaud = ROM_BAUD, onFallback = () => {} }) {
  const E = esp || await loadEsptool();
  if (conn.state === "idle") throw err("notconn");
  const port = await conn.pause("flash");
  let loader = null, transport = null, ended = false, cur = baud;
  const release = async () => {
    if (ended) return false;
    ended = true;
    try { if (transport) await transport.disconnect(); } catch { /* closed */ }
    try { await conn.resume(); return false; } catch { return true; }
  };
  const connect = async (b) => {
    transport = new E.Transport(port, false);
    loader = new E.ESPLoader({ transport, baudrate: b, romBaudrate: romBaud, terminal: { clean() {}, writeLine: log, write: log }, debugLogging: false });
    loader.runStub = async function () { log("ROM loader only (no stub)"); return this.chip; };   // --no-stub, as in the firmware's flash command
    cur = b;
    await loader.main("default_reset");
  };
  const info = { chip: "", flashSize: "", flashBytes: 0, baud };
  const fallBack = async (stage, e) => {
    const from = cur;
    log(`${stage === "write" ? "write" : "baud change"} at ${from} failed (${(e && e.message) || e}): again at ${romBaud}`);
    try { if (transport) await transport.disconnect(); } catch { /* closed */ }
    info.baud = romBaud;
    onFallback({ from, to: romBaud, stage, message: (e && e.message) || String(e) });
    await connect(romBaud);   // the reset into the bootloader happens again inside main()
  };
  try {
    try { await connect(baud); }
    catch (e) { if (baud <= romBaud) throw e; await fallBack("connect", e); }
    info.chip = loader.chip.CHIP_NAME;
    const flashSize = await loader.detectFlashSize();   // "16MB"
    info.flashSize = flashSize || ""; info.flashBytes = flashBytesOf(flashSize);
    return {
      info,
      // parts [{name, offset, data}]; progress f in 0..1 over all parts, plus what is happening. A failed first write above romBaud means "fall back and go on"
      // (the retries that are left then run at romBaud); at romBaud a failure is simply retried.
      async write(parts, { onProgress = () => {}, retries = 2 } = {}) {
        const sizes = parts.map((p) => p.data.length), total = sizes.reduce((a, b) => a + b, 0);
        let lastErr;
        for (let attempt = 0; attempt <= retries; attempt++) {
          try {
            await loader.writeFlash({
              fileArray: parts.map((p) => ({ data: p.data, address: p.offset })),
              flashMode: "keep", flashFreq: "keep", flashSize: "keep",   // the build's bootloader header (dio / 80 MHz / 16 MB) is written as it is
              eraseAll: false, compress: true,
              reportProgress: (i, written, t) => onProgress((sizes.slice(0, i).reduce((a, b) => a + b, 0) + (written * sizes[i]) / t) / total, { phase: "write", index: i, name: parts[i].name }),
            });
            lastErr = null;
            break;
          } catch (e) {
            lastErr = e; log("retry " + (attempt + 1) + ": " + (e && e.message));
            if (cur > romBaud && attempt < retries) { try { await fallBack("write", e); } catch (e2) { lastErr = e2; break; } }
          }
        }
        if (lastErr) throw lastErr;
        for (let i = 0; i < parts.length; i++) {
          onProgress(1, { phase: "verify", index: i, name: parts[i].name });
          const want = md5Hex(parts[i].data), got = await romMd5(loader, parts[i].offset, parts[i].data.length);
          if (want !== got) throw err("verify", parts[i].name);
        }
      },
      async end() { return release(); },
    };
  } catch (e) { await release(); throw e; }
}
