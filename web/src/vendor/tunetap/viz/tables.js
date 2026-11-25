// SPDX-License-Identifier: LGPL-2.1-or-later
// The ramp the owner chose, as byte tables per channel: plain, loud (quiet is nearly the
// background: the waterfall) and glow (the goniometer's phosphor, as a layer to blend). Rebuilt
// only when the ramp or the palette's colours change: a frame looks colours up, it does not make
// them. Ported from chipchill crates/chipchill-viz/src/pages/tables.rs.
import { palOf, samePal, lut, lutLoud, ramp as rampColour, rampAt, tone } from './colour.js';

const F = Math.fround;

// The tables of a palette (colour.js palOf) and a ramp index; serial tells one build from another.
export function buildTables(pal, ramp, serial) {
  const r = rampAt(ramp);
  const t = { pal, ramp, serial, plain: [lut(pal, r, 0), lut(pal, r, 1)], loud: [lutLoud(pal, r, 0), lutLoud(pal, r, 1)], glow: [new Uint32Array(256), new Uint32Array(256)] };
  for (let ch = 0; ch < 2; ch++) {
    for (let g = 0; g < 256; g++) {
      // older light is dimmer and further down the ramp
      const c = rampColour(pal, r, ch, F(F(0.35) + F(F(F(0.65) * g) / 255)));
      t.glow[ch][g] = tone(c, 64 + Math.trunc((191 * g) / 255), pal.light);
    }
  }
  return t;
}

// A page's tables, kept between frames.
export class TableCache {
  constructor() {
    this.t = null;
    this.serial = 0;
  }

  // The tables for this frame's theme and the choices' ramp and hues: the same ones as last frame
  // unless those changed. Building them allocates, which is setting a page up, not drawing it.
  now(theme, cfg) {
    const pal = palOf(theme, cfg.duo1, cfg.duo2);
    const ramp = cfg.ramp;
    if (!(this.t && this.t.ramp === ramp && samePal(this.t.pal, pal))) {
      this.serial++;
      this.t = buildTables(pal, ramp, this.serial);
    }
    return this.t;
  }
}
