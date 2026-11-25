// SPDX-License-Identifier: LGPL-2.1-or-later
// What the firmware says about itself, read once per (re)connection: `ver` (firmware version) and `hw` (the pins of the card and the LED, read-only).
// Shown in Settings > Diagnostics (versions, the hardware block), on the LED page (LED disabled) and in the settings list. Also here: the entry
// on the not-connected screens that points to the flasher page (flash/, relative to this page).
import { app, $, on, emit, setT, setRaw } from "../core/app.js";
import { send } from "../core/link.js";
import { parseVer, parseHw } from "../lib/fw.js";

let ver = null, hw = null, loaded = false;   // loaded: the answers of this connection have arrived (null then means "the firmware did not say")
export const fwVersion = () => ver;
export const hwInfo = () => hw;
// is the LED switched off in the hardware settings? (null while unknown)
export const ledDisabled = () => (hw && hw.led ? hw.led.on === 0 : null);

// the order of the commands is the order they are sent in (session.js calls this last)
export async function loadFw() {
  const v = await send("ver");
  if (app.link !== "on" && !v.length) return;
  const h = await send("hw");
  if (app.link !== "on" && !h.length) return;
  ver = parseVer(v); hw = parseHw(h); loaded = true;
  render(); emit("fw");
}

// a part is { key, p } (translated, follows the language) or { raw } (as is); a line is parts joined by " · "
function line(parts, cls) {
  const d = document.createElement("div"); if (cls) d.className = cls;
  parts.forEach((p, i) => { if (i) d.append(" · "); const s = document.createElement("span"); if (p.key) setT(s, p.key, p.p); else s.textContent = p.raw; d.appendChild(s); });
  return d;
}
function row(label, lines) {
  const d = document.createElement("div"); d.className = "kv";
  const k = document.createElement("span"); k.className = "mut"; setT(k, label);
  const v = document.createElement("div"); v.className = "kvv"; v.append(...lines);
  d.append(k, v); return d;
}
const word = (prefix, v, fallbackOnly) => (new RegExp("^(" + fallbackOnly + ")$").test(v) ? { key: prefix + v } : { raw: v || "" });

function sdRow(sd) {
  const spi = sd.mode === "spi", w = sd.width === undefined ? 4 : sd.width;
  const pins = spi ? [["CLK", sd.clk], ["MOSI", sd.mosi], ["MISO", sd.miso], ["CS", sd.cs]]
    : [["CLK", sd.clk], ["CMD", sd.cmd], ...[0, 1, 2, 3].slice(0, w === 1 ? 1 : 4).map(i => ["D" + i, sd["d" + i]])];
  return row("card.title", [
    line([spi ? { key: "hw.mode.spi" } : { key: "hw.mode.sdmmc", p: { width: w } }]),
    line([{ raw: pins.filter(p => p[1] !== undefined).map(p => p[0] + " " + p[1]).join(" · ") }], "pins"),
    line([word("hw.src.", sd.src, "default|custom"), word("hw.sdstate.", sd.state, "mounted|none")], "meta")]);
}
function ledRow(l) {
  return row("hw.led", l.on === 0 ? [line([{ key: "hw.led.disabled" }])]
    : [line([{ key: "hw.led.line", p: { gpio: l.gpio, order: String(l.order || "").toUpperCase() } }]), line([word("hw.src.", l.src, "default|custom")], "meta")]);
}

function render() {
  const box = $("hwBox"); box.textContent = "";
  $("hwPending").hidden = !(hw && hw.pending > 0);
  if (hw && (hw.sd || hw.led)) { if (hw.sd) box.appendChild(sdRow(hw.sd)); if (hw.led) box.appendChild(ledRow(hw.led)); }
  else { const d = document.createElement("div"); d.className = "mut"; if (loaded) setT(d, "hw.none"); else d.textContent = "—"; box.appendChild(d); }
  if (ver) {
    setRaw($("fwVer"), ver.version);
    if (ver.idf || ver.chip || ver.built) setT($("fwDetail"), "diag.fw.detail", { idf: ver.idf || "?", chip: ver.chip || "?", built: ver.built || "?" }); else setRaw($("fwDetail"), "");
  } else { if (loaded) setT($("fwVer"), "diag.fw.none"); else setRaw($("fwVer"), "—"); setRaw($("fwDetail"), ""); }
  const off = ledDisabled() === true;   // the LED page: no brightness slider that does nothing
  $("ledOff").hidden = !off; $("ledCard").hidden = off;
}

// the entry to the flasher on the not-connected screens (Now and Settings)
const renderFlashHint = () => { $("flashHint").hidden = !(app.link === "off" && (app.route === "now" || app.route === "settings")); };

export function initFw() {
  on("linkoff", () => { ver = null; hw = null; loaded = false; render(); emit("fw"); });
  on("linkui", renderFlashHint); on("linkoff", renderFlashHint); on("route", renderFlashHint);
  on("lang", render);
  render(); renderFlashHint();
}
