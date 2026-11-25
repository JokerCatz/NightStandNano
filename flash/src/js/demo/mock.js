// SPDX-License-Identifier: LGPL-2.1-or-later
// A pretend board for the tests and for ?demo: a Web Serial port (MockPort) behind which a NightStand Nano console runs (boot log, noisy log lines
// in between, the ver / hw / hw sd / hw led / hw reset / reboot protocol, refusals, `pending`), and a fake esptool-js (createMockEsp) that "flashes"
// with progress, the ROM-mode MD5 answer shape included. Nothing here talks to hardware.
//
// The `hw` part is a port of app/main/hw.c and board/board_default.c (same checks in the same order, same line formats, same refusals), so what the page
// sees here is what the real firmware says. Keep them in step.
//
// Options of createMockBoard / MockPort:  noise (log lines in between, default true), split (max bytes per chunk, 0 = whole), bootMs,
// sd:"mounted"|"none"|"unreadable" (what a card in the built-in socket does), spiCard:"none"|"mounted"|"unreadable" (what a card does on the pins of an SPI setting;
// "none" = nothing is wired there), refuse (reason for every setter), version, built, old (an old firmware without the hw command), silent (never answers).
import { md5Hex } from "../core/md5.js";

// ---- board/board_default.c ----
const DEFAULT_SD = { mode: "sdmmc", clk: 24, cmd: 25, d0: 20, d1: 21, d2: 22, d3: 23, width: 4, mosi: -1, miso: -1, cs: -1 };
const DEFAULT_LED = { gpio: 60, order: "grb" };
const SLOT0 = { clk: 24, cmd: 25, d0: 20, d1: 21, d2: 22, d3: 23 };          // soc/sdmmc_pins.h SDMMC_SLOT0_IOMUX_PIN_NUM_*
const RESERVED = [26, 27, 28, 30, 31, 32, 33, 34, 36, 37, 38, 39, 40, 58, 59, 60, 61];   // k_reserved[]; 29 and 41 do not exist (SOC_GPIO_VALID_GPIO_MASK)
const GPIO_MAX = 61, NO_GPIO = [29, 41];
const ORDERS = ["rgb", "grb", "brg", "rbg", "gbr", "bgr"];
const ESC = "\x1b";
const NOISE = [
  "I (%t) bt_audio: a2dp idle",
  "I (%t) nsn: heap free 214380 min 198112",
  "W (%t) wifi: no stored network",
  "I (%t) card: no change",
  "D (%t) night: tick",
];

