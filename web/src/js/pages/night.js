// SPDX-License-Identifier: LGPL-2.1-or-later
// #/settings/night: night mode on / off and its daily schedule (the schedule needs a valid clock: see lib/time.js).
import { $, segSet, segBind, setT } from "../core/app.js";
import { send } from "../core/link.js";
import { loadFx } from "./fx.js";

let nightRep = null;   // { on, sched: [from, to] | null } from the last `night` reply
export const nightInfo = () => nightRep;

export async function loadNight() {
  const r = await send("night"), l = r.find(x => x.startsWith("night:")) || "";
  if (!l) return;   // no reply (the link dropped)
  const on = /night: on/.test(l);
  segSet("night", on ? "on" : "off");
  const m = l.match(/schedule (\d\d:\d\d)-(\d\d:\d\d)/);
  nightRep = { on, sched: m ? [m[1], m[2]] : null };
  if (m) setT($("nightv"), "night.sched", { from: m[1], to: m[2] }); else setT($("nightv"), "night.nosched");
  if (m) { $("nFrom").value = m[1]; $("nTo").value = m[2]; }
}

export function initNight() {
  segBind("night", v => send("night " + v).then(() => { loadNight(); loadFx(); }));
  $("btnNight").onclick = async () => { if ($("nFrom").value && $("nTo").value) { await send(`night auto ${$("nFrom").value} ${$("nTo").value}`); loadNight(); } };
  $("btnNightOff").onclick = async () => { await send("night auto off"); loadNight(); };
}
