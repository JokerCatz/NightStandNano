// SPDX-License-Identifier: LGPL-2.1-or-later
// 錄音帶, the cassette: a compact cassette to scale (IEC 60094: 100.4 x 63.8 mm, hubs 42.5 mm apart)
// with its reels turning at 4.76 cm/s, the small pack faster, the tape moving from one pack to the
// other as it plays. The shell and the label are the owner's choice; the label is a plate printed
// on the shell, and can be seen through.
// A pixel port of chipchill crates/chipchill-viz/src/pages/cassette.rs (rgxx pages.c), expression
// for expression (f32 steps through Math.fround, f64 where chipchill uses f64, integer division and
// `as i32` truncating), checked against chipchill's own pictures and Labels
// (tests/viz-cassette.test.mjs, tests/fixtures/viz-cassette.json).
//
// The text (the side, the title, the C-xx tag, the artist, the counter) is not drawn here: it is
// handed to the host as labels (js/viz/labels.js, chipchill label.rs), which the host draws over
// the picture in the platform's font. frame() returns { labels }.
//
// tunetap differences (a radio stream has no song, no position and no length):
// - title: the ICY now-playing title when the station sends one, else the station's name; artist:
//   the station's name when a title is shown, else nothing (the host maps them: host.js).
// - the counter (chipchill's position in the song) is the play time since this station started
//   (the host's elapsedMs: it stands still while paused and starts at 0 for a new station).
// - the packs: chipchill moves the tape from the left pack to the right as the song plays
//   (progress = pos / len). A stream has no length, so the play time is laid onto a NOTIONAL C-90
//   (90 minutes, C90_MS): the reels shift slowly, as a real C-90's do, and after 90 minutes the
//   tape starts over (a side turned). The tag says "C-90", a tape's own name, rather than chipchill's
//   "C-<length>" (which would claim the stream lasts 90:00). Nothing else implies a duration.
// - no 角色 label (chipchill's character label, the 初音 preset): character art is out of scope.
// - the page does not hear the music (neither does chipchill's): needs nothing of the tap, and its
//   status is 'ok' with or without audio data.
import { mix, rgb, red, green, blue } from '../pixel.js';
import { Fit, textScale } from '../page.js';
import { RGXX_METRICS } from '../labels.js';
import { CASSETTE, TAPE_SHELL, TAPE_LABEL, TAPE_ALPHA, keyDef, valueLabelKey } from '../cfg.js';

const F = Math.fround;
const d = (a, b) => Math.trunc(a / b); // Rust's integer division
// Rust's `as i32` from a float: towards zero, NaN 0, saturating.
const i32 = (v) => (Number.isNaN(v) ? 0 : Math.trunc(Math.min(Math.max(v, -2147483648), 2147483647)));
// Rust's f64::round: half away from zero.
const round = (v) => (v < 0 ? -Math.round(-v) : Math.round(v));

export const needs = Object.freeze({ freq: 'none', time: 'none' });
export const C90_MS = 90 * 60 * 1000;

// How far row yy of a rounded rectangle is bitten into on each side.
function rrectIn(yy, h, r) {
  const bite = (dd) => r - i32(Math.sqrt(r * r - dd * dd));
  if (yy < r) return bite(r - yy);
  if (yy > h - 1 - r) return bite(yy - (h - 1 - r));
  return 0;
}

function rrect(c, x, y, w, h, r, col) {
  for (let yy = 0; yy < h; yy++) {
    const bite = rrectIn(yy, h, r);
    c.fill(x + bite, y + yy, w - 2 * bite, 1, col);
  }
}

function screw(c, f, cx, cy) {
  c.disc(cx, cy, f.i(7), rgb(150, 150, 160));
  c.disc(cx, cy, f.i(5), rgb(110, 110, 122));
  c.fill(cx - f.i(4), cy, f.i(9), f.px(1), rgb(50, 50, 60));
  c.fill(cx, cy - f.i(4), f.px(1), f.i(9), rgb(50, 50, 60));
}

