// SPDX-License-Identifier: LGPL-2.1-or-later
// #/settings/diag: status line, sparklines of CPU load and free memory (the last 2 minutes, kept in memory only), build stamp, log.
import { app, $, on, t, setRaw } from "../core/app.js";
import { EQ } from "../lib/eq.js";
import { sendQuiet } from "../core/link.js";
import { KEY_ACTS } from "./keys.js";
import { describeEvent, newEventState, eventsCmd, applyEvents } from "../lib/events.js";

// the stamp is a content hash of src/ put in by build.mjs (see web/README.md); "dev" when the page runs unbundled
export const BUILD = typeof __BUILD__ === "string" ? __BUILD__ : "dev";

// ---- rolling history for the sparklines (memory only, last 120 polls = 2 minutes) ----
const HIST = { c0: [], c1: [], mem: [] }, HIST_N = 120;
function pushHist(st) {
  const add = (a, v) => { a.push(v); if (a.length > HIST_N) a.shift(); };
  if (Array.isArray(st.cpu) && st.cpu[0] >= 0 && st.cpu[1] >= 0) { add(HIST.c0, st.cpu[0]); add(HIST.c1, st.cpu[1]); }
  if (typeof st.mem === "number") add(HIST.mem, st.mem);
}
export function drawSparks() {
  if (app.route !== "settings/diag") return;
  const rows = [["spc0", HIST.c0, 0, 100, v => Math.round(v) + "%"], ["spc1", HIST.c1, 0, 100, v => Math.round(v) + "%"],
    ["spm", HIST.mem, null, null, v => Math.round(v / 1024) + " KB"]];
  for (const [id, arr, lo, hi, fmt] of rows) {
    const row = $(id); row.hidden = arr.length === 0; if (!arr.length) continue;
    row.lastElementChild.textContent = fmt(arr[arr.length - 1]);
    EQ.drawSpark(row.querySelector("canvas"), arr, lo, hi);
  }
}
function renderSys() {
  const state = app.state; if (!state) return;
  const cpu = Array.isArray(state.cpu) && state.cpu[0] >= 0 && state.cpu[1] >= 0 ? " · " + t("sys.cpu", { a: Math.round(state.cpu[0]), b: Math.round(state.cpu[1]) }) : "";
  setRaw($("sysv"), t("sys.line", { wifi: t(state.wifi ? "sys.wifiOn" : "sys.wifiOff"), kb: Math.round(state.mem / 1024), n: state.und }) + cpu);
}

// ---- recent events: `events` / `events <seq>`, polled every 3 s while this page is open, visible and connected; quiet (not in the log pane) ----
const EV = newEventState(); let evBusy = false, evAgain = false;
const pad = n => String(n).padStart(2, "0");
const clock = ms => { const d = new Date(ms); return pad(d.getHours()) + ":" + pad(d.getMinutes()) + ":" + pad(d.getSeconds()); };
function evAction(a) { return a in KEY_ACTS ? { t: KEY_ACTS[a] } : a === "pause" || a === "ignored" ? { t: "ev.a." + a } : a; }
function evText(text) {
  const d = describeEvent(text);
  if (d.kind === "wired") return t("ev.wired", { b: { t: "keys.btn." + d.b }, g: { t: "keys.g." + d.g }, a: evAction(d.a) });
  if (d.kind === "bt") {
    const k = d.name ? t("ev.k." + d.name) : t("ev.k.code", { code: d.code });
    return t("ev.bt", { k: d.name && d.a === "ignored" ? k + " (" + d.code + ")" : k, a: evAction(d.a) });
  }
  return d.text;
}
function renderEvents() {
  const ul = $("evList"); ul.textContent = "";
  for (const e of EV.list) {
    const li = document.createElement("li"), tm = document.createElement("time");
    tm.textContent = clock(e.at); tm.dateTime = new Date(e.at).toISOString();
    li.append(tm, evText(e.text)); ul.appendChild(li);
  }
  $("evEmpty").hidden = EV.list.length > 0;
}
async function pollEvents(force) {
  if (app.scanBusy || app.link !== "on" || document.hidden || app.route !== "settings/diag") return;
  if (evBusy) { if (force === true) evAgain = true; return; }   // a button press while a poll is in flight: one more right after it
  evBusy = true;
  try {
    for (let i = 0; i < 2; i++) {   // the second round: the box's sequence restarted, ask again from the start
      const r = await sendQuiet(eventsCmd(EV)), now = Date.now();
      if (!r.length) break;
      const res = applyEvents(EV, r, now);
      if (res.added || res.restart || force) renderEvents();
      if (!res.restart) break;
    }
  } finally { evBusy = false; if (evAgain) { evAgain = false; pollEvents(true); } }
}
export function initDiag() {
  $("evRefresh").onclick = () => pollEvents(true);
  $("evClear").onclick = () => { EV.list.length = 0; renderEvents(); };
  setInterval(pollEvents, 3000);
  on("route", key => { if (key === "settings/diag") pollEvents(true); });
  on("lang", renderEvents);
  document.addEventListener("visibilitychange", () => { if (!document.hidden) pollEvents(true); });
  renderEvents();
  $("buildStamp").textContent = BUILD;
  on("status", state => { pushHist(state); renderSys(); drawSparks(); });
  on("lang", renderSys);
  window.addEventListener("resize", drawSparks);
  if (window.matchMedia) window.matchMedia("(prefers-color-scheme: dark)").addEventListener("change", drawSparks);
}
