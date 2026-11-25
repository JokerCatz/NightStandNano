// SPDX-License-Identifier: LGPL-2.1-or-later
// NightStand Nano: equalizer helpers for the control page. An ES module (unit-tested under node).
//   - biquad coefficients (RBJ audio-EQ cookbook) and their magnitude response, for peaking / low shelf / high shelf at 48 kHz.
//     This is only an illustration for the graph: the box does the real filtering.
//   - parseAutoEq(): the AutoEq / Equalizer APO "ParametricEQ.txt" format.
//   - drawResponse(), drawSpark(): small canvas plots that take their colours from the page's CSS variables.
const FS = 48000;
const MAX_BANDS = 10;
const LIM = { fc: [20, 20000], gain: [-24, 24], q: [0.1, 10], preamp: [-24, 6] };
const clamp = (x, lo, hi) => Math.min(hi, Math.max(lo, x));

// type: 0 peaking, 1 low shelf, 2 high shelf. Returns [b0, b1, b2, a1, a2], already divided by a0.
function coeffs(type, fc, gainDb, q, fs) {
  fs = fs || FS;
  const w0 = 2 * Math.PI * clamp(fc, 1, fs * 0.49) / fs, cw = Math.cos(w0), sw = Math.sin(w0);
  const A = Math.pow(10, gainDb / 40), al = sw / (2 * Math.max(q, 1e-3));
  let b0, b1, b2, a0, a1, a2;
  if (type === 1) {
    const s = 2 * Math.sqrt(A) * al;
    b0 = A * ((A + 1) - (A - 1) * cw + s); b1 = 2 * A * ((A - 1) - (A + 1) * cw); b2 = A * ((A + 1) - (A - 1) * cw - s);
    a0 = (A + 1) + (A - 1) * cw + s; a1 = -2 * ((A - 1) + (A + 1) * cw); a2 = (A + 1) + (A - 1) * cw - s;
  } else if (type === 2) {
    const s = 2 * Math.sqrt(A) * al;
    b0 = A * ((A + 1) + (A - 1) * cw + s); b1 = -2 * A * ((A - 1) + (A + 1) * cw); b2 = A * ((A + 1) + (A - 1) * cw - s);
    a0 = (A + 1) - (A - 1) * cw + s; a1 = 2 * ((A - 1) - (A + 1) * cw); a2 = (A + 1) - (A - 1) * cw - s;
  } else {
    b0 = 1 + al * A; b1 = -2 * cw; b2 = 1 - al * A; a0 = 1 + al / A; a1 = -2 * cw; a2 = 1 - al / A;
  }
  return [b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0];
}
// |H(e^jw)| in dB at f Hz
function magDb(c, f, fs) {
  const w = 2 * Math.PI * f / (fs || FS), c1 = Math.cos(w), s1 = Math.sin(w), c2 = Math.cos(2 * w), s2 = Math.sin(2 * w);
  const nr = c[0] + c[1] * c1 + c[2] * c2, ni = -(c[1] * s1 + c[2] * s2);
  const dr = 1 + c[3] * c1 + c[4] * c2, di = -(c[3] * s1 + c[4] * s2);
  return 10 * Math.log10((nr * nr + ni * ni) / (dr * dr + di * di));
}
// total response in dB at each frequency: the preamp plus every band (bands: [{type, fc, gain, q}])
function response(bands, preamp, freqs, fs) {
  const cs = bands.map((b) => coeffs(b.type, b.fc, b.gain, b.q, fs));
  return freqs.map((f) => cs.reduce((s, c) => s + magDb(c, f, fs), preamp || 0));
}

// ---- AutoEq / Equalizer APO text ----
// Returns { preamp: number|null, bands: [{type, fc, gain, q}] (at most 10, clamped), skipped, overflow }.
//   skipped: filters that are OFF, of an unsupported type (LP, HP, BP, notch, ...) or without a frequency.
//   overflow: valid filters beyond the 10th, which are dropped.
const TYPES = { PK: 0, PEQ: 0, LS: 1, LSC: 1, HS: 2, HSC: 2 };
const num = (s) => parseFloat(String(s).replace(",", "."));
function parseAutoEq(text) {
  const out = { preamp: null, bands: [], skipped: 0, overflow: 0 };
  let valid = 0;
  for (const raw of String(text || "").split(/\r?\n/)) {
    const line = raw.trim();
    let m = /^preamp\s*:\s*([-+]?\d+(?:[.,]\d+)?)/i.exec(line);
    if (m) { out.preamp = clamp(num(m[1]), LIM.preamp[0], LIM.preamp[1]); continue; }
    m = /^filter\s*\d*\s*:\s*(on|off)\s+([a-z]+)\b(.*)$/i.exec(line);
    if (!m) continue;
    const type = TYPES[m[2].toUpperCase()], rest = m[3];
    const fc = /\bfc\s+([-+]?\d+(?:[.,]\d+)?)/i.exec(rest), gain = /\bgain\s+([-+]?\d+(?:[.,]\d+)?)/i.exec(rest), q = /\bq\s+([-+]?\d+(?:[.,]\d+)?)/i.exec(rest);
    if (m[1].toLowerCase() === "off" || type === undefined || !fc) { out.skipped++; continue; }
    valid++;
    if (out.bands.length >= MAX_BANDS) { out.overflow++; continue; }
    out.bands.push({
      type,
      fc: clamp(num(fc[1]), LIM.fc[0], LIM.fc[1]),
      gain: gain ? clamp(num(gain[1]), LIM.gain[0], LIM.gain[1]) : 0,
      q: q ? clamp(num(q[1]), LIM.q[0], LIM.q[1]) : 0.7,
    });
  }
  return out;
}

