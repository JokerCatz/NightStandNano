// SPDX-License-Identifier: LGPL-2.1-or-later
// The one Web Serial connection of the page: nothing else touches a SerialPort. Derived from the owner's JukeDial web tool (tools/web/conn.js:
// the idle -> open -> flashing -> open -> idle state machine, pause()/resume() lending the port to esptool-js, request/answer matching,
// teardown), changed for Nano: the console is plain text lines at 115200 (not a JSON protocol), replies are found by a line filter
// (the log is noisy), the port chooser offers USB-serial bridges (not only the chip's own USB), and opening the port resets the board
// the classic UART-bridge way (DTR/RTS) so the state is known.
//
//   state: idle -> console -> flashing -> console -> idle
//   open({any, port})       ask for a port (any = no filter) or use `port`; false when the user cancelled
//   close()                 let go of the port
//   request(cmd, {accept, until, timeout, partial})  send a line, collect lines accept() likes until until(lines) is true -> Promise<lines>
//   poll(cmd, opts)         request() again and again (every `every` ms, `total` ms at most): the board may still be booting
//   send(cmd)               one line out, no answer expected (reboot, raw console input)
//   pause(why) -> port      stop reading, close the port, fail waiting requests (code 'busy'); resume() reopens it and resets the board
//   on('state'|'line'|'tx'|'close', fn)
// Error codes: nosupport notconn busy timeout lost open
import { createFramer } from "./proto.js";

export const FILTERS = [{ usbVendorId: 0x1a86 }, { usbVendorId: 0x10c4 }, { usbVendorId: 0x0403 }, { usbVendorId: 0x303a }];   // CH340, CP210x, FTDI, Espressif
const VENDORS = { 0x1a86: "CH340-family", 0x10c4: "CP210x", 0x0403: "FTDI", 0x303a: "Espressif" };
const hex4 = (n) => n.toString(16).padStart(4, "0");
const defaultSleep = (ms) => new Promise((r) => setTimeout(r, ms));

export function portLabel(port) {
  let i = {};
  try { i = (port && port.getInfo && port.getInfo()) || {}; } catch { /* no info */ }
  if (!i.usbVendorId) return "serial port";
  return (VENDORS[i.usbVendorId] || "USB serial") + " " + hex4(i.usbVendorId) + ":" + hex4(i.usbProductId || 0);
}

// The classic reset into the application: EN low for 100 ms (RTS), IO0 high (DTR off), then both released. Safe on a plain USB-serial adapter.
export async function resetToApp(port, sleep = defaultSleep) {
  try {
    await port.setSignals({ dataTerminalReady: false, requestToSend: true });
    await sleep(100);
    await port.setSignals({ dataTerminalReady: false, requestToSend: false });
  } catch { /* the adapter has no control lines: nothing to do */ }
}

