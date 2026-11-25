// SPDX-License-Identifier: LGPL-2.1-or-later
// 瀑布, the waterfall: the spectrum's history as a picture that is scrolled, never repainted, on
// the shared pixel canvas. A pixel port of chipchill crates/chipchill-viz/src/pages/waterfall.rs
// (rgxx pages.c), expression for expression (f32 steps through Math.fround, integer division
// truncating), checked against chipchill's own pictures (tests/viz-waterfall.test.mjs,
// tests/fixtures/viz-waterfall.json).
//
// Every 60 ms each lane moves one line along and only the new line is drawn; a frame is one copy
// of the picture to the canvas. A lane is a part of the picture with its own channel: the layout
// says which lanes there are (L | R mirrored with the bass of both in the middle, L+R across the
// whole width, L over R), the direction which way they move (down, up, or across as a
// spectrogram). 64 bands, or 32 (the 64 two by two) where a lane has half the room. The heat map
// is the "loud" table (colour.js lutLoud): quiet is nearly the background, on paper the panel.
//
// tunetap differences:
// - no text: chipchill's 16K / 40 / L | R words, the seconds of the time axis and the L / R tags
//   (with their chips) are Labels for the platform's font; the canvas is scaled up pixelated, so
//   they are left out, as on the spectrum. The time axis's tick marks (pixels) are drawn.
// - with no audio data (audio: null) the page starts a new picture and lays down silence: the
//   empty waterfall a fresh chipchill page draws when fed silence; status() says 'no-data'.
// - needs is stereo whatever the layout: SPLIT and STACKED draw L and R apart, SUM their mean of
//   the per-channel levels, as chipchill (a mono stream gives L = R).
import { Fit } from '../page.js';
import { steps } from '../anim.js';
import { TableCache } from '../tables.js';
import { WATERFALL, WF_LAYOUT, WF_DIR, keyDef, valueLabelKey } from '../cfg.js';
import { createBands, resetBands, BandAnalyser } from '../bands.js';

const F = Math.fround;
const d = (a, b) => Math.trunc(a / b); // Rust's integer division
// A new line every 60 ms: 5 s down the canvas, 8.6 s across it at rgxx's size.
const TICK = F(0.06);

export const needs = Object.freeze({ freq: 'stereo', time: 'none' });

// A lane: where in the picture, which channel (0 L, 1 R, 2 both), how many cells, whether its cells
// run the other way, and whether each cell is the louder of a pair of the 64 bands.
const lane = (x, y, w, h, ch, n, flip, half) => ({ x, y, w, h, ch, n, flip, half });
const NONE = lane(0, 0, 0, 0, 0, 0, false, false);

// The lanes of a picture pw x ph (rgxx's is 576 x 336).
export function lanes(layout, dir, pw, ph, f) {
  if (dir !== WF_DIR.ACROSS) {
    if (layout === WF_LAYOUT.SUM) return [lane(0, 0, pw, ph, 2, 64, false, false), NONE, 1];
    if (layout === WF_LAYOUT.STACKED) return [lane(0, 0, pw, d(ph, 2), 0, 64, false, false), lane(0, d(ph, 2), pw, d(ph, 2), 1, 64, false, false), 2];
    // split: the bass of both in the middle
    return [lane(0, 0, d(pw, 2), ph, 0, 32, true, true), lane(d(pw, 2), 0, d(pw, 2), ph, 1, 32, false, true), 2];
  }
  // across: frequency runs up, 5 px a band at rgxx's size; what is left over of the height (8 px
  // there) goes top and bottom
  const band = Math.max(d(ph - f.i(16), 64), 0);
  const y = d(ph - 64 * band, 2);
  if (layout === WF_LAYOUT.SUM) return [lane(0, y, pw, 64 * band, 2, 64, false, false), NONE, 1];
  // split: R hangs down from the middle
  return [lane(0, y, pw, 32 * band, 0, 32, false, true), lane(0, y + 32 * band, pw, 32 * band, 1, 32, layout === WF_LAYOUT.SPLIT, true), 2];
}

function value(z, l, cell) {
  const b = l.flip ? l.n - 1 - cell : cell;
  let v0, v1;
  if (l.half) {
    const a0 = z.fine[0][2 * b], c0 = z.fine[0][2 * b + 1];
    const a1 = z.fine[1][2 * b], c1 = z.fine[1][2 * b + 1];
    v0 = a0 > c0 ? a0 : c0;
    v1 = a1 > c1 ? a1 : c1;
  } else {
    v0 = z.fine[0][b];
    v1 = z.fine[1][b];
  }
  if (l.ch === 2) return F(F(v0 + v1) / 2);
  return l.ch ? v1 : v0;
}

