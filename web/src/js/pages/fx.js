// SPDX-License-Identifier: LGPL-2.1-or-later
// #/settings/fx: the sound effects the box applies (generic sliders built from `fx params`) and the equalizer editor
// (graph, band list, add / remove, AutoEq import). The page itself never plays sound.
import { app, $, on, t, setT } from "../core/app.js";
import { I18N } from "../core/i18n.js";
import { send, sendFast } from "../core/link.js";
import { EQ } from "../lib/eq.js";
import { loadNight } from "./night.js";

// ---- sound effects: built from "fx params" ----
//   APP fxp <app> on <0|1> <name>      /      APP fxp <app> <param> <lo> <hi> <def> <cur>
// The names are in strings.js (fx.app.<id>, fx.p.<param>), three languages. An app the table doesn't know shows the name the box
// sent with it; a parameter it doesn't know shows its id.
const fxAppName = (app, fb) => I18N.has("fx.app." + app) ? t("fx.app." + app) : (fb || app);
const fxParName = par => I18N.has("fx.p." + par) ? t("fx.p." + par) : par;
function fxLabels() {   // run again on a language change: the panel (checkboxes, slider positions) is not rebuilt
  for (const e of document.querySelectorAll("[data-fxapp]")) e.textContent = fxAppName(e.dataset.fxapp, e.dataset.fb);
  for (const e of document.querySelectorAll("[data-fxp]")) { e.textContent = fxParName(e.dataset.fxp); e.nextElementSibling.setAttribute("aria-label", e.textContent); }
}
//   APP fxb equalizer <band 0..9> <type 0|1|2> <fc> <gain_db> <q>     (only the equalizer; type 0 peaking, 1 low shelf, 2 high shelf)
// The equalizer gets its own editor (below) instead of the generic "count" slider; every other app is generic.
async function loadFx() {
  const lines = await send("fx params"), body = $("fxBody");
  if (app.link !== "on" && !lines.length) return;   // dropped meanwhile: keep the panel as it is
  const oldTa = body.querySelector(".eqimp textarea"), oldOpen = body.querySelector(".eqimp") && body.querySelector(".eqimp").open, oldText = oldTa ? oldTa.value : "";
  body.textContent = "";
  const hasBands = lines.some(l => l.startsWith("APP fxb equalizer "));
  let box = null, rows = null;
  eq = null;
  for (const l of lines) {
    const p = l.split(" "); if (p[0] !== "APP") continue;
    const app = p[2];
    if (p[1] === "fxb") {
      if (app !== "equalizer" || !eq) continue;
      const i = +p[3]; if (!(i >= 0 && i < EQ.MAX_BANDS)) continue;
      eq.bands[i] = { type: Math.min(2, Math.max(0, +p[4] | 0)), fc: +p[5], gain: +p[6], q: +p[7] };
      continue;
    }
    if (p[1] !== "fxp") continue;
    if (p[3] === "on") {
      box = document.createElement("div"); box.className = "fxapp";
      const lab = document.createElement("label"), c = document.createElement("input"); c.type = "checkbox"; c.checked = p[4] === "1";
      rows = document.createElement("div"); rows.hidden = !c.checked;
      const myRows = rows;
      c.onchange = () => { myRows.hidden = !c.checked; send("fx on " + app + " " + (c.checked ? 1 : 0)).then(eqDraw); };
      const nm = document.createElement("span"); nm.dataset.fxapp = app; nm.dataset.fb = p.slice(5).join(" ");
      lab.append(c, nm); box.append(lab, rows); body.appendChild(box);
      if (app === "equalizer" && hasBands) eq = { rows, preamp: 0, count: 0, bands: Array.from({ length: EQ.MAX_BANDS }, () => ({ type: 0, fc: 1000, gain: 0, q: 1 })) };
    } else if (rows) {
      const par = p[3], lo = +p[4], hi = +p[5], cur = +p[7];
      if (eq && eq.rows === rows && par === "count") { eq.count = Math.min(EQ.MAX_BANDS, Math.max(0, Math.round(cur))); continue; }
      const r = document.createElement("div"); r.className = "fxp";
      const nm = document.createElement("span"); nm.className = "mut"; nm.dataset.fxp = par;
      const sl = document.createElement("input"); sl.type = "range"; sl.min = lo; sl.max = hi; sl.step = (hi - lo) / 200; sl.value = cur;
      const v = document.createElement("span"); v.className = "mut"; const show = x => { v.textContent = String(+(+x).toPrecision(3)); }; show(cur);
      const isPre = eq && eq.rows === rows && par === "preamp_db";
      if (isPre) { eq.preamp = cur; eq.preSl = sl; eq.preShow = show; }
      sl.oninput = () => { show(sl.value); if (isPre) { eq.preamp = +sl.value; eqDraw(); } };
      sl.onchange = () => send("fx set " + app + " " + par + " " + sl.value);
      r.append(nm, sl, v); rows.appendChild(r);
    }
  }
  if (eq) { eqBuild(); const ta = body.querySelector(".eqimp textarea"); if (ta && oldText) { ta.value = oldText; ta.parentElement.open = true; } else if (oldOpen) body.querySelector(".eqimp").open = true; }
  if (!body.children.length) { const sp = document.createElement("span"); sp.className = "mut"; setT(sp, "fx.none"); body.appendChild(sp); }
  fxLabels();
}