export function createConn({ serial = globalThis.navigator && globalThis.navigator.serial, sleep = defaultSleep, resetOnOpen = true, bootSettle = 150 } = {}) {
  let port = null, writer = null, reader = null, loop = null, state = "idle", why = "", stopping = false, chain = Promise.resolve();
  const subs = { state: [], line: [], tx: [], close: [] };
  const taps = new Set();   // active request collectors
  const enc = new TextEncoder();
  const err = (code, detail) => { const e = new Error(code + (detail ? ": " + detail : "")); e.code = code; e.detail = detail || ""; return e; };
  const emit = (k, ...a) => { for (const f of subs[k]) try { f(...a); } catch (e) { console.error(e); } };
  const setState = (s, w) => { if (s === state && (w || "") === why) return; state = s; why = w || ""; emit("state", s, why); };
  const failAll = (e) => { for (const t of [...taps]) t.fail(e); taps.clear(); };

  function onLine(l) { emit("line", l); for (const t of [...taps]) t.line(l); }

  async function readLoop() {
    const fr = createFramer();
    reader = port.readable.getReader();
    try { for (;;) { const { value, done } = await reader.read(); if (done) break; for (const l of fr.push(value)) onLine(l); } }
    catch { /* unplugged or cancelled */ }
    finally { try { reader.releaseLock(); } catch { /* released */ } reader = null; }
    if (!stopping) teardown(false);   // the read side ended by itself: the cable came out
  }
  async function startIO() {
    await port.open({ baudRate: 115200, bufferSize: 65536 });
    stopping = false;
    writer = port.writable.getWriter();
    loop = readLoop();
    if (resetOnOpen) { await resetToApp(port, sleep); await sleep(bootSettle); }
  }
  async function stopIO() {
    stopping = true;
    const l = loop;
    if (reader) try { await reader.cancel(); } catch { /* gone */ }
    if (l) try { await l; } catch { /* gone */ }
    loop = null;
    try { if (writer) writer.releaseLock(); } catch { /* released */ }
    writer = null;
    try { await port.close(); } catch { /* closed */ }
  }
  function teardown(byUser, closed) {
    const was = state !== "idle";
    try { if (writer) writer.releaseLock(); } catch { /* released */ }
    writer = null;
    const p = port; port = null; loop = null;
    failAll(err("lost"));
    if (p && !closed) Promise.resolve().then(() => p.close()).catch(() => {});
    setState("idle");
    if (was) emit("close", byUser);
  }
  if (serial && serial.addEventListener) serial.addEventListener("disconnect", (e) => { if (port && e.target === port) teardown(false); });

  async function open(o = {}) {
    if (state !== "idle") throw err("busy");
    let p = o.port;
    if (!p) {
      if (!serial) throw err("nosupport");
      try { p = await serial.requestPort(o.any ? {} : { filters: FILTERS }); }
      catch (e) { if (e && e.name === "NotFoundError") return false; throw err("open", e && e.message); }
    }
    port = p;
    try { await startIO(); }
    catch (e) {
      try { if (writer) writer.releaseLock(); } catch { /* released */ }
      writer = null; port = null;
      throw err("open", e && e.message);
    }
    setState("console");
    return true;
  }
  async function close() {
    if (!port) return;
    if (state === "flashing") { const p = port; port = null; setState("idle"); emit("close", true); try { await p.close(); } catch { /* closed */ } return; }
    failAll(err("lost"));
    await stopIO();
    teardown(true, true);
  }
  const write = async (line) => { if (!writer) throw err("notconn"); await writer.write(enc.encode(line + "\n")); };

  function request(cmd, { accept = () => true, until = () => true, timeout = 2500, partial = false } = {}) {
    const run = async () => {
      if (state === "flashing") throw err("busy");
      if (state !== "console" || !writer) throw err("notconn");
      return new Promise((res, rej) => {
        const got = [];
        const tap = {
          line: (l) => { if (accept(l)) { got.push(l); if (until(got)) { cleanup(); res(got); } } },
          fail: (e) => { cleanup(); rej(e); },
        };
        const t = setTimeout(() => { cleanup(); if (partial && got.length) res(got); else rej(err("timeout")); }, timeout);
        const cleanup = () => { clearTimeout(t); taps.delete(tap); };
        taps.add(tap);
        emit("tx", cmd);
        write(cmd).catch((e) => { cleanup(); rej(err("lost", e && e.message)); });
      });
    };
    const p = chain.then(run, run);   // one request at a time: replies of two commands never mix
    chain = p.catch(() => {});
    return p;
  }
  async function poll(cmd, opts = {}) {
    const { every = 700, total = 10000, ...rest } = opts, t0 = Date.now();
    for (;;) {
      try { return await request(cmd, { timeout: every, ...rest }); }
      catch (e) { if (e.code !== "timeout" || Date.now() - t0 + every > total) throw e; }
    }
  }
  async function send(cmd) {
    if (state === "flashing") throw err("busy");
    if (state !== "console" || !writer) throw err("notconn");
    emit("tx", cmd);
    await write(cmd);
  }
  async function pause(r = "") {
    if (state !== "console") throw err(state === "flashing" ? "busy" : "notconn");
    setState("flashing", r);
    failAll(err("busy"));   // before the reader goes: nothing new is written, what waits is told
    await stopIO();
    return port;
  }
  async function resume() {
    if (state !== "flashing" || !port) throw err("notconn");
    try { await startIO(); } catch { teardown(false); throw err("lost"); }
    setState("console");
  }

  return {
    get state() { return state; }, get why() { return why; }, get port() { return port; }, get up() { return state === "console" && !!writer; },
    get supported() { return !!serial; }, get label() { return port ? portLabel(port) : ""; },
    open, close, request, poll, send, pause, resume,
    on(k, f) { subs[k].push(f); },
  };
}