// A level as a byte of the table ((v * 255) as usize; the levels are 0..1).
const byte = (v) => { const i = Math.trunc(F(v * 255)); return i > 0 ? (i < 255 ? i : 255) : 0; };

// One more line in a lane: everything moves a line along, the new line goes in at the start.
function advance(px, pw, line, z, l, dir, t) {
  const lut = t.loud[l.ch === 0 ? 0 : 1];
  if (dir === WF_DIR.ACROSS) {
    // time runs to the right, the newest column on the left; frequency up
    const ch = d(l.h, l.n);
    if (l.w > line) {
      for (let y = l.y; y < l.y + l.h; y++) {
        const row = y * pw + l.x;
        px.copyWithin(row + line, row, row + (l.w - line));
      }
    }
    const w = Math.min(line, l.w);
    for (let cell = 0; cell < l.n; cell++) {
      const c = lut[byte(value(z, l, cell))];
      for (let k = 0; k < ch; k++) {
        const at = (l.y + l.h - 1 - cell * ch - k) * pw + l.x;
        px.fill(c, at, at + Math.max(w, 0));
      }
    }
    return;
  }
  if (l.h < line) return;
  const cw = d(l.w, l.n);
  if (dir === WF_DIR.DOWN) {
    for (let y = l.y + l.h - 1; y >= l.y + line; y--) {
      const to = y * pw + l.x, from = (y - line) * pw + l.x;
      px.copyWithin(to, from, from + l.w);
    }
  } else {
    for (let y = l.y; y < l.y + l.h - line; y++) {
      const to = y * pw + l.x, from = (y + line) * pw + l.x;
      px.copyWithin(to, from, from + l.w);
    }
  }
  const first = dir === WF_DIR.DOWN ? l.y : l.y + l.h - line;
  const cells = Math.max(cw - 1, 0);
  for (let k = 0; k < line; k++) {
    const row = (first + k) * pw + l.x;
    px.fill(lut[0], row, row + Math.max(l.w, 0)); // the gaps between the cells stay quiet
    for (let cell = 0; cell < l.n; cell++) {
      const at = row + cell * cw;
      px.fill(lut[byte(value(z, l, cell))], at, at + cells);
    }
  }
}

// The waterfall painter: what chipchill's Waterfall keeps between frames (its picture, the colour
// tables, which choices the picture was drawn with, the tick's accumulator) and its draw.
// view = { bands, cfg (resolved), theme, dt, moving }.
export class WaterfallPainter {
  constructor() {
    this.pic = new Uint32Array(0);
    this.size = [0, 0, 0];
    this.tables = new TableCache();
    this.drawn = null;
    this.acc = { v: 0 };
    this.lines = 0; // lines laid down since the picture began (for tests)
  }

  // Start a new picture on the next draw (the silent rest without audio data).
  reset() {
    this.drawn = null;
  }

  draw(c, v) {
    const th = v.theme;
    const y0 = c.y0, y1 = c.y1, width = c.width;
    // rgxx's page is 640 x 358 and its picture 576 x 336 in it, 32 in from either side
    const f = Fit.min(Fit.new(width, 640), Fit.new(y1 - y0, 358));
    const l = (n) => f.i(n);
    const wx = l(32), py = y0 + l(4);
    const pw = Math.max(width - 2 * wx, 0), ph = Math.max(y1 - y0 - l(22), 0), line = f.px(4);
    const sz = this.size;
    if (sz[0] !== pw || sz[1] !== ph || sz[2] !== line) {
      // a picture of another size is a new picture; making it is setting the page up
      this.pic = new Uint32Array(pw * ph);
      this.size = [pw, ph, line];
      this.drawn = null;
    }
    const t = this.tables.now(th, v.cfg);
    const layout = v.cfg.wf_layout, dir = v.cfg.wf_dir;
    const dr = this.drawn;
    if (!dr || dr[0] !== layout || dr[1] !== dir || dr[2] !== t.serial) {
      // other choices, other colours: a new picture
      this.pic.fill(t.loud[1][0]);
      this.drawn = [layout, dir, t.serial];
      this.acc.v = 0;
      this.lines = 0;
    }
    const [l0, l1, nLanes] = lanes(layout, dir, pw, ph, f);
    const lines = d(dir === WF_DIR.ACROSS ? pw : ph, line);
    if (v.moving && lines > 0) {
      // At a rate slower than the tick, one frame lays down all the lines its time holds, at most
      // a picture's worth. The remainder is carried, because frames are never neat multiples of
      // 60 ms.
      const n = steps(this.acc, v.dt, TICK, d(ph, line));
      for (let i = 0; i < n; i++) {
        advance(this.pic, pw, line, v.bands, l0, dir, t);
        if (nLanes > 1) advance(this.pic, pw, line, v.bands, l1, dir, t);
      }
      this.lines += n;
    }
    for (let y = 0; y < ph && py + y < y1; y++) {
      const row = c.rowMut(py + y);
      if (!row) continue;
      const from = y * pw;
      const n = Math.min(pw, Math.max(row.length - wx, 0));
      row.set(this.pic.subarray(from, from + n), wx);
    }
    // the scale: a tick a second along the time axis (chipchill's words at its ends are labels)
    const tick = (k) => d(k * 1000, 60) * line;
    const tk0 = f.px(4), tk1 = f.px(1);
    if (dir === WF_DIR.ACROSS) {
      for (let k = 1; tick(k) < pw; k++) c.fill(wx + tick(k), py + ph + l(1), tk1, tk0, th.dim);
    } else {
      for (let k = 1; tick(k) < ph; k++) c.fill(wx + pw + l(2), dir === WF_DIR.DOWN ? py + tick(k) : py + ph - tick(k), tk0, tk1, th.dim);
    }
  }
}

