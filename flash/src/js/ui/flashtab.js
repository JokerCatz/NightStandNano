// SPDX-License-Identifier: LGPL-2.1-or-later
// The "Flash firmware" tab: pick the release file(s), see what will be written where, connect and check the board, write, verify, next steps.
// Derived from the owner's JukeDial web tool (tools/web/flashtab.js: file picking, session through Flasher, checks, progress, error wording).
// Changes for Nano: one merged image or three files instead of a board choice, no backup step, an explicit off-by-default "erase settings",
// inline confirmation instead of window.confirm, next-steps box with a link to the control page.
import { $, el, show } from "./dom.js";
import { analyze, planWrite, chipCheck, hex } from "../core/release.js";
import { beginSession } from "../core/flasher.js";

export function initFlashTab({ conn, esp, t, lang }) {
  let files = [], rel = null, relErr = null, sess = null, phase = "idle", stat = null, parsing = 0, flashErr = null, found = null, blocked = [], done = false, lostPort = false, speedNote = null;
  const kb = (n) => (n >= 1048576 ? (n / 1048576).toFixed(2) + " MB" : Math.max(1, Math.round(n / 1024)) + " KB");
  const partName = (r) => t("fl.part." + r);

  function errText(e) {
    const x = e && e.extra;
    switch (e && e.code) {
      case "none": case "read": return t("fl.err." + e.code);
      case "count": return t("fl.err.count", { n: x });
      case "need_all": case "roles": return t("fl.err." + e.code, { role: partName(x) });
      case "not_merged": case "mixed": return t("fl.err." + e.code, { file: x });
      case "no_magic": case "chip": case "hash": case "size": case "no_pt": case "pt_md5": case "no_desc": return t("fl.err." + e.code, { file: x || "" });
      case "pt_layout": return t("fl.err.pt_layout", { what: x || "" });
      case "gap": return t("fl.err.gap", { at: x || "" });
      case "app_big": return t("fl.err.app_big", { size: x || "" });
      case "lib": return t("fl.err.lib", { msg: x || "" });
      case "verify": return t("fl.fail.verify", { name: partName(x) });
      default: return t("fl.err.other", { msg: (e && e.message) || String(e) });
    }
  }

  // ---- reading the picked files ----
  async function take(list) {
    if (!list.length || phase === "writing" || phase === "checking") return;
    if (sess) await cancel();
    const my = ++parsing;
    files = []; rel = null; relErr = null; flashErr = null; done = false; found = null;
    try {
      for (const f of list) files.push({ name: f.name, data: new Uint8Array(await f.arrayBuffer()) });
    } catch { relErr = { code: "read" }; }
    if (!relErr) { try { rel = await analyze(files); } catch (e) { relErr = e; } }
    if (my === parsing) render();
  }

  // ---- rendering ----
  function render() {
    const busy = phase === "checking" || phase === "writing";
    const info = $("#fl-info"); info.replaceChildren();
    if (relErr) info.append(el("p", { class: "chip bad" }, errText(relErr)));
    else if (rel) {
      info.append(el("p", { class: "small" }, rel.kind === "merged" ? t("fl.kind.merged") : t("fl.kind.three")));
      info.append(el("table", { class: "parts" },
        el("thead", {}, el("tr", {}, ...["fl.col.part", "fl.col.file", "fl.col.offset", "fl.col.size"].map((k) => el("th", {}, t(k))))),
        el("tbody", {}, ...rel.parts.map((p) => el("tr", {}, el("td", {}, partName(p.role)), el("td", { class: "mono" }, p.file), el("td", { class: "num mono" }, hex(p.offset)), el("td", { class: "num" }, kb(p.size)))))));
      info.append(el("p", { class: "small mut" }, t("fl.ver", { version: rel.version || "?", built: ((rel.buildDate || "") + " " + (rel.buildTime || "")).trim() || "?", idf: rel.idf || "?" })));
      info.append(el("p", { class: "small mut" }, t("fl.flashparams", rel.flash)));
      if (!rel.flash.matches) info.append(el("p", { class: "chip warn small" }, t("fl.flashparams.diff")));
      if (rel.projectName && rel.projectName !== "nightstand_nano") info.append(el("p", { class: "chip warn small" }, t("fl.project.diff", { name: rel.projectName })));
      info.append(el("p", { class: "chip ok small" }, t("fl.ok")));
    }
    show($("#fl-opts"), !!rel);
    if (rel) {
      const erase = $("#fl-erase").checked, plan = planWrite(rel, { eraseSettings: erase });
      show($("#fl-erase-warn"), erase);
      $("#fl-erase-warn").textContent = t("fl.erase.warn", { off: hex(rel.nvs.off), size: kb(rel.nvs.size) });
      const list = (a) => a.map((p) => partName(p.name) + " " + hex(p.offset)).join(", ");
      const kept = plan.kept.map((e) => e.name + " " + hex(e.offset)).join(", ");
      const pl = $("#fl-plan"); pl.replaceChildren(...[el("div", {}, el("b", {}, t("fl.plan.title"))),
        ...plan.parts.filter((p) => !plan.erased.some((e) => e.offset === p.offset)).map((p) => el("div", { class: "mono" }, `${partName(p.name)}  ${hex(p.offset)}  ${kb(p.data.length)}`)),
        plan.erased.length ? el("div", { class: "warn" }, t("fl.plan.erased", { list: list(plan.erased) + " (" + kb(rel.nvs.size) + ")" })) : null,
        kept ? el("div", { class: "mut" }, t("fl.plan.kept", { list: kept })) : null].filter(Boolean));
    }
    // buttons
    const ok = !!rel && conn.supported;
    show($("#fl-check"), phase === "idle" || phase === "checking" || phase === "done"); $("#fl-check").disabled = !ok || busy;
    show($("#fl-write"), phase === "ready" || phase === "confirm"); $("#fl-write").disabled = phase !== "ready";
    show($("#fl-cancel"), phase === "ready" || phase === "confirm" || phase === "blocked");
    $("#fl-erase").disabled = busy || phase === "confirm"; $("#fl-baud").disabled = busy || phase === "ready" || phase === "confirm" || phase === "blocked";
    show($("#fl-speed-note"), !!speedNote); $("#fl-speed-note").textContent = speedNote ? t("fl.speed.fb." + speedNote.stage, { from: speedNote.from, to: speedNote.to }) : "";
    show($("#fl-confirm"), phase === "confirm");
    show($("#fl-found"), !!found && (phase === "ready" || phase === "confirm" || phase === "blocked"));
    if (found) $("#fl-found").textContent = t("fl.found", found) + (blocked.length ? "" : " " + t("fl.ready"));
    const prog = phase === "checking" || phase === "writing";
    show($("#fl-prog"), prog);
    $("#fl-stat").textContent = stat ? t(stat.key, stat.p) : "";
    const errs = blocked.map((c) => (c === "chip" ? t("fl.err.chipcheck", { chip: sess ? sess.info.chip : "?" }) : t("fl.err.small", { size: sess && sess.info.flashSize || "?", need: Math.ceil(rel.flashNeeded / 1048576) })));
    const fe = flashErr ? [flashErr()] : errs;
    show($("#fl-err"), fe.length > 0); $("#fl-err").textContent = fe.join(" ");
    show($("#fl-done"), done);
    if (lostPort) { show($("#fl-err"), true); $("#fl-err").textContent = t("fl.lost"); }
    $("#fl-bar").hidden = phase !== "writing";
  }

  // ---- actions ----
  const logEl = $("#fl-log");
  const log = (s) => { logEl.append(document.createTextNode(String(s).replace(/\s+$/, "") + "\n")); while (logEl.childNodes.length > 300) logEl.firstChild.remove(); logEl.scrollTop = logEl.scrollHeight; };

  async function check() {
    if (!rel || phase === "checking" || phase === "writing") return;
    flashErr = null; done = false; lostPort = false; blocked = []; found = null; speedNote = null;
    phase = "checking"; stat = { key: "fl.checking" }; render();
    try {
      if (conn.state === "idle") {
        const ok = await conn.open();
        if (!ok) { phase = "idle"; stat = null; render(); return; }
      }
      sess = await beginSession({ conn, esp, log, baud: Number($("#fl-baud").value) || undefined, onFallback: (f) => { speedNote = f; if (found) found.baud = f.to; render(); } });
      blocked = chipCheck(sess.info, rel);
      found = { chip: sess.info.chip, size: sess.info.flashSize || "?", baud: sess.info.baud };
      phase = blocked.length ? "blocked" : "ready"; stat = null;
    } catch (e) {
      console.error(e);
      sess = null; phase = "idle"; stat = null;
      flashErr = () => (e.code === "lib" ? errText(e) : e.code === "open" ? t("conn.err.open", { msg: e.detail }) : t("fl.fail.connect", { msg: e.message }));
      $("#fl-manual").open = true;
    }
    render();
  }
  async function cancel() {
    const s = sess; sess = null; phase = "idle"; stat = null; found = null; blocked = []; render();
    if (s) lostPort = await s.end().catch(() => true);
    render();
  }
  function askWrite() {
    if (phase !== "ready") return;
    if ($("#fl-erase").checked) { $("#fl-confirm-text").textContent = t("fl.erase.warn", { off: hex(rel.nvs.off), size: kb(rel.nvs.size) }); phase = "confirm"; render(); } else write();
  }
  async function write() {
    if (!sess || !rel || (phase !== "ready" && phase !== "confirm")) return;
    const plan = planWrite(rel, { eraseSettings: $("#fl-erase").checked });
    phase = "writing"; flashErr = null; stat = { key: "fl.writing", p: { name: "", pct: 0 } }; $("#fl-bar").value = 0; render();
    const s = sess;
    try {
      await s.write(plan.parts, {
        onProgress: (f, m) => {
          $("#fl-bar").value = Math.round(f * 1000);
          const name = partName(m.name);
          stat = m.phase === "verify" ? { key: "fl.verifying", p: { name } } : { key: "fl.writing", p: { name, pct: Math.floor(f * 100) } };
          $("#fl-stat").textContent = t(stat.key, stat.p);
        },
      });
      stat = { key: "fl.finishing" }; render();
      sess = null; lostPort = await s.end();
      phase = "done"; done = true; stat = null; found = null;
      window.__nsnFlashDone = true;
    } catch (e) {
      console.error(e);
      sess = null; stat = null;
      flashErr = () => (e.code === "verify" ? errText(e) : t("fl.fail.write", { msg: e.message || String(e) }));
      phase = "idle"; render();
      lostPort = await s.end().catch(() => true);
      $("#fl-manual").open = true;
      window.__nsnFlashFailed = true;
    }
    render();
  }

  // ---- wiring ----
  const input = $("#fl-file");
  $("#fl-pick").addEventListener("click", () => input.click());
  input.addEventListener("change", () => { const l = [...input.files]; input.value = ""; take(l); });
  const drop = $("#fl-drop");
  drop.addEventListener("dragover", (e) => { e.preventDefault(); drop.classList.add("over"); });
  drop.addEventListener("dragleave", () => drop.classList.remove("over"));
  drop.addEventListener("drop", (e) => { e.preventDefault(); drop.classList.remove("over"); take([...e.dataTransfer.files]); });
  $("#fl-check").addEventListener("click", check);
  $("#fl-write").addEventListener("click", askWrite);
  $("#fl-cancel").addEventListener("click", cancel);
  $("#fl-confirm-yes").addEventListener("click", write);
  $("#fl-confirm-no").addEventListener("click", () => { phase = "ready"; render(); });
  $("#fl-erase").addEventListener("change", render);
  conn.on("state", (s) => { if (s === "console" && phase === "done") lostPort = false; render(); });
  lang.onChange(render);
  render();
  return { render, take, get phase() { return phase; } };
}
