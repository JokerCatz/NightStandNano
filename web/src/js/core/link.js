// SPDX-License-Identifier: LGPL-2.1-or-later
// The Bluetooth link: GATT setup, the command channel (one command at a time), the reconnect loop, the Connect button.
// send(cmd) -> Promise<reply lines>. The polled `st` lives in status.js; what to load after a connect is in session.js.
import { app, $, sleep, emit, DEMO, setT } from "./app.js";
import { redact } from "../lib/wifi.js";
import { refreshStatus } from "./status.js";
import { afterConnectLoads } from "./session.js";

export const UUID = n => `7f2a000${n}-a1c4-4e53-9a31-6e73616e6f00`;
export const SVC = UUID(1), CMD = UUID(2), EVT = UUID(3), SPEC = UUID(4);
export const dec = new TextDecoder(), enc = new TextEncoder();

let pollTimer = null, rcTok = 0;
const RC_DELAYS = [0, 500, 1000, 2000, 4000], RC_EVERY = 5000, RC_MAX = 60000;   // ms before each attempt; later attempts every RC_EVERY; stop starting new ones after RC_MAX
let linkWaiters = [];
// resolves true when the link is up, false when it is (or becomes) given up / closed by the user
export const waitLink = () => app.link === "on" ? Promise.resolve(true) : app.link === "off" ? Promise.resolve(false) : new Promise(r => linkWaiters.push(r));
function settleWaiters(ok) { const w = linkWaiters; linkWaiters = []; for (const r of w) r(ok); }

// ---- command channel: one command at a time; the reply is every line that arrives until the box has been quiet for 250 ms ----
// A drop resolves the command in flight with the lines it had so far (usually none) and everything still queued with no lines;
// a send() made while reconnecting waits in the queue for the link (or resolves empty when the attempts are given up).
let pending = null, queue = [], evtBuf = "";
export function onEvt(ev) {
  evtBuf += dec.decode(ev.target.value);
  let i;
  while ((i = evtBuf.indexOf("\n")) >= 0) {
    const line = evtBuf.slice(0, i); evtBuf = evtBuf.slice(i + 1);
    if (!(pending && pending.quiet)) log("< " + line);   // a quiet command's reply stays out of the log pane (polls)
    if (pending) {
      pending.lines.push(line);
      if (pending.until) { if (pending.until.test(line)) finish(); }   // sendUntil: only the terminating line ends it (the overall timeout stays)
      else { clearTimeout(pending.t); pending.t = setTimeout(finish, 250); }
    }
  }
}
function onSpec(e) { const v = e.target.value; if (v.byteLength >= 34) for (let i = 0; i < 32; i++) app.target[i] = v.getUint8(2 + i); }
function finish() { const p = pending; pending = null; if (p) { clearTimeout(p.t); p.resolve(p.lines); } pump(); }
function pump() {
  if (pending || !queue.length || !app.chCmd) return;
  const p = pending = queue.shift(); p.lines = [];
  if (!p.quiet) log("> " + logCmd(p.cmd));
  app.chCmd.writeValueWithResponse(enc.encode(p.cmd)).then(() => { if (pending !== p) return; if (p.fast) finish(); else p.t = setTimeout(finish, p.until ? p.ms : 1500); })
    .catch(e => { log("write failed: " + e); if (pending === p) finish(); });
}
function enqueue(cmd, fast, until, ms, quiet) {
  return new Promise(resolve => {
    if (app.link === "off" && !app.chCmd) { resolve([]); return; }
    queue.push({ cmd, resolve, fast, until, ms, quiet }); pump();
  });
}
export const send = cmd => enqueue(cmd, false);
// Same as send(), but neither the command nor its reply is written to the log pane (for polls that would flood it).
export const sendQuiet = cmd => enqueue(cmd, false, undefined, undefined, true);
// For commands that reply with nothing on success (fx set ...): same queue, same order, but resolves (with no lines) as soon as the GATT
// write has completed instead of after the reply's quiet gap. Anything the box does say is picked up by the next command's reply.
export const sendFast = cmd => enqueue(cmd, true);
// For a reply that comes late and in pieces (`wifi scan`: the box is busy ~3 s, then answers): resolves with every line up to and
// including the first one that matches `re`, or after `ms` with whatever arrived. Nothing else is sent meanwhile (the queue is serial).
export const sendUntil = (cmd, re, ms) => enqueue(cmd, false, re, ms || 10000);
function dropCommands() {   // the link is gone: nothing in flight may hang
  const p = pending; pending = null; if (p) { clearTimeout(p.t); p.resolve(p.lines || []); }
  const q = queue; queue = []; for (const c of q) c.resolve([]);
}
// Log hook: every sent command goes through these before it is written to the log pane (the Wi-Fi password is blanked here).
const LOG_REDACT = [redact];
const logCmd = c => LOG_REDACT.reduce((s, f) => f(s), c);
export function log(s) { const p = $("log"); p.textContent = (p.textContent + s + "\n").split("\n").slice(-80).join("\n"); p.scrollTop = p.scrollHeight; }

