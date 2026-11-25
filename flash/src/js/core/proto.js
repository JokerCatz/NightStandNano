// SPDX-License-Identifier: LGPL-2.1-or-later
// The firmware's console protocol (UART0, 115200 baud, text lines), as pure functions: framing, parsing, validation, command building.
// No DOM, no serial port: conn.js moves the bytes, the hardware tab draws the result, the tests call these directly.
//
//   ver                      -> ver <version> idf=<idf> chip=esp32s31 built=<first 9 hex digits of the ELF hash>   (version: 0.1.0-ee14d98-dirty, v0.1.0, ...)
//   hw                       -> hw sd clk=.. cmd=.. d0=.. d1=.. d2=.. d3=.. width=.. src=.. state=.. mode=sdmmc       (the STORED pins; state = the running mount)
//                               hw sd clk=.. mosi=.. miso=.. cs=.. src=.. state=.. mode=spi
//                               hw sdmmc routing=iomux clk=.. cmd=.. d0=.. d1=.. d2=.. d3=.. spi=any                   (the only pins SDMMC mode accepts)
//                               hw led gpio=.. order=.. on=.. src=..
//                               hw reserved <list>   hw gpios 0-<max>   hw pending 0|1                                 (1 = stored differs from running)
//   hw sd <clk> <cmd> <d0> <d1> <d2> <d3> <width>   hw sd spi <clk> <mosi> <miso> <cs>   hw led <gpio> [order] | off   hw reset
//                            -> "hw sd: ok, reboot to apply" | "hw sd: refused <reason>"   (app/main/hw.c has the reasons and their order)
//   reboot                   -> (no reply; the boot log follows)
// The console log is noisy: replies are found by their prefix at the start of a line, everything else is just log.

export const SD_PINS = ["clk", "cmd", "d0", "d1", "d2", "d3"];       // SDMMC mode
export const SPI_PINS = ["clk", "mosi", "miso", "cs"];               // SPI mode
export const LED_ORDERS = ["rgb", "grb", "brg", "rbg", "gbr", "bgr"];

