// SPDX-License-Identifier: LGPL-2.1-or-later
// #/settings/output: the volume limit (the volume slider itself is in the control bar and stops at this number), the highest sample rate
// the DAC is run at (`dacmax`), and the DAC's sample-rate test (`ratetest`).
// The test: `ratetest` starts it (the box stops playback and plays a quiet tone at each rate), `ratetest status` is read once when the page opens
// and then every second for as long as a test runs AND this page is the current route; nothing is sent from other pages.
import { app, $, on, t, setT, segSet, segBind } from "../core/app.js";
import { send } from "../core/link.js";
import { refreshStatus } from "../core/status.js";
import { parseRatetest, parseRatetestReply, khz } from "../lib/fw.js";

const here = () => app.route === "settings/output" && app.link === "on";
let rt = null, msg = null, timer = 0, busy = false, dacUntil = 0;   // rt: the last parsed status; msg: { key, p } shown instead of the state (a refusal)

async function readStatus() {
  if (busy) return; busy = true;
  try {
    const lines = await send("ratetest status");
    if (!lines.length && app.link !== "on") return;
    const r = parseRatetest(lines);
    if (r) { rt = r; if (msg && msg.key !== "rt.unsupported") msg = null; } else if (lines.length) msg = { key: "rt.unsupported" };
    renderRt();
  } finally { busy = false; }
  schedule();
}
// polling: only while a test runs and the page is on screen
function schedule() {
  clearTimeout(timer); timer = 0;
  if (rt && rt.state === "running" && here()) timer = setTimeout(() => { timer = 0; if (rt && rt.state === "running" && here()) readStatus(); }, 1000);
}
function refusedMsg(why) {
  const k = "rt.refused." + String(why).toLowerCase().replace(/[^a-z]+/g, "_").replace(/^_|_$/g, "");
  return t(k) !== k ? { key: k } : { key: "rt.refused.other", p: { why } };
}

function renderRt() {
  const running = !!rt && rt.state === "running", on_ = app.link === "on";
  $("rtStart").disabled = !on_ || running; $("rtCancel").disabled = !on_ || !running;
  const st = $("rtState"), bar = $("rtBar"), tbl = $("rtTable"), mx = $("rtMax");
  const rates = rt ? rt.rates : [];
  if (msg) setT(st, msg.key, msg.p);
  else if (!rt) setT(st, "rt.state.none");
  else if (running) {
    const done = rates.filter(r => r.verdict !== "untested" || r.set !== "na").length;
    if (rt.now) setT(st, "rt.state.running", { rate: khz(rt.now), done: Math.min(done, rates.length), n: rates.length }); else setT(st, "rt.state.running0");
  } else if (rt.state === "failed") { if (rt.reason) setT(st, "rt.state.failedWhy", { why: rt.reason }); else setT(st, "rt.state.failed"); }
  else if (rt.state === "done" || rt.state === "cancelled") setT(st, "rt.state." + rt.state);
  else setT(st, rates.length ? "rt.state.idle" : "rt.state.none");
  st.dataset.state = msg ? "refused" : rt ? rt.state : "none";
  // progress: how many of the listed rates have a result
  bar.hidden = !running || !rates.length;
  if (!bar.hidden) bar.firstElementChild.style.width = Math.round(100 * rates.filter(r => r.set !== "na" || r.verdict !== "untested").length / rates.length) + "%";
  tbl.hidden = !rates.length;
  const body = $("rtBody"); body.textContent = "";
  for (const r of rates) {
    const tr = document.createElement("tr"), c1 = document.createElement("td"), c2 = document.createElement("td"), c3 = document.createElement("td");
    c1.textContent = khz(r.rate) + " kHz";
    const tag = document.createElement("span"); tag.className = "rtag " + r.verdict; setT(tag, "rt.v." + r.verdict); c2.appendChild(tag);
    if (r.verdict === "bad") { const w = document.createElement("div"); w.className = "mut"; if (r.set === "fail") setT(w, "rt.why.setfail"); else setT(w, "rt.why.xfer", { n: r.xferBad }); c2.appendChild(w); }
    c3.textContent = r.fps ? r.fps + " Hz" : "—";
    tr.append(c1, c2, c3); body.appendChild(tr);
  }
  const fin = rt && !running && (rt.state === "done" || rt.max);
  mx.hidden = !fin;
  if (fin) { if (rt.max) setT(mx, "rt.max", { r: khz(rt.max) }); else setT(mx, "rt.max.none"); }
}

async function start() {
  msg = null;
  const r = parseRatetestReply(await send("ratetest"));
  if (r && r.refused) { msg = refusedMsg(r.refused); renderRt(); return; }
  if (!r) { msg = { key: "rt.unsupported" }; renderRt(); return; }
  rt = rt && rt.state === "running" ? rt : { state: "running", dac: "", now: 0, reason: "", rates: [], max: null };
  renderRt(); refreshStatus(); readStatus();
}
async function cancel() {
  const r = parseRatetestReply(await send("ratetest cancel"));
  msg = r && r.refused ? refusedMsg(r.refused) : null;
  await readStatus(); refreshStatus();
}

// opening the page: what the box knows now (a finished or running test)
export function loadRatetest() { msg = null; return readStatus(); }

export function initOutput() {
  $("vmax").oninput = () => {
    const v = +$("vmax").value; app.vmaxUntil = Date.now() + 3000; $("vmaxv").textContent = v; $("vol").max = v;
    if (+$("vol").value > v) { $("vol").value = v; $("volv").textContent = v; }
  };
  $("vmax").onchange = async () => { app.vmaxUntil = Date.now() + 3000; await send("vollimit " + $("vmax").value); app.vmaxUntil = 0; refreshStatus(); };
  segBind("dacmax", async v => { dacUntil = Date.now() + 3000; await send("dacmax " + (v === "0" ? "auto" : v)); dacUntil = 0; refreshStatus(); });
  $("rtStart").onclick = start; $("rtCancel").onclick = cancel;
  on("status", state => { if (typeof state.dacmax === "number" && Date.now() > dacUntil) segSet("dacmax", state.dacmax); });
  on("route", () => { if (!here()) { clearTimeout(timer); timer = 0; } });
  on("linkoff", () => { clearTimeout(timer); timer = 0; rt = null; msg = null; renderRt(); });
  on("lang", renderRt);
  renderRt();
}
