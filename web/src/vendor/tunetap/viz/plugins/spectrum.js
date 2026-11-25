// SPDX-License-Identifier: LGPL-2.1-or-later
// 頻譜, the spectrum: shape x mapping x ramp, caps, a reflection, on the shared pixel canvas.
// A pixel port of chipchill crates/chipchill-viz/src/pages/spectrum.rs (rgxx viz.c), expression
// for expression (f32 steps through Math.fround), checked against chipchill's own pictures
// (tests/viz-spectrum.test.mjs). Four shapes: LED segments with L and R side by side in each of
// the 16 bands, 32 solid bars (L and R averaged), neon tubes (each channel a layer added to the
// picture, subtracted on a light palette) and a ring.
//
// tunetap differences: no text is drawn (chipchill leaves its dB/Hz words and L/R/FFT tags as
// labels for the platform; here the canvas is scaled up pixelated, so they are left out), and the
// ring's middle is the panel disc (chipchill puts the cover or an icon label there). A mono stream
// gives R = L, so the two channels of SEG and of the ring stand the same.
import { mix, tone } from '../pixel.js';
import { palOf, samePal, lut, colour, map, mapAt, mapFlat, rampAt } from '../colour.js';
import { Fit, stage } from '../page.js';
import { SPECTRUM, SHAPE, keyDef, valueLabelKey } from '../cfg.js';
import { BANDS, FINE, createBands, resetBands, BandAnalyser } from '../bands.js';

const F = Math.fround;
const FINE32 = FINE / 2; // solid, tube and ring draw 32 bands: the 64, two by two
const SPOKES = 2 * FINE32;
const DEPTH = 58; // rows of the reflection
const PI32 = F(Math.PI);

export const needs = Object.freeze({ freq: 'stereo', time: 'none' });

function lutAt(k, ch, f) {
  return k.lut[ch][f <= 0 ? 0 : f >= 1 ? 255 : Math.trunc(F(f * 255))];
}

// The 64 bands two by two: the louder of each pair.
function fine32(z) {
  const lv = [new Float32Array(FINE32), new Float32Array(FINE32)];
  const pk = [new Float32Array(FINE32), new Float32Array(FINE32)];
  for (let ch = 0; ch < 2; ch++) {
    for (let i = 0; i < FINE32; i++) {
      const a = z.fine[ch][2 * i], b = z.fine[ch][2 * i + 1];
      lv[ch][i] = a > b ? a : b;
      const p = z.finePeak[ch][2 * i], q = z.finePeak[ch][2 * i + 1];
      pk[ch][i] = p > q ? p : q;
    }
  }
  return [lv, pk];
}

// LED segments, L and R side by side in each of the 16 bands. Returns [x0, width of the bars].
function shapeSeg(c, v, f, k, m, peaks, base, top) {
  const avail = c.width - f.i(60), inner = f.i(2), gap = f.i(6);
  const group = Math.trunc((avail + gap) / BANDS);
  const bw = Math.trunc((group - inner - gap) / 2);
  const x0 = f.i(40) + Math.trunc((avail - (BANDS * group - gap)) / 2);
  const seg = f.px(3), step = f.px(5);
  for (let b = 0; b < BANDS; b++) {
    for (let ch = 0; ch < 2; ch++) {
      const lv = v.bands.level[ch][b];
      const h = Math.trunc(F(lv * top));
      const x = x0 + b * group + ch * (bw + inner);
      for (let yy = 0; yy + seg <= h; yy += step) {
        c.fill(x, base - yy - seg, bw, seg, colour(k.pal, k.ramp, m, ch, b, BANDS, F(yy / top), lv));
      }
      const ph = Math.trunc(F(v.bands.peak[ch][b] * top));
      if (peaks && ph > 2) c.fill(x, base - ph - f.i(6), bw, f.px(2), v.theme.fg);
    }
  }
  return [x0, BANDS * group - gap];
}