// ---- connection ----
// The GATT setup is one function for the first connect and every reconnect: connect, find the service and characteristics, start the
// notifications. It sets chCmd last, so a half-done attempt leaves the page without a command channel.
async function realGattSetup() {
  const srv = await app.dev.gatt.connect();
  const svc = await srv.getPrimaryService(SVC);
  const cmd = await svc.getCharacteristic(CMD);
  app.chEvt = await svc.getCharacteristic(EVT); app.chSpec = await svc.getCharacteristic(SPEC);
  app.chEvt.removeEventListener("characteristicvaluechanged", onEvt); app.chEvt.addEventListener("characteristicvaluechanged", onEvt); await app.chEvt.startNotifications();
  app.chSpec.removeEventListener("characteristicvaluechanged", onSpec); app.chSpec.addEventListener("characteristicvaluechanged", onSpec); await app.chSpec.startNotifications();
  evtBuf = ""; app.chCmd = cmd;
}
let gattSetup = realGattSetup;
export const setGattSetup = fn => { gattSetup = fn; };   // ?demo swaps in the fake box
export async function connect() {
  if (!DEMO && !navigator.bluetooth) { $("warn").style.display = "block"; return; }
  try {
    if (!DEMO) app.dev = await navigator.bluetooth.requestDevice({ filters: [{ services: [SVC] }], optionalServices: [SVC] });
    app.dev.addEventListener("gattserverdisconnected", onDisconnected);   // the same function reference: adding it again is a no-op
    app.userDisc = false;
    await gattSetup();
    await afterConnect();
  } catch (e) { log("connect: " + e); app.userDisc = true; app.chCmd = null; try { app.dev && app.dev.gatt.disconnect(); } catch { /* already gone */ } setConn(false); }
}
async function afterConnect() {
  setConn(true); settleWaiters(true); pump();
  clearInterval(pollTimer); pollTimer = setInterval(refreshStatus, 1000);
  await refreshStatus(); await afterConnectLoads();
}
function onDisconnected() {
  clearInterval(pollTimer); app.chCmd = null; evtBuf = ""; dropCommands();
  if (app.link === "reconnecting") return;                       // a failed attempt of the loop below: it carries on by itself
  if (app.userDisc || app.link === "off") { endLink(); return; }     // the 中斷 button
  app.link = "reconnecting"; app.target.fill(0); setLinkUI();        // the box dropped the link, went out of range or rebooted
  reconnectLoop();
}
function endLink() {   // not connected, and not trying to be
  rcTok++; app.link = "off"; app.castStarting = false; emit("endlink"); settleWaiters(false); dropCommands(); setConn(false);
}
// Backoff: at once, then 0.5 s, 1 s, 2 s, 4 s, then every 5 s; no new attempt starts after about 60 s from the drop. The same device object
// is used, so there is no chooser and no user gesture; if the browser refuses connect() the attempts fail and the page ends up "not connected".
async function reconnectLoop() {
  const tok = ++rcTok, t0 = Date.now();
  for (let n = 0; ; n++) {
    const d = n < RC_DELAYS.length ? RC_DELAYS[n] : RC_EVERY;
    if (d) await sleep(d);
    if (tok !== rcTok || app.link !== "reconnecting") return;
    if (Date.now() - t0 > RC_MAX) { log("reconnect: giving up"); app.userDisc = true; try { app.dev.gatt.disconnect(); } catch { /* gone */ } endLink(); return; }
    try { await gattSetup(); } catch (e) { app.chCmd = null; log("reconnect " + (n + 1) + ": " + e); continue; }
    if (tok !== rcTok || app.link !== "reconnecting") return;
    log("reconnected"); await afterConnect(); return;
  }
}
export function setConn(on) { app.link = on ? "on" : "off"; setLinkUI(); }
export function setLinkUI() {
  const on = app.link === "on", rc = app.link === "reconnecting";
  app.connected = on;
  $("dConn").className = "dot" + (on ? " on" : rc ? " warn" : "");
  setT($("connTxt"), app.castStarting && app.link !== "off" ? "conn.castStarting" : on ? "conn.on" : rc ? "conn.reconnecting" : "conn.off");
  setT($("btnConnect"), on ? "btn.disconnect" : rc ? "btn.cancel" : "btn.connect");
  for (const el of document.querySelectorAll(".ctl")) el.disabled = !on;
  if (app.link === "off") { app.target.fill(0); app.state = null; emit("linkoff"); }
  emit("linkui");
}
export function initLink() {
  $("btnConnect").onclick = () => {
    if (app.link === "reconnecting") { app.userDisc = true; try { app.dev.gatt.disconnect(); } catch { /* gone */ } endLink(); return; }   // "cancel"
    if (app.link === "on" && app.dev && app.dev.gatt.connected) { app.userDisc = true; app.dev.gatt.disconnect(); } else connect();
  };
}
