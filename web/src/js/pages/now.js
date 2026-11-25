// SPDX-License-Identifier: LGPL-2.1-or-later
// #/now: the visualizer, title and tags, the status line, progress, the output choice (+ Auracast) and the sleep timer.
// The visual lives in the .stage element (beside the page on a wide screen); the transport and the volume are in the control bar (core/bar.js).
import { app, $, on, emit, sleep, segSet, segBind, known, DEMO, t, setT, setRaw } from "../core/app.js";
import { send, log, waitLink, setLinkUI } from "../core/link.js";
import { refreshStatus } from "../core/status.js";
import { renderVolFor } from "../core/bar.js";
import { createNanoViz } from "../lib/viz.js";
import { mountViz } from "../lib/viz-ui.js";

let hintUntil = 0;
export function flashHint() { hintUntil = Date.now() + 4000; renderMeta(); setTimeout(renderMeta, 4100); }

// the status line under the title, the sleep timer's countdown, and what the play button will do when nothing is playing
export function renderMeta() {
  const state = app.state;
  if (!state) return;
  const mmss = ms => { const s = Math.max(0, Math.round(ms / 1000)); return Math.floor(s / 60) + ":" + String(s % 60).padStart(2, "0"); };
  const where = { dac: t("out.dac"), bt: t("out.btShort"), cast: t("out.castShort") }[state.out];
  setRaw($("meta"), known("st.", state.st) + (state.fmt ? " · " + state.fmt + " " + (state.rate / 1000) + " kHz" : "") + (where ? " · " + where : "") +
    (state.n > 1 ? ` · ${state.trk}/${state.n}` : "") + (state.len > 0 ? " · " + mmss(state.pos) + " / " + mmss(state.len) : ""));
  setRaw($("timerv"), state.sleep > 0 ? t("timer.left", { m: Math.ceil(state.sleep / 60) }) : "");
  const idle = state.st === "idle", hint = $("idleHint");
  if (idle && !state.bm && Date.now() < hintUntil) setT(hint, "now.pickHint");
  else if (idle && state.bm) setT(hint, "now.resumeHint");
  else setRaw(hint, "");
}

