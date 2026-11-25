// SPDX-License-Identifier: LGPL-2.1-or-later
// Colour as arithmetic for the visuals: a ramp (色帶) says which colour stands for 0..1, a mapping
// (上色) says what of a bar is measured against it, and the light-palette rules keep every ramp
// legible on paper. Ported from chipchill crates/chipchill-viz/src/colour.rs (rgxx viz_color.c),
// expression for expression. chipchill computes in f32; every float step here goes through
// Math.fround so the tables come out as chipchill's to the bit (tests check all 18 themes).
// Colours are pixel.js's packed values.
import { rgb, red, green, blue, mix, tone, blend } from './pixel.js';

export { tone, blend };

const F = Math.fround;

export const RAMP = Object.freeze({ PALETTE: 0, FIRE: 1, ICE: 2, WINAMP: 3, RAINBOW: 4, DUO: 5, MONO: 6 });
export const RAMP_COUNT = 7;
export const MAP = Object.freeze({ BOTTOM: 0, TIP: 1, BAND: 2, LEVEL: 3 });
export const MAP_COUNT = 4;
// The tip gradient's length as a part of the full height: fixed, not per bar.
export const TIP = F(0.42);

// A ramp by its index; anything out of range is the palette's (rgxx's switch default).
export const rampAt = (i) => (Number.isInteger(i) && i >= 0 && i < RAMP_COUNT ? i : RAMP.PALETTE);
export const mapAt = (i) => (i === 1 || i === 2 || i === 3 ? i : MAP.BOTTOM);
// The whole bar has one colour.
export const mapFlat = (m) => m === MAP.BAND || m === MAP.LEVEL;

// What a ramp needs of the theme and of the owner's two hues (rgxx viz_pal): duo steps are
// tens of degrees (cfg's duo1/duo2, 0..35).
export function palOf(t, duo1Step, duo2Step) {
  return { bg: t.bg, panel: t.panel, fg: t.fg, a1: t.cyan, a2: t.mag, a3: t.yel, ok: t.lime, purp: t.purp, light: !!t.light, duo1: duo1Step * 10, duo2: duo2Step * 10 };
}

export function samePal(a, b) {
  if (!a || !b) return false;
  for (const k of ['bg', 'panel', 'fg', 'a1', 'a2', 'a3', 'ok', 'purp', 'light', 'duo1', 'duo2']) if (a[k] !== b[k]) return false;
  return true;
}

// A colour from hue (degrees), saturation and value (0..1).
export function hsv(h, s, v) {
  h = ((Math.trunc(h) % 360) + 360) % 360;
  s = F(s); v = F(v);
  const c = F(v * s);
  const x = F(c * F(1 - Math.abs(F(F(F(h / 60) % 2) - 1))));
  const m = F(v - c);
  let r = 0, g = 0, b = 0;
  switch (Math.trunc(h / 60)) {
    case 0: r = c; g = x; break;
    case 1: r = x; g = c; break;
    case 2: g = c; b = x; break;
    case 3: g = x; b = c; break;
    case 4: r = x; b = c; break;
    default: r = c; b = x; break;
  }
  const q = (u) => Math.round(F(F(u + m) * 255));
  return rgb(q(r), q(g), q(b));
}

// A ramp through fixed colours: where each stands (0..1) and what it is.
const st = (f, r, g, b) => Object.freeze([F(f), r, g, b]);
const FIRE = [st(0, 64, 0, 0), st(0.4, 196, 16, 0), st(0.75, 255, 116, 0), st(1, 255, 226, 96)];
const ICE = [st(0, 0, 16, 72), st(0.45, 0, 110, 220), st(0.8, 90, 230, 255), st(1, 235, 255, 255)];
const WINAMP = [st(0, 24, 176, 40), st(0.55, 230, 220, 30), st(0.8, 255, 140, 0), st(1, 255, 40, 30)];
// The same ramps for a light palette: on paper the ink is dark, so the loud end is the dark end
// and the quiet end nearly the paper. Each keeps its hues, in the other order of lightness.
const FIRE_LIGHT = [st(0, 255, 236, 200), st(0.4, 255, 170, 40), st(0.75, 210, 40, 0), st(1, 90, 0, 0)];
const ICE_LIGHT = [st(0, 230, 245, 255), st(0.45, 90, 180, 255), st(0.8, 0, 90, 200), st(1, 0, 20, 80)];
const WINAMP_LIGHT = [st(0, 170, 230, 130), st(0.55, 190, 170, 0), st(0.8, 220, 100, 0), st(1, 150, 0, 10)];

