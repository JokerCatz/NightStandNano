// SPDX-License-Identifier: LGPL-2.1-or-later
// The box's recent events, fetched incrementally (`events` the first time, then `events <next>`) with the quiet send (nothing in the command log).
// One page-side list (EV) and one fetch function serve both Diagnostics (renders the list) and Music (shows a skip notice), so the sequence
// handling exists once and two pages never fetch at the same time. Whoever calls fetchEvents decides when (route, visibility, playing).
import { app, emit } from "./app.js";
import { sendQuiet } from "./link.js";
import { newEventState, eventsCmd, applyEvents } from "../lib/events.js";

export const EV = newEventState();
let busy = false, again = false;
// Fetches what is new; emits "events" ({ added: [{ id, at, text }] newest first, restart, force }) after a reply with news (or any reply when forced).
// A call while one is in flight is dropped (force: one more fetch right after it).
export async function fetchEvents(force) {
  if (app.scanBusy || app.link !== "on" || document.hidden) return;
  if (busy) { if (force === true) again = true; return; }
  busy = true;
  try {
    for (let i = 0; i < 2; i++) {   // the second round: the box's sequence restarted, ask again from the start
      const r = await sendQuiet(eventsCmd(EV)), now = Date.now();
      if (!r.length) break;
      const res = applyEvents(EV, r, now);
      if (res.added || res.restart || force) emit("events", { added: EV.list.slice(0, res.added), restart: res.restart, force: force === true });
      if (!res.restart) break;
    }
  } finally { busy = false; if (again) { again = false; fetchEvents(true); } }
}
