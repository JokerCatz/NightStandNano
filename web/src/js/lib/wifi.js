// SPDX-License-Identifier: LGPL-2.1-or-later
// NightStand Nano: the Wi-Fi page (Settings > Wi-Fi). An ES module; the parsers and helpers need no DOM
// (they are unit-tested under node, see web/test).
//
// Box commands used: `wifi`, `wifi set <ssid>|<password>`, `wifi forget`, `wifi scan` (see the firmware's cmd.c).
//   wifi <state> ip=<ip> rssi=<n> time=[(unsynced) ]YYYY-MM-DD HH:MM:SS src=nvs|card|none ssid="<escaped>"
//   wifi ap <rssi> <open|secured> "<escaped ssid>"   ...   wifi scan: <n> found | busy | failed
// The password is write-only: the box never returns it, the page never keeps it (no storage, the field is cleared when it is
// sent) and the log pane gets `wifi set <ssid>|••••` (redact(), hooked into the page's logging of sent commands).
//
// opts: { box, send(cmd) -> Promise<lines>, sendUntil(cmd, re, ms) -> Promise<lines>, getState() -> st|null, isOn() -> bool,
//         tabOpen() -> bool, onBusy(bool) (a scan is running: the page pauses its own polling) }
// returns: { load(), onState(st), reset(), rerender() }   (texts follow a language change by themselves: they are all set with setT)
import { I18N } from "../core/i18n.js";

