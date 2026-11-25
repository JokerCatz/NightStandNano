// SPDX-License-Identifier: LGPL-2.1-or-later
// The text on a page, left for the platform to draw: chipchill's Labels (crates/chipchill-viz/src/
// label.rs). A page draws pictures into the pixel canvas and hands its text over as labels saying
// what, where, how big and in which colours; the host draws them over the finished picture, on a
// second canvas at the screen's own resolution (so the words are sharp while the picture is
// pixelated). Nothing a page draws depends on text except where the text goes, which a page asks
// of a measure the platform gives (rgxx's own metrics are RGXX_METRICS, what the goldens use).
//
// A label, in the pixel canvas's coordinates (chipchill's Label, with the text already chosen):
//   { kind: 'title'|'artist'|'str'|'short', text, font: 'hud'|'wide', scale, x, y (the top left of
//     the first character), colour (pixel.js packed), chip (packed colour of the tag's plate, or
//     null), maxW (at most this wide, cut short with an ellipsis, or null) }
// 'hud' is rgxx's 8x8 font of numbers and tags (8 px a character, times scale), 'wide' its full
// font (16 px high, 8 wide for Latin and 16 for CJK, times scale).
//
// The layer (createLabelLayer) is drawn as chipchill's Android platform draws them
// (android ui/playing/Picture.kt): the font 8 or 16 x scale pixels high scaled to the screen, the
// baseline at 0.85 of that height, a tag's plate a rounded rectangle 2 pixels round its words; and
// as label.rs says, text past maxW is cut short with an ellipsis. Here every label is also clipped
// to its own box and to the canvas (it never overflows), and the canvas is drawn again only when
// the labels or the size change (the cassette's counter changes once a second; a frame with the
// same words costs a comparison).
import { red, green, blue } from './pixel.js';

export const FONT_STACK = 'system-ui, "PingFang TC", "Noto Sans CJK TC", "Microsoft JhengHei", sans-serif';
const ELLIPSIS = '…';
const BASE = 0.85; // the baseline, as a part of the font's height (Picture.kt)

// How high a label's font is, in the canvas's pixels.
export const fontHeight = (l) => (l.font === 'wide' ? 16 : 8) * (l.scale || 1);

// Whether Unifont draws c 16 pixels wide: the CJK blocks and the fullwidth forms (label.rs wide).
export function wideChar(cp) {
  return (cp >= 0x1100 && cp <= 0x115f) || (cp >= 0x2e80 && cp <= 0xa4cf) || (cp >= 0xac00 && cp <= 0xd7a3) || (cp >= 0xf900 && cp <= 0xfaff)
    || (cp >= 0xfe30 && cp <= 0xfe4f) || (cp >= 0xff00 && cp <= 0xff60) || (cp >= 0xffe0 && cp <= 0xffe6);
}

// rgxx's metrics (label.rs RgxxMetrics): 8 pixels a character in the 8x8 font; in Unifont 8 for a
// halfwidth character and 16 for a fullwidth one, all times scale.
export const RGXX_METRICS = Object.freeze({
  width(font, scale, text) {
    let cells = 0;
    for (const ch of String(text ?? '')) cells += font === 'wide' && wideChar(ch.codePointAt(0)) ? 2 : 1;
    return 8 * scale * cells;
  },
});

// Where a label's pixels can be, drawn with measure m: [x, y, w, h] (label.rs Label::bounds).
export function bounds(l, m = RGXX_METRICS) {
  let w = m.width(l.font, l.scale, l.text ?? '');
  if (l.maxW !== null && l.maxW !== undefined) w = Math.min(w, l.maxW);
  const h = fontHeight(l);
  if (l.chip !== null && l.chip !== undefined) return l.font === 'wide' ? [l.x, l.y - 4, w + 8, 19] : [l.x, l.y, w + 6, 11];
  return [l.x, l.y, w, h];
}

const FIELDS = ['kind', 'text', 'font', 'scale', 'x', 'y', 'colour', 'chip', 'maxW'];
// Two lists of labels say the same (no allocation).
export function sameLabels(a, b) {
  if (a === b) return true;
  if (!a || !b || a.length !== b.length) return false;
  for (let i = 0; i < a.length; i++) for (const f of FIELDS) if ((a[i][f] ?? null) !== (b[i][f] ?? null)) return false;
  return true;
}

const css = (c) => `rgb(${red(c)},${green(c)},${blue(c)})`;
const fontOf = (px) => `${px}px ${FONT_STACK}`;

// The longest start of text that fits in w with an ellipsis after it (whole code points), by
// measureW(s) -> width. The text itself when it fits; '' when not even the ellipsis does.
export function fitText(text, w, measureW) {
  if (measureW(text) <= w) return text;
  const cps = [...text];
  let lo = 0, hi = cps.length; // the longest prefix that fits is in [lo, hi)
  if (measureW(ELLIPSIS) > w) return '';
  while (hi - lo > 1) {
    const mid = (lo + hi) >> 1;
    if (measureW(cps.slice(0, mid).join('') + ELLIPSIS) <= w) lo = mid; else hi = mid;
  }
  return lo === 0 ? ELLIPSIS : cps.slice(0, lo).join('').trimEnd() + ELLIPSIS;
}

