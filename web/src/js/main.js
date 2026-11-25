// SPDX-License-Identifier: LGPL-2.1-or-later
// NightStand Nano control page: the entry. Order matters: the string table and i18n first (the libraries read NSN_I18N), then the
// pages in the order their status handlers must run, then the router, then (only with ?demo) the fake box, which connects by itself.
import { I18N } from "./core/i18n.js";
import { DEMO } from "./core/app.js";
import { hydrateIcons } from "./core/icons.js";
import { initLink } from "./core/link.js";
import { initNow } from "./pages/now.js";
import { initLyrics } from "./pages/lyrics.js";
import { initDiag } from "./pages/diag.js";
import { initDevice } from "./pages/device.js";
import { initMusic } from "./pages/music.js";
import { initRadio } from "./pages/radio.js";
import { initAmbient } from "./pages/ambient.js";
import { initFx } from "./pages/fx.js";
import { initNight } from "./pages/night.js";
import { initKeys } from "./pages/keys.js";
import { initOutput } from "./pages/set-output.js";
import { initLed } from "./pages/set-led.js";
import { initFw } from "./pages/fw.js";
import { initSettings } from "./pages/settings.js";
import { initLang } from "./pages/lang.js";
import { initBar } from "./core/bar.js";
import { initHelp } from "./core/help.js";
import { initRouter, demoRoute } from "./core/router.js";
import { initSession } from "./core/session.js";

I18N.init();
hydrateIcons();
initLink();
initNow();         // status: visualizer, title, status line, output
initLyrics();      // status: the lyrics fetch comes before the card / Wi-Fi handlers, as it always did
initDiag();
initDevice();      // status: card, then Wi-Fi
initMusic();
initRadio();
initAmbient();
initFx();
initNight();
initKeys();
initOutput();
initLed();
initFw();
initBar();
initLang();
initSettings();
initHelp();
initSession();
if (DEMO && !location.hash) { const r = demoRoute(new URLSearchParams(location.search).get("demo")); if (r) location.hash = "#/" + r; }
initRouter();
// Enter or Space on a list row (files, scenes, Wi-Fi networks) acts like a click
document.addEventListener("keydown", e => { if ((e.key === "Enter" || e.key === " ") && e.target.tagName === "LI") { e.preventDefault(); e.target.click(); } });
if (DEMO) import("./demo/box.js").then(m => m.startDemo());
