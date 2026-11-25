// SPDX-License-Identifier: LGPL-2.1-or-later
// The page shell: language select, connection card, tabs, console pane, footer. The two tabs live in flashtab.js and hwtab.js.
import { $, el, show } from "./dom.js";
import { initFlashTab } from "./flashtab.js";
import { initHwTab } from "./hwtab.js";
import { isVerLine, parseVer, redact } from "../core/proto.js";

export function startApp({ conn, esp, I18N, stamp, demo, tab }) {
  const t = (k, p) => I18N.t(k, p);
  const lang = { onChange: (f) => I18N.onChange(f) };
  let ver = null, verState = "none", connErr = null;   // verState: none | waiting | ok | silent

  // ---- language ----
  const sel = $("#lang");
  const fillLang = () => {
    const cur = I18N.pref();
    sel.replaceChildren(el("option", { value: "auto" }, t("lang.auto")), el("option", { value: "en" }, "English"), el("option", { value: "zh-TW" }, "繁體中文"), el("option", { value: "zh-CN" }, "简体中文"));
    sel.value = cur;
  };
  sel.addEventListener("change", () => I18N.setLang(sel.value));

  // ---- tabs ----
  const tabs = { flash: [$("#tab-btn-flash"), $("#tab-flash")], hw: [$("#tab-btn-hw"), $("#tab-hw")] };
  function pickTab(name) { for (const [k, [b, p]] of Object.entries(tabs)) { b.setAttribute("aria-selected", String(k === name)); p.hidden = k !== name; } }
  for (const [k, [b]] of Object.entries(tabs)) b.addEventListener("click", () => pickTab(k));
  pickTab(tab === "hw" ? "hw" : "flash");

  // ---- tabs' logic ----
  const flash = initFlashTab({ conn, esp, t, lang });
  const hw = initHwTab({ conn, t, lang });

  // ---- connection card ----
  const verDetail = (v) => (v.commit ? t(v.dirty ? "conn.fw.dirty" : "conn.fw.commit", { commit: v.commit }) : v.dirty ? t("conn.fw.dirty0") : "");
  function renderConn() {
    const s = conn.state;
    $("#conn-dot").className = "dot" + (s === "console" ? " on" : s === "flashing" ? " busy" : "");
    $("#conn-text").textContent = s === "console" ? t("conn.on", { port: conn.label }) : s === "flashing" ? t("conn.flashing") : t("conn.off");
    show($("#btn-connect"), s === "idle"); $("#btn-connect").disabled = !conn.supported; $("#btn-anyport").disabled = !conn.supported; show($("#btn-disconnect"), s === "console");
    show($("#btn-anyport").parentElement, s === "idle");
    const fw = $("#conn-fw");
    show(fw, s === "console" && verState !== "none");
    fw.textContent = verState === "ok" ? t("conn.fw", { version: ver.base || ver.version, detail: verDetail(ver), idf: ver.idf || "?", built: ver.built || "?" }) : verState === "waiting" ? t("conn.waitver") : verState === "silent" ? t("conn.nover") : "";
    const ce = $("#conn-err"); show(ce, !!connErr); ce.textContent = connErr ? connErr() : "";
    $("#con-input").disabled = s !== "console"; $("#con-send").disabled = s !== "console";
    document.body.dataset.conn = s;
  }
  async function connect(any) {
    connErr = null; renderConn();
    try { await conn.open({ any }); }
    catch (e) { connErr = () => (e.code === "open" ? t("conn.err.open", { msg: e.detail }) : t(e.code === "nosupport" ? "warn.noserial" : "conn.lost")); renderConn(); }
  }
  $("#btn-connect").addEventListener("click", () => connect(false));
  $("#btn-anyport").addEventListener("click", () => connect(true));
  $("#btn-disconnect").addEventListener("click", () => conn.close());

  // what to do whenever the console comes up (first connect, after a flash): ask `ver` until the board is up, then read the hardware settings
  async function consoleUp() {
    ver = null; verState = "waiting"; renderConn();
    try {
      const lines = await conn.poll("ver", { accept: isVerLine, until: () => true, every: 700, total: 14000 });
      ver = parseVer(lines[0]); verState = "ok";
    } catch (e) { if (conn.state !== "console") return; verState = "silent"; }
    renderConn();
    if (conn.state === "console") await hw.read();
  }
  conn.on("state", (s) => {
    if (s === "console") consoleUp();
    else { if (s === "idle") { ver = null; verState = "none"; } renderConn(); }
  });
  conn.on("close", (byUser) => { if (!byUser) { connErr = () => t("conn.lost"); renderConn(); } });
  lang.onChange(() => { fillLang(); renderConn(); $("#build-stamp").textContent = t("foot.build", { stamp }); });

  // ---- console pane ----
  const log = $("#con-log"), MAX = 500;
  function addLog(text) {
    const near = log.scrollHeight - log.scrollTop - log.clientHeight < 40;
    log.append(document.createTextNode(text + "\n"));
    while (log.childNodes.length > MAX) log.firstChild.remove();
    if (near) log.scrollTop = log.scrollHeight;
  }
  conn.on("line", (l) => addLog(redact(l)));
  conn.on("tx", (c) => addLog("> " + redact(c)));
  conn.on("close", () => addLog("--"));
  $("#con-form").addEventListener("submit", (e) => {
    e.preventDefault();
    const v = $("#con-input").value.trim(); if (!v) return;
    $("#con-input").value = "";
    conn.send(v).catch(() => {});
  });
  $("#con-clear").addEventListener("click", () => log.replaceChildren());

  // ---- start ----
  show($("#demo-banner"), !!demo);
  show($("#noserial"), !conn.supported);
  fillLang(); renderConn();
  $("#build-stamp").textContent = t("foot.build", { stamp });
  return { flash, hw, pickTab };
}
