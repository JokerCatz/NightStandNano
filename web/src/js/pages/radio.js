// SPDX-License-Identifier: LGPL-2.1-or-later
// #/radio: the station state, the favourites on the card and the station search (lib/radio.js), and the direct-URL box.
import { app, $, on, known, t, setT, setRaw } from "../core/app.js";
import { send, log } from "../core/link.js";
import { refreshStatus } from "../core/status.js";
import { mountRadio } from "../lib/radio.js";

export const loadStations = () => app.radio ? app.radio.reload() : Promise.resolve();   // the favourites are the card's radio.txt

const radioUrl = () => { const u = $("radioUrl").value.trim(); if (/^https?:\/\/[^\s]+$/i.test(u)) return u; setT($("radioNote"), "url.bad"); return ""; };
function renderRstate() { const s = app.state; if (s) setRaw($("rstate"), s.rstate ? known("rs.", s.rstate) : t("radio.stateNone")); }

export function initRadio() {
  try { app.radio = mountRadio({ favBox: $("favBox"), findBox: $("findBox"), send, refresh: refreshStatus, connected: () => !!app.chCmd }); }
  catch (e) { log("radio: " + e); setT($("findBox"), "viz.needHttp"); setRaw($("favBox"), ""); }
  $("btnRadio").onclick = async () => { const u = radioUrl(); if (u) { setRaw($("radioNote"), ""); await send("radio " + u); refreshStatus(); } };
  $("btnRadioFav").onclick = async () => { const u = radioUrl(); if (!u) return; if (!app.radio) { setT($("radioNote"), "url.noFav"); return; }
    const r = await app.radio.addUrl(u); setT($("radioNote"), r.key, r.params); $("radioNote").classList.toggle("bad", !r.ok); };
  on("status", renderRstate);
  on("lang", renderRstate);
}
