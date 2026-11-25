// SPDX-License-Identifier: LGPL-2.1-or-later
// NightStand Nano: the clock of the box. An ES module (the pure helpers are unit-tested under node);
// mount() builds the 時間 page.
//
// Box commands used (see the firmware's cmd.c):
//   time                      -> time <unix s> <local ISO 8601 with offset> src=none|client|ntp tz="<POSIX TZ>"
//   time set <unix s> [<TZ>]  -> time set: ok | time set: zone only, clock is from ntp | time set: refused ...
//   time tz <TZ>              -> time tz: ok | time tz: refused ...
// The box has no battery clock: after every power-up its time is unknown until Wi-Fi/NTP or a client sets it, and the night-mode
// schedule needs a valid clock. So after every connect the page calls sync(): src none/client -> `time set <phone now> <tz>`;
// src ntp -> only `time tz <tz>`, and only when the box's zone differs and "set the zone from this phone" is on (nsn.tzsync).
//
// The zone sent is a fixed-offset POSIX string built from the phone's CURRENT offset, e.g. Asia/Taipei -> "UTC-8" (POSIX signs are
// inverted: UTC+8 is "-8", UTC-5 is "5"; +05:30 is "-5:30"). No DST rules are sent: when the phone's offset changes (daylight saving
// starts or ends) the next connect (or "sync now") sends the new string, so the box follows at the next visit of this page.
//
// opts: { box, send(cmd) -> Promise<lines>, isOn() -> bool, tabOpen() -> bool, log(s) }
// returns: { sync(), load(), reset(), rerender() }
import { I18N } from "../core/i18n.js";

const KEY = "nsn.tzsync";