// Skins: a shell and a label, chosen apart. Colours only: the cassette's drawing is one. The
// palette's rows are filled from the theme when they are drawn, the clear shell from the
// background it is seen against. Black x cream is the cassette as it always was.
const SHELLS = Object.freeze([
  { rim: rgb(70, 68, 86), body: rgb(34, 32, 42), head: rgb(46, 44, 58), frame: rgb(90, 86, 110), glass: rgb(22, 20, 30), palette: false, clear: false },
  // body, head and glass: the night seen through smoke
  { rim: rgb(150, 156, 172), body: 0, head: 0, frame: rgb(170, 176, 190), glass: 0, palette: false, clear: true },
  { rim: rgb(176, 176, 188), body: rgb(226, 226, 232), head: rgb(204, 204, 214), frame: rgb(150, 150, 162), glass: rgb(22, 20, 30), palette: false, clear: false },
  { rim: 0, body: 0, head: 0, frame: 0, glass: rgb(22, 20, 30), palette: true, clear: false },
]);
// chipchill's third label (角色, mint and pink with the side "01") is left out with the character.
const LABELS = Object.freeze([
  { paper: rgb(238, 230, 204), ink: rgb(36, 34, 70), rule: rgb(200, 190, 170), note: rgb(170, 60, 90), stripe: [rgb(255, 64, 150), rgb(255, 172, 40), rgb(0, 196, 226)], side: 'A', palette: false },
  { paper: 0, ink: 0, rule: 0, note: 0, stripe: [0, 0, 0], side: 'A', palette: true },
]);

export function shellNow(i, t) {
  const k = { ...SHELLS[Number.isInteger(i) && i >= 0 && i < 4 ? i : TAPE_SHELL.BLACK] };
  if (k.clear) {
    // what is seen through it: the shell dark, the window and the hub wells lit from inside
    k.body = mix(t.bg, rgb(190, 200, 215), 72);
    k.head = mix(t.bg, rgb(190, 200, 215), 100);
    k.glass = mix(k.body, rgb(214, 222, 232), 150);
  }
  if (k.palette) {
    k.rim = mix(t.cyan, t.bg, 90);
    k.body = t.panel2;
    k.head = t.panel;
    k.frame = t.faint;
  }
  return k;
}

export function labelNow(i, t) {
  const k = { ...LABELS[Number.isInteger(i) && i >= 0 && i < LABELS.length ? i : TAPE_LABEL.CREAM] };
  if (k.palette) {
    // something light to write on: the text colour of a dark palette, the card of a light one
    k.paper = t.light ? t.panel : t.fg;
    k.ink = t.light ? t.fg : t.bg;
    k.rule = mix(k.paper, k.ink, 50);
    k.note = t.light ? t.cyan : mix(t.mag, t.black, 90);
    k.stripe = [t.mag, t.yel, t.cyan];
  }
  return k;
}

// The WCAG contrast of two colours, 1..21 (rgxx's rg_contrast).
export function contrast(a, b) {
  const lin = (v) => { const c = v / 255; return c <= 0.03928 ? c / 12.92 : Math.pow((c + 0.055) / 1.055, 2.4); };
  const lum = (c) => 0.2126 * lin(red(c)) + 0.7152 * lin(green(c)) + 0.0722 * lin(blue(c));
  const la = lum(a), lb = lum(b);
  return la > lb ? (la + 0.05) / (lb + 0.05) : (lb + 0.05) / (la + 0.05);
}

// 標籤透明度: the label is a plate printed on the shell. Under it there are only ever two flat
// colours: the shell's body, and the two packs the clear shell draws before the label goes on. So a
// row of the plate is the shell's blend with at most two spans of the packs' blend cut into it: one
// mix per colour per shape, every pixel written once. 不透明 is the plain shape.
class Plate {
  constructor(o) { Object.assign(this, o); }

  // One row: the shell's blend, with the packs' blend where the discs filled them.
  row(c, x, y, w, onShell, onPack) {
    let cur = x;
    const end = x + w;
    for (let side = 0; side < 2; side++) {
      if (!this.packs) break;
      const r = side !== 0 ? this.rr : this.rl;
      const dy = y - this.hubY;
      if (dy < -r || dy > r) continue;
      const cx = side !== 0 ? this.hr : this.hl;
      const half = i32(Math.sqrt(r * r - dy * dy));
      const a = Math.max(cx - half, cur), b = Math.min(cx + half + 1, end); // exactly the span the disc filled
      if (b <= a) continue;
      if (a > cur) c.fill(cur, y, a - cur, 1, onShell);
      c.fill(a, y, b - a, 1, onPack);
      cur = b;
    }
    if (cur < end) c.fill(cur, y, end - cur, 1, onShell);
  }