const gpioExists = (g) => g >= 0 && g <= GPIO_MAX && !NO_GPIO.includes(g);
const gpioReserved = (g) => !gpioExists(g) || RESERVED.includes(g);
const sdSpi = (clk, mosi, miso, cs) => ({ mode: "spi", clk, cmd: -1, d0: -1, d1: -1, d2: -1, d3: -1, width: 1, mosi, miso, cs });
const sdUsed = (p) => (p.mode === "spi" ? [p.clk, p.mosi, p.miso, p.cs] : p.width === 4 ? [p.clk, p.cmd, p.d0, p.d1, p.d2, p.d3] : [p.clk, p.cmd, p.d0]);   // sd_used()
const sdSame = (a, b) => { const pa = sdUsed(a), pb = sdUsed(b); return a.mode === b.mode && pa.length === pb.length && pa.every((v, i) => v === pb[i]) && (a.mode === "spi" || a.width === b.width); };
const isDefaultSdPin = (g) => sdUsed(DEFAULT_SD).includes(g);
const isDefaultLedPin = (g) => DEFAULT_LED.gpio === g;
// check_pins(): exist, not reserved (unless exempt), distinct. -> null or the reason
function checkPins(pins, exempt) {
  for (const g of pins) if (!gpioExists(g)) return "range";
  for (const g of pins) if (gpioReserved(g) && !(exempt && exempt(g))) return `reserved ${g}`;
  for (let i = 0; i < pins.length; i++) for (let j = 0; j < i; j++) if (pins[i] === pins[j]) return `duplicate ${pins[i]}`;
  return null;
}
function checkSd(p, ledGpio) {
  if (p.mode === "sdmmc") {
    if (p.width !== 1 && p.width !== 4) return "width";
    if (p.width === 1 && (p.d1 !== -1 || p.d2 !== -1 || p.d3 !== -1)) return "width";
  }
  const pins = sdUsed(p), bad = checkPins(pins, isDefaultSdPin);
  if (bad) return bad;
  if (ledGpio >= 0 && pins.includes(ledGpio)) return `sd/led conflict ${ledGpio}`;
  if (p.mode === "sdmmc") {
    const fixed = p.clk === SLOT0.clk && p.cmd === SLOT0.cmd && p.d0 === SLOT0.d0 && (p.width === 1 || (p.d1 === SLOT0.d1 && p.d2 === SLOT0.d2 && p.d3 === SLOT0.d3));
    if (!fixed) return "not supported sdmmc-pins-fixed";
  }
  return null;
}
function checkLed(l, sd) {
  if (!ORDERS.includes(l.order)) return "order";
  if (l.gpio < 0) return null;
  const bad = checkPins([l.gpio], isDefaultLedPin);
  if (bad) return bad;
  if (sdUsed(sd).includes(l.gpio)) return `sd/led conflict ${l.gpio}`;
  return null;
}
// split() / num() / parse_sd() of hw.c: the words after `hw sd`; -> { p } or { why }
function parseSd(args) {
  const words = args.split(" ").filter(Boolean), k = words.length > 8 ? 9 : words.length;
  const spi = k > 0 && words[0] === "spi", n = spi ? 4 : 7, off = spi ? 1 : 0;
  if (k !== n + off) return { why: "syntax" };
  const v = [];
  for (let i = 0; i < n; i++) {
    const w = words[i + off];
    if (!/^[+-]?\d+$/.test(w)) return { why: "syntax" };
    const x = Number(w);
    if (x < -1000 || x > 1000) return { why: "syntax" };
    v.push(x);
  }
  if (v.some((x) => x < -1 || x > 127)) return { why: "range" };
  return { p: spi ? sdSpi(v[0], v[1], v[2], v[3]) : { mode: "sdmmc", clk: v[0], cmd: v[1], d0: v[2], d1: v[3], d2: v[4], d3: v[5], width: v[6], mosi: -1, miso: -1, cs: -1 } };
}

