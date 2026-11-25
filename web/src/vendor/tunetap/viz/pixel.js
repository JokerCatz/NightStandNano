// SPDX-License-Identifier: LGPL-2.1-or-later
// The pixels every visual draws on: boxes, discs, rings, lines, light added to what is there, the
// few fills rgxx's theme drew, over a buffer of one Uint32 a pixel, row after row.
// Ported from chipchill crates/chipchill-viz/src/canvas.rs (same author): the same names, the same
// integer arithmetic, the same order of operations and the same clipping, so a later port of a
// chipchill page is mechanical.
//
// Packing: unlike chipchill's XRGB8888 (0xFFRRGGBB) the colour here is what ImageData's bytes
// R, G, B, A read as ONE little-endian Uint32: 0xAABBGGRR. rgb(r, g, b) packs it with A = 255, so a
// buffer can be copied into ImageData.data as it is. (Every browser tunetap runs on is
// little-endian.) red/green/blue unpack it. Nothing else here depends on the byte order.
//
// A page is given a band of rows (y0..y1) and nothing writes outside it (chipchill's setRows).

export function rgb(r, g, b) {
  return (0xff000000 | ((b & 255) << 16) | ((g & 255) << 8) | (r & 255)) >>> 0;
}
export const red = (c) => c & 255;
export const green = (c) => (c >>> 8) & 255;
export const blue = (c) => (c >>> 16) & 255;

// a towards b by t (0 is a, 255 is b): rgxx's rg_mix, the same integer arithmetic
export function mix(a, b, t) {
  return rgb(
    Math.trunc((red(a) * (255 - t) + red(b) * t) / 255),
    Math.trunc((green(a) * (255 - t) + green(b) * t) / 255),
    Math.trunc((blue(a) * (255 - t) + blue(b) * t) / 255),
  );
}

// c scaled by t/255 as a layer value; on a light palette its complement, which is then
// subtracted (colour.rs tone, rgxx viz_tone).
export function tone(c, t, light) {
  if (light) {
    return rgb(Math.trunc(((255 - red(c)) * t) / 255), Math.trunc(((255 - green(c)) * t) / 255), Math.trunc(((255 - blue(c)) * t) / 255));
  }
  return rgb(Math.trunc((red(c) * t) / 255), Math.trunc((green(c) * t) / 255), Math.trunc((blue(c) * t) / 255));
}

// The layer on dst, saturating: added, or subtracted on a light palette (colour.rs blend).
export function blend(dst, layer, light) {
  if (light) {
    return rgb(Math.max(red(dst) - red(layer), 0), Math.max(green(dst) - green(layer), 0), Math.max(blue(dst) - blue(layer), 0));
  }
  return rgb(Math.min(red(dst) + red(layer), 255), Math.min(green(dst) + green(layer), 255), Math.min(blue(dst) + blue(layer), 255));
}

export class Canvas {
  // px: an optional Uint32Array of at least width * height pixels; a new one otherwise.
  constructor(width, height, px) {
    this.width = Math.max(0, width | 0);
    this.height = Math.max(0, height | 0);
    this.px32 = px ?? new Uint32Array(this.width * this.height);
    if (this.px32.length < this.width * this.height) throw new RangeError(`a ${width}x${height} canvas needs ${width * height} pixels`);
    this.y0 = 0;
    this.y1 = this.height;
  }

  // The rows y0..y1 are the page's; clamped to the buffer.
  setRows(y0, y1) {
    this.y0 = Math.min(Math.max(y0, 0), this.height);
    this.y1 = Math.min(Math.max(y1, this.y0), this.height);
  }

  inside(x, y) {
    return x >= 0 && x < this.width && y >= this.y0 && y < this.y1;
  }

  // The page's rows, all of one colour.
  clear(c) {
    this.px32.fill(c, this.y0 * this.width, this.y1 * this.width);
  }

  // One pixel.
  px(x, y, c) {
    if (this.inside(x, y)) this.px32[y * this.width + x] = c;
  }

  // Row y whole, as a view into the pixels (chipchill row_mut), if it is one of the page's rows;
  // null otherwise.
  rowMut(y) {
    if (y < this.y0 || y >= this.y1) return null;
    const at = y * this.width;
    return this.px32.subarray(at, at + this.width);
  }

  // A filled rectangle (rgxx box / ui_fill).
  fill(x, y, w, h, c) {
    const x0 = Math.max(x, 0), y0 = Math.max(y, this.y0);
    const x1 = Math.min(x + w, this.width), y1 = Math.min(y + h, this.y1);
    if (x1 <= x0 || y1 <= y0) return;
    for (let yy = y0; yy < y1; yy++) {
      const at = yy * this.width;
      this.px32.fill(c, at + x0, at + x1);
    }
  }

