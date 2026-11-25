// SPDX-License-Identifier: LGPL-2.1-or-later
// What is read from the box after every (re)connect, and what is read again when a page is opened (the route's loaders).
// The order of loadOnce() is the order the commands are sent in.
import { app, on } from "./app.js";
import { ls } from "../pages/music.js";
import { loadOut } from "../pages/now.js";
import { loadNight } from "../pages/night.js";
import { loadStations } from "../pages/radio.js";
import { loadScenes } from "../pages/ambient.js";
import { loadFx, eqDraw } from "../pages/fx.js";
import { loadKeys } from "../pages/keys.js";
import { lyrScroll } from "../pages/lyrics.js";
import { drawSparks } from "../pages/diag.js";
import { loadRatetest } from "../pages/set-output.js";
import { loadFw } from "../pages/fw.js";

function loadOnce() { app.ui.time.sync(); app.ui.wifi.load(); loadOut(); loadNight(); loadStations(); loadScenes(); loadFx(); loadKeys(); app.ui.card.load(); loadFw(); }
export async function afterConnectLoads() { await ls(app.cwd); loadOnce(); }

// opening a page: draw what needs a visible canvas / scroll position, then (when connected) reload what that page shows
const ENTER = {
  "now/lyrics": () => { lyrScroll(); },
  "radio": () => loadStations(),
  "ambient": () => loadScenes(),
  "settings/night": () => loadNight(),
  "settings/output": () => loadRatetest(),
  "settings/fx": () => { eqDraw(); return loadFx(); },
  "settings/diag": () => { drawSparks(); },
  "settings/keys": () => loadKeys(),
  "settings/card": () => app.ui.card.load(),
  "settings/wifi": () => app.ui.wifi.load(),
  "settings/time": () => app.ui.time.load(),
};
// the drawing part is not gated on the connection, the loading part is (as it always was: `if (!chCmd) return`)
const DRAW_ONLY = new Set(["now/lyrics", "settings/diag"]);
export function initSession() {
  on("route", (key, was) => {
    if (key === was) return;
    const f = ENTER[key]; if (!f) return;
    if (!app.chCmd && !DRAW_ONLY.has(key)) { if (key === "settings/fx") eqDraw(); return; }
    f();
  });
}