  fill(c, x, y, w, h, col) {
    if (this.a >= 255) { c.fill(x, y, w, h, col); return; }
    const onShell = mix(this.shell, col, this.a), onPack = mix(this.pack, col, this.a);
    for (let yy = y; yy < y + h; yy++) this.row(c, x, yy, w, onShell, onPack);
  }

  rrect(c, x, y, w, h, r, col) {
    if (this.a >= 255) { rrect(c, x, y, w, h, r, col); return; }
    const onShell = mix(this.shell, col, this.a), onPack = mix(this.pack, col, this.a);
    for (let yy = 0; yy < h; yy++) {
      const bite = rrectIn(yy, h, r);
      this.row(c, x + bite, y + yy, w - 2 * bite, onShell, onPack);
    }
  }
}

// What the writing does about a plate that has gone whatever colour the shell made it, asked of
// the plate over the shell's BODY: the skin's ink (4.5:1), the label's own paper colour on a plate
// that went dark, or a thin opaque bar of paper behind each line with ink on that.
const INK = 0, SWAP = 1, BACK = 2;
function writing(lb, eff) {
  if (contrast(lb.ink, eff) >= 4.5) return INK;
  if (contrast(lb.paper, eff) >= 4.5) return SWAP;
  return BACK;
}

// m:ss (label.rs Short via cassette.rs time); minutes past 59 keep counting.
export function time(ms) {
  const s = Math.trunc(ms / 1000);
  return `${Math.trunc(s / 60)}:${String(s % 60).padStart(2, '0')}`;
}

const PLATE_A = [255, 191, 128, 64];

// The cassette painter: what chipchill's Cassette keeps (how far the reels have turned) and its draw.
// view = { theme, cfg (resolved), title, artist, posMs, lenMs, tag (the C-xx tag's words; chipchill's
// "C-" + time(lenMs) when not given), dt (f32 seconds), moving, measure (label.rs Measure) }.
// Returns the labels, in the order chipchill makes them.
export class CassettePainter {
  constructor() {
    this.al = 0;
    this.ar = 0;
  }

