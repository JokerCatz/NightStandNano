// SPDX-License-Identifier: LGPL-2.1-or-later
// NightStand Nano: the memory-card page (Settings > Memory card), and the banner under the header that warns when the card is missing or
// unreadable. An ES module exporting mount(opts).
//
// Box commands used: `card`, `cardtest [raw|file|cancel]`, `cardfmt [<code>|cancel|status]` (see the firmware's cmd.c / card_test.c).
// Formatting is offered ONLY while the box reports the card as unreadable (st.card === "bad"); while it is ok or none nothing about it
// is in the DOM. The two confirmation codes are shown by the page and must be TYPED back by the user (never prefilled): the box sends
// the codes to whoever asked, so the typing is what makes it a real decision. Whatever is typed is sent as typed; a wrong entry makes
// the box cancel the whole request and the page returns to the start.
//
// opts: { box, banner, send(cmd) -> Promise<lines>, getState() -> st|null, isOn() -> bool, tabOpen() -> bool, openTab(), log(s) }
// returns: { onState(st), load(), reset() }   (texts follow a language change by themselves: they are all set with setT)
import { I18N } from "../core/i18n.js";

const I = I18N, t = I.t, setT = I.setT, setRaw = I.setRaw;
const TTL = 30;   // seconds the box keeps a format request alive

const el = (tag, cls, text) => { const e = document.createElement(tag); if (cls) e.className = cls; if (text !== undefined) e.textContent = text; return e; };
const elT = (tag, cls, key, params) => { const e = el(tag, cls); setT(e, key, params); return e; };

// ---- parsing ----
function kv(s) {   // key=value / key="quoted value" pairs
  const o = {}, re = /(\w+)=(?:"([^"]*)"|(\S+))/g; let m;
  while ((m = re.exec(s))) o[m[1]] = m[2] !== undefined ? m[2] : m[3];
  return o;
}
const num = v => (typeof v === "string" && /^\d+(\.\d+)?$/.test(v) ? +v : null);
export function parseCard(lines) {
  const l = lines.find(x => /^card state=/.test(x)); if (!l) return null;
  const o = kv(l);
  return { state: o.state, size: num(o.size), free: num(o.free), fs: o.fs && o.fs !== "?" ? o.fs : "", name: o.name || "", fmt: o.fmt || "idle" };
}
export function parseTest(lines) {
  const r = { seen: false, running: false, kind: "", pct: 0, fail: null, raw: null, file: null, basis: null, rows: [] };
  for (const l of lines) {
    let m;
    if ((m = /^cardtest state (\w+)(?: (raw|file) (\d+)%)?/.exec(l))) { r.seen = true; r.running = m[1] === "running"; if (m[2]) { r.kind = m[2]; r.pct = +m[3]; } }
    else if ((m = /^cardtest fail (raw|file) (.+)$/.exec(l))) r.fail = { kind: m[1], why: m[2].trim() };
    else if ((m = /^cardtest (raw|file) (\S.*)$/.exec(l))) r[m[1]] = kv(m[2]);
    else if ((m = /^cardtest basis (raw|file) speed=([\d.]+)/.exec(l))) r.basis = { kind: m[1], speed: +m[2] };
    else if ((m = /^cardtest mark (raw|file) speed=([\d.]+)/.exec(l))) r.rows.push({ mark: m[1], speed: +m[2] });
    else if ((m = /^cardtest fmt "([^"]*)"\s*(.*)$/.exec(l))) { const o = kv(m[2]); r.rows.push({ name: m[1], need: o.need, grade: o.grade || "unknown" }); }
  }
  return r;
}
export function fmtBytes(n) {
  if (n === null) return null;
  if (n >= 1e12) return (n / 1e12).toFixed(2) + " TB";
  if (n >= 1e9) return (n / 1e9).toFixed(2) + " GB";
  if (n >= 1e6) return Math.round(n / 1e6) + " MB";
  return Math.round(n / 1e3) + " KB";
}
const mbs = v => (v === undefined || v === null || v === "" || isNaN(+v) ? "?" : (+v).toFixed(2));
const when = s => (s ? String(s).replace("T", " ") : "");
// a reason the box gave, in words: card.reason.<reason with spaces as _> when the page knows it, else as sent
const reasonKey = r => "card.reason." + String(r).toLowerCase().replace(/[^a-z]+/g, "_").replace(/^_|_$/g, "");
const reasonP = r => (I.has(reasonKey(r)) ? { t: reasonKey(r) } : String(r));
function fmtWhy(r) {   // a refusal of the format request
  if (/readable/i.test(r)) return { t: "card.fmt.why.readable" };
  if (/wrong|mismatch|invalid|bad code|incorrect/i.test(r)) return { t: "card.fmt.why.wrong" };
  if (/late|expire|time/i.test(r)) return { t: "card.fmt.why.late" };
  return reasonP(r);
}

