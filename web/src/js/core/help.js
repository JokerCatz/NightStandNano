// SPDX-License-Identifier: LGPL-2.1-or-later
// The explanations: every page marks its explanations with class "help"; a page shows them while it has class "help-on".
// Global switch (Settings > Language & help) remembered in localStorage "nsn.help" ("1" / "0"); a first visit has no value and shows them,
// with a notice (once) that they can be hidden. The "?" button in a page header overrides the switch for that page until the page is reloaded.
// Warnings about consequences are not marked "help" and are never hidden.
import { $ } from "./app.js";

const KEY = "nsn.help";
const read = () => { try { const v = localStorage.getItem(KEY); return v === "1" ? true : v === "0" ? false : null; } catch { return null; } };
const write = v => { try { localStorage.setItem(KEY, v ? "1" : "0"); } catch { /* not kept */ } };
let all = read() !== false;   // the switch (unset = on)
const over = new Map();       // page key -> true / false: the "?" button of that page, memory only

export const helpAll = () => all;
export function setHelpAll(v) { all = v; write(v); over.clear(); applyAll(); $("helpNotice").hidden = true; const sw = $("helpSw"); if (sw) sw.checked = v; }
const pageOn = key => over.has(key) ? over.get(key) : all;
function applyPage(el) {
  const key = el.dataset.page, onNow = pageOn(key);
  el.classList.toggle("help-on", onNow);
  const b = el.querySelector(".hbtn"); if (b) b.setAttribute("aria-pressed", String(onNow));
}
function applyAll() { for (const el of document.querySelectorAll(".page")) applyPage(el); }

export function initHelp() {
  applyAll();
  $("helpSw").checked = all;
  $("helpSw").onchange = () => setHelpAll($("helpSw").checked);
  $("helpNotice").hidden = read() !== null;
  $("helpOk").onclick = () => setHelpAll(all);
  $("helpOff").onclick = () => setHelpAll(false);
  for (const el of document.querySelectorAll(".page")) { const b = el.querySelector(".hbtn"); if (b) b.onclick = () => { over.set(el.dataset.page, !pageOn(el.dataset.page)); applyPage(el); }; }
}