  draw(c, v) {
    const labels = [];
    const m = v.measure ?? RGXX_METRICS;
    const th = v.theme;
    const title = v.title ?? '', artist = v.artist ?? '';
    const y0 = c.y0, y1 = c.y1, width = c.width;
    // rgxx's page is 640 x 404; the cassette is drawn to scale in it, and scales with it
    const f = Fit.min(Fit.new(width, 640), Fit.new(y1 - y0, 404));
    const l = (k) => f.i(k);
    const ts = textScale(f.f());
    const mm = 5.9 * f.f(); // pixels per millimetre
    const w = i32(100.4 * mm), h = i32(63.8 * mm); // 592 x 376
    const x = d(width - w, 2), y = y0 + d(y1 - y0 - h, 2);
    const mid = d(width, 2);
    const hubY = y + i32(27.5 * mm);
    const hl = mid - i32(21.25 * mm), hr = mid + i32(21.25 * mm);
    const rmin = 11.0 * mm, rmax = 24.0 * mm; // hub to a full C-60 pack
    const lenMs = v.lenMs ?? 0, posMs = v.posMs ?? 0;
    const p = lenMs <= 0 ? 0 : Math.min(Math.max(F(F(posMs) / F(lenMs)), 0), 1);
    const rl = Math.sqrt(rmin * rmin + (rmax * rmax - rmin * rmin) * F(1 - p));
    const rr = Math.sqrt(rmin * rmin + (rmax * rmax - rmin * rmin) * p);
    if (v.moving) {
      // 4.76 cm/s: the small pack turns faster
      const dt28 = F(F(v.dt) * 28);
      this.al += (dt28 * mm) / rl;
      this.ar += (dt28 * mm) / rr;
    }
    const sk = shellNow(v.cfg.tape_shell, th);
    const lb = labelNow(v.cfg.tape_label, th);
    const shell = sk.body, shell2 = sk.head, tape = rgb(96, 62, 38);

    // the shell, with the write-protect notches on the top edge
    rrect(c, x, y, w, h, l(10), sk.rim);
    rrect(c, x + l(2), y + l(2), w - l(4), h - l(4), l(9), shell);
    c.fill(x + i32(8.0 * mm), y, i32(5.0 * mm), i32(3.0 * mm), th.bg);
    c.fill(x + w - i32(13.0 * mm), y, i32(5.0 * mm), i32(3.0 * mm), th.bg);
    if (sk.clear) {
      // a clear shell shows the packs where the label does not cover them
      c.disc(hl, hubY, i32(rl), mix(shell, tape, 190));
      c.disc(hr, hubY, i32(rr), mix(shell, tape, 190));
    }

    // The label: shell, packs, plate, writing, then the window and the hubs over it all, which are
    // holes in the label and stay opaque whatever the plate is doing.
    const lx = x + i32(5.0 * mm), ly = y + i32(5.0 * mm), lw = w - i32(10.0 * mm), lh = i32(42.0 * mm);
    const ai = v.cfg.tape_alpha;
    const pl = new Plate({ a: PLATE_A[Number.isInteger(ai) && ai >= 0 && ai < 4 ? ai : TAPE_ALPHA.FULL], shell, pack: shell, packs: false, hl, hr, hubY, rl: i32(rl), rr: i32(rr) });
    if (sk.clear) {
      pl.pack = mix(shell, tape, 190);
      pl.packs = true;
    }
    const eff = pl.a >= 255 ? lb.paper : mix(shell, lb.paper, pl.a);
    const wr = pl.a >= 255 ? INK : writing(lb, eff);
    const back = wr === BACK;
    const ink = wr === SWAP ? lb.paper : lb.ink;
    let note = lb.note;
    const rule = lb.rule;
    // The accent (the C-xx tag and the running time) is writing too, and on a see-through plate the
    // first thing to go: unless it sits on a bar, it comes towards the writing's colour, and gives
    // way to it if halfway is not enough.
    if (pl.a < 255 && wr !== BACK && contrast(note, eff) < 4.5) {
      const lift = mix(lb.note, ink, 150);
      note = contrast(lift, eff) >= 4.5 ? lift : ink;
    }
    pl.rrect(c, lx, ly, lw, lh, l(6), lb.paper);
    const sy = ly + i32(11.0 * mm); // 80s stripes
    pl.fill(c, lx, sy, lw, f.px(5), lb.stripe[0]);
    pl.fill(c, lx, sy + l(5), lw, f.px(5), lb.stripe[1]);
    pl.fill(c, lx, sy + l(10), lw, f.px(5), lb.stripe[2]);
    c.fill(lx + l(10), ly + l(10), l(30), l(30), lb.ink); // the side, in its box: opaque, so it always reads
    // "A" fills its box at double size; "01" needs two characters in it
    const sideScale = lb.side.length > 1 ? ts : 2 * ts;
    const sideW = m.width('wide', sideScale, lb.side);
    labels.push({ kind: 'str', text: lb.side, font: 'wide', scale: sideScale, x: lx + l(10) + d(l(30) - sideW, 2), y: sideScale === 2 * ts ? ly + l(9) : ly + l(17), colour: lb.paper, chip: null, maxW: null });
    const tx = lx + l(52), tw = lw - l(52) - l(110);
    pl.fill(c, tx, ly + l(44), tw + l(100), f.px(1), rule); // the lines to write on
    // one bar for the whole top line, from just left of the title to the label's inner edge
    if (back) c.fill(tx - l(6), ly + l(8), lx + lw - l(6) - (tx - l(6)), l(36), lb.paper);
    if (m.width('wide', 2 * ts, title) <= tw) {
      labels.push({ kind: 'title', text: title, font: 'wide', scale: 2 * ts, x: tx, y: ly + l(10), colour: ink, chip: null, maxW: null });
    } else {
      labels.push({ kind: 'title', text: title, font: 'wide', scale: ts, x: tx, y: ly + l(22), colour: ink, chip: null, maxW: tw + l(100) });
    }
    const tag = v.tag ?? `C-${time(lenMs)}`;
    const tagX = lx + lw - l(12) - m.width('hud', 2 * ts, tag);
    labels.push({ kind: 'short', text: tag, font: 'hud', scale: 2 * ts, x: tagX, y: ly + l(16), colour: note, chip: null, maxW: null });

    // the window: a rounded slot between the hubs, the packs showing through it
    const wx = hl - i32(9.0 * mm), wx2 = hr + i32(9.0 * mm), wy = hubY - i32(8.0 * mm), wh = i32(16.0 * mm);
    rrect(c, wx - l(3), wy - l(3), wx2 - wx + l(6), wh + l(6), d(wh, 2) + l(3), sk.frame);
    rrect(c, wx, wy, wx2 - wx, wh, d(wh, 2), sk.glass);
    // the packs show only through the pill: every row is cut to its span
    const pr = d(wh, 2);
    const rli = i32(rl), rri = i32(rr), layer = rgb(120, 82, 52);
    for (let yy = wy; yy < wy + wh; yy++) {
      const dy = yy + 0.5 - (wy + pr);
      if (Math.abs(dy) > pr) continue;
      const half = i32(Math.sqrt(pr * pr - dy * dy));
      const a = wx + pr - half, b = wx2 - pr + half;
      for (let side = 0; side < 2; side++) {
        const r = side !== 0 ? rr : rl;
        if (Math.abs(dy) >= r) continue;
        const cx = side !== 0 ? hr : hl;
        const hh = i32(Math.sqrt(r * r - dy * dy));
        const x0 = Math.max(cx - hh, a), x1 = Math.min(cx + hh, b);
        if (x1 > x0) c.fill(x0, yy, x1 - x0, 1, tape);
      }
      for (let k = 1; k < 4; k++) {
        // the wound layers catch the light
        for (let side = 0; side < 2; side++) {
          const r = (side !== 0 ? rri : rli) - k * l(9);
          if (r <= 0 || Math.abs(dy) > r) continue;
          const cx = side !== 0 ? hr : hl;
          const hh = i32(round(Math.sqrt(r * r - dy * dy)));
          for (const e of [-1, 1]) {
            const qx = cx + e * hh;
            if (qx >= a && qx < b) c.px(qx, yy, layer);
          }
        }
      }
      if (((yy - wy) & 1) !== 0) c.scanlines(a, yy - 1, b - a, 2, 35); // the smoky plastic
    }
    // the hubs, turning, in their holes in the label
    for (let side = 0; side < 2; side++) {
      const cx = side !== 0 ? hr : hl;
      const ang = side !== 0 ? this.ar : this.al;
      c.disc(cx, hubY, i32(7.5 * mm), sk.glass);
      c.disc(cx, hubY, i32(6.0 * mm), rgb(236, 234, 228));
      for (let t = 0; t < 6; t++) {
        const an = ang + (t * Math.PI) / 3.0;
        const kx = cx + i32(round(4.2 * mm * Math.cos(an)));
        const ky = hubY + i32(round(4.2 * mm * Math.sin(an)));
        c.fill(kx - l(3), ky - l(3), l(7), l(7), rgb(60, 58, 74));
      }
      c.disc(cx, hubY, i32(2.3 * mm), rgb(40, 38, 52));
    }
    // below the window on the label: where it is, handwritten
    const now = v.counter ?? time(posMs);
    pl.fill(c, lx + l(10), ly + lh - l(14), lw - l(20), f.px(1), rule);
    const nowW = m.width('wide', ts, now);
    if (back) c.fill(lx + l(6), ly + lh - l(34), lw - l(12), l(20), lb.paper); // and one for the whole foot line
    labels.push({ kind: 'artist', text: artist, font: 'wide', scale: ts, x: lx + l(12), y: ly + lh - l(32), colour: ink, chip: null, maxW: lw - l(150) });
    labels.push({ kind: 'short', text: now, font: 'wide', scale: ts, x: lx + lw - l(12) - nowW, y: ly + lh - l(32), colour: note, chip: null, maxW: null });

    // the head area: a trapezoid, wider at the bottom, with its five openings
    const ty = y + h - i32(11.0 * mm), tt = i32(64.0 * mm), tb = i32(76.0 * mm);
    const rows = y + h - l(2) - ty;
    for (let r = 0; r < rows; r++) {
      const ww = tt + d((tb - tt) * r, rows);
      c.fill(mid - d(ww, 2), ty + r, ww, 1, shell2);
    }
    const oy = y + h - i32(6.0 * mm);
    const hole = rgb(12, 10, 16);
    c.fill(mid - i32(6.0 * mm), oy - l(4), i32(12.0 * mm), i32(6.0 * mm), hole); // head opening
    c.fill(mid - i32(2.5 * mm), oy - l(2), i32(5.0 * mm), i32(3.0 * mm), rgb(200, 180, 140)); // the felt pad
    for (const side of [-1, 1]) {
      c.disc(mid + side * i32(20.0 * mm), oy, i32(1.6 * mm), hole); // capstan holes
      c.fill(mid + side * i32(29.0 * mm) - l(6), oy - l(5), l(12), l(10), hole); // guide holes
    }
    c.fill(mid - d(tb, 2) + l(8), y + h - l(9), tb - l(16), l(3), tape); // the tape along the bottom edge
    // five screws: the corners and the middle of the head area
    screw(c, f, x + l(12), y + l(12));
    screw(c, f, x + w - l(13), y + l(12));
    screw(c, f, x + l(12), y + h - l(13));
    screw(c, f, x + w - l(13), y + h - l(13));
    screw(c, f, mid, ty + l(10));
    // the deck's lamp, just outside
    if (v.moving) c.disc(x + w - l(30), ty + l(16), l(5), rgb(255, 50, 60));
    return labels;
  }
}