export function createMockBoard(opts = {}) {
  const o = { noise: true, split: 0, bootMs: 900, sd: "mounted", spiCard: "none", refuse: "", version: "0.1.0-ee14d98-dirty", built: "874f52d83", old: false, silent: false, ...opts };
  const timers = new Set();
  let out = null;           // (string) => void : bytes to the host
  let mode = "app";         // app | booting | download | off
  let t0 = Date.now();
  const board = {
    opts: o,
    store: { sd: null, led: null },   // NVS: absent = the board default (hw_sd / hw_led)
    running: { sd: { ...DEFAULT_SD }, led: { ...DEFAULT_LED } },
    reserved: [...RESERVED, ...NO_GPIO].sort((a, b) => a - b), gpios: { min: 0, max: GPIO_MAX },
    log: [],   // every line the host sent (for tests)
    get mode() { return mode; },
    attach(fn) { out = fn; },
  };
  const ms = () => Date.now() - t0 + 300;
  const later = (fn, ms) => { const h = setTimeout(() => { timers.delete(h); fn(); }, ms); if (h && h.unref) h.unref(); timers.add(h); };   // unref: a test process may end while the board still has timers
  const emit = (text) => {
    if (!out) return;
    if (!o.split) { out(text); return; }
    for (let i = 0; i < text.length; i += o.split) out(text.slice(i, i + o.split));
  };
  const line = (s) => emit(s + "\r\n");

  // load_both(): the stored values, or the defaults when nothing is stored or what is stored does not pass the checks
  function loadBoth() {
    let sd = board.store.sd ? { ...board.store.sd } : { ...DEFAULT_SD }, led = board.store.led ? { ...board.store.led } : { ...DEFAULT_LED };
    let sdC = !!board.store.sd, ledC = !!board.store.led;
    if (ledC && checkLed(led, sdC ? sd : DEFAULT_SD)) { led = { ...DEFAULT_LED }; ledC = false; }
    if (sdC && checkSd(sd, led.gpio)) { sd = { ...DEFAULT_SD }; sdC = false; }
    if (sdC && sdSame(sd, DEFAULT_SD)) sdC = false;
    if (ledC && led.gpio === DEFAULT_LED.gpio && led.order === DEFAULT_LED.order) ledC = false;
    return { sd, led, sdC, ledC };
  }
  const cardState = () => {
    const sd = board.running.sd;
    if (sd.mode === "spi") return o.spiCard;       // nothing is wired to the pins of an SPI setting in the mock unless asked
    return o.sd;                                   // SDMMC runs on the fixed pins only, 4-bit or 1-bit
  };
  const pending = () => { const c = loadBoth(); return !sdSame(c.sd, board.running.sd) || c.led.order !== board.running.led.order || c.led.gpio !== board.running.led.gpio; };

  function noiseLine() { return NOISE[Math.floor(Math.random() * NOISE.length)].replace("%t", String(ms())); }
  function scheduleNoise() {
    if (!o.noise) return;
    later(() => { if (mode === "app") { line(ESC + "[0;32m" + noiseLine() + ESC + "[0m"); scheduleNoise(); } }, 600 + Math.random() * 900);
  }
  board.boot = () => {
    for (const h of timers) clearTimeout(h);
    timers.clear();
    mode = "booting"; t0 = Date.now();
    const c = loadBoth();
    board.running = { sd: c.sd, led: c.led };
    if (o.silent) return;
    const sd = c.sd, pins = sdUsed(sd).join(" ");
    const steps = ["ESP-ROM:esp32s31-20260101", "Build:Jan  1 2026", "rst:0x1 (POWERON)", "I (31) boot: ESP-IDF v6.1 2nd stage bootloader", "I (120) boot: Loaded app from partition at offset 0x10000",
      "I (480) nsn: NightStand Nano " + o.version, `I (520) hw: sd ${sd.mode === "spi" ? "spi" : "sdmmc"} ${c.sdC ? "custom" : "default"}: ${pins}; led ${c.ledC ? "custom" : "default"} gpio ${c.led.gpio} ${c.led.order}`,
      "I (700) card: mount " + (cardState() === "mounted" ? "ok" : cardState() === "none" ? "no card" : "failed 0x107"), "I (780) nsn: ready"];
    steps.forEach((s, i) => later(() => line(s), (o.bootMs * i) / steps.length));
    later(() => { mode = "app"; scheduleNoise(); }, o.bootMs);
  };
  board.enterDownload = () => { for (const h of timers) clearTimeout(h); timers.clear(); mode = "download"; };
  board.stop = () => { for (const h of timers) clearTimeout(h); timers.clear(); mode = "off"; out = null; };

  // show() of hw.c
  function show() {
    const c = loadBoth(), sd = c.sd, src = c.sdC ? "custom" : "default";
    line(sd.mode === "spi" ? `hw sd clk=${sd.clk} mosi=${sd.mosi} miso=${sd.miso} cs=${sd.cs} src=${src} state=${cardState()} mode=spi`
      : `hw sd clk=${sd.clk} cmd=${sd.cmd} d0=${sd.d0} d1=${sd.d1} d2=${sd.d2} d3=${sd.d3} width=${sd.width} src=${src} state=${cardState()} mode=sdmmc`);
    line(`hw sdmmc routing=iomux clk=${SLOT0.clk} cmd=${SLOT0.cmd} d0=${SLOT0.d0} d1=${SLOT0.d1} d2=${SLOT0.d2} d3=${SLOT0.d3} spi=any`);
    if (o.noise) line(ESC + "[0;33mW (" + ms() + ") wifi: scan done" + ESC + "[0m");
    line(`hw led gpio=${c.led.gpio} order=${c.led.order} on=${c.led.gpio >= 0 ? 1 : 0} src=${c.ledC ? "custom" : "default"}`);
    line("hw reserved " + board.reserved.join(","));
    line(`hw gpios 0-${GPIO_MAX}`);
    line("hw pending " + (pending() ? 1 : 0));
  }
  const saveSd = (p) => { board.store.sd = sdSame(p, DEFAULT_SD) ? null : { ...p }; return true; };
  const saveLed = (l) => { board.store.led = l.gpio === DEFAULT_LED.gpio && l.order === DEFAULT_LED.order ? null : { ...l }; return true; };
  function setSd(args) {
    const r = parseSd(args);
    if (!r.p) return line(`hw sd: refused ${r.why}`);
    const why = checkSd(r.p, loadBoth().led.gpio);
    if (why) return line(`hw sd: refused ${why}`);
    line(saveSd(r.p) ? "hw sd: ok, reboot to apply" : "hw sd: refused storage");
  }
  function setLed(args) {
    const t = args.split(" ").filter(Boolean), k = t.length > 2 ? 3 : t.length;
    const l = { ...DEFAULT_LED };
    if (k === 1 && t[0] === "off") l.gpio = -1;
    else {
      if (k < 1 || k > 2 || !/^[+-]?\d+$/.test(t[0]) || Math.abs(Number(t[0])) > 1000) return line("hw led: refused syntax");
      const g = Number(t[0]);
      if (g < 0 || g > 127) return line("hw led: refused range");
      l.gpio = g;
      if (k === 2) { if (t[1].length >= 4) return line("hw led: refused order"); l.order = t[1]; }
    }
    const why = checkLed(l, loadBoth().sd);
    if (why) return line(`hw led: refused ${why}`);
    line(saveLed(l) ? "hw led: ok, reboot to apply" : "hw led: refused storage");
  }
  // nsn_hw_exec(): args after "hw" (no leading blanks); setters only on the wired console (this is it)
  function hwExec(args) {
    if (!args) return show();
    const a = args.slice(0, 95), i = a.indexOf(" "), head = i < 0 ? a : a.slice(0, i), rest = i < 0 ? null : a.slice(i + 1);
    if (o.refuse && ((head === "sd" || head === "led") && rest !== null)) return line(`hw ${head}: refused ${o.refuse}`);
    if (head === "sd" && rest !== null) return setSd(rest);
    if (head === "led" && rest !== null) return setLed(rest);
    if (head === "reset" && rest === null) { board.store.sd = null; board.store.led = null; return line("hw reset: ok, reboot to apply"); }
    line("?");
  }

  function cmd(raw) {
    const s = raw.trim();
    board.log.push(s);
    if (!s) return;
    if (mode !== "app") return;   // booting: the console is not up yet
    if (o.silent) return;
    if (s === "ver") return line(`ver ${o.version} idf=v6.1 chip=esp32s31 built=${o.built}`);
    if (o.old && /^hw\b/.test(s)) return line("unknown command: " + s);
    if (/^hw(\s|$)/.test(s)) return hwExec(s.slice(2).trim());
    if (s === "reboot") return board.boot();
    if (/^wifi set\b/.test(s)) return line("wifi: ok");
    line("unknown command: " + s.split(/\s+/)[0]);
  }
  board.receive = (text) => { for (const l of String(text).split(/\r?\n/)) if (l.length) cmd(l); };
  return board;
}

