// SPDX-License-Identifier: LGPL-2.1-or-later
// Shared state and tiny helpers. Every module that needs "the current status" or "is the link up" reads it from `app`, and the modules
// talk to each other through on()/emit() (events: status, linkui, linkoff, endlink, route, cast, lang) or by importing a function.
import { I18N } from "./i18n.js";

export const $ = id => document.getElementById(id);
export const sleep = ms => new Promise(r => setTimeout(r, ms));
export const DEMO = new URLSearchParams(location.search).has("demo");
export const { t, setT, setRaw } = I18N;

export const app = {
  state: null,            // the last parsed `st` reply, null while not connected
  link: "off",            // "off" (not connected) | "on" | "reconnecting" (the box dropped the link; the page keeps trying)
  connected: false,       // link === "on"
  dev: null, chCmd: null, chEvt: null, chSpec: null,
  userDisc: false, castStarting: false,
  cwd: "/sdcard",         // the folder shown on the Music page
  selOut: "",             // the output selected on the box ("" until `out` has been read)
  scanBusy: false,        // a Wi-Fi scan is running: the box handles no commands meanwhile, so the page's own polling pauses
  target: new Uint8Array(32),   // latest spectrum bands from the box, read by the visualizer
  viz: null,              // set when the visualizer has been mounted
  radio: null,            // set when the station finder has been mounted
  vmaxUntil: 0, gapUntil: 0,    // after the user's own change a poll that was already in flight must not move the control back
  route: "",              // the current route key, e.g. "now", "settings/wifi" (router.js)
  ui: {},                 // the mounted section modules: card, wifi, time (lib/*.js)
};

const handlers = new Map();
export function on(ev, fn) { if (!handlers.has(ev)) handlers.set(ev, []); handlers.get(ev).push(fn); }
export function emit(ev, ...args) { for (const fn of (handlers.get(ev) || []).slice()) fn(...args); }

// ---- small UI helpers ----
export function segSet(id, v) { for (const b of $(id).children) b.classList.toggle("on", b.dataset.v === String(v)); }
export function segBind(id, fn) { for (const b of $(id).children) b.onclick = () => { segSet(id, b.dataset.v); fn(b.dataset.v); }; }
export function emptyLi(key) { const li = document.createElement("li"); li.className = "mut"; li.style.cursor = "default"; setT(li, key); return li; }
export const MODES = ["folder", "shuffle", "one", "once"], RGS = ["off", "track", "album"];
// a value the box sent, shown in words when the page knows it (prefix + the value as a key), else as received
export const known = (prefix, v) => { const k = prefix + String(v).toLowerCase().replace(/[^a-z]+/g, "_").replace(/^_|_$/g, ""); return I18N.has(k) ? t(k) : String(v); };