// ---- the plugin ---------------------------------------------------------------------------------
const option = (k, i) => ({ value: i, labelKey: valueLabelKey(k, i) });
const selectOf = (k) => ({ key: k, type: 'select', labelKey: keyDef(k).labelKey, options: Array.from({ length: keyDef(k).n }, (_, i) => option(k, i)), showWhen: (cfg) => CASSETTE.rows(cfg).includes(k) });
// The controls in the order of chipchill's menu rows (cfg.rs page_keys).
const SCHEMA = Object.freeze([selectOf('tape_shell'), selectOf('tape_label'), selectOf('tape_alpha')]);
const PRESETS = Object.freeze(Array.from({ length: CASSETTE.presetCount() }, (_, i) => Object.freeze({ id: i, labelKey: CASSETTE.presetLabelKey(i), cfg: Object.freeze(CASSETTE.applyPreset(CASSETTE.plain(), i)) })));

// What the page is shown of a stream: the notional C-90's place for the packs, the play time on
// the counter, "C-90" on the tag.
export function streamView(elapsedMs) {
  const e = Number.isFinite(elapsedMs) && elapsedMs > 0 ? Math.trunc(elapsedMs) : 0;
  return { posMs: e % C90_MS, lenMs: C90_MS, tag: 'C-90', counter: time(e) };
}