export function mount(o) {
  const { box, banner, send, getState, isOn, tabOpen } = o;
  const S = { card: null, test: null, testMsg: null, fmt: { phase: "idle", plan: null, code: "", deadline: 0, msg: null, result: null } };
  let loading = null, lastCard = null, testTimer = 0, fmtTimer = 0, cdTimer = 0, polling = false, fmtPolling = false, fmtBusy = false, fmtSig = "";

  // ---- layout ----
  const factsEl = el("div", "cfacts"), testEl = el("div"), fmtEl = el("div");
  box.textContent = "";
  box.append(factsEl, testEl, fmtEl);
  const bn = { text: el("span", "grow"), btn: elT("button", "", "card.warn.open") };
  banner.textContent = ""; banner.append(bn.text, bn.btn);
  bn.btn.onclick = () => o.openTab();

  // what kind of card the box says it has: "ok" | "bad" | "none" | "" (unknown)
  function kind() {
    const st = getState();
    if (st && st.card) return st.card;
    if (!st && !isOn()) return "";
    return S.card ? { mounted: "ok", unreadable: "bad", none: "none" }[S.card.state] || "" : "";
  }

  // ---- banner ----
  function renderBanner() {
    const st = getState(), k = st && st.card;
    const show = !!st && (k === "none" || k === "bad");
    banner.hidden = !show;
    if (show) setT(bn.text, k === "none" ? "card.warn.none" : "card.warn.bad");
  }

  // ---- facts ----
  function renderFacts() {
    const k = kind(), c = S.card;
    factsEl.textContent = "";
    if (!isOn() && !getState()) { factsEl.append(elT("div", "mut", "card.wait")); return; }
    if (k === "none") { factsEl.append(elT("div", "cstate", "card.state.none")); return; }
    if (k === "bad") {
      factsEl.append(elT("div", "cstate bad", "card.state.bad"));
      if (c && c.state === "unreadable" && (c.size !== null || c.name)) {
        const d = el("div", "mut"), size = c.size !== null ? fmtBytes(c.size) : { t: "card.unknown" };
        if (c.name) setT(d, "card.facts.bad", { size, name: c.name }); else if (c.size !== null) setRaw(d, size);
        factsEl.append(d);
      }
      return;
    }
    if (k === "ok" && c && c.state === "mounted") {
      const d = el("div", "cstate");
      setT(d, "card.facts.ok", { size: c.size !== null ? fmtBytes(c.size) : { t: "card.checking" }, free: c.free !== null ? fmtBytes(c.free) : { t: "card.checking" },
        fs: c.fs || { t: "card.checking" }, name: c.name || { t: "card.noname" } });
      factsEl.append(d);
      return;
    }
    factsEl.append(elT("div", "mut", k === "ok" ? "card.checking" : "card.na"));
  }

  // ---- speed test ----
  const KINDS = { raw: "card.test.raw", file: "card.test.file" };
  function renderTest() {
    testEl.textContent = "";
    if (kind() !== "ok") return;
    const T = S.test, running = !!(T && T.running);
    testEl.append(elT("div", "csub", "card.test.title"));
    const row = el("div", "row");
    for (const kd of ["raw", "file"]) {
      const b = elT("button", "ctl", KINDS[kd]); b.dataset.kind = kd; b.disabled = !isOn() || running;
      b.onclick = () => startTest(kd); row.append(b);
    }
    testEl.append(row, elT("div", "mut cnote", "card.test.note"));
    if (running) {
      const p = el("div", "cprogwrap");
      p.append(elT("div", "cprogtxt", T.kind === "file" ? "card.test.running.file" : "card.test.running.raw", { pct: T.pct }));
      const bar = el("div", "cprog"); bar.setAttribute("role", "progressbar"); bar.setAttribute("aria-valuemin", "0"); bar.setAttribute("aria-valuemax", "100"); bar.setAttribute("aria-valuenow", T.pct);
      const i = el("i"); i.style.width = T.pct + "%"; bar.append(i);
      const cb = elT("button", "ctl", "btn.cancel"); cb.disabled = !isOn(); cb.onclick = cancelTest;
      const r2 = el("div", "row"); bar.style.flex = "1 1 8rem"; r2.append(bar, cb);
      p.append(r2); testEl.append(p);
    }
    if (S.testMsg) { const m = elT("div", "cmsg" + (S.testMsg.bad ? " bad" : ""), S.testMsg.key, S.testMsg.params); m.setAttribute("role", "status"); testEl.append(m); }
    if (T && T.fail && !running) {
      const m = elT("div", "cmsg bad", "card.test.fail", { kind: { t: KINDS[T.fail.kind] }, why: reasonP(T.fail.why) }); testEl.append(m);
    }
    if (T && !running) renderResults(T);
  }
  function group(titleKey, time) {
    const gp = el("div", "cgrp"), h = el("div", "cgh");
    h.append(elT("strong", "", titleKey));
    if (time) h.append(el("span", "mut", when(time)));
    gp.append(h); return gp;
  }
  function kvRows(gp, rows) {
    const d = el("div", "ckv");
    for (const [k, v] of rows) { d.append(elT("span", "", k)); const s = el("span"); if (typeof v === "string") s.textContent = v; else setT(s, v.t); d.append(s); }
    gp.append(d);
  }
  function renderResults(T) {
    const res = el("div", "cres");
    if (T.raw) {
      const gp = group("card.test.raw", T.raw.time), r = T.raw;
      kvRows(gp, [["card.res.read3", `${mbs(r.avg16)} / ${mbs(r.slow16)} / ${mbs(r.max64)} MB/s`],
        ["card.res.worst", num(r.worst_us) !== null ? (num(r.worst_us) / 1000).toFixed(1) + " ms" : "?"]]);
      res.append(gp);
    }
    if (T.file) {
      const gp = group("card.test.file", T.file.time), f = T.file;
      kvRows(gp, [["card.res.write", mbs(f.write) + " MB/s"], ["card.res.read", mbs(f.read) + " MB/s"], ["card.res.verify", { t: f.verify === "1" ? "card.res.pass" : "card.res.nopass" }]]);
      res.append(gp);
    }
    const fmts = T.rows.filter(x => x.name !== undefined);
    if (fmts.length) {
      const gp = el("div", "cgrp");
      gp.append(el("div", "cgh")); gp.firstChild.append(elT("strong", "", "card.ladder.title"));
      if (T.basis && T.basis.speed > 0) gp.append(elT("div", "mut cnote", "card.ladder.basis", { what: { t: KINDS[T.basis.kind] } }));
      for (const r of T.rows) {
        if (r.mark) {
          if (!(r.speed > 0)) continue;
          const m = el("div", "cmark" + (T.basis && T.basis.kind === r.mark ? " basis" : ""));
          m.append(elT("span", "", r.mark === "raw" ? "card.ladder.markRaw" : "card.ladder.markFile", { v: r.speed.toFixed(2) }));
          gp.append(m);
        } else {
          const l = el("div", "cl");
          l.append(el("span", "cln", r.name));
          const need = elT("span", "mut", "card.ladder.need", { v: mbs(r.need) });
          const g2 = ["ok", "marginal", "slow"].includes(r.grade) ? r.grade : "unknown";
          const b = elT("span", "gb " + g2, "card.grade." + g2);
          l.append(need, b); gp.append(l);
        }
      }
      gp.append(elT("div", "mut cnote help", "card.ladder.hint"));
      res.append(gp);
    }
    testEl.append(res);
  }
  async function startTest(kd) {
    S.testMsg = null;
    const r = await send("cardtest " + kd);
    const line = r.find(l => /^cardtest:/.test(l)) || "";
    let m;
    if (/^cardtest: started/.test(line)) S.test = Object.assign(S.test || parseTest([]), { running: true, kind: kd, pct: 0, fail: null });
    else if ((m = /^cardtest: refused\s*(.*)$/.exec(line))) S.testMsg = { key: "card.test.refused", params: { why: reasonP(m[1].trim() || "?") }, bad: true };
    else S.testMsg = { key: "card.test.noreply", bad: true };
    renderTest(); schedule();
    if (S.test && S.test.running) pollTest();
  }
  async function cancelTest() {
    const r = await send("cardtest cancel");
    S.testMsg = r.some(l => /cancelled/.test(l)) ? { key: "card.test.cancelled" } : null;
    await pollTest();
  }
  async function pollTest() {
    if (polling) return; polling = true;
    try {
      const p = parseTest(await send("cardtest"));
      if (p.seen) { S.test = p; }
    } finally { polling = false; }
    renderTest(); schedule();
  }
  function schedule() {
    const want = isOn() && !document.hidden && tabOpen() && S.test && S.test.running;
    if (want && !testTimer) testTimer = setInterval(() => { if (!isOn() || document.hidden || !tabOpen()) { schedule(); return; } pollTest(); }, 1000);
    else if (!want && testTimer) { clearInterval(testTimer); testTimer = 0; }
    const fwant = isOn() && S.fmt.phase === "running";
    if (fwant && !fmtTimer) fmtTimer = setInterval(pollFmt, 1000);
    else if (!fwant && fmtTimer) { clearInterval(fmtTimer); fmtTimer = 0; }
  }

  // ---- format (only for an unreadable card) ----
  const F = S.fmt;
  function fmtVisible() { return kind() === "bad" || F.phase === "running" || F.phase === "result"; }
  function setPhase(phase, extra) {
    Object.assign(F, { phase, plan: F.plan, code: "", deadline: 0, msg: null, result: null }, extra || {});
    clearInterval(cdTimer); cdTimer = 0;
    if (phase === "plan1" || phase === "plan2") { F.deadline = Date.now() + TTL * 1000; cdTimer = setInterval(tickCountdown, 250); }
    if (phase === "idle" && !extra) F.plan = null;
    renderFmt(true); schedule();
  }
  function tickCountdown() {
    const s = Math.ceil((F.deadline - Date.now()) / 1000), sp = fmtEl.querySelector(".ccount");
    if (s <= 0) { send("cardfmt cancel"); setPhase("idle", { msg: { key: "card.fmt.expired", bad: true } }); return; }
    if (sp) setT(sp, "card.fmt.countdown", { s });
  }
  function renderFmt(force) {
    const vis = fmtVisible();
    const sig = JSON.stringify([vis, F.phase, F.plan, F.code, F.msg, F.result, isOn()]);
    if (!force && sig === fmtSig) return;
    fmtSig = sig;
    fmtEl.textContent = "";
    if (!vis) return;
    const wrap = el("div", "cfmt" + (F.phase === "plan2" ? " final" : "")); wrap.setAttribute("role", "group");
    fmtEl.append(wrap);
    if (F.phase === "idle") {
      wrap.append(elT("div", "csub nomt", "card.fmt.title"), elT("div", "", "card.fmt.explain"));
      if (F.msg) { const m = elT("div", "cmsg" + (F.msg.bad ? " bad" : ""), F.msg.key, F.msg.params); m.setAttribute("role", "status"); wrap.append(m); }
      const b = elT("button", "ctl danger", "card.fmt.start"); b.style.marginTop = ".7rem"; b.disabled = !isOn(); b.onclick = startFmt; wrap.append(b);
    } else if (F.phase === "plan1" || F.phase === "plan2") {
      const second = F.phase === "plan2";
      wrap.append(elT("div", "csub nomt", second ? "card.fmt.s2.title" : "card.fmt.s1.title"));
      if (F.plan) wrap.append(elT("div", "", "card.fmt.plan", { fs: F.plan.fs || "?", label: F.plan.label || "?", size: F.plan.size !== null ? fmtBytes(F.plan.size) : "?" }));
      wrap.append(elT("div", "cwarn", "card.fmt.warn"));
      wrap.append(elT("div", "", second ? "card.fmt.s2.how" : "card.fmt.s1.how"));
      const code = el("div", "ccode", F.code); code.setAttribute("aria-label", t("card.fmt.code.aria")); code.setAttribute("data-i18n-aria", "card.fmt.code.aria"); wrap.append(code);
      const inp = el("input"); inp.type = "text"; inp.inputMode = "numeric"; inp.maxLength = 4; inp.autocomplete = "off"; inp.spellcheck = false;
      inp.className = "ccodein"; inp.setAttribute("data-i18n-aria", "card.fmt.input.aria"); inp.setAttribute("aria-label", t("card.fmt.input.aria"));
      inp.setAttribute("data-i18n-placeholder", "card.fmt.input.ph"); inp.setAttribute("placeholder", t("card.fmt.input.ph"));
      inp.oninput = () => { const v = inp.value.replace(/\D/g, "").slice(0, 4); if (v !== inp.value) inp.value = v; ok.disabled = !isOn() || fmtBusy || !v; };
      const ok = elT("button", "ctl " + (second ? "danger" : "primary"), second ? "card.fmt.s2.go" : "card.fmt.confirm"); ok.disabled = true;
      const go = () => { const v = inp.value; if (v && !fmtBusy && isOn()) confirmFmt(v); };
      ok.onclick = go; inp.onkeydown = e => { if (e.key === "Enter") { e.preventDefault(); go(); } };
      const r1 = el("div", "row"); inp.style.flex = "1 1 7rem"; r1.append(inp, ok); wrap.append(r1);
      const r2 = el("div", "row cfoot"); const cd = el("span", "mut ccount grow"); setT(cd, "card.fmt.countdown", { s: Math.max(0, Math.ceil((F.deadline - Date.now()) / 1000)) });
      const cb = elT("button", "ctl", "btn.cancel"); cb.disabled = !isOn(); cb.onclick = cancelFmt; r2.append(cd, cb); wrap.append(r2);
      if (!second) setTimeout(() => { if (inp.isConnected && tabOpen()) { try { inp.focus({ preventScroll: true }); } catch { /* no focus */ } } }, 0);
    } else if (F.phase === "running") {
      wrap.append(elT("div", "csub nomt", "card.fmt.title"), elT("div", "", "card.fmt.running"));
      const bar = el("div", "cprog ind"); bar.setAttribute("role", "progressbar"); bar.append(el("i")); wrap.append(bar);
    } else if (F.phase === "result") {
      const okr = F.result && F.result.ok;
      const m = F.result && F.result.why ? elT("div", "cmsg bad", "card.fmt.failed", { why: reasonP(F.result.why) }) : elT("div", "cmsg" + (okr ? " good" : " bad"), okr ? "card.fmt.done" : "card.fmt.failed0");
      m.setAttribute("role", "status");
      const b = elT("button", "ctl", "card.fmt.dismiss"); b.style.marginTop = ".6rem"; b.onclick = () => { setPhase("idle"); renderAll(); };
      wrap.append(m, b);
    }
  }
  const fmtLine = r => r.find(l => /^cardfmt:/.test(l)) || "";
  // the replies of `cardfmt` and `cardfmt <code>`: confirm <code> | confirm again <code> | started | refused <why> | (nothing)
  function fmtReply(r, fromStep) {
    const l = fmtLine(r); let m;
    if ((m = /^cardfmt: confirm again (\d+)/.exec(l))) return setPhase("plan2", { code: m[1] });
    if ((m = /^cardfmt: confirm (\d+)/.exec(l))) return setPhase("plan1", { code: m[1] });
    if (/^cardfmt: started/.test(l)) return setPhase("running");
    if ((m = /^cardfmt: refused\s*(.*)$/.exec(l))) return setPhase("idle", { msg: { key: "card.fmt.refused", params: { why: fmtWhy(m[1].trim()) }, bad: true } });
    setPhase("idle", { msg: { key: "card.fmt.noreply", bad: true } });
  }
  async function startFmt() {
    if (fmtBusy) return; fmtBusy = true;
    try {
      const r = await send("cardfmt");
      const pl = r.find(l => /^cardfmt plan\b/.test(l));
      if (pl) { const k = kv(pl); F.plan = { fs: k.fs, label: k.label, size: num(k.size) }; } else F.plan = null;
      fmtReply(r, 0);
    } finally { fmtBusy = false; }
  }
  async function confirmFmt(v) {
    fmtBusy = true; const was = F.phase;
    for (const e of fmtEl.querySelectorAll("button,input")) e.disabled = true;
    try { const r = await send("cardfmt " + v); fmtReply(r, was === "plan1" ? 1 : 2); } finally { fmtBusy = false; }
  }
  async function cancelFmt() {
    fmtBusy = true; clearInterval(cdTimer); cdTimer = 0;
    try { await send("cardfmt cancel"); } finally { fmtBusy = false; }
    setPhase("idle", { msg: { key: "card.fmt.cancelled" } });
  }
  async function pollFmt() {
    if (fmtPolling || !isOn()) return; fmtPolling = true;
    try {
      const r = await send("cardfmt status");
      const m = /^cardfmt state (\w+)(?:\s+(.*))?$/.exec(r.find(l => /^cardfmt state\b/.test(l)) || "");
      if (m && F.phase === "running") {
        if (m[1] === "done") { setPhase("result", { result: { ok: true } }); load(); }
        else if (m[1] === "failed") { setPhase("result", { result: { ok: false, why: (m[2] || "").trim() } }); load(); }
        else if (m[1] === "idle") { setPhase("idle"); load(); }
      }
    } finally { fmtPolling = false; }
  }

  // ---- loading ----
  function renderAll() { renderBanner(); renderFacts(); renderTest(); renderFmt(); schedule(); }
  function load() {
    if (loading) return loading;
    loading = (async () => {
      try {
        if (!isOn()) return;
        const c = parseCard(await send("card"));
        if (c) S.card = c;
        lastCard = (getState() || {}).card || lastCard;
        if (S.card && S.card.state === "mounted") {
          const p = parseTest(await send("cardtest")); if (p.seen) S.test = p;
        }
        if (F.phase === "running" || (S.card && (S.card.state === "unreadable" || S.card.fmt === "running"))) {
          const r = await send("cardfmt status");
          const m = /^cardfmt state (\w+)/.exec(r.find(l => /^cardfmt state\b/.test(l)) || "");
          if (m && m[1] === "running" && F.phase !== "running") setPhase("running");
        }
      } finally { loading = null; renderAll(); }
    })();
    return loading;
  }

  function onState(st) {
    const k = st ? st.card : null;
    if (st && k !== lastCard) { lastCard = k; if (isOn()) load(); }
    // a confirmation in progress is void as soon as the card is no longer the unreadable one
    if ((F.phase === "plan1" || F.phase === "plan2") && k !== "bad") { clearInterval(cdTimer); cdTimer = 0; Object.assign(F, { phase: "idle", plan: null, code: "", msg: null }); }
    renderAll();
  }
  function reset() {
    lastCard = null; S.card = null; S.test = null; S.testMsg = null;
    clearInterval(cdTimer); cdTimer = 0; Object.assign(F, { phase: "idle", plan: null, code: "", msg: null, result: null });
    renderAll();
  }
  document.addEventListener("visibilitychange", () => { if (!document.hidden && isOn() && tabOpen()) load(); else schedule(); });
  renderAll();
  return { onState, load, reset, rerender: renderAll, info: () => S.card };
}

