// SPDX-License-Identifier: LGPL-2.1-or-later
// #/settings: the grid of settings pages. Each entry has an icon, the page's name and one short line of its current state; an entry with a
// problem (card unreadable or missing, wrong Wi-Fi password, no clock) gets a badge and says so in that line.
import { app, $, on, t, setT, setRaw } from "../core/app.js";
import { I18N } from "../core/i18n.js";
import { fmtBytes } from "../lib/card.js";
import { fxOnCount } from "./fx.js";
import { nightInfo } from "./night.js";
import { keysMatch } from "./keys.js";
import { BUILD } from "./diag.js";
import { helpAll } from "../core/help.js";
import { ledDisabled } from "./fw.js";
import { khz } from "../lib/fw.js";

const LANG_NAMES = { en: "English", "zh-TW": "繁體中文", "zh-CN": "简体中文" };
// level: "" | "warn" | "bad" (a badge appears for the last two). what: [key, params] (follows the language), a string (as is) or null (empty)
function ent(id, what, level = "") {
  const a = $("ent-" + id), small = a.querySelector("small"), badge = a.querySelector(".badge");
  if (Array.isArray(what)) setT(small, what[0], what[1]); else setRaw(small, what || "");
  a.classList.toggle("warn", level === "warn"); a.classList.toggle("bad", level === "bad");
  badge.hidden = !level;
}

function render() {
  const st = app.state, on_ = app.link === "on" && !!st, off = ["conn.off"];
  // sound
  ent("output", on_ && typeof st.vmax === "number" ? (st.dacmax > 0 ? ["ent.output.rate", { n: st.vmax, r: khz(st.dacmax) }] : ["ent.output", { n: st.vmax }]) : on_ ? null : off);
  const n = fxOnCount();
  ent("fx", !on_ ? off : st.fxshed ? ["ent.fx.shed"] : n === null ? null : n ? ["ent.fx.n", { n }] : ["ent.fx.none"]);
  const ni = nightInfo();
  ent("night", !on_ ? off : !ni ? null : ["ent.night", { s: { t: ni.on ? "night.on" : "night.off" }, r: ni.sched ? ni.sched[0] + "–" + ni.sched[1] : { t: "night.nosched" } }]);
  // device
  const w = app.ui.wifi && app.ui.wifi.info();
  if (!on_) ent("wifi", off);
  else if (!w) ent("wifi", null);
  else if (w.state === "connected") ent("wifi", ["ent.wifi.on", { ssid: w.ssid }]);
  else {
    const k = "wifi.state." + w.state.toLowerCase().replace(/[^a-z]+/g, "_").replace(/^_|_$/g, "");
    ent("wifi", I18N.has(k) ? [k] : w.state, /wrong|not found|no credentials/i.test(w.state) ? (/no credentials/i.test(w.state) ? "warn" : "bad") : "");
  }
  const T = app.ui.time && app.ui.time.info(), clock = app.ui.time && app.ui.time.clock();
  if (!on_) ent("time", off);
  else if (!T) ent("time", null);
  else if (T.src === "none") ent("time", ["time.src.none"], "warn");
  else ent("time", ["ent.time", { t: clock ? clock.slice(11, 16) : "—", src: { t: "time.src." + T.src } }]);
  const ck = st && st.card, c = app.ui.card && app.ui.card.info();
  if (!on_) ent("card", off);
  else if (ck === "none") ent("card", ["ent.card.none"], "warn");
  else if (ck === "bad") ent("card", ["ent.card.bad"], "bad");
  else if (c && c.state === "mounted" && c.free !== null) ent("card", ["ent.card.free", { free: fmtBytes(c.free) }]);
  else ent("card", ck === "ok" ? ["card.checking"] : null);
  const km = keysMatch();
  ent("keys", !on_ ? off : km === null ? null : km ? ["keys.preset." + km] : ["keys.custom"]);
  ent("led", [!on_ ? "conn.off" : ledDisabled() ? "hw.led.disabled" : "ent.led"]);
  // display and diagnostics
  const mode = app.viz && app.viz.host.modes().find(m => m.id === app.viz.host.mode());
  ent("viz", mode ? [mode.labelKey] : null);
  ent("lang", [helpAll() ? "ent.lang.on" : "ent.lang.off", { l: LANG_NAMES[I18N.lang()] }]);
  ent("diag", on_ ? ["ent.diag", { kb: Math.round(st.mem / 1024), n: st.und }] : t("diag.build") + " " + BUILD);
}

export function initSettings() {
  on("status", render); on("linkoff", render); on("lang", render); on("fw", render);
  on("route", key => { if (key === "settings") render(); });
  render();
}
export const renderSettings = render;