// A Web Serial port (just enough of it): readable/writable streams per open, setSignals, getInfo, close.
export class MockPort {
  constructor(opts = {}) {
    this.board = createMockBoard(opts);
    this.signals = [];
    this.baud = 0; this.isOpen = false; this.readable = null; this.writable = null;
    this.vid = opts.vid || 0x1a86; this.pid = opts.pid || 0x7523;
    this._rts = false; this._dtr = false;
    this.booted = false;
  }
  getInfo() { return { usbVendorId: this.vid, usbProductId: this.pid }; }
  async open({ baudRate }) {
    if (this.isOpen) throw new Error("InvalidStateError: port already open");
    this.isOpen = true; this.baud = baudRate;
    const enc = new TextEncoder(), dec = new TextDecoder();
    let ctl;
    this.readable = new ReadableStream({ start: (c) => { ctl = c; }, cancel: () => { this.board.attach(null); } });
    this.board.attach((text) => { try { ctl.enqueue(enc.encode(text)); } catch { /* cancelled */ } });
    this.writable = new WritableStream({ write: (chunk) => { this.board.receive(dec.decode(chunk, { stream: true })); } });
    this._ctl = ctl;
    if (!this.booted) { this.booted = true; this.board.boot(); }
  }
  async close() {
    this.isOpen = false;
    this.board.attach(null);
    const r = this.readable, w = this.writable; this.readable = null; this.writable = null;
    try { if (r && !r.locked) await r.cancel(); } catch { /* done */ }
    try { if (w && !w.locked) await w.close(); } catch { /* done */ }
  }
  async setSignals(s) {
    this.signals.push(s);
    if ("requestToSend" in s) {
      const was = this._rts;
      this._rts = !!s.requestToSend;
      if ("dataTerminalReady" in s) this._dtr = !!s.dataTerminalReady;
      if (was && !this._rts && !this._dtr) this.board.boot();   // EN released with IO0 high: the application starts
    }
  }
  unplug() { this.board.stop(); const ctl = this._ctl; try { ctl && ctl.close(); } catch { /* closed */ } this.isOpen = false; if (this.onunplug) this.onunplug(this); }
}