// ---- framing -------------------------------------------------------------------------------------------------------------------
const ANSI = /\x1b\[[0-9;?]*[A-Za-z]/g;
export const stripAnsi = (s) => String(s).replace(ANSI, "");

// Bytes (or text) in, whole lines out. CRLF, lone CR and LF all end a line; a partial last line waits for the next chunk; empty lines are dropped.
export function createFramer(maxLine = 4096) {
  const dec = typeof TextDecoder !== "undefined" ? new TextDecoder("utf-8") : null;
  let buf = "";
  return {
    push(chunk) {
      buf += typeof chunk === "string" ? chunk : dec.decode(chunk, { stream: true });
      const out = [];
      const parts = buf.replace(/\r\n/g, "\n").replace(/\r/g, "\n").split("\n");
      buf = parts.pop();
      for (const p of parts) { const l = stripAnsi(p).trim(); if (l) out.push(l); }
      if (buf.length > maxLine) { const l = stripAnsi(buf).trim(); buf = ""; if (l) out.push(l); }
      return out;
    },
    reset() { buf = ""; },
  };
}

// ---- parsing -------------------------------------------------------------------------------------------------------------------
const kv = (s) => { const o = {}; for (const m of s.matchAll(/(\w+)=(\S*)/g)) o[m[1]] = m[2]; return o; };
const int = (v, d = null) => { const n = Number.parseInt(v, 10); return Number.isFinite(n) ? n : d; };

// "0.1.0-ee14d98-dirty" -> base 0.1.0, commit ee14d98, dirty; "v0.1.0" -> base 0.1.0; "v1.2.3-4-gabc1234" -> base 1.2.3, commit abc1234; anything else: the text as the base
export function describeVersion(v) {
  let s = String(v || "").trim();
  const dirty = /-dirty$/.test(s);
  if (dirty) s = s.slice(0, -6);
  s = s.replace(/^v(?=\d)/, "");
  const m = /^(\d[\w.+-]*?)(?:-\d+-g([0-9a-f]{4,})|-([0-9a-f]{6,}))?$/.exec(s);
  return { base: m ? m[1] : s, commit: m ? m[2] || m[3] || "" : "", dirty };
}
export function parseVer(line) {
  const m = /^ver\s+(\S+)(.*)$/.exec(stripAnsi(line).trim());
  if (!m) return null;
  const built = /\bbuilt=(.*)$/.exec(m[2]);
  const o = kv(built ? m[2].slice(0, built.index) : m[2]);
  return { version: m[1], ...describeVersion(m[1]), idf: o.idf || "", chip: o.chip || "", built: built ? built[1].trim() : "" };
}
export const isVerLine = (line) => parseVer(line) !== null;

// One `hw ...` read line into the accumulator `acc`; returns true when the line was a hw read line. Acks ("hw sd: ok") and echoes (no key=value) are not.
export function parseHwLine(line, acc) {
  const s = stripAnsi(line).trim();
  let m;
  if ((m = /^hw sdmmc\s+(\w+=.*)$/.exec(s))) {
    const o = kv(m[1]);
    acc.sdmmc = { routing: o.routing || "", clk: int(o.clk), cmd: int(o.cmd), d0: int(o.d0), d1: int(o.d1), d2: int(o.d2), d3: int(o.d3), spi: o.spi || "" };
    return true;
  }
  if ((m = /^hw sd (\w+=.*)$/.exec(s))) {
    const o = kv(m[1]);
    const common = { src: o.src || "default", state: o.state || "none" };
    acc.sd = (o.mode === "spi" || (o.mode === undefined && o.mosi !== undefined))
      ? { mode: "spi", clk: int(o.clk), mosi: int(o.mosi), miso: int(o.miso), cs: int(o.cs), ...common }
      : { mode: "sdmmc", clk: int(o.clk), cmd: int(o.cmd), d0: int(o.d0), d1: int(o.d1), d2: int(o.d2), d3: int(o.d3), width: int(o.width, 4), ...common };
    return true;
  }
  if ((m = /^hw led (\w+=.*)$/.exec(s))) {
    const o = kv(m[1]);
    acc.led = { gpio: int(o.gpio, -1), order: o.order || "grb", on: int(o.on, 0) === 1, src: o.src || "default" };
    return true;
  }
  if ((m = /^hw reserved(?:\s+(.*))?$/.exec(s))) {
    acc.reserved = (m[1] || "").split(/[\s,]+/).filter(Boolean).map((x) => int(x)).filter((n) => n !== null);
    return true;
  }
  if ((m = /^hw gpios\s+(\d+)\s*-\s*(\d+)\s*$/.exec(s))) { acc.gpios = { min: int(m[1]), max: int(m[2]) }; return true; }
  if ((m = /^hw pending\s+([01])\b/.exec(s))) { acc.pending = m[1] === "1"; return true; }
  return false;
}
// All of a `hw` reply (lines in any order, noise allowed). complete = the closing `hw pending` line arrived.
export function parseHw(lines) {
  const acc = {};
  for (const l of lines) parseHwLine(l, acc);
  return { sd: acc.sd || null, sdmmc: acc.sdmmc || null, led: acc.led || null, reserved: acc.reserved || [], gpios: acc.gpios || { min: 0, max: 61 }, pending: !!acc.pending, complete: acc.pending !== undefined };
}
// Is this a line a `hw` read can contain (used as the request filter)?
export const isHwReadLine = (line) => parseHwLine(line, {});

// "hw sd: ok, reboot to apply" / "hw led: refused range" / "hw reset: ok, reboot to apply"
export function parseAck(line) {
  const m = /^hw (sd|led|reset):\s*(ok|refused)\b[\s,:]*(.*)$/.exec(stripAnsi(line).trim());
  if (!m) return null;
  return { kind: m[1], ok: m[2] === "ok", text: m[3].trim() };
}
export const isAckLine = (kind) => (line) => { const a = parseAck(line); return !!a && a.kind === kind; };

// The reason after "refused": known ones become a code (the page translates them), the rest stays verbatim.
export function parseReason(text) {
  const s = String(text || "").trim();
  let m;
  if (/^range\b/.test(s)) return { code: "range" };
  if (/^syntax\b/.test(s)) return { code: "syntax" };
  if (/^storage\b/.test(s)) return { code: "storage" };
  if ((m = /^reserved\s+(-?\d+)/.exec(s))) return { code: "reserved", n: int(m[1]) };
  if ((m = /^duplicate\s+(-?\d+)/.exec(s))) return { code: "duplicate", n: int(m[1]) };
  if (/^width\b/.test(s)) return { code: "width" };
  if (/^order\b/.test(s)) return { code: "order" };
  if ((m = /^sd\/led conflict\s+(-?\d+)/.exec(s))) return { code: "conflict", n: int(m[1]) };
  if ((m = /^not supported\s*(.*)$/.exec(s))) return { code: "notsup", why: m[1].trim() };
  return { code: "other", raw: s };
}

// ---- commands ------------------------------------------------------------------------------------------------------------------
// sd = { mode: "sdmmc", clk, cmd, d0..d3, width } or { mode: "spi", clk, mosi, miso, cs }
export function sdCommand(sd) {
  if (sd.mode === "spi") return `hw sd spi ${sd.clk} ${sd.mosi} ${sd.miso} ${sd.cs}`;
  const w = sd.width === 1 ? 1 : 4;
  const d = (k) => (w === 1 ? -1 : sd[k]);
  return `hw sd ${sd.clk} ${sd.cmd} ${sd.d0} ${d("d1")} ${d("d2")} ${d("d3")} ${w}`;
}
export function ledCommand(led) {
  return led.on ? `hw led ${led.gpio} ${led.order}` : "hw led off";
}

// A line typed in the console pane (or echoed back) must never show a Wi-Fi password: everything after "wifi set" is replaced.
export function redact(line) {
  return String(line).replace(/^(\s*wifi\s+set)\b.*$/i, "$1 ***");
}

// ---- validation ----------------------------------------------------------------------------------------------------------------
// Mirrors app/main/hw.c (check_sd / check_led) so the page can say what is wrong before anything is sent; the firmware stays the judge.
// Issues are {field, code, n?} in the order the firmware would refuse them (the first one is its reason); fields are the pin names, "led", "order", "width", "mode".
// Codes: int (not a number: never sent) range (firmware: range) nopin (a GPIO this chip does not have: firmware also says range) reserved duplicate conflict width order notsup.
// A reserved pin is accepted for its own role when it is the board's default there: the SD pins of the `hw sdmmc` line, the LED pin while `hw led` says src=default
// (hw.ledDefault remembers it after the LED was changed; unknown = refused, "restore defaults" brings the stock pin back).
const isInt = (v) => typeof v === "number" && Number.isInteger(v);
export const sdFields = (sd) => (sd.mode === "spi" ? SPI_PINS : sd.width === 1 ? ["clk", "cmd", "d0"] : SD_PINS);
export const sdPinList = (sd) => sdFields(sd).map((f) => sd[f]);
// Chip facts that `hw reserved` does not carry (from docs/porting.md, the table in app/main/board/board_default.c): why a pin is on the list.
const NOPIN = [29, 41];
export function pinReason(n) {
  if (NOPIN.includes(n)) return "nopin";
  if ((n >= 26 && n <= 28) || (n >= 30 && n <= 32)) return "flash";
  if (n === 33 || n === 34) return "usb";
  if (n >= 36 && n <= 40) return "strap";
  if (n === 58 || n === 59) return "console";
  if (n === 60) return "strap60";
  if (n === 61) return "strap";
  return "other";
}
const pinExists = (n, hw) => n >= hw.gpios.min && n <= hw.gpios.max && !(NOPIN.includes(n) && hw.reserved.includes(n));
const sdmmcPins = (hw) => (hw.sdmmc ? SD_PINS.map((f) => hw.sdmmc[f]) : []);
export function validateSd(sd, hw, led = hw && hw.led) {
  const out = [], fields = sdFields(sd);
  if (sd.mode !== "spi") {
    if (sd.width !== 1 && sd.width !== 4) return [{ field: "width", code: "width" }];
    if (sd.width === 1 && ["d1", "d2", "d3"].some((f) => sd[f] !== -1)) return [{ field: "width", code: "width" }];
  }
  const bad = new Set();
  for (const f of fields) if (!isInt(sd[f])) { out.push({ field: f, code: "int" }); bad.add(f); }
  for (const f of fields) if (!bad.has(f) && !pinExists(sd[f], hw)) { out.push({ field: f, code: NOPIN.includes(sd[f]) ? "nopin" : "range", n: sd[f] }); bad.add(f); }
  const exempt = new Set(sdmmcPins(hw)), res = new Set(hw.reserved);
  for (const f of fields) if (!bad.has(f) && res.has(sd[f]) && !exempt.has(sd[f])) { out.push({ field: f, code: "reserved", n: sd[f] }); bad.add(f); }
  const seen = new Map();
  for (const f of fields) {
    if (bad.has(f)) continue;
    const n = sd[f];
    if (seen.has(n)) { out.push({ field: f, code: "duplicate", n }); out.push({ field: seen.get(n), code: "duplicate", n }); } else seen.set(n, f);
  }
  if (led && led.on && led.gpio >= 0) for (const f of fields) if (sd[f] === led.gpio && !bad.has(f)) { out.push({ field: f, code: "conflict", n: led.gpio }); break; }
  if (sd.mode !== "spi" && hw.sdmmc && !out.length) {
    const fixed = sd.clk === hw.sdmmc.clk && sd.cmd === hw.sdmmc.cmd && sd.d0 === hw.sdmmc.d0 && (sd.width === 1 || (sd.d1 === hw.sdmmc.d1 && sd.d2 === hw.sdmmc.d2 && sd.d3 === hw.sdmmc.d3));
    if (!fixed) out.push({ field: "mode", code: "notsup" });
  }
  return dedupe(out);
}
export function validateLed(led, hw, sd = hw && hw.sd) {
  const out = [], n = led.gpio;
  if (!led.on) return out;
  if (!LED_ORDERS.includes(led.order)) out.push({ field: "order", code: "order" });
  if (!isInt(n)) out.push({ field: "led", code: "int" });
  else if (!pinExists(n, hw)) out.push({ field: "led", code: NOPIN.includes(n) ? "nopin" : "range", n });
  else if (hw.reserved.includes(n) && n !== hw.ledDefault) out.push({ field: "led", code: "reserved", n });
  else if (sd && sdPinList(sd).includes(n)) out.push({ field: "led", code: "conflict", n });
  return out;
}
function dedupe(list) {
  const seen = new Set();
  return list.filter((i) => { const k = i.field + ":" + i.code; if (seen.has(k)) return false; seen.add(k); return true; });
}

// Next number in direction dir (+1/-1) that is not reserved, within [min, max]; null when there is none (the input keeps its value).
export function stepGpio(n, dir, hw) {
  const res = new Set(hw.reserved);
  for (let v = n + dir; v >= hw.gpios.min && v <= hw.gpios.max; v += dir) if (!res.has(v)) return v;
  return null;
}