// ---- Auracast (the fourth output) ----
// "out" -> "out <auto|dac|bt|cast> (playing on <none|dac|bt|cast>)"; "cast" -> "cast: off[ (...)]" or `cast: on "name" frames=N underflows=N ... send_err=N ... streams=11`.
let castRep = null, castErr = "";
export const castState = () => castRep;
export function setOut(v) { app.selOut = v; segSet("out", v); if (v === "cast") loadCast(); renderCast(); renderVolFor(); }
function parseCast(r) {
  const l = r.find(x => x.startsWith("cast:")); if (!l) return null;
  const on = /^cast: on\b/.test(l), num = k => { const m = new RegExp("\\b" + k + "=(\\d+)").exec(l); return m ? +m[1] : 0; };
  const sm = /\bstreams=(\d+)/.exec(l);
  return { on, und: num("underflows"), err: num("send_err"), streams: sm ? [...sm[1]].filter(c => c === "1").length : 0 };
}
async function loadCast() {
  const r = await send("cast"); if (app.link !== "on") return;
  const c = parseCast(r); if (c) { castRep = c; renderCast(); }
}
export function renderCast() {
  const state = app.state;
  const show = app.selOut === "cast" && app.link === "on";
  $("castInfo").hidden = !show;
  $("dCast").className = "dot" + ((state && state.out === "cast") || (castRep && castRep.on && app.link === "on") ? " on" : "");
  emit("cast");   // the sound-effects page shows its own notes while casting
  if (!show) return;
  let txt;
  if (castErr) txt = t("cast.err", { e: castErr });
  else if (!castRep) txt = t("cast.wait");
  else if (!castRep.on) txt = t("cast.off");
  else {
    txt = castRep.streams >= 2 ? t("cast.on") : t("cast.part", { n: castRep.streams });
    if (castRep.und || castRep.err) txt += " " + t("cast.counts", { u: castRep.und, e: castRep.err });
  }
  setRaw($("castSt"), txt);
}
export async function loadOut() {
  const r = await send("out"); if (app.link !== "on") return;
  const m = (r.find(l => /^out \w+ \(playing on /.test(l)) || "").match(/^out (\w+)/);
  if (m) { castErr = ""; setOut(m[1]); }
}
// out <v>: "cast" may start LE Audio, which drops the BLE link ("... reconnects in a moment"); the reconnect loop brings it back and `out` then confirms.
async function chooseOut(v) {
  castErr = ""; setOut(v);
  const r = await send("out " + v);
  const starting = r.some(l => /reconnects in a moment/.test(l)) || (v === "cast" && !r.length && app.link !== "on");
  if (starting) {
    app.castStarting = true; setLinkUI();
    for (const t0 = Date.now(); app.link === "on" && Date.now() - t0 < 5000;) await sleep(100);   // the box drops the link a moment after its reply
    const ok = await waitLink(); app.castStarting = false; setLinkUI();
    if (ok) { await loadOut(); refreshStatus(); loadCast(); }
    return;
  }
  if (app.link !== "on") return;
  const bad = r.find(l => /^out \w+: /.test(l));
  if (bad) { castErr = bad.replace(/^out \w+: /, ""); await loadOut(); renderCast(); }   // the box kept its old output: show that one, and why
  else { const m = r.find(l => /^out \w+ \(playing on /.test(l)); if (m) setOut(m.match(/^out (\w+)/)[1]); }
  refreshStatus(); if (v === "cast") loadCast();
}

// ---- the visualizer: tunetap's canvas host, fed with the 32 bands (lib/viz.js) ----
function mountVisual() {
  try {
    const viz = createNanoViz({ getBytes: () => app.target });
    app.viz = viz;
    if (DEMO) { const m = new URLSearchParams(location.search).get("viz"); if (m) viz.host.setMode(m); }
    mountViz({ viz, stage: $("vizStage"), modeRow: $("vizMode"), options: $("vizOpts") });
    if (app.state) viz.setState(app.state);
  } catch (e) {
    log("viz: " + e);
    setT($("vizStage"), "viz.needHttp");
    $("vizStage").style.cssText = "display:flex;align-items:center;justify-content:center;text-align:center;padding:1rem;color:var(--mut);font-size:.9rem";
    setRaw($("vizOpts"), "");
  }
}

export function initNow() {
  mountVisual();
  for (const b of $("out").children) b.onclick = () => chooseOut(b.dataset.v);
  segBind("timer", v => send("timer " + v).then(refreshStatus));
  setInterval(() => { if (app.link === "on" && app.selOut === "cast" && !document.hidden && !app.scanBusy) loadCast(); }, 4000);
  document.addEventListener("visibilitychange", () => { if (!document.hidden && app.link === "on" && app.selOut === "cast") loadCast(); });

  on("status", state => {
    if (app.viz) app.viz.setState(state);
    // main line: the tag title (or the ICY title of a radio stream), else the file name; the file name stays in the tooltip
    $("file").textContent = state.title || state.file || "—";
    $("file").title = state.title && state.file ? state.file : "";
    const tags = [state.artist, state.album].filter(Boolean).join(" — ");
    $("tags").textContent = tags; $("tags").hidden = !tags;
  });
  on("status", state => {
    renderMeta();
    $("prog").style.width = state.len > 0 ? Math.min(100, 100 * state.pos / state.len) + "%" : "0";
    $("dDac").className = "dot" + (state.dac ? " on" : ""); $("dBt").className = "dot" + (state.bt ? " on" : "");
    renderCast();
    if (!(state.sleep > 0)) segSet("timer", "0");
  });
  on("linkoff", () => {
    hintUntil = 0; setRaw($("idleHint"), ""); setT($("meta"), "meta.off");
    if (app.viz) app.viz.setState(null);
    renderCast();
  });
  on("lang", () => { renderMeta(); renderCast(); });
}
