// SPDX-License-Identifier: LGPL-2.1-or-later
// The box's recent events (`events` / `events <seq>`): parser, the page-side list and the readable form of an entry.
//   ev <seq> <age s> <text>   one per entry, oldest first;   ev end <next seq>   last line.
// Texts: `wired <button><gesture> <action>`, `bt 0x<hex> <action>`, `skip <file>: too heavy for <output>`, `skip <file>: chip <names>`;
// anything else is shown as received.
import { outParam, chipDetail, heavyDetail } from "./fw.js";
export const MAX_EVENTS = 100;

// AVRCP passthrough operation ids the page names (the rest is shown as hex)
export const AVRCP_NAMES = { 0x41: "volup", 0x42: "voldn", 0x43: "mute", 0x44: "play", 0x45: "stop", 0x46: "pause", 0x48: "rew", 0x49: "ff", 0x4b: "fwd", 0x4c: "bwd" };

// lines -> { entries: [{ seq, age, text }], next } (next = null when the reply has no `ev end` line)
export function parseEvents(lines) {
  const entries = []; let next = null;
  for (const l of lines || []) {
    let m;
    if ((m = /^ev end (\d+)\s*$/.exec(l))) next = +m[1];
    else if ((m = /^ev (\d+) (\d+) (.*\S)\s*$/.exec(l))) entries.push({ seq: +m[1], age: +m[2], text: m[3] });
  }
  return { entries, next };
}

// text -> { kind: "wired", b, g, a } | { kind: "bt", code, name, a } | { kind: "skip", file, out } (too heavy) | { kind: "skip", file, chips, mixed } | { kind: "raw", text }
export function describeEvent(text) {
  let m;
  if ((m = /^skip (.+?): (too heavy for \S+)$/.exec(text)) && heavyDetail(m[2])) return { kind: "skip", file: m[1], out: heavyDetail(m[2]) };
  if ((m = /^skip (.+): (chip\s+.+)$/.exec(text))) { const c = chipDetail(m[2]); if (c) return { kind: "skip", file: m[1], chips: c.chips, mixed: c.mixed }; }
  if ((m = /^wired ([cpm])([123h]) (\S+)$/.exec(text))) return { kind: "wired", b: m[1], g: m[2], a: m[3] };
  if ((m = /^bt 0x([0-9a-fA-F]{1,4}) (\S+)$/.exec(text))) {
    const n = parseInt(m[1], 16);
    return { kind: "bt", code: "0x" + n.toString(16).toUpperCase().padStart(2, "0"), name: AVRCP_NAMES[n] || null, a: m[2] };
  }
  return { kind: "raw", text };
}

// a skip description -> { key, p } for t(key, p); sep joins the chip names. An output other than dac gets the hint that a lighter output (the DAC) can play it.
export function skipView(d, sep) {
  const p = { f: d.file };
  if (d.chips) { p.c = d.chips.join(sep); return { key: d.mixed ? "ev.skip.chipMixed" : "ev.skip.chip", p }; }
  p.o = outParam(d.out); p.d = { t: "out.dac" };
  return { key: d.out === "dac" ? "ev.skip.heavy" : "ev.skip.heavyTry", p };
}

// The page-side list: newest first, at most MAX_EVENTS. state = { next: null | number, list: [{ id, at, text }] }.
export const newEventState = () => ({ next: null, list: [], id: 0 });
// the argument of the next poll: "events" the first time (and after a restart), else "events <next>"
export const eventsCmd = (st) => st.next == null ? "events" : "events " + st.next;
// Applies a reply made at `now` (ms). Returns { added, restart }: restart = the box's sequence went back (it rebooted), so nothing
// of this reply is trusted as "new since next"; the caller asks again with a plain `events` (st.next is reset for that).
export function applyEvents(st, lines, now) {
  const { entries, next } = parseEvents(lines);
  if (next == null) return { added: 0, restart: false };   // no `ev end`: not an events reply
  if (st.next != null && next < st.next) { st.next = null; return { added: 0, restart: true }; }
  for (const e of entries) st.list.unshift({ id: ++st.id, at: now - e.age * 1000, text: e.text });   // entries come oldest first
  if (st.list.length > MAX_EVENTS) st.list.length = MAX_EVENTS;
  st.next = next;
  return { added: entries.length, restart: false };
}