  // A one-pixel outline (ui_frame).
  frame(x, y, w, h, c) {
    this.fill(x, y, w, 1, c);
    this.fill(x, y + h - 1, w, 1, c);
    this.fill(x, y, 1, h, c);
    this.fill(x + w - 1, y, 1, h, c);
  }

  // One row of pixels (hspan).
  hspan(x, y, w, c) {
    if (y < this.y0 || y >= this.y1) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > this.width) w = this.width - x;
    if (w > 0) {
      const at = y * this.width + x;
      this.px32.fill(c, at, at + w);
    }
  }

  // A filled circle of radius r.
  disc(cx, cy, r, c) {
    for (let y = -r; y <= r; y++) {
      const h = Math.trunc(Math.sqrt(r * r - y * y));
      this.fill(cx - h, cy + y, 2 * h + 1, 1, c);
    }
  }

  // A circle's outline, as 720 points.
  ring(cx, cy, r, c) {
    for (let a = 0; a < 720; a++) {
      const t = (a * Math.PI) / 360;
      this.px(cx + Math.round(r * Math.cos(t)), cy + Math.round(r * Math.sin(t)), c);
    }
  }

  // A line of thickness w.
  line(x0, y0, x1, y1, w, c) {
    const dx = x1 - x0, dy = y1 - y0;
    const n = Math.max(Math.abs(dx) > Math.abs(dy) ? Math.abs(dx) : Math.abs(dy), 1);
    for (let i = 0; i <= n; i++) {
      const x = x0 + Math.trunc((dx * i) / n), y = y0 + Math.trunc((dy * i) / n);
      if (w <= 1) {
        this.px(x, y, c);
      } else if (w === 2) {
        this.px(x - 1, y - 1, c);
        this.px(x, y - 1, c);
        this.px(x - 1, y, c);
        this.px(x, y, c);
      } else {
        const h = Math.trunc(w / 2);
        this.fill(x - h, y - h, w, w, c);
      }
    }
  }

  // Light added to what is there (beams, glows), saturating.
  add(x, y, r, g, b) {
    if (!this.inside(x, y)) return;
    const i = y * this.width + x;
    const v = this.px32[i];
    this.px32[i] = rgb(Math.min(red(v) + r, 255), Math.min(green(v) + g, 255), Math.min(blue(v) + b, 255));
  }

  // A layer's value (tone) onto one pixel: added, or subtracted on a light palette.
  blend(x, y, layer, light) {
    if (!this.inside(x, y)) return;
    const i = y * this.width + x;
    this.px32[i] = blend(this.px32[i], layer, light);
  }

  // Every other row of a rectangle darkened by strength (0..255), starting with its second
  // (rg_scanlines).
  scanlines(x, y, w, h, strength) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > this.width) w = this.width - x;
    if (y + h > this.height) h = this.height - y;
    const k = (v) => Math.trunc((v * (255 - strength)) / 255);
    for (let r = y + 1; r < y + h; r += 2) {
      if (r < this.y0 || r >= this.y1) continue;
      for (let i = 0; i < Math.max(w, 0); i++) {
        const at = r * this.width + x + i;
        const v = this.px32[at];
        this.px32[at] = rgb(k(red(v)), k(green(v)), k(blue(v)));
      }
    }
  }

  // A panel with its corners cut and a one-pixel edge, the diagonals two thick (rg_cut_panel).
  cutPanel(x, y, w, h, cut, fill, edge) {
    for (let r = 0; r < h; r++) {
      const l = r < cut ? cut - r : 0;
      const rr = r > h - 1 - cut ? r - (h - 1 - cut) : 0;
      const x0 = x + l, x1 = x + w - 1 - rr;
      if (x1 < x0) continue;
      if (r === 0 || r === h - 1) {
        this.fill(x0, y + r, x1 - x0 + 1, 1, edge);
        continue;
      }
      this.fill(x0 + 1, y + r, x1 - x0 - 1, 1, fill);
      this.fill(x0, y + r, 1, 1, edge);
      this.fill(x1, y + r, 1, 1, edge);
      if (r < cut) this.fill(x0 + 1, y + r, 1, 1, edge);
      if (r > h - 1 - cut) this.fill(x1 - 1, y + r, 1, 1, edge);
    }
  }
}

// The canvas onto a 2D context at (0, 0) with putImageData. One ImageData per context and size,
// kept between frames.
const images = new WeakMap();
export function blit(ctx2d, canvas) {
  const { width: w, height: h } = canvas;
  if (!ctx2d || !w || !h) return;
  let img = images.get(ctx2d);
  if (!img || img.width !== w || img.height !== h) {
    img = ctx2d.createImageData(w, h);
    images.set(ctx2d, img);
  }
  img.data.set(new Uint8ClampedArray(canvas.px32.buffer, canvas.px32.byteOffset, w * h * 4));
  ctx2d.putImageData(img, 0, 0);
}