// 32 full bars, L and R averaged.
function shapeSolid(c, v, f, k, m, peaks, base, top) {
  const [lv, pk] = fine32(v.bands);
  const pitch = Math.trunc((c.width - f.i(64)) / FINE32);
  const x0 = f.i(44), bw = Math.trunc((pitch * 15) / 18);
  for (let i = 0; i < FINE32; i++) {
    const a = F(F(lv[0][i] + lv[1][i]) / 2);
    const p = pk[0][i] > pk[1][i] ? pk[0][i] : pk[1][i];
    const h = Math.trunc(F(a * top));
    const x = x0 + i * pitch;
    if (mapFlat(m)) {
      c.fill(x, base - h, bw, h, lutAt(k, 1, map(m, i, FINE32, a, a)));
    } else {
      for (let yy = 0; yy < h; yy++) c.hspan(x, base - 1 - yy, bw, lutAt(k, 1, map(m, i, FINE32, F(yy / top), a)));
    }
    const ph = Math.trunc(F(p * top));
    if (peaks && ph > 2) c.fill(x, base - (ph + f.i(12) > top ? top : ph + f.i(12)), bw, f.px(3), v.theme.fg);
  }
}

// Neon tubes: each channel a layer (a faint fill under the line, a glow round it, the core) added
// to the picture (subtracted on a light palette), R running the ramp the other way round.
function shapeLine(c, v, f, k, m, base, top) {
  const [lv] = fine32(v.bands);
  const x0 = f.i(44), w = c.width - f.i(64), light = k.pal.light;
  if (w <= 0) return;
  const py = new Int32Array(FINE32), fill = new Uint32Array(FINE32), glow = new Uint32Array(FINE32), core = new Uint32Array(FINE32);
  for (let ch = 1; ch >= 0; ch--) {
    for (let i = 0; i < FINE32; i++) {
      const fv = map(m, i, FINE32, lv[ch][i], lv[ch][i]);
      const col = lutAt(k, ch, ch ? F(1 - fv) : fv);
      py[i] = base - Math.trunc(F(lv[ch][i] * top));
      fill[i] = tone(col, 51, light);
      glow[i] = tone(col, 115, light);
      core[i] = tone(col, 255, light);
    }
    const n = FINE32 - 1;
    for (let x = 0; x < w; x++) {
      const i = Math.trunc((x * n) / w);
      const xa = Math.trunc((i * w) / n), xb = Math.trunc(((i + 1) * w) / n);
      const span = Math.max(xb - xa, 1); // under 31 columns two bands can share one
      const y = py[i] + Math.trunc(((py[i + 1] - py[i]) * (x - xa)) / span);
      const yn = py[i] + Math.trunc(((py[i + 1] - py[i]) * (x + 1 - xa)) / span);
      const lo = Math.min(y, yn) - 1, hi = Math.max(y, yn) + 1; // the core joins this column to the next
      for (let yy = lo - 3; yy < base; yy++) {
        const layer = yy < lo || (yy > hi && yy <= hi + 3) ? glow[i] : yy <= hi ? core[i] : fill[i];
        c.blend(x0 + x, yy, layer, light);
      }
    }
  }
}

// The spectrum painter: what chipchill's Spectrum keeps between frames (its colour tables, the
// ring's spokes, the reflection's fade) and its draw. view = { bands, cfg (resolved), theme, full }.
export class SpectrumPainter {
  constructor() {
    this.look = null;
    this.cs = new Float32Array(SPOKES);
    this.sn = new Float32Array(SPOKES);
    for (let i = 0; i < SPOKES; i++) {
      const a = -Math.PI / 2 + ((i + 0.5) / SPOKES) * 2 * Math.PI;
      this.cs[i] = Math.cos(a);
      this.sn[i] = Math.sin(a);
    }
    this.alpha = new Int32Array(DEPTH);
    for (let r = 0; r < DEPTH; r++) this.alpha[r] = Math.trunc(256 * 0.38 * Math.pow(1 - r / DEPTH, 1.5));
  }

  lookOf(v) {
    const pal = palOf(v.theme, v.cfg.duo1, v.cfg.duo2);
    const ramp = rampAt(v.cfg.ramp);
    if (!(this.look && this.look.ramp === ramp && samePal(this.look.pal, pal))) {
      this.look = { pal, ramp, lut: [lut(pal, ramp, 0), lut(pal, ramp, 1)] }; // a new theme or ramp: new tables
    }
    return this.look;
  }

