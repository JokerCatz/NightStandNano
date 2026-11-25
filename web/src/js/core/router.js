// SPDX-License-Identifier: LGPL-2.1-or-later
// Routing by the URL hash: #/now, #/settings/wifi, ... Pages are the <section class="page" data-page="..."> elements; the router shows the
// one whose route is current, marks the navigation, and tells the rest of the page with emit("route", key, previousKey).
// Browser Back works (every change of the hash is a history entry) and so do deep links. A page's "back" arrow goes to its parent route,
// by history.back() when the parent is where the user came from.
import { app, $, emit } from "./app.js";

// route key -> { page: element id, nav: the primary destination that is highlighted, parent: where the back arrow goes }
export const ROUTES = {
  "now": { page: "p-now", nav: "now" },
  "now/lyrics": { page: "p-lyrics", nav: "now", parent: "now" },
  "music": { page: "p-music", nav: "music" },
  "radio": { page: "p-radio", nav: "radio" },
  "ambient": { page: "p-ambient", nav: "ambient" },
  "settings": { page: "p-settings", nav: "settings" },
  ...Object.fromEntries(["output", "fx", "night", "viz", "wifi", "time", "card", "keys", "led", "lang", "diag"]
    .map(id => ["settings/" + id, { page: "p-" + id, nav: "settings", parent: "settings" }])),
};
// ?demo=<name>: the old tab names of the previous layout open the page that now holds their content
const DEMO_ALIAS = { files: "music", noise: "ambient", bed: "settings/night", sys: "settings/diag", lyrics: "now/lyrics" };
export function demoRoute(name) {
  if (!name) return "";
  const k = DEMO_ALIAS[name] || name;
  return ROUTES[k] ? k : ROUTES["settings/" + k] ? "settings/" + k : "";
}

const keyOf = () => { const k = location.hash.replace(/^#\/?/, "").replace(/\/+$/, ""); return ROUTES[k] ? k : "now"; };
let first = true, prevKey = "";
function apply() {
  const key = keyOf(), was = app.route;
  if (location.hash !== "#/" + key) history.replaceState(null, "", "#/" + key);
  app.route = key; document.body.dataset.route = key;
  for (const [k, r] of Object.entries(ROUTES)) $(r.page).hidden = k !== key;
  for (const a of document.querySelectorAll("#nav [data-nav]")) { if (a.dataset.nav === ROUTES[key].nav) a.setAttribute("aria-current", "page"); else a.removeAttribute("aria-current"); }
  $("content").scrollTop = 0;
  if (!first && was !== key) { const h = $(ROUTES[key].page).querySelector("h2"); if (h) { h.tabIndex = -1; h.focus({ preventScroll: true }); } }
  first = false;
  emit("route", key, was);
}
export function initRouter() {
  for (const a of document.querySelectorAll(".back")) a.addEventListener("click", e => { if (history.length > 1 && prevKey === ROUTES[keyOf()].parent) { e.preventDefault(); history.back(); } });
  window.addEventListener("hashchange", () => { prevKey = app.route; apply(); });
  apply();
}