// ---- pure helpers ----
// offsetMin: minutes EAST of UTC (-Date.getTimezoneOffset()); abbr: used only when it is 3-5 letters, else "UTC"
export function buildTz(offsetMin, abbr) {
  const name = /^[A-Za-z]{3,5}$/.test(abbr || "") ? abbr : "UTC";
  const posix = -Math.round(offsetMin);                  // inverted sign
  const a = Math.abs(posix), h = Math.floor(a / 60), m = a % 60;
  return name + (posix < 0 ? "-" : "") + h + (m ? ":" + String(m).padStart(2, "0") : "");
}
// the phone's zone string right now; `d` and `zone` are parameters for the tests
export function tzString(d, zone) {
  d = d || new Date();
  let abbr = "";
  try {
    zone = zone || new Intl.DateTimeFormat().resolvedOptions().timeZone;
    const p = new Intl.DateTimeFormat("en-US", { timeZone: zone, timeZoneName: "short" }).formatToParts(d).find(x => x.type === "timeZoneName");
    abbr = p ? p.value : "";
  } catch { /* no Intl zone: UTC<offset> below */ }
  return buildTz(-d.getTimezoneOffset(), abbr);
}
const RE_TIME = /^time (\d+) (\S+) src=(none|client|ntp) tz="(.*)"$/;
export function parseTime(lines) {
  for (const raw of lines || []) {
    const m = RE_TIME.exec(String(raw).trimEnd()); if (!m) continue;
    const o = /([+-])(\d\d):?(\d\d)$/.exec(m[2]);
    return { unix: +m[1], iso: m[2], src: m[3], tz: m[4].replace(/\\(["\\])/g, "$1"), offsetMin: o ? (o[1] === "-" ? -1 : 1) * (+o[2] * 60 + +o[3]) : 0 };
  }
  return null;
}
// "YYYY-MM-DD HH:MM:SS" of unix seconds at a fixed offset (minutes east)
export function fmtLocal(unix, offsetMin) {
  const d = new Date((unix + offsetMin * 60) * 1000);
  return d.toISOString().slice(0, 19).replace("T", " ");
}
const utcLabel = m => "UTC" + (m < 0 ? "-" : "+") + String(Math.floor(Math.abs(m) / 60)).padStart(2, "0") + ":" + String(Math.abs(m) % 60).padStart(2, "0");
function autoOn() { try { return globalThis.localStorage.getItem(KEY) !== "0"; } catch { return true; } }

export function mount(o) {
  const I = I18N, setT = I.setT, setRaw = I.setRaw;
  const { box, send, isOn, tabOpen } = o;
  const el = (tag, cls, text) => { const e = document.createElement(tag); if (cls) e.className = cls; if (text !== undefined) e.textContent = text; return e; };
  const elT = (tag, cls, key, params) => { const e = el(tag, cls); setT(e, key, params); return e; };
  let T = null, at = 0, syncing = null, busy = false, msg = null;

  const clockV = el("span"), srcV = el("span"), zoneV = el("span");
  const kv = el("div", "ckv");
  kv.append(elT("span", "", "time.k.now"), clockV, elT("span", "", "time.k.src"), srcV, elT("span", "", "time.k.zone"), zoneV);
  const noClock = elT("div", "mut cnote", "time.none.note"); noClock.hidden = true;
  const syncBtn = elT("button", "ctl", "time.sync");
  const chk = el("input", "ctl"); chk.type = "checkbox"; chk.checked = autoOn();
  const chkLab = el("label", "rchk"); chkLab.append(chk, elT("span", "", "time.auto"));
  const msgEl = el("div", "cmsg"); msgEl.setAttribute("role", "status");
  const row = el("div", "row"); row.style.marginTop = ".6rem"; row.append(syncBtn);
  box.textContent = ""; box.append(kv, noClock, row, chkLab, msgEl);

  function setMsg(key, params, kind) { msg = key; msgEl.className = "cmsg" + (kind ? " " + kind : ""); if (key) setT(msgEl, key, params); else setRaw(msgEl, ""); }
  function render() {
    const on = isOn();
    syncBtn.disabled = !on || busy;
    if (!T) { setRaw(clockV, "\u2014"); setRaw(srcV, "\u2014"); setRaw(zoneV, "\u2014"); noClock.hidden = true; return; }
    setT(srcV, "time.src." + T.src);
    noClock.hidden = T.src !== "none";
    setRaw(zoneV, utcLabel(T.offsetMin) + (T.tz ? " \u00b7 " + T.tz : ""));
    tick();
  }
  function tick() {
    if (!T || T.src === "none") { setRaw(clockV, "\u2014"); return; }
    setRaw(clockV, fmtLocal(T.unix + Math.floor((performance.now() - at) / 1000), T.offsetMin));
  }
  setInterval(tick, 1000);   // ticks locally between refreshes

  async function read() {
    if (!isOn()) return null;
    const p = parseTime(await send("time"));
    if (p) { T = p; at = performance.now(); }
    render();
    return p;
  }
  const nowSec = () => Math.floor(Date.now() / 1000);
  // after every connect: set a clock the box does not have, keep the zone of one it does
  function sync() {
    if (syncing) return syncing;
    syncing = (async () => {
      try {
        const p = await read(); if (!p) return;
        const mine = tzString(), auto = chk.checked;
        if (p.src === "none" || p.src === "client") {
          const r = await send("time set " + nowSec() + (auto ? " " + mine : ""));
          if (!r.some(l => /^time set: (ok|zone only)/.test(l))) o.log("time set: " + (r[0] || "no reply"));
        } else if (p.src === "ntp" && auto && p.tz !== mine) {
          const r = await send("time tz " + mine);
          if (!r.some(l => /^time tz: ok/.test(l))) o.log("time tz: " + (r[0] || "no reply"));
        } else return;
        await read();
      } finally { syncing = null; }
    })();
    return syncing;
  }
  // the button: always sets the clock (the box ignores it when the clock comes from ntp, and then only takes the zone)
  async function syncNow() {
    if (busy || !isOn()) return;
    busy = true; render(); setMsg(null);
    try {
      const r = await send("time set " + nowSec() + (chk.checked ? " " + tzString() : ""));
      const line = r.find(l => /^time set:/.test(l)) || "", m = /^time set: refused\s*(.*)$/.exec(line);
      if (/^time set: ok/.test(line)) setMsg("time.res.ok", null, "good");
      else if (/^time set: zone only/.test(line)) setMsg("time.res.zone", null, "good");
      else if (m) setMsg("time.res.refused", { why: m[1] || "?" }, "bad");
      else setMsg("time.res.noreply", null, "bad");
      await read();
    } finally { busy = false; render(); }
  }
  syncBtn.onclick = syncNow;
  chk.onchange = () => { try { globalThis.localStorage.setItem(KEY, chk.checked ? "1" : "0"); } catch { /* not kept */ } if (chk.checked && isOn()) sync(); };
  function reset() { T = null; busy = false; setMsg(null); render(); }
  setInterval(() => { if (isOn() && tabOpen() && !document.hidden && !syncing && !busy) read(); }, 30000);
  render();
  return { sync, load: read, reset, rerender: render, info: () => T,
    clock: () => (!T || T.src === "none" ? null : fmtLocal(T.unix + Math.floor((performance.now() - at) / 1000), T.offsetMin)) };
}