const sleep = ms => new Promise(r => setTimeout(r, ms));
const unesc = s => s.replace(/\\(["\\])/g, "$1");

// ---- pure helpers ----
export function utf8Len(s) {   // bytes of the UTF-8 encoding (a lone surrogate counts as U+FFFD, like TextEncoder)
  let n = 0;
  for (const ch of String(s)) { const c = ch.codePointAt(0); n += c < 0x80 ? 1 : c < 0x800 ? 2 : c < 0x10000 ? 3 : 4; }
  return n;
}
const RE_WIFI = /^wifi (.+?) ip=(\S*) rssi=(-?\d+|\S*) time=(.*?) src=(nvs|card|none) ssid="(.*)"$/;
export function parseWifi(lines) {
  for (const raw of lines || []) {
    const m = RE_WIFI.exec(String(raw).trimEnd()); if (!m) continue;
    return { state: m[1], ip: m[2], rssi: /^-?\d+$/.test(m[3]) ? +m[3] : null, synced: !/^\(unsynced\)/.test(m[4]), time: m[4].replace(/^\(unsynced\)\s*/, ""),
      src: m[5], ssid: unesc(m[6]) };
  }
  return null;
}
export function parseScan(lines) {
  const r = { aps: [], status: null, n: 0 };
  for (const raw of lines || []) {
    const l = String(raw).trimEnd(); let m;
    if ((m = /^wifi ap (-?\d+) (open|secured) "(.*)"$/.exec(l))) r.aps.push({ rssi: +m[1], secured: m[2] === "secured", ssid: unesc(m[3]) });
    else if ((m = /^wifi scan: (?:(\d+) found|(busy)|(failed))/.exec(l))) { r.status = m[2] || m[3] || "found"; r.n = m[1] ? +m[1] : r.aps.length; }
  }
  return r;
}
export const stateKey = s => "wifi.state." + String(s).toLowerCase().replace(/[^a-z]+/g, "_").replace(/^_|_$/g, "");
export function sigLevel(rssi) { return rssi >= -55 ? 4 : rssi >= -65 ? 3 : rssi >= -75 ? 2 : 1; }   // 4 very strong ... 1 weak
// null when the pair can be sent, else { key, params } of the message to show
export function validate(ssid, pw) {
  if (/[\x00-\x1f\x7f]/.test(ssid) || /[\x00-\x1f\x7f]/.test(pw)) return { key: "wifi.err.ctrl" };
  const sb = utf8Len(ssid), pb = utf8Len(pw);
  if (!sb) return { key: "wifi.err.ssidEmpty" };
  if (ssid.includes("|")) return { key: "wifi.err.bar" };   // the command splits at the first "|"
  if (sb > 32) return { key: "wifi.err.ssidLong", params: { n: sb } };
  if (pb && (pb < 8 || pb > 63)) return { key: "wifi.err.pwLen", params: { n: pb } };
  return null;
}
// the box's refusal reason -> a string key (or null: show it as sent)
export function whyKey(r) {
  r = String(r || "");
  return /usage/i.test(r) ? "wifi.why.usage" : /^ssid/i.test(r) ? "wifi.why.ssid" : /^password/i.test(r) ? "wifi.why.pw"
    : /control/i.test(r) ? "wifi.why.ctrl" : /cannot store/i.test(r) ? "wifi.why.store" : null;
}
// Log hook: a command that carries the password is shown with the password blanked (split at the first "|", as the box does).
export function redact(cmd) {
  if (typeof cmd !== "string" || !cmd.startsWith("wifi set ")) return cmd;
  const i = cmd.indexOf("|");
  return i >= 0 && i < cmd.length - 1 ? cmd.slice(0, i + 1) + "\u2022\u2022\u2022\u2022" : cmd;
}

export function mount(o) {
  const I = I18N, setT = I.setT, setRaw = I.setRaw, t = I.t;
  const { box, send, sendUntil, getState, isOn, tabOpen } = o;
  const el = (tag, cls, text) => { const e = document.createElement(tag); if (cls) e.className = cls; if (text !== undefined) e.textContent = text; return e; };
  const elT = (tag, cls, key, params) => { const e = el(tag, cls); setT(e, key, params); return e; };
  const S = { info: null, scan: { busy: false, aps: null, msg: null }, msg: null, joining: false, tok: 0, lastWifi: undefined };
  let timer = 0, refreshing = null;

  // ---- layout ----
  const head = el("div", "cstate"), facts = el("div", "ckv");
  const scanRow = el("div", "row"), scanBtn = elT("button", "ctl", "wifi.scan.btn"), scanMsg = el("span", "mut grow");
  scanMsg.setAttribute("role", "status"); scanRow.append(scanBtn, scanMsg);
  const scanList = el("ul", "wscan"); scanList.hidden = true;
  const nameIn = el("input", "ctl"), pwIn = el("input", "ctl"), pwBtn = elT("button", "ctl", "wifi.pw.show");
  nameIn.type = "text"; nameIn.autocomplete = "off"; nameIn.spellcheck = false; nameIn.setAttribute("autocapitalize", "off"); nameIn.setAttribute("autocorrect", "off");
  pwIn.type = "password"; pwIn.autocomplete = "off"; pwIn.spellcheck = false; pwIn.setAttribute("autocapitalize", "off"); pwIn.setAttribute("autocorrect", "off");
  pwIn.setAttribute("data-i18n-placeholder", "wifi.pw.ph"); pwIn.placeholder = t("wifi.pw.ph");
  pwBtn.setAttribute("aria-pressed", "false");
  const lab = (key, ...kids) => { const l = el("label", "wlab"); l.append(elT("span", "mut", key), ...kids); return l; };
  const pwRow = el("div", "wpw"); pwRow.append(pwIn, pwBtn);
  const note = elT("div", "mut cnote", "wifi.insecure");
  const limits = elT("div", "mut cnote help", "wifi.limits");
  const joinBtn = elT("button", "ctl primary", "wifi.join"), forgetBtn = elT("button", "ctl", "wifi.forget");
  const act = el("div", "row"); act.style.marginTop = ".7rem"; act.append(joinBtn, forgetBtn);
  const msgEl = el("div", "cmsg"); msgEl.setAttribute("role", "status");
  box.textContent = "";
  box.append(head, facts, scanRow, scanList, lab("wifi.name.label", nameIn), lab("wifi.pw.label", pwRow), note, limits, act, msgEl);
  scanRow.style.marginTop = ".8rem";

  function setMsg(key, params, kind) { S.msg = key ? { key, params, kind } : null; msgEl.className = "cmsg" + (kind ? " " + kind : ""); if (key) setT(msgEl, key, params); else setRaw(msgEl, ""); }

  // ---- render ----
  const bars = level => { const s = el("span", "sig"); s.setAttribute("aria-hidden", "true"); for (let i = 1; i <= 4; i++) s.append(el("i", i <= level ? "on" : "")); return s; };
  function render() {
    const on = isOn(), p = S.info;
    facts.textContent = "";
    head.className = "cstate";
    if (!p) setT(head, on ? "wifi.reading" : "card.wait");
    else {
      const k = stateKey(p.state);
      if (I.has(k)) setT(head, k); else setRaw(head, p.state);
      if (/wrong|not found/i.test(p.state)) head.classList.add("bad");
      const row = (key, fill) => { facts.append(elT("span", "", key)); const v = el("span"); fill(v); facts.append(v); };
      if (p.ssid) row("wifi.k.net", v => setRaw(v, p.ssid));
      if (p.state === "connected" && p.rssi !== null) row("wifi.k.signal", v => { const lv = sigLevel(p.rssi); v.append(bars(lv)); const w = el("span"); setT(w, "wifi.sig.fmt", { q: { t: "wifi.sig." + lv }, rssi: p.rssi }); v.append(w); });
      if (p.state === "connected" && p.ip && p.ip !== "0.0.0.0") row("wifi.k.ip", v => setRaw(v, p.ip));
      if (p.src !== "none") row("wifi.k.src", v => setT(v, "wifi.src." + p.src));
    }
    scanBtn.disabled = !on || S.scan.busy;
    setT(scanBtn, S.scan.busy ? "wifi.scan.busyBtn" : "wifi.scan.btn");
    joinBtn.disabled = !on || S.joining;
    forgetBtn.disabled = !on || S.joining || (!!p && p.src === "none");
    pwBtn.disabled = false;
    for (const e of [nameIn, pwIn]) e.disabled = !on;
  }
  function renderScan() {
    const sc = S.scan;
    if (sc.busy) setT(scanMsg, "wifi.scan.running"); else if (sc.msg) setT(scanMsg, sc.msg.key, sc.msg.params); else setRaw(scanMsg, "");
    scanList.hidden = !(sc.aps && sc.aps.length);
    scanList.textContent = "";
    for (const ap of sc.aps || []) {
      const li = el("li"); li.tabIndex = 0; li.className = "wap";
      const nm = el("span", "nm"); nm.textContent = ap.ssid || "\u00a0";
      li.append(bars(sigLevel(ap.rssi)), nm);
      if (ap.secured) { const k = el("span", "lock", "\uD83D\uDD12"); k.setAttribute("role", "img"); k.setAttribute("data-i18n-aria", "wifi.scan.secured"); k.setAttribute("aria-label", t("wifi.scan.secured")); li.append(k); }
      else li.append(elT("span", "rtag", "wifi.scan.open"));
      li.onclick = () => { nameIn.value = ap.ssid; pwIn.value = ""; setMsg(null); (ap.secured ? pwIn : joinBtn).focus({ preventScroll: true }); };
      scanList.append(li);
    }
    render();
  }

  // ---- reading the state ----
  function refresh() {
    if (refreshing) return refreshing;
    refreshing = (async () => {
      try {
        if (!isOn()) return;
        const p = parseWifi(await send("wifi"));
        if (p) S.info = p;
      } finally { refreshing = null; render(); }
    })();
    return refreshing;
  }
  const load = () => refresh();

  // ---- scan: the box is busy ~3 s and answers only at the end; nothing else is sent meanwhile (the queue is serial, and the page pauses its polling) ----
  async function scan() {
    if (S.scan.busy || !isOn()) return;
    S.scan.busy = true; S.scan.msg = null; o.onBusy(true); renderScan();
    let r = [];
    try { r = await sendUntil("wifi scan", /^wifi scan:/, 10000); }
    finally { S.scan.busy = false; o.onBusy(false); }
    const p = parseScan(r);
    if (p.status === "found") { S.scan.aps = p.aps; S.scan.msg = p.aps.length ? { key: "wifi.scan.found", params: { n: p.aps.length } } : { key: "wifi.scan.none" }; }
    else if (p.status === "busy") S.scan.msg = { key: "wifi.scan.isBusy" };
    else if (p.status === "failed") S.scan.msg = { key: "wifi.scan.failed" };
    else S.scan.msg = { key: "wifi.scan.noreply" };
    renderScan();
  }
  scanBtn.onclick = scan;

  // ---- connect: validate, send, then watch `wifi` for up to ~20 s ----
  async function join() {
    if (S.joining || !isOn()) return;
    const ssid = nameIn.value, pw = pwIn.value, bad = validate(ssid, pw);
    if (bad) { setMsg(bad.key, bad.params, "bad"); return; }
    S.joining = true; render();
    const tok = ++S.tok;
    const reply = send("wifi set " + ssid + "|" + pw);   // the command is queued with its own copy of the text
    pwIn.value = ""; hidePw();                            // never kept: the field is empty from here on
    setMsg("wifi.set.sending", null, "");
    let r; try { r = await reply; } finally { S.joining = false; }
    const line = r.find(l => /^wifi set:/.test(l)) || "";
    let m;
    if (/^wifi set: ok/.test(line)) { render(); await watch(ssid, tok); return; }
    if ((m = /^wifi set: refused\s*(.*)$/.exec(line))) { const k = whyKey(m[1]); setMsg("wifi.refused", { why: k ? { t: k } : m[1] || "?" }, "bad"); }
    else setMsg("wifi.set.noreply", null, "bad");
    refresh();
  }
  async function watch(ssid, tok) {
    const t0 = Date.now(); let sawConnecting = false;
    setMsg("wifi.set.trying", { s: 20 }, "");
    while (Date.now() - t0 < 20000) {
      await sleep(1000);
      if (tok !== S.tok || !isOn()) return;
      await refresh();
      const p = S.info, el0 = Date.now() - t0;
      if (tok !== S.tok) return;
      if (p && p.ssid === ssid) {
        if (p.state === "connecting") sawConnecting = true;
        if (p.state === "connected") { setMsg("wifi.set.ok", { ssid }, "good"); return; }
        if ((sawConnecting || el0 >= 3000) && p.state === "wrong password") { setMsg("wifi.set.wrong", null, "bad"); return; }
        if ((sawConnecting || el0 >= 3000) && p.state === "not found") { setMsg("wifi.set.notfound", { ssid }, "bad"); return; }
      }
      setMsg("wifi.set.trying", { s: Math.max(0, 20 - Math.round(el0 / 1000)) }, "");
    }
    if (tok === S.tok) setMsg("wifi.set.timeout", null, "");
  }
  async function forget() {
    if (S.joining || !isOn()) return;
    if (!globalThis.confirm(t("wifi.forget.confirm"))) return;
    S.tok++;
    const r = await send("wifi forget");
    if (r.some(l => /^wifi forget: ok/.test(l))) setMsg("wifi.forget.done", null, "good"); else setMsg("wifi.forget.fail", null, "bad");
    refresh();
  }
  function hidePw() { pwIn.type = "password"; pwBtn.setAttribute("aria-pressed", "false"); setT(pwBtn, "wifi.pw.show"); }
  pwBtn.onclick = () => { const show = pwIn.type === "password"; pwIn.type = show ? "text" : "password"; pwBtn.setAttribute("aria-pressed", String(show)); setT(pwBtn, show ? "wifi.pw.hide" : "wifi.pw.show"); };
  joinBtn.onclick = join; forgetBtn.onclick = forget;
  for (const e of [nameIn, pwIn]) e.onkeydown = ev => { if (ev.key === "Enter") { ev.preventDefault(); join(); } };

  // ---- state from the page's 1 s poll / housekeeping ----
  function onState(st) {
    const w = st ? st.wifi : undefined;
    if (st && w !== S.lastWifi) { const first = S.lastWifi === undefined; S.lastWifi = w; if (!first && isOn() && !S.scan.busy) refresh(); }
    render();
  }
  function reset() {
    S.info = null; S.tok++; S.joining = false; S.lastWifi = undefined; S.scan = { busy: false, aps: null, msg: null }; o.onBusy(false);
    setMsg(null); renderScan();
  }
  timer = setInterval(() => { if (isOn() && tabOpen() && !document.hidden && !S.scan.busy && !S.joining) refresh(); }, 10000);   // rssi drifts
  render(); renderScan();
  return { load, onState, reset, rerender: () => { render(); }, info: () => S.info };
}

