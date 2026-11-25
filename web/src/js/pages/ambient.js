// SPDX-License-Identifier: LGPL-2.1-or-later
// #/ambient: the one-tap sleep preset, noise colours and scenes (layered ambient sounds from the card).
import { app, $, emptyLi, setT } from "../core/app.js";
import { send } from "../core/link.js";
import { refreshStatus } from "../core/status.js";
import { loadNight } from "./night.js";
import { loadFx } from "./fx.js";

export async function loadScenes() {
  const r = await send("scene"), ul = $("scenes"); if (app.link !== "on" && !r.length) return; ul.textContent = "";
  for (const l of r) {
    const m = l.match(/^(\d+) (.+)$/); if (!m) continue;
    const li = document.createElement("li"); li.tabIndex = 0; li.textContent = m[2];
    li.onclick = () => send("scene " + m[1]).then(refreshStatus);
    ul.appendChild(li);
  }
  if (!ul.children.length) ul.appendChild(emptyLi("scenes.none"));
}

export function initAmbient() {
  for (const c of ["white", "pink", "brown", "blue", "violet", "grey", "green", "velvet"]) {
    const b = document.createElement("button"); b.className = "ctl"; b.disabled = true; setT(b, "noise." + c); b.onclick = () => send("noise " + c).then(refreshStatus);
    $("noise").appendChild(b);
  }
  $("btBed").onclick = async () => { await send("bedtime 30"); refreshStatus(); loadNight(); loadFx(); };
  $("btWake").onclick = async () => { await send("wake"); refreshStatus(); loadNight(); loadFx(); };
}
