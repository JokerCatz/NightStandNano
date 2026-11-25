// SPDX-License-Identifier: LGPL-2.1-or-later
// The control bar that is on every screen: title, previous / play-pause / next / stop, THE volume slider with its value, and the output the
// volume belongs to. Volume and the volume limit are one thing: the slider stops at the limit (the limit's own slider is on #/settings/output).
import { app, $, on, known, t, setT, setRaw } from "../core/app.js";
import { send } from "./link.js";
import { refreshStatus } from "./status.js";
import { flashHint } from "../pages/now.js";
import { setIcon } from "./icons.js";

const setAria = (el, key) => { el.setAttribute("data-i18n-aria", key); el.setAttribute("aria-label", t(key)); };

// which output the volume belongs to: the one in use, or the selected one while nothing plays on it
export function renderVolFor() {
  const outs = ["dac", "bt", "cast"], state = app.state;
  const k = state && outs.includes(state.out) ? state.out : outs.includes(app.selOut) ? app.selOut : "";
  const hide = !k || app.link !== "on";
  $("volFor").hidden = hide; $("cbOut").hidden = hide;
  if (k) {
    setT($("volFor"), "vol.for." + k);
    setIcon($("cbOutIc"), k); setT($("cbOutTxt"), { dac: "out.dac", bt: "out.btShort", cast: "out.castShort" }[k]);
    $("cbOut").setAttribute("data-i18n-title", "vol.for." + k); $("cbOut").title = t("vol.for." + k);
  }
}
// volume (the output in use has its own), the volume limit (it also caps the main slider) and gapless. After the user's own change the
// controls ignore the polled values for a moment, so a poll that was already in flight cannot move them back.
export function renderVolume() {
  const state = app.state;
  if (!state) return;
  if (typeof state.vmax === "number" && Date.now() > app.vmaxUntil) { $("vol").max = state.vmax; $("vmax").value = state.vmax; $("vmaxv").textContent = state.vmax; }
  if (document.activeElement !== $("vol")) { $("vol").value = state.vol; $("volv").textContent = state.vol; }
  if (typeof state.gapless === "number" && Date.now() > app.gapUntil) $("gapless").checked = !!state.gapless;
  renderVolFor();
}
function renderTitle() {
  const state = app.state;
  if (!state) { setRaw($("cbTitle"), "—"); setT($("cbSub"), "meta.off"); return; }
  $("cbTitle").textContent = state.title || state.file || "—";
  setRaw($("cbSub"), [known("st.", state.st), state.artist].filter(Boolean).join(" · "));
}
function renderPlay() {
  const playing = !!app.state && app.state.st === "playing";
  setIcon($("btnPlay").firstElementChild, playing ? "pause" : "play");
  setAria($("btnPlay"), app.state ? (playing ? "tr.pauseOnly" : "tr.playOnly") : "tr.play");
}

export function initBar() {
  // playing -> pause; paused -> resume; idle with a bookmark (a card file stopped part-way) -> resume continues from it; idle without -> a hint
  $("btnPlay").onclick = async () => {
    const state = app.state;
    if (!state) return;
    if (state.st === "playing") await send("pause");
    else if (state.st === "paused") await send("resume");
    else if (state.st === "idle") {
      if (!state.bm) { flashHint(); return; }
      const r = await send("resume");
      if (r.some(l => /nothing to resume/i.test(l))) flashHint();
    } else return;
    refreshStatus();
  };
  $("btnStop").onclick = async () => { await send("stop"); refreshStatus(); };
  $("btnNext").onclick = async () => { await send("next"); refreshStatus(); };
  $("btnPrev").onclick = async () => { await send("prev"); refreshStatus(); };
  $("vol").oninput = () => { $("volv").textContent = $("vol").value; };
  $("vol").onchange = () => send("vol " + $("vol").value);
  on("status", () => { renderTitle(); renderPlay(); });
  on("status", () => renderVolume());
  on("linkoff", () => { renderTitle(); renderPlay(); renderVolFor(); });
  on("lang", () => { renderTitle(); renderVolFor(); });
}
