// SPDX-License-Identifier: LGPL-2.1-or-later
// The three device pages built by the section modules: Wi-Fi (lib/wifi.js), the clock (lib/time.js) and the memory card (lib/card.js),
// the last one with the warning banner. Each module keeps its own polling and only works while its page is the current route.
import { app, $, on } from "../core/app.js";
import { send, sendUntil, log } from "../core/link.js";
import { mount as mountCard } from "../lib/card.js";
import { mount as mountWifi } from "../lib/wifi.js";
import { mount as mountTime } from "../lib/time.js";

export function initDevice() {
  const isOn = () => app.link === "on", getState = () => app.state, here = r => () => app.route === r;
  app.ui.card = mountCard({ box: $("cardBox"), banner: $("cardWarn"), send, getState, isOn, tabOpen: here("settings/card"), openTab: () => { location.hash = "#/settings/card"; }, log });
  app.ui.wifi = mountWifi({ box: $("wifiBox"), send, sendUntil, getState, isOn, tabOpen: here("settings/wifi"), onBusy: b => { app.scanBusy = b; } });
  app.ui.time = mountTime({ box: $("timeBox"), send, isOn, tabOpen: here("settings/time"), log });
  on("status", state => { app.ui.card.onState(state); app.ui.wifi.onState(state); });
  on("linkoff", () => { app.ui.card.reset(); app.ui.wifi.reset(); app.ui.time.reset(); });
  on("lang", () => { app.ui.card.rerender(); app.ui.wifi.rerender(); app.ui.time.rerender(); });
}
