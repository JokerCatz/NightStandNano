// SPDX-License-Identifier: LGPL-2.1-or-later
// The "Hardware setup" tab: read the box's SD and LED pin settings over the UART console, edit them within the valid range, send them,
// restart the board and read them again (so someone wiring an SD socket sees at once whether the card is found).
// New for Nano (JukeDial has no hardware-pin protocol); the protocol itself is in core/proto.js.
import { $, el, show } from "./dom.js";
import { LED_ORDERS, SD_PINS, SPI_PINS, isAckLine, isHwReadLine, parseAck, parseHw, parseReason, pinReason, sdCommand, ledCommand, stepGpio, validateLed, validateSd } from "../core/proto.js";

export function initHwTab({ conn, t, lang, timing = {} }) {
  const T = { restartWait: 1500, pollEvery: 800, pollTotal: 25000, recheck: 3000, ...timing };
  let hw = null, form = null, view = "off", result = null, askDefaults = false, restarting = false, seq = 0, restartedMsg = null, whyPin = null, ledDefault = null;
  const touched = new Set();   // SPI fields the user has typed in (an untouched blank field is not shown as an error)
  const fieldName = (f) => t("hw.field." + f);

  // ---- form <-> device ----
  // sd.mode "sdmmc" | "spi"; width for SDMMC (the pins are the fixed ones of the `hw sdmmc` line); spi = the four SPI pins (blank = NaN)
  const formFrom = (h) => ({
    sd: { mode: h.sd.mode, width: h.sd.mode === "sdmmc" && h.sd.width === 1 ? 1 : 4,
      spi: h.sd.mode === "spi" ? { clk: h.sd.clk, mosi: h.sd.mosi, miso: h.sd.miso, cs: h.sd.cs } : { clk: NaN, mosi: NaN, miso: NaN, cs: NaN } },
    led: { on: h.led.on, gpio: h.led.on && h.led.gpio >= 0 ? h.led.gpio : (ledDefault ?? NaN), order: LED_ORDERS.includes(h.led.order) ? h.led.order : "grb" },
  });
  // what the form says as one SD setting (the shape of the `hw sd` line and of validateSd)
  const sdOf = (f, h) => (f.sd.mode === "spi" ? { mode: "spi", ...f.sd.spi }
    : { mode: "sdmmc", ...Object.fromEntries(SD_PINS.map((k) => [k, f.sd.width === 1 && ["d1", "d2", "d3"].includes(k) ? -1 : h.sdmmc[k]])), width: f.sd.width });

  // ---- reading ----
  async function read({ keepForm = false } = {}) {
    if (!conn.up) { view = "off"; render(); return null; }
    const my = ++seq;
    if (view !== "body") view = "reading";
    render();
    try {
      const lines = await conn.request("hw", { accept: isHwReadLine, until: (ls) => ls.some((l) => /^hw pending/.test(l)), timeout: 3000, partial: true });
      return accept(parseHw(lines), my, keepForm);
    } catch (e) {
      if (my === seq) { view = conn.up ? "noanswer" : "off"; hw = view === "noanswer" ? null : hw; render(); }
      return null;
    }
  }
  function accept(h, my, keepForm) {
    if (my !== seq) return null;
    if (!h.sd || !h.led || !h.sdmmc) { hw = null; view = "noanswer"; render(); return null; }
    if (h.led.src === "default" && h.led.on) ledDefault = h.led.gpio;   // the board's own LED pin: the one reserved pin the LED may use
    h.ledDefault = ledDefault;
    hw = h; if (!keepForm || !form) { form = formFrom(h); touched.clear(); }
    view = "body"; render();
    return h;
  }

  // ---- sending ----
  async function send(cmd, kind) {
    try {
      const [line] = await conn.request(cmd, { accept: isAckLine(kind), until: () => true, timeout: 3500 });
      return parseAck(line);
    } catch { view = conn.up ? "noanswer" : "off"; render(); return null; }
  }
  const resWhy = (n) => t("hw.res." + (hw ? pinReason(n) : "other"));
  const reasonText = (reason) => {
    const r = parseReason(reason);
    if (r.code === "notsup") return /sdmmc-pins-fixed/.test(r.why) ? t("hw.why.notsup.sdmmc") : t("hw.why.notsup.other", { raw: r.why || reason });
    if (r.code === "other") return t("hw.why.other", { raw: reason });
    return t("hw.why." + r.code, { n: r.n, why: r.code === "reserved" ? resWhy(r.n) : "" });
  };
  async function apply(kind) {
    const issues = kind === "sd" ? validateSd(sdOf(form, hw), hw) : validateLed(form.led, hw);
    if (issues.length) return;
    result = null; restartedMsg = null; render();
    const ack = await send(kind === "sd" ? sdCommand(sdOf(form, hw)) : ledCommand(form.led), kind);
    if (!ack) return;
    if (ack.ok) { result = { ok: true, key: "hw.ack.ok" }; await read({ keepForm: false }); }
    else result = { ok: false, key: "hw.refused", p: { why: reasonText(ack.text) }, raw: `hw ${ack.kind}: refused ${ack.text}` };
    render();
  }
  async function defaults() {
    askDefaults = false; result = null; restartedMsg = null; render();
    const ack = await send("hw reset", "reset");
    if (!ack) return;
    if (ack.ok) { result = { ok: true, key: "hw.ack.reset" }; await read(); }
    else result = { ok: false, key: "hw.refused", p: { why: reasonText(ack.text) }, raw: `hw ${ack.kind}: refused ${ack.text}` };
    render();
  }
  async function restart() {
    if (!conn.up || restarting) return;
    restarting = true; result = null; restartedMsg = null; render();
    try { await conn.send("reboot"); } catch { restarting = false; render(); return; }
    await new Promise((r) => setTimeout(r, T.restartWait));
    const my = ++seq;
    try {
      const lines = await conn.poll("hw", { accept: isHwReadLine, until: (ls) => ls.some((l) => /^hw pending/.test(l)), every: T.pollEvery, total: T.pollTotal, partial: true });
      restarting = false;
      const h = accept(parseHw(lines), my, false);
      if (h) { restartedMsg = h.sd.state; render(); setTimeout(recheck, T.recheck); }
    } catch { restarting = false; if (my === seq) { view = conn.up ? "noanswer" : "off"; render(); } }
    render();
  }
  async function recheck() {   // the card may mount a moment after the first answer
    if (!conn.up || restarting) return;
    const my = ++seq;
    try {
      const lines = await conn.request("hw", { accept: isHwReadLine, until: (ls) => ls.some((l) => /^hw pending/.test(l)), timeout: 3000, partial: true });
      const h = parseHw(lines);
      if (my === seq && h.sd && h.led && hw && (h.sd.state !== hw.sd.state)) { hw = h; restartedMsg = h.sd.state; render(); }
    } catch { /* keep what is shown */ }
  }

  // ---- inputs ----
  const spiBox = $("#hw-spi-pins"), mmcBox = $("#hw-sdmmc-pins");
  const inputs = {}, fixedOut = {};
  for (const f of SPI_PINS) {
    const input = el("input", { type: "number", step: "1", inputmode: "numeric", id: "hw-pin-" + f });
    const label = el("label", { class: "field", id: "hw-field-" + f }, el("span"), input);
    inputs[f] = input; spiBox.append(label);
    wirePin(input, f, (v) => { form.sd.spi[f] = v; touched.add(f); });
  }
  for (const f of SD_PINS) {
    const out = el("output", { class: "mono", id: "hw-fixed-" + f });
    fixedOut[f] = out; mmcBox.append(el("div", { class: "field fixed", id: "hw-fixedf-" + f }, el("span"), out));
  }
  const ledGpio = $("#hw-led-gpio"); wirePin(ledGpio, "led", (v) => { form.led.gpio = v; });
  function wirePin(input, f, set) {
    let prev = null;
    input.addEventListener("focus", () => { prev = input.value === "" ? null : Number(input.value); });
    input.addEventListener("keydown", (e) => {
      if ((e.key !== "ArrowUp" && e.key !== "ArrowDown") || !hw) return;
      e.preventDefault();
      const cur = input.value === "" ? (hw ? hw.gpios.min - 1 : 0) : Number(input.value);
      const n = stepGpio(cur, e.key === "ArrowUp" ? 1 : -1, hw);
      if (n !== null) { input.value = n; set(n); prev = n; render(); }
    });
    input.addEventListener("input", (e) => {
      let v = input.value === "" ? NaN : Number(input.value);
      // the spinner buttons: a step onto a reserved number jumps over it
      if (Number.isInteger(v) && prev !== null && Math.abs(v - prev) === 1 && hw && hw.reserved.includes(v) && !e.inputType) { const n = stepGpio(v, v - prev, hw); if (n !== null) { v = n; input.value = n; } }
      set(Number.isInteger(v) ? v : NaN); prev = Number.isInteger(v) ? v : null;
      render();
    });
  }
  for (const r of document.querySelectorAll('input[name="hw-sd-mode"]')) r.addEventListener("change", (e) => { if (e.target.checked && form) { form.sd.mode = e.target.value; render(); } });
  $("#hw-width").addEventListener("change", (e) => { form.sd.width = Number(e.target.value); render(); });
  $("#hw-led-on").addEventListener("change", (e) => { form.led.on = e.target.checked; render(); });
  const orderSel = $("#hw-led-order");
  for (const o of LED_ORDERS) orderSel.append(el("option", { value: o }, o.toUpperCase()));
  orderSel.addEventListener("change", (e) => { form.led.order = e.target.value; render(); });
  $("#hw-sd-apply").addEventListener("click", () => apply("sd"));
  $("#hw-led-apply").addEventListener("click", () => apply("led"));
  $("#hw-restart").addEventListener("click", restart);
  $("#hw-reread").addEventListener("click", () => { result = null; restartedMsg = null; read(); });
  $("#hw-defaults").addEventListener("click", () => { askDefaults = true; render(); });
  $("#hw-defaults-no").addEventListener("click", () => { askDefaults = false; render(); });
  $("#hw-defaults-yes").addEventListener("click", defaults);

  // ---- rendering ----
  function issueText(i, hwv) {
    const field = i.field === "mode" || i.field === "width" ? "" : fieldName(i.field), n = i.n;
    switch (i.code) {
      case "int": return t("hw.iss.int", { field });
      case "range": return t("hw.iss.range", { field, min: hwv.gpios.min, max: hwv.gpios.max });
      case "nopin": return t("hw.iss.nopin", { field, n });
      case "reserved": return t("hw.iss.reserved", { field, n, why: resWhy(n) });
      case "duplicate": return t("hw.iss.duplicate", { field, n });
      case "conflict": return t("hw.iss.conflict", { field, n, other: t(i.field === "led" ? "hw.other.sd" : "hw.other.led") });
      case "order": return t("hw.iss.order");
      case "width": return t("hw.iss.width");
      case "notsup": return t("hw.iss.notsup");
      default: return i.code;
    }
  }
  function renderReserved() {
    const box = $("#hw-reserved");
    if (!hw.reserved.length) { box.replaceChildren(); show($("#hw-reserved-why"), false); return; }
    box.replaceChildren(el("span", { class: "pinchips-label" }, t("hw.reserved")), " ",
      el("span", { class: "pinchips" }, ...hw.reserved.map((n) => el("button", {
        type: "button", class: "pinchip", title: resWhy(n), "aria-pressed": String(whyPin === n), "aria-label": "GPIO " + n + ": " + resWhy(n),
        onclick: () => { whyPin = whyPin === n ? null : n; renderReserved(); },
      }, String(n)))));
    const w = $("#hw-reserved-why"); show(w, whyPin !== null); w.textContent = whyPin !== null ? t("hw.reserved.why", { n: whyPin, why: resWhy(whyPin) }) : "";
  }
  function render() {
    show($("#hw-off"), view === "off"); show($("#hw-reading"), view === "reading"); show($("#hw-noanswer"), view === "noanswer");
    show($("#hw-body"), view === "body" && !!hw);
    for (const f of SPI_PINS) $("#hw-field-" + f).querySelector("span").textContent = fieldName(f);
    for (const f of SD_PINS) $("#hw-fixedf-" + f).querySelector("span").textContent = fieldName(f);
    if (view !== "body" || !hw || !form) return;
    const busy = restarting, spi = form.sd.mode === "spi";
    const sdv = sdOf(form, hw);
    const sdIssues = validateSd(sdv, hw).filter((i) => !(i.code === "int" && !touched.has(i.field))), ledIssues = validateLed(form.led, hw);
    const sdBlank = spi && SPI_PINS.some((f) => !Number.isInteger(form.sd.spi[f]));
    const st = hw.sd.state;
    const chip = $("#hw-sd-state"); chip.textContent = t("hw.sd.state." + (["mounted", "unreadable", "none"].includes(st) ? st : "none")); chip.className = "chip " + (st === "mounted" ? "ok" : st === "unreadable" ? "warn" : "");
    $("#hw-sd-meta").textContent = t("hw.src." + (hw.sd.src === "custom" ? "custom" : "default")) + " · " + t("hw.sd.mode", { mode: hw.sd.mode === "spi" ? "SPI" : "SDMMC" });
    show($("#hw-sd-statenote"), hw.pending);
    $("#hw-range").textContent = t("hw.gpio.range", hw.gpios);
    renderReserved();
    // connection type
    $("#hw-mode-sdmmc").checked = !spi; $("#hw-mode-spi").checked = spi;
    for (const r of document.querySelectorAll('input[name="hw-sd-mode"]')) r.disabled = busy;
    show($("#hw-sd-sdmmc"), !spi); show($("#hw-sd-spi"), spi);
    show($("#hw-powercycle"), spi || hw.sd.mode === "spi");
    // SDMMC: the fixed pins (read-only, from the `hw sdmmc` line), only the width is chosen
    $("#hw-width").value = String(form.sd.width); $("#hw-width").disabled = busy;
    for (const f of SD_PINS) {
      const off = form.sd.width === 1 && (f === "d1" || f === "d2" || f === "d3");
      fixedOut[f].textContent = off ? t("hw.fixed.off") : String(hw.sdmmc[f]);
      $("#hw-fixedf-" + f).classList.toggle("off", off);
    }
    // SPI: four free pins
    const bad = new Set(sdIssues.map((i) => i.field));
    for (const f of SPI_PINS) {
      const inp = inputs[f]; inp.min = hw.gpios.min; inp.max = hw.gpios.max; inp.disabled = busy;
      const v = Number.isNaN(form.sd.spi[f]) ? "" : String(form.sd.spi[f]);
      if (inp.value !== v) inp.value = v;
      $("#hw-field-" + f).classList.toggle("bad", spi && bad.has(f));
    }
    const lines = sdIssues.map((i) => issueText(i, hw));
    if (sdBlank && !lines.length) lines.push(t("hw.iss.blank"));
    $("#hw-sd-issues").replaceChildren(...lines.map((x) => el("li", {}, x)));
    $("#hw-sd-apply").disabled = sdBlank || validateSd(sdv, hw).length > 0 || busy;
    // LED
    $("#hw-led-on").checked = form.led.on; $("#hw-led-on").disabled = busy;
    ledGpio.min = hw.gpios.min; ledGpio.max = hw.gpios.max; ledGpio.disabled = !form.led.on || busy;
    const lv = Number.isNaN(form.led.gpio) ? "" : String(form.led.gpio); if (ledGpio.value !== lv) ledGpio.value = lv;
    orderSel.value = form.led.order; orderSel.disabled = !form.led.on || busy;
    ledGpio.parentElement.classList.toggle("bad", ledIssues.some((i) => i.field === "led")); ledGpio.parentElement.classList.toggle("off", !form.led.on);
    $("#hw-led-src").textContent = t("hw.src." + (hw.led.src === "custom" ? "custom" : "default"));
    $("#hw-led-issues").replaceChildren(...ledIssues.map((i) => el("li", {}, issueText(i, hw))));
    $("#hw-led-apply").disabled = ledIssues.length > 0 || busy;
    // pending / result / restart
    show($("#hw-pending"), hw.pending && !restarting);
    $("#hw-restart").className = hw.pending ? "primary" : ""; $("#hw-restart").disabled = busy;
    $("#hw-defaults").disabled = busy; $("#hw-reread").disabled = busy;
    show($("#hw-defaults-ask"), askDefaults);
    show($("#hw-restarting"), restarting);
    const r = $("#hw-result");
    if (restartedMsg && !restarting) { show(r, true); r.className = "chip " + (restartedMsg === "mounted" ? "ok" : "warn"); r.textContent = t("hw.restarted", { state: t("hw.sd.state." + (["mounted", "unreadable"].includes(restartedMsg) ? restartedMsg : "none")) }); }
    else if (result) { show(r, true); r.className = "chip " + (result.ok ? "ok" : "bad"); r.textContent = t(result.key, result.p) + (result.raw ? t("hw.refused.raw", { raw: result.raw }) : ""); }
    else show(r, false);
  }

  conn.on("state", (s) => { if (s === "idle") { hw = null; form = null; whyPin = null; ledDefault = null; view = "off"; result = null; restarting = false; restartedMsg = null; render(); } else if (s === "flashing") { view = "off"; render(); } });
  lang.onChange(render);
  render();
  return { read, render, get hw() { return hw; }, get form() { return form; } };
}