// A measure from a 2D context (the platform's font, chipchill View.measure): widths in the
// canvas's pixels, for a font 8 or 16 x scale pixels high. Measured once per font and text at a
// fixed size and scaled (text widths are linear in the size), kept in a small cache.
const MEASURE_PX = 64;
export function contextMeasure(ctx) {
  const cache = new Map();
  const unit = (text) => {
    let w = cache.get(text);
    if (w === undefined) {
      try {
        ctx.font = fontOf(MEASURE_PX);
        w = (ctx.measureText(text)?.width ?? 0) / MEASURE_PX;
      } catch { w = RGXX_METRICS.width('wide', 1, text) / 16; }
      if (!Number.isFinite(w)) w = 0;
      if (cache.size > 256) cache.clear();
      cache.set(text, w);
    }
    return w;
  };
  return { width: (font, scale, text) => Math.ceil(unit(String(text ?? '')) * (font === 'wide' ? 16 : 8) * scale) };
}

// The layer: draws labels onto ctx (the labels canvas, device pixels) for a pixel canvas of
// pw x ph shown at the labels canvas's size. set(labels, pw, ph) draws when something changed;
// clear() empties it. Calls nothing on ctx while there have never been labels.
export function createLabelLayer({ canvas, ctx }) {
  let last = null; // the labels drawn, or null for an empty canvas
  let size = ''; // the sizes they were drawn at
  let draws = 0;
  const measure = contextMeasure(ctx);

  function wipe() {
    try { ctx.setTransform(1, 0, 0, 1, 0, 0); ctx.clearRect(0, 0, canvas.width, canvas.height); } catch { /* nothing to clear */ }
  }

  function paint(labels, pw, ph) {
    wipe();
    const kx = canvas.width / pw, ky = canvas.height / ph;
    if (!(kx > 0) || !(ky > 0)) return;
    for (const l of labels) {
      if (!l || typeof l.text !== 'string' || !l.text) continue;
      const hgt = fontHeight(l);
      const font = fontOf(hgt);
      ctx.font = font;
      const mw = (s) => ctx.measureText(s).width;
      // at most maxW, and never past the canvas's right edge
      let room = pw - l.x;
      if (l.maxW !== null && l.maxW !== undefined) room = Math.min(room, l.maxW);
      if (l.chip !== null && l.chip !== undefined) room -= l.font === 'wide' ? 8 : 6;
      if (room <= 0) continue;
      const text = fitText(l.text, room, mw);
      if (!text) continue;
      const tw = Math.min(mw(text), room);
      const [bx, by, bw, bh] = l.chip !== null && l.chip !== undefined
        ? (l.font === 'wide' ? [l.x, l.y - 4, tw + 8, 19] : [l.x, l.y, tw + 6, 11])
        : [l.x, l.y, tw, hgt];
      ctx.save();
      ctx.setTransform(kx, 0, 0, ky, 0, 0);
      ctx.beginPath();
      ctx.rect(Math.max(bx - 1, 0), Math.max(by - 1, 0), Math.min(bw + 2, pw - Math.max(bx - 1, 0)), Math.min(bh + 2, ph - Math.max(by - 1, 0)));
      ctx.clip();
      let tx = l.x;
      let ty = l.y;
      if (l.chip !== null && l.chip !== undefined) {
        // rgxx's rg_tag: the words 3 pixels in (4 for wide text), on a plate round them
        ctx.fillStyle = css(l.chip);
        const pad = 2;
        ctx.beginPath();
        if (typeof ctx.roundRect === 'function') ctx.roundRect(bx, by, bw, bh, pad); else ctx.rect(bx, by, bw, bh);
        ctx.fill();
        tx = l.x + (l.font === 'wide' ? 4 : 3);
        ty = l.font === 'wide' ? l.y - 3 : l.y + 2;
      }
      ctx.fillStyle = css(l.colour);
      ctx.textBaseline = 'alphabetic';
      ctx.textAlign = 'left';
      ctx.fillText(text, tx, ty + hgt * BASE);
      ctx.restore();
    }
    draws++;
  }

  return {
    measure,
    // Draws labels (an array, or null / empty for none) for a pw x ph pixel canvas; returns
    // whether the canvas was drawn again.
    set(labels, pw, ph) {
      const list = Array.isArray(labels) && labels.length ? labels : null;
      const sz = `${canvas.width}x${canvas.height}/${pw}x${ph}`;
      if (!list) {
        if (last === null) return false;
        wipe();
        last = null;
        return true;
      }
      if (last !== null && size === sz && sameLabels(list, last)) return false;
      try { paint(list, pw, ph); } catch { wipe(); }
      // a copy: the page may reuse its objects
      last = list.map((l) => ({ ...l }));
      size = sz;
      return true;
    },
    // Empties the canvas (a new mode, the host stopped); the next set() draws again.
    clear() {
      if (last === null) return;
      wipe();
      last = null;
    },
    // The labels on the canvas now (for tests and the smoke's hook).
    shown: () => (last ? last.map((l) => ({ ...l })) : []),
    draws: () => draws,
  };
}
