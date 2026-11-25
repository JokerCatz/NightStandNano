// SPDX-License-Identifier: LGPL-2.1-or-later
// #/settings/keys: the wired headset's buttons. `keys` / `keys preset N` / `keys set <c|p|m> <1|2|3|h> <action>`.
import { app, $, t, setT, setRaw } from "../core/app.js";
import { send, log } from "../core/link.js";

//   keys c 1=play 2=next 3=prev h=none / keys p ... / keys m ... / keys last c2 next 3 s ago  (or: keys last none)
const KEY_BTNS = ["c", "p", "m"], KEY_GS = ["1", "2", "3", "h"];
export const KEY_ACTS = { none: "keys.a.none", play: "keys.a.play", next: "keys.a.next", prev: "keys.a.prev", "vol+": "keys.a.volup", "vol-": "keys.a.voldn", stop: "keys.a.stop" };
const KEY_PRESETS = {   // what `keys preset N` loads (firmware); only used to highlight the matching button
  1: { c: "play next prev none", p: "none none none none", m: "none none none none" },
  2: { c: "none none none none", p: "vol+ next none vol+", m: "vol- prev none vol-" },
  3: { c: "play next prev none", p: "vol+ none none vol+", m: "vol- none none vol-" } };
let keyMap = null, keySel = {}, keyPollBusy = false, keyMatch = null;   // keyMatch: the preset (1..3) the box's map equals, 0 = custom, null = not read yet
function buildKeys() {
  const grid = $("keyGrid");
  for (const b of KEY_BTNS) {
    const row = document.createElement("div"); row.className = "keyrow";
    const h = document.createElement("strong"); setT(h, "keys.btn." + b); row.appendChild(h);
    const cells = document.createElement("div"); cells.className = "keycells";
    for (const g of KEY_GS) {
      const lab = document.createElement("label"), sp = document.createElement("span"), sel = document.createElement("select");
      setT(sp, "keys.g." + g);
      sel.className = "viz-select ctl"; sel.disabled = true;
      sel.setAttribute("data-i18n-aria", "keys.sel"); sel.setAttribute("data-i18n-p", JSON.stringify({ b: { t: "keys.btn." + b }, g: { t: "keys.g." + g } }));
      sel.setAttribute("aria-label", t("keys.sel", { b: { t: "keys.btn." + b }, g: { t: "keys.g." + g } }));
      for (const [a, k] of Object.entries(KEY_ACTS)) { const o = document.createElement("option"); o.value = a; setT(o, k); sel.appendChild(o); }
      sel.onchange = () => keysCmd(`keys set ${b} ${g} ${sel.value}`);
      keySel[b + g] = sel; lab.append(sp, sel); cells.appendChild(lab);
    }
    row.appendChild(cells); grid.appendChild(row);
  }
  for (const btn of $("keyPresets").children) btn.onclick = () => keysCmd("keys preset " + btn.dataset.n);
}
function parseKeys(r) {
  const map = {};
  for (const l of r) {
    const m = /^keys ([cpm])((?: [123h]=\S+)+)\s*$/.exec(l); if (!m) continue;
    map[m[1]] = {}; for (const x of m[2].trim().split(/\s+/)) map[m[1]][x[0]] = x.slice(2);
  }
  const last = r.find(l => /^keys last\b/.test(l)) || "";
  const lm = /^keys last ([cpm])([123h]) (\S+) (\d+) s ago/.exec(last);
  return { map: KEY_BTNS.every(b => map[b]) ? map : null, last: lm ? { b: lm[1], g: lm[2], a: lm[3], s: +lm[4] } : last ? "none" : null };
}
const keyName = (pre, v, known) => known ? { t: pre + v } : v;   // an unknown token is shown as received
function renderKeyLast(last) {
  if (last === null) return;
  if (last === "none") { setT($("keyLast"), "keys.last.none"); return; }
  setT($("keyLast"), "keys.last.some", { b: keyName("keys.btn.", last.b, true), g: keyName("keys.g.", last.g, true),
    a: last.a in KEY_ACTS ? { t: KEY_ACTS[last.a] } : last.a, s: last.s });
}
function renderKeys(p) {
  if (p.map) {
    keyMap = p.map;
    for (const b of KEY_BTNS) for (const g of KEY_GS) {
      const sel = keySel[b + g], v = p.map[b][g] || "none";
      for (const o of [...sel.options]) if (o.dataset.extra) o.remove();
      if (!(v in KEY_ACTS)) { const o = document.createElement("option"); o.value = v; o.dataset.extra = "1"; setRaw(o, v); sel.appendChild(o); }
      sel.value = v;
    }
    let match = 0;
    for (const n of [1, 2, 3]) if (KEY_BTNS.every(b => KEY_GS.every((g, i) => p.map[b][g] === KEY_PRESETS[n][b].split(" ")[i]))) match = n;
    for (const btn of $("keyPresets").children) btn.classList.toggle("on", +btn.dataset.n === match);
    $("keyCustom").hidden = !!match; keyMatch = match;
  }
  renderKeyLast(p.last);
}
async function keysCmd(cmd) {
  const r = await send(cmd); if (!r.length) return;
  const p = parseKeys(r);
  if (!p.map) { log("keys: " + (r[0] || "")); loadKeys(); return; }
  renderKeys(p);
}
export async function loadKeys() {
  const r = await send("keys"); if (!r.length) return;
  renderKeys(parseKeys(r));
}
async function pollKeysLast() {
  if (keyPollBusy || app.scanBusy || app.link !== "on" || document.hidden || app.route !== "settings/keys") return;
  keyPollBusy = true;
  try { const r = await send("keys"); if (r.length) renderKeyLast(parseKeys(r).last); } finally { keyPollBusy = false; }
}
export const keysMatch = () => keyMatch;
export function initKeys() {
  buildKeys();
  setInterval(pollKeysLast, 2000);
}