  // The bands round a ring, L clockwise down the right, R down the left.
  radial(c, v, f, m, peaks) {
    const [lv, pk] = fine32(v.bands);
    const [y0, y1] = stage(c);
    const cx = Math.trunc(c.width / 2), cy = y0 + Math.trunc((y1 - y0) / 2) - 1;
    // The ring is as big as the shorter side lets it: the hole, the bands' reach and the middle
    // are shares of that side; every band stands at least `floor`, narrowing towards its tip.
    const short = Math.min(c.width, y1 - y0);
    const r0 = Math.trunc((short * 16) / 100), rmax = Math.trunc((short * 24) / 100);
    const floor = Math.trunc((short * 3) / 100);
    const { cs, sn } = this;
    const k = this.lookOf(v);
    for (let i = 0; i < SPOKES; i++) {
      const ch = i >= FINE32 ? 1 : 0;
      const j = ch ? SPOKES - 1 - i : i;
      const a = lv[ch][j];
      const ra = r0 + f.i(6), rb = r0 + f.i(6) + floor + Math.trunc(F(a * (rmax - floor)));
      for (let r = ra; r < rb; r++) {
        const up = F((r - ra) / (rb - ra));
        const col = lutAt(k, ch, map(m, j, FINE32, F(a * up), a));
        const arc = F(F(r * PI32) / SPOKES);
        const half = Math.max(F(arc * F(F(0.9) - F(F(0.45) * up))), 0.5);
        for (let o = -half; o <= half; o = F(o + 1)) {
          c.px(cx + Math.trunc(F(F(cs[i] * r) - F(sn[i] * o))), cy + Math.trunc(F(F(sn[i] * r) + F(cs[i] * o))), col);
        }
      }
      if (peaks) {
        const rp = Math.min(r0 + f.i(8) + Math.trunc(F(pk[ch][j] * rmax)) + f.i(10), r0 + f.i(8) + rmax);
        c.fill(cx + Math.trunc(F(cs[i] * rp)) - f.i(2), cy + Math.trunc(F(sn[i] * rp)) - f.i(2), f.px(4), f.px(4), v.theme.fg);
      }
    }
    const big = Math.trunc((short * 12) / 100);
    c.disc(cx, cy, big, v.theme.panel);
    c.ring(cx, cy, r0, v.theme.cyan);
  }

  // A fading mirror image under the baseline: the rows above it, flipped, blended in less and less.
  reflect(c, f, x, w, base) {
    const [y0, y1] = stage(c);
    const px = c.px32, width = c.width;
    const depth = Math.max(f.i(DEPTH), 1), gap = f.i(3);
    for (let r = 0; r < depth; r++) {
      if (base - 1 - r < y0 || base + gap + r >= y1) break; // the page's rows
      const src = (base - 1 - r) * width + x, dst = (base + gap + r) * width + x;
      const a = this.alpha[Math.trunc((r * DEPTH) / depth)];
      for (let i = 0; i < w; i++) {
        const d = px[dst + i], s = px[src + i];
        let rr = d & 255, g = (d >>> 8) & 255, b = (d >>> 16) & 255;
        rr += ((s & 255) - rr) * a >> 8;
        g += (((s >>> 8) & 255) - g) * a >> 8;
        b += (((s >>> 16) & 255) - b) * a >> 8;
        px[dst + i] = (0xff000000 | (b << 16) | (g << 8) | rr) >>> 0;
      }
    }
  }