// ---- equalizer editor: graph + band list + add/remove + AutoEq import ----
// Commands: fx set equalizer b<N>_type|b<N>_fc|b<N>_gain_db|b<N>_q <value>, fx set equalizer count <n>, preamp_db <dB>.
// Changes are sent on `change`; the graph follows `input` while dragging. Removing band k sends the values of every later band
// into the slot before it, then count-1.
const EQ_TYPES = ["eq.type.pk", "eq.type.ls", "eq.type.hs"];
let eq = null, eqNote = null;
const eqEl = (tag, cls) => { const e = document.createElement(tag); if (cls) e.className = cls; return e; };
function eqAria(e, key, n) { e.setAttribute("data-i18n-aria", key); e.setAttribute("data-i18n-p", JSON.stringify({ n })); e.setAttribute("aria-label", t(key, { n })); }
function eqDraw() { if (eq && eq.cv && app.route === "settings/fx") EQ.drawResponse(eq.cv, eq.bands.slice(0, eq.count), eq.preamp); }
const eqFieldName = { type: "type", fc: "fc", gain: "gain_db", q: "q" };
const eqSend = (i, f) => send(`fx set equalizer b${i}_${eqFieldName[f]} ${eq.bands[i][f]}`);
async function eqSendBand(i) { for (const f of ["type", "fc", "gain", "q"]) await sendFast(`fx set equalizer b${i}_${eqFieldName[f]} ${eq.bands[i][f]}`); }
const eqSendCount = () => sendFast("fx set equalizer count " + eq.count);
// how each slider maps to its value: fc and Q on a log scale (0..1000 positions), gain linear
const EQ_FIELDS = [
  { f: "fc", lab: "eq.fc", aria: "eq.fc.aria", num: "eq.fcnum.aria", lo: 20, hi: 20000, step: 1, rmin: 0, rmax: 1000, rstep: 1,
    pos: v => 1000 * Math.log(v / 20) / Math.log(1000), val: p => 20 * Math.pow(1000, p / 1000), round: v => v < 100 ? Math.round(v * 10) / 10 : Math.round(v) },
  { f: "gain", lab: "eq.gain", aria: "eq.gain.aria", num: "eq.gainnum.aria", lo: -24, hi: 24, step: 0.1, rmin: -24, rmax: 24, rstep: 0.1,
    pos: v => v, val: p => p, round: v => Math.round(v * 10) / 10 },
  { f: "q", lab: "eq.q", aria: "eq.q.aria", num: "eq.qnum.aria", lo: 0.1, hi: 10, step: 0.05, rmin: 0, rmax: 1000, rstep: 1,
    pos: v => 1000 * Math.log(v / 0.1) / Math.log(100), val: p => 0.1 * Math.pow(100, p / 1000), round: v => Math.round(v * 100) / 100 },
];
function eqBandRow(i) {
  const b = eq.bands[i], row = eqEl("div", "eqb"), head = eqEl("div", "eqh");
  const idx = eqEl("strong"); idx.textContent = i + 1;
  const sel = eqEl("select", "viz-select");
  EQ_TYPES.forEach((k, v) => { const o = eqEl("option"); o.value = v; setT(o, k); sel.appendChild(o); });
  sel.value = b.type; eqAria(sel, "eq.type.aria", i + 1);
  sel.onchange = () => { b.type = +sel.value; eqDraw(); eqSend(i, "type"); };
  const rm = eqEl("button", "rm ctl"); rm.textContent = "✕"; eqAria(rm, "eq.remove", i + 1); rm.onclick = () => eqRemove(i);
  head.append(idx, sel, rm); row.appendChild(head);
  for (const o of EQ_FIELDS) {
    const wrap = eqEl("div", "eqf"), lab = eqEl("span", "mut"), r = eqEl("input"), nu = eqEl("input"); setT(lab, o.lab);
    r.type = "range"; r.min = o.rmin; r.max = o.rmax; r.step = o.rstep; r.value = o.pos(b[o.f]); eqAria(r, o.aria, i + 1);
    nu.type = "number"; nu.min = o.lo; nu.max = o.hi; nu.step = o.step; nu.value = b[o.f]; nu.inputMode = "decimal"; eqAria(nu, o.num, i + 1);
    r.oninput = () => { b[o.f] = o.round(Math.min(o.hi, Math.max(o.lo, o.val(+r.value)))); nu.value = b[o.f]; eqDraw(); };
    r.onchange = () => eqSend(i, o.f);
    nu.oninput = () => { const x = parseFloat(nu.value); if (isFinite(x)) { b[o.f] = Math.min(o.hi, Math.max(o.lo, x)); r.value = o.pos(b[o.f]); eqDraw(); } };
    nu.onchange = () => { const x = parseFloat(nu.value); if (isFinite(x)) { b[o.f] = Math.min(o.hi, Math.max(o.lo, x)); eqSend(i, o.f); } nu.value = b[o.f]; r.value = o.pos(b[o.f]); eqDraw(); };
    wrap.append(lab, r, nu); row.appendChild(wrap);
  }
  return row;
}
function eqRenderBands() {
  eq.list.textContent = "";
  for (let i = 0; i < eq.count; i++) eq.list.appendChild(eqBandRow(i));
  if (!eq.count) { const e = eqEl("div", "mut"); setT(e, "eq.empty"); eq.list.appendChild(e); }
  setT(eq.title, "eq.bands.title", { n: eq.count, max: EQ.MAX_BANDS });
  eq.addBtn.disabled = !app.connected || eq.count >= EQ.MAX_BANDS;
  eqDraw();
}
async function eqAdd() {
  if (eq.count >= EQ.MAX_BANDS) return;
  const i = eq.count++; eq.bands[i] = { type: 0, fc: 1000, gain: 0, q: 1 };
  eqRenderBands(); await eqSendBand(i); await eqSendCount();
}
async function eqRemove(k) {
  for (let j = k; j < eq.count - 1; j++) eq.bands[j] = { ...eq.bands[j + 1] };
  eq.count--; eqRenderBands();
  for (let j = k; j < eq.count; j++) await eqSendBand(j);
  await eqSendCount();
  await loadFx();   // one normal round trip: re-reads the box's own values and flushes anything it said
}
async function eqReset() {
  eq.count = 0; eq.preamp = 0; if (eq.preSl) { eq.preSl.value = 0; eq.preShow(0); }
  eqRenderBands(); await eqSendCount(); await send("fx set equalizer preamp_db 0");
}
async function eqImport(text, btn, note) {
  const r = EQ.parseAutoEq(text);
  if (!r.bands.length) { eqNote = { key: "eq.imp.none" }; setT(note, eqNote.key); return; }
  btn.disabled = true; setT(note, "eq.imp.busy");
  await sendFast("fx set equalizer preamp_db " + (r.preamp === null ? 0 : r.preamp));
  for (let i = 0; i < r.bands.length; i++) {
    const b = r.bands[i];
    await sendFast(`fx set equalizer b${i}_type ${b.type}`); await sendFast(`fx set equalizer b${i}_fc ${b.fc}`);
    await sendFast(`fx set equalizer b${i}_gain_db ${b.gain}`); await sendFast(`fx set equalizer b${i}_q ${b.q}`);
  }
  await sendFast("fx set equalizer count " + r.bands.length);
  await sendFast("fx on equalizer 1");
  eqNote = r.overflow ? { key: "eq.imp.over", p: { n: r.bands.length, m: r.skipped, max: EQ.MAX_BANDS, k: r.overflow } } : { key: "eq.imp.done", p: { n: r.bands.length, m: r.skipped } };
  await loadFx(); loadNight();
}
function eqBuild() {
  const rows = eq.rows;
  eq.cv = eqEl("canvas", "eqg"); eq.cv.setAttribute("role", "img"); eq.cv.setAttribute("data-i18n-aria", "eq.graph.aria"); eq.cv.setAttribute("aria-label", t("eq.graph.aria"));
  if (window.ResizeObserver) new ResizeObserver(() => eqDraw()).observe(eq.cv);
  const bar = eqEl("div", "eqbar"); eq.title = eqEl("strong");
  eq.addBtn = eqEl("button", "ctl"); setT(eq.addBtn, "eq.add"); eq.addBtn.onclick = eqAdd;
  const rs = eqEl("button", "ctl"); setT(rs, "eq.reset"); rs.onclick = eqReset;
  bar.append(eq.title, eq.addBtn, rs);
  eq.list = eqEl("div");
  const det = eqEl("details", "eqimp"), sum = eqEl("summary"), ta = eqEl("textarea"), ib = eqEl("button", "ctl"), note = eqEl("div", "mut eqnote");
  setT(sum, "eq.imp.title"); ta.rows = 6; ta.spellcheck = false; ta.placeholder = "Preamp: -6.1 dB\nFilter 1: ON PK Fc 105 Hz Gain -3.2 dB Q 0.70"; ta.setAttribute("data-i18n-aria", "eq.imp.aria"); ta.setAttribute("aria-label", t("eq.imp.aria"));
  setT(ib, "eq.imp.btn"); ib.onclick = () => eqImport(ta.value, ib, note);
  if (eqNote) { setT(note, eqNote.key, eqNote.p); det.open = true; }
  det.append(sum, ta, ib, note);
  rows.append(eq.cv, bar, eq.list, det);
  eqRenderBands();
}

export function initFx() {
  $("fxOff").onclick = async () => { await send("fx off"); loadFx(); loadNight(); };
  const notes = () => {
    const state = app.state;
    $("fxCast").hidden = !(app.link === "on" && (app.selOut === "cast" || (state && state.out === "cast")));
    $("fxShed").hidden = !(app.link === "on" && state && state.fxshed);
  };
  on("cast", notes); on("status", notes);
  on("lang", fxLabels);
  window.addEventListener("resize", eqDraw);
  if (window.matchMedia) window.matchMedia("(prefers-color-scheme: dark)").addEventListener("change", eqDraw);
}
export { loadFx, eqDraw };
// how many effects are switched on (for the one-line state in the settings list); null before the list is loaded
export const fxOnCount = () => { const b = $("fxBody").querySelectorAll(".fxapp input[type=checkbox]"); return b.length ? [...b].filter(c => c.checked).length : null; };