function through(s, f) {
  for (let i = 0; i < s.length - 1; i++) {
    if (f <= s[i + 1][0]) {
      const t = F(F(f - s[i][0]) / F(s[i + 1][0] - s[i][0]));
      const at = (a, b) => Math.trunc(F(a + F((b - a) * t)));
      return rgb(at(s[i][1], s[i + 1][1]), at(s[i][2], s[i + 1][2]), at(s[i][3], s[i + 1][3]));
    }
  }
  const l = s[s.length - 1];
  return rgb(l[1], l[2], l[3]);
}

// How bright a colour is to the eye, 0..255.
export function lum(c) {
  return Math.trunc((red(c) * 299 + green(c) * 587 + blue(c) * 114) / 1000);
}

// c at brightness target: darkened towards black, or lightened towards white, keeping its hue.
export function fitLum(c, target) {
  const l = lum(c);
  target = Math.min(Math.max(target, 0), 255);
  if (l === target) return c;
  if (l > target) {
    const k = F(target / Math.max(l, 1));
    return rgb(Math.trunc(F(red(c) * k)), Math.trunc(F(green(c) * k)), Math.trunc(F(blue(c) * k)));
  }
  return mix(c, rgb(255, 255, 255), Math.min(Math.max(Math.trunc(((target - l) * 255) / Math.max(255 - l, 1)), 0), 255));
}

// The brightness a light palette's ramp has at f: pale when quiet, ink when loud.
const paperLum = (f) => Math.trunc(F(225 - F(190 * f)));

// The colour at f (0..1) of a ramp. ch: 0 the left channel, 1 the right or both; only the
// palette's ramp tells them apart.
export function ramp(p, r, ch, f) {
  f = F(f);
  f = f > 0 ? Math.min(f, 1) : 0; // NaN too
  r = rampAt(r);
  let c;
  switch (r) {
    case RAMP.FIRE: c = through(p.light ? FIRE_LIGHT : FIRE, f); break;
    case RAMP.ICE: c = through(p.light ? ICE_LIGHT : ICE, f); break;
    case RAMP.WINAMP: c = through(p.light ? WINAMP_LIGHT : WINAMP, f); break;
    // On paper the rainbow and the two hues darken as they louden, their brightness a straight
    // line from pale to ink whatever the hue.
    case RAMP.RAINBOW:
      c = hsv(Math.trunc(F(280 - F(280 * f))), 0.85, 1);
      if (p.light) c = fitLum(c, paperLum(f));
      break;
    case RAMP.DUO:
      c = mix(hsv(p.duo1, 0.85, 1), hsv(p.duo2, 0.85, 1), Math.trunc(F(f * 255)));
      if (p.light) c = fitLum(c, paperLum(f));
      break;
    case RAMP.MONO: c = p.a1; break;
    default:
      if (f < 0.5) c = mix(ch ? p.a1 : p.ok, p.purp, Math.trunc(F(F(f * 2) * 255)));
      else if (f < F(0.8)) c = mix(p.purp, p.a2, Math.trunc(F(F(F(f - 0.5) / F(0.3)) * 255)));
      else c = p.a3;
  }
  // On a light palette whose panel is not white (the Game Boy's green) the whole ramp is taken
  // down in step, so that its quiet end is the panel's brightness, not above it.
  const k = F(lum(p.panel) / 240);
  if (p.light && r !== RAMP.PALETTE && k < 1) return fitLum(c, Math.trunc(F(lum(c) * k)));
  return c;
}

// What a mapping measures: bar i of n, at height k (0..1 of the full height), the bar's level v.
export function map(m, i, n, k, v) {
  switch (m) {
    case MAP.TIP: {
      const d = F(F(v - k) / TIP); // how far under the bar's top, in tip lengths
      return F(1 - Math.min(Math.max(d, 0), 1));
    }
    case MAP.BAND: return n > 1 ? F(i / (n - 1)) : 0;
    case MAP.LEVEL: return v;
    default: return k;
  }
}

// A ramp and a mapping at once.
export function colour(p, r, m, ch, i, n, k, v) {
  return ramp(p, r, ch, map(m, i, n, k, v));
}

// 256 steps of a ramp, for pages that colour by a byte.
export function lut(p, r, ch) {
  const out = new Uint32Array(256);
  for (let i = 0; i < 256; i++) out[i] = ramp(p, r, ch, F(i / 255));
  return out;
}

// The same, with loudness as brightness as well as colour: quiet is nearly the background (the
// waterfall). On a light palette quiet is the panel.
export function lutLoud(p, r, ch) {
  const dark = p.light ? p.panel : p.bg;
  const out = new Uint32Array(256);
  for (let i = 0; i < 256; i++) {
    const f = F(i / 255);
    const up = F(f * 1.5) > 1 ? 1 : F(f * 1.5);
    out[i] = mix(dark, ramp(p, r, ch, f), Math.trunc(F(F(Math.pow(up, F(1.4))) * 255)));
  }
  return out;
}