  draw(c, v) {
    const t = v.theme;
    const [y0, y1] = stage(c);
    const width = c.width;
    // rgxx's page is 640 x 358: the bars take the width, the rows are measured from its ends
    const f = Fit.min(Fit.new(width, 640), Fit.new(y1 - y0, 358));
    const l = (n) => f.i(n);
    const m = mapAt(v.cfg.map);
    // The whole screen has the room for the peaks, whatever was chosen.
    const peaks = v.cfg.peaks !== 0 || !!v.full;
    const sh = v.cfg.shape;
    if (sh === SHAPE.RADIAL) { // a ring has no floor: no grid, never a reflection
      this.radial(c, v, f, m, peaks);
      return;
    }
    const refl = v.cfg.reflect !== 0;
    const base = refl ? y1 - l(72) : y1 - l(42); // the mirror image needs the room under the bars
    const top = base - y0 - (refl ? l(16) : l(26));
    if (top <= 0) return; // no room for bars
    const dash = f.px(3), gap = f.px(6);
    for (let db = 0; db < 4; db++) { // the dB grid
      const y = base - top + Math.trunc((db * top) / 4);
      for (let x = l(40); x < width - l(20); x += gap) c.fill(x, y, dash, 1, t.faint);
    }
    const k = this.lookOf(v);
    if (sh === SHAPE.SOLID) shapeSolid(c, v, f, k, m, peaks, base, top);
    else if (sh === SHAPE.LINE) shapeLine(c, v, f, k, m, base, top);
    else shapeSeg(c, v, f, k, m, peaks, base, top);
    if (refl) {
      c.fill(l(40), base + 1, width - l(60), 1, mix(t.bg, t.faint, 180));
      this.reflect(c, f, l(40), width - l(60), base);
    }
  }
}

// ---- the plugin ---------------------------------------------------------------------------------
const option = (k, i) => ({ value: i, labelKey: valueLabelKey(k, i) });
const selectOf = (k) => ({ key: k, type: 'select', labelKey: keyDef(k).labelKey, options: Array.from({ length: keyDef(k).n }, (_, i) => option(k, i)), showWhen: (cfg) => SPECTRUM.rows(cfg).includes(k) });
const hueOf = (k) => ({ key: k, type: 'range', labelKey: keyDef(k).labelKey, min: 0, max: keyDef(k).n - 1, step: 1, format: 'hue', showWhen: (cfg) => SPECTRUM.rows(cfg).includes(k) });
// The controls in the order of chipchill's menu rows.
const SCHEMA = Object.freeze([selectOf('shape'), selectOf('map'), selectOf('peaks'), selectOf('reflect'), selectOf('ramp'), hueOf('duo1'), hueOf('duo2')]);
const PRESETS = Object.freeze(Array.from({ length: SPECTRUM.presetCount() }, (_, i) => Object.freeze({ id: i, labelKey: SPECTRUM.presetLabelKey(i), cfg: Object.freeze(SPECTRUM.applyPreset(SPECTRUM.plain(), i)) })));

// create({ canvas, ctx2d, theme, seed }) -> instance (design amendment A/D). The plugin draws on the
// host's pixel canvas (frame's `pixels`) over the backdrop the host painted; it keeps no timers and
// touches no DOM.
export function create() {
  const painter = new SpectrumPainter();
  const analyser = new BandAnalyser();
  const bands = createBands();
  let state = 'ok';
  let alive = true;
  const groups = Object.freeze(Array.from({ length: SPECTRUM.groupCount() }, (_, g) => Object.freeze({
    id: SPECTRUM.groupId(g),
    labelKey: SPECTRUM.groupLabelKey(g),
    count: SPECTRUM.tourCount(g),
    pos: (cfg) => SPECTRUM.tourPos(cfg, g),
    step: (cfg, shown) => SPECTRUM.tourStep(cfg, shown, g),
  })));
  return {
    needs,
    schema: SCHEMA,
    presets: PRESETS,
    defaults: () => SPECTRUM.defaults(),
    validate: (obj) => SPECTRUM.validate(obj),
    applyPreset: (cfg, id) => SPECTRUM.applyPreset(cfg, id),
    presetOf: (cfg) => SPECTRUM.preset(cfg),
    resolveFollow: (cfg, seed) => SPECTRUM.resolve(cfg, seed),
    groups: () => groups,
    resize() { resetBands(bands); },
    // audio: { freq: { l, r }, rate, bins } or null; cfg: the resolved choices.
    frame({ dt, audio, theme, cfg, pixels, full }) {
      if (!alive) return;
      const freq = audio?.freq;
      state = freq?.l ? 'ok' : 'no-data';
      analyser.run(bands, freq?.l ? freq : null, audio?.rate || 48000, (audio?.bins || 1024) * 2, dt);
      painter.draw(pixels, { bands, cfg, theme, full: !!full });
    },
    status: () => (alive ? state : 'off'),
    bands: () => bands,
    dispose() { alive = false; },
  };
}
