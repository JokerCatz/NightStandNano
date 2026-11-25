// SPDX-License-Identifier: LGPL-2.1-or-later
// #/settings/diag: status line, sparklines of CPU load and free memory (the last 2 minutes, kept in memory only), build stamp, log.
import { app, $, on, t, setRaw } from "../core/app.js";
import { EQ } from "../lib/eq.js";

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
export function initDiag() {
  $("buildStamp").textContent = BUILD;
  on("status", state => { pushHist(state); renderSys(); drawSparks(); });
  on("lang", renderSys);
  window.addEventListener("resize", drawSparks);
  if (window.matchMedia) window.matchMedia("(prefers-color-scheme: dark)").addEventListener("change", drawSparks);
}
