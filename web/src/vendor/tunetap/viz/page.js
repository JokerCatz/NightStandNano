// SPDX-License-Identifier: LGPL-2.1-or-later
// What every visual page shares: how big it is drawn against rgxx's size (Fit), the rows an
// instrument stands in on a tall canvas (stage), and the backdrop drawn under every page.
// Ported from chipchill crates/chipchill-viz/src/pages/mod.rs.
//
// Any size: each page lays itself out from the canvas's width and rows, at the scale at which
// rgxx's page (640 x 358 for the instruments) fits both ways, as a ratio of whole numbers, so that
// at rgxx's size every length and place is rgxx's own. What runs across the page (the spectrum's
// bars) runs across the whole width; on a canvas taller than wide (a phone held upright) an
// instrument keeps to a square in the middle.

export class Fit {
  constructor(num, den) {
    this.num = Math.max(num, 1);
    this.den = Math.max(den, 1);
  }
  // num / den; a size of nothing is taken as a pixel.
  static new(num, den) { return new Fit(num, den); }
  // The smaller of two: what fits both ways.
  static min(a, b) { return a.num * b.den <= b.num * a.den ? a : b; }
  // A length or an offset of rgxx's, at this size (towards zero).
  i(v) { return Math.trunc((v * this.num) / this.den); }
  // …and at least a pixel, for strokes.
  px(v) { return Math.max(this.i(v), 1); }
  // Back: a length on the canvas in rgxx's pixels.
  inv(v) { return Math.trunc((v * this.den) / this.num); }
  // As a number to multiply by: exactly 1 at rgxx's size.
  f() { return this.num / this.den; }
}

// The rows an instrument stands in: the page's own, unless the canvas is taller than wide, when
// it keeps to a square in the middle. Returns [y0, y1].
export function stage(c) {
  const y0 = c.y0, y1 = c.y1, w = c.width;
  if (y1 - y0 <= w) return [y0, y1];
  const top = y0 + Math.trunc((y1 - y0 - w) / 2);
  return [top, top + w];
}

// rgxx's backdrop on the neon style, which every page was drawn over: the background, and a dot
// of the grid colour every 16 pixels from (8, 8).
export function backdrop(c, t) {
  c.clear(t.bg);
  for (let y = 8; y < c.height; y += 16) for (let x = 8; x < c.width; x += 16) c.px(x, y, t.grid);
}

// The size of a page's text at a scale: rgxx's at 1, twice at 2, never less than rgxx's (pages/mod.rs
// text_scale). Only the labels use it (js/viz/labels.js).
export function textScale(s) {
  return Math.max(Math.trunc(s + 0.25), 1);
}