// ---- canvas drawing ----
function setup(cv) {
  const dpr = globalThis.devicePixelRatio || 1, w = cv.clientWidth, h = cv.clientHeight;
  if (!w || !h) return null;
  const pw = Math.round(w * dpr), ph = Math.round(h * dpr);
  if (cv.width !== pw) cv.width = pw;
  if (cv.height !== ph) cv.height = ph;
  const x = cv.getContext("2d"); x.setTransform(dpr, 0, 0, dpr, 0, 0); x.clearRect(0, 0, w, h);
  const cs = globalThis.getComputedStyle(cv), col = (n) => cs.getPropertyValue(n).trim() || "#888";
  return { x, w, h, line: col("--line"), mut: col("--mut"), acc: col("--acc"), fg: col("--fg") };
}
function drawResponse(cv, bands, preamp) {
  const s = setup(cv); if (!s) return;
  const { x, w, h } = s, L = 30, R = 6, T = 8, B = 14, pw = w - L - R, ph = h - T - B;
  const N = Math.max(40, Math.round(pw / 2)), fa = Math.log(20), fb = Math.log(20000);
  const freqs = []; for (let i = 0; i < N; i++) freqs.push(Math.exp(fa + (fb - fa) * i / (N - 1)));
  const db = response(bands, preamp, freqs);
  const peak = Math.max(0, ...db.map(Math.abs)), rng = [6, 12, 18, 24, 36].find((r) => r >= peak) || 48, rngv = Math.max(12, rng);
  const X = (f) => L + pw * (Math.log(f) - fa) / (fb - fa), Y = (d) => T + ph * (0.5 - clamp(d, -rngv, rngv) / (2 * rngv));
  x.font = "10px system-ui, sans-serif"; x.lineWidth = 1;
  // grid: 0 dB and +-range/2, +-range; octave-ish frequency lines
  x.strokeStyle = s.line; x.fillStyle = s.mut; x.textBaseline = "middle"; x.textAlign = "right";
  for (const d of [-rngv, -rngv / 2, 0, rngv / 2, rngv]) {
    const y = Math.round(Y(d)) + 0.5; x.globalAlpha = d === 0 ? 1 : 0.5;
    x.beginPath(); x.moveTo(L, y); x.lineTo(L + pw, y); x.stroke();
    x.globalAlpha = 1; if (d === 0 || Math.abs(d) === rngv) x.fillText((d > 0 ? "+" : "") + d, L - 4, y);
  }
  x.textAlign = "center"; x.textBaseline = "alphabetic";
  for (const [f, lab] of [[50], [100, "100"], [200], [500], [1000, "1k"], [2000], [5000], [10000, "10k"]]) {
    const px = Math.round(X(f)) + 0.5; x.globalAlpha = lab ? 0.8 : 0.4;
    x.beginPath(); x.moveTo(px, T); x.lineTo(px, T + ph); x.stroke(); x.globalAlpha = 1;
    if (lab) x.fillText(lab, px, h - 2);
  }
  // the curve, with a faint fill down/up to 0 dB
  x.beginPath(); freqs.forEach((f, i) => { const px = X(f), py = Y(db[i]); i ? x.lineTo(px, py) : x.moveTo(px, py); });
  x.strokeStyle = s.acc; x.lineWidth = 2; x.lineJoin = "round"; x.stroke();
  x.lineTo(X(20000), Y(0)); x.lineTo(X(20), Y(0)); x.closePath(); x.fillStyle = s.acc; x.globalAlpha = 0.14; x.fill(); x.globalAlpha = 1;
  // a dot per band at its centre frequency, on the curve
  x.fillStyle = s.acc; x.strokeStyle = globalThis.getComputedStyle(cv).getPropertyValue("--card").trim() || "#fff"; x.lineWidth = 1.5;
  for (const b of bands) {
    const f = clamp(b.fc, 20, 20000), d = response(bands, preamp, [f])[0];
    x.beginPath(); x.arc(X(f), Y(d), 3.5, 0, 2 * Math.PI); x.fill(); x.stroke();
  }
}
// a sparkline of arr (oldest first); min/max fixed, or null to fit the data
function drawSpark(cv, arr, min, max) {
  const s = setup(cv); if (!s || arr.length < 1) return;
  const { x, w, h } = s, P = 3;
  let lo = min, hi = max;
  if (lo === null || hi === null) {
    lo = Math.min(...arr); hi = Math.max(...arr);
    if (hi - lo < 1) { const c = (hi + lo) / 2; lo = c - 0.5; hi = c + 0.5; }
    const pad = (hi - lo) * 0.1; lo -= pad; hi += pad;
  }
  const X = (i) => (arr.length < 2 ? w : P + (w - 2 * P) * i / (arr.length - 1)) , Y = (v) => h - P - (h - 2 * P) * clamp((v - lo) / (hi - lo), 0, 1);
  x.strokeStyle = s.line; x.lineWidth = 1; x.beginPath(); x.moveTo(0, h - 0.5); x.lineTo(w, h - 0.5); x.stroke();
  x.beginPath(); arr.forEach((v, i) => { i ? x.lineTo(X(i), Y(v)) : x.moveTo(X(i), Y(v)); });
  x.strokeStyle = s.acc; x.lineWidth = 1.5; x.lineJoin = "round"; x.stroke();
  x.lineTo(X(arr.length - 1), h); x.lineTo(X(0), h); x.closePath(); x.fillStyle = s.acc; x.globalAlpha = 0.13; x.fill(); x.globalAlpha = 1;
}

export const EQ = { FS, MAX_BANDS, LIM, clamp, coeffs, magDb, response, parseAutoEq, drawResponse, drawSpark };
export { FS, MAX_BANDS, LIM, clamp, coeffs, magDb, response, parseAutoEq, drawResponse, drawSpark };