export function createMockSerial(opts = {}) {
  const ports = [];
  const listeners = [];
  return {
    ports,
    requestCalls: [],
    async requestPort(o) { this.requestCalls.push(o); const p = new MockPort(opts); p.addEventListener = () => {}; p.onunplug = (pp) => { for (const l of listeners) l({ target: pp }); }; ports.push(p); return p; },
    addEventListener(t, f) { if (t === "disconnect") listeners.push(f); },
  };
}

// ---- fake esptool-js ---------------------------------------------------------------------------------------------------------
// opts: speed (ms per step, default 8), steps, stopAt (fraction: hang there, for screenshots), failAt (fraction: throw), corrupt (flash differs from the file), noRomMd5Shape (answer 32 chars),
// maxBaud (the bridge cannot change to a speed above this: main() throws), writeMaxBaud (the change works but every write above this speed fails)
export function createMockEsp(opts = {}) {
  const st = { stubCalls: 0, mains: 0, written: new Map(), writeCalls: [], opened: null, ...opts };
  const o = { speed: 8, steps: 40, ...opts };
  class Transport {
    constructor(port) { this.port = port; this.tracing = false; }
    async connect(baud) { await this.port.open({ baudRate: baud }); }
    async disconnect() { await this.port.close(); }
    getInfo() { return "mock"; } getVid() { return this.port.vid; } getPid() { return this.port.pid; }
  }
  class ESPLoader {
    constructor(a) { this.transport = a.transport; this.baudrate = a.baudrate; this.romBaudrate = a.romBaudrate; this.IS_STUB = false; this.terminal = a.terminal; this.chip = null; st.loader = this; }
    async runStub() { st.stubCalls++; this.IS_STUB = true; return this.chip; }
    async main() {
      st.mains++;
      await this.transport.connect(this.romBaudrate);
      const b = this.transport.port.board; if (b) b.enterDownload();
      this.terminal.writeLine("Connecting...");
      this.chip = { CHIP_NAME: o.chip || "ESP32-S31" };
      this.terminal.writeLine("Chip is " + this.chip.CHIP_NAME);
      await this.runStub();
      if (this.romBaudrate !== this.baudrate) {
        if (o.maxBaud !== undefined && this.baudrate > o.maxBaud) throw new Error("Failed to change the baud rate to " + this.baudrate + " (mock)");
        this.terminal.writeLine("Changed baud rate to " + this.baudrate);
      }
      return this.chip;
    }
    async detectFlashSize() { return o.flashSize === undefined ? "16MB" : o.flashSize; }
    async writeFlash({ fileArray, reportProgress }) {
      st.writeCalls.push(fileArray.map((f) => ({ address: f.address, length: f.data.length, baud: this.baudrate })));
      if (o.writeMaxBaud !== undefined && this.baudrate > o.writeMaxBaud) throw new Error("A fatal error occurred: write failed at " + this.baudrate + " baud (mock)");
      for (let i = 0; i < fileArray.length; i++) {
        const f = fileArray[i], n = o.steps;
        reportProgress(i, 0, f.data.length);
        for (let k = 1; k <= n; k++) {
          await new Promise((r) => setTimeout(r, o.speed));
          const frac = (i + k / n) / fileArray.length;
          if (o.stopAt !== undefined && frac >= o.stopAt) await new Promise(() => {});
          if (o.failAt !== undefined && frac >= o.failAt) throw new Error("A fatal error occurred: write failed (mock)");
          reportProgress(i, Math.round((f.data.length * k) / n), f.data.length);
        }
        const copy = new Uint8Array(f.data); if (o.corrupt) copy[0] ^= 1;
        st.written.set(f.address, copy);
      }
    }
    async flashMd5sum(offset, len) {
      const d = st.written.get(offset);
      const hexs = md5Hex(d ? d.subarray(0, len) : new Uint8Array(len));
      if (o.noRomMd5Shape) return hexs;
      return [...hexs].map((c) => c.charCodeAt(0).toString(16)).join("");   // what esptool-js 0.7.1 returns in ROM mode: the ASCII digits, hex-encoded again
    }
  }
  return { Transport, ESPLoader, state: st };
}