// create({ canvas, ctx2d, theme, seed }) -> instance (design amendment A/D). The plugin draws on the
// host's pixel canvas (frame's `pixels`) over the backdrop the host painted and returns its text as
// labels; it keeps no timers, fetches nothing and touches no DOM.
export function create() {
  const painter = new CassettePainter();
  let alive = true;
  const groups = Object.freeze(Array.from({ length: CASSETTE.groupCount() }, (_, g) => Object.freeze({
    id: CASSETTE.groupId(g),
    labelKey: CASSETTE.groupLabelKey(g),
    count: CASSETTE.tourCount(g),
    pos: (cfg) => CASSETTE.tourPos(cfg, g),
    step: (cfg, shown) => CASSETTE.tourStep(cfg, shown, g),
  })));
  return {
    needs,
    schema: SCHEMA,
    presets: PRESETS,
    defaults: () => CASSETTE.defaults(),
    validate: (obj) => CASSETTE.validate(obj),
    applyPreset: (cfg, id) => CASSETTE.applyPreset(cfg, id),
    presetOf: (cfg) => CASSETTE.preset(cfg),
    resolveFollow: (cfg, seed) => CASSETTE.resolve(cfg, seed),
    groups: () => groups,
    // Laid out from the canvas each frame (chipchill's any size): nothing to keep.
    resize() {},
    // title / artist: what the label says (host.js maps the station's name and its ICY title);
    // elapsedMs: the play time; playing: the reels turn (by dt) and the lamp is lit; measure: the
    // platform's text widths (labels.js), rgxx's metrics when not given. Audio is not used.
    frame({ dt, theme, cfg, playing, pixels, title = '', artist = '', elapsedMs = 0, measure = null }) {
      if (!alive) return undefined;
      dt = F(Number.isFinite(dt) && dt > 0 ? dt : 0);
      const labels = painter.draw(pixels, { theme, cfg, title: String(title ?? ''), artist: String(artist ?? ''), ...streamView(elapsedMs), dt, moving: !!playing, measure: measure ?? RGXX_METRICS });
      return { labels };
    },
    status: () => (alive ? 'ok' : 'off'),
    painter: () => painter,
    dispose() { alive = false; },
  };
}