// ---- the plugin ---------------------------------------------------------------------------------
const option = (k, i) => ({ value: i, labelKey: valueLabelKey(k, i) });
const selectOf = (k) => ({ key: k, type: 'select', labelKey: keyDef(k).labelKey, options: Array.from({ length: keyDef(k).n }, (_, i) => option(k, i)), showWhen: (cfg) => WATERFALL.rows(cfg).includes(k) });
const hueOf = (k) => ({ key: k, type: 'range', labelKey: keyDef(k).labelKey, min: 0, max: keyDef(k).n - 1, step: 1, format: 'hue', showWhen: (cfg) => WATERFALL.rows(cfg).includes(k) });
// The controls in the order of chipchill's menu rows (cfg.rs page_keys).
const SCHEMA = Object.freeze([selectOf('wf_layout'), selectOf('wf_dir'), selectOf('ramp'), hueOf('duo1'), hueOf('duo2')]);
const PRESETS = Object.freeze(Array.from({ length: WATERFALL.presetCount() }, (_, i) => Object.freeze({ id: i, labelKey: WATERFALL.presetLabelKey(i), cfg: Object.freeze(WATERFALL.applyPreset(WATERFALL.plain(), i)) })));

// create({ canvas, ctx2d, theme, seed }) -> instance (design amendment A/D). The plugin draws on the
// host's pixel canvas (frame's `pixels`) over the backdrop the host painted; it keeps no timers,
// fetches nothing and touches no DOM.
export function create() {
  const painter = new WaterfallPainter();
  const analyser = new BandAnalyser();
  const bands = createBands();
  let state = 'no-data'; // until audio data is seen
  let alive = true;
  const groups = Object.freeze(Array.from({ length: WATERFALL.groupCount() }, (_, g) => Object.freeze({
    id: WATERFALL.groupId(g),
    labelKey: WATERFALL.groupLabelKey(g),
    count: WATERFALL.tourCount(g),
    pos: (cfg) => WATERFALL.tourPos(cfg, g),
    step: (cfg, shown) => WATERFALL.tourStep(cfg, shown, g),
  })));
  return {
    needs,
    schema: SCHEMA,
    presets: PRESETS,
    defaults: () => WATERFALL.defaults(),
    validate: (obj) => WATERFALL.validate(obj),
    applyPreset: (cfg, id) => WATERFALL.applyPreset(cfg, id),
    presetOf: (cfg) => WATERFALL.preset(cfg),
    resolveFollow: (cfg, seed) => WATERFALL.resolve(cfg, seed),
    groups: () => groups,
    // The picture is made again at the next frame when its size changed (WaterfallPainter.draw).
    resize() {},
    // audio: { freq: { l, r }, rate, bins } or null; cfg: the resolved choices; playing: the picture
    // scrolls only while the player plays.
    frame({ dt, audio, theme, cfg, playing, pixels }) {
      if (!alive) return;
      dt = F(Number.isFinite(dt) && dt > 0 ? dt : 0);
      const freq = audio?.freq;
      const live = !!freq?.l;
      state = live ? 'ok' : 'no-data';
      if (live) {
        analyser.run(bands, freq, audio.rate || 48000, (audio.bins || 1024) * 2, dt);
      } else {
        // nothing to show: a new picture of silence, nothing pretends to be the music
        resetBands(bands);
        painter.reset();
      }
      painter.draw(pixels, { bands, cfg, theme, dt, moving: !!playing });
    },
    status: () => (alive ? state : 'off'),
    painter: () => painter,
    dispose() { alive = false; },
  };
}
