// SPDX-License-Identifier: LGPL-2.1-or-later
// The visuals' palettes: chipchill's 18 themes, copied exactly from chipchill
// crates/chipchill-viz/src/colour.rs (Theme::ALL; rgxx's seven first, then chipchill's own), and
// one derived from the app's own CSS tokens (the `app` choice). A theme only changes colours,
// never a layout. Only the visual's own background and ramps use them; the app chrome keeps its
// own look (design amendment F).
import { rgb, mix } from './pixel.js';
import { hsv, lum } from './colour.js';

// rgxx's order: bg grid panel panel2 fg dim faint a1 a2 a3 ok purp warn moon black
export const FIELDS = Object.freeze(['bg', 'grid', 'panel', 'panel2', 'fg', 'dim', 'faint', 'cyan', 'mag', 'yel', 'lime', 'purp', 'warn', 'moon', 'black']);

const RAW = [
  ['neon', [[7, 5, 18], [34, 22, 66], [16, 12, 36], [30, 22, 64], [228, 234, 255], [120, 110, 172], [52, 40, 96], [0, 230, 255], [255, 40, 160], [255, 212, 40], [90, 255, 140], [150, 80, 255], [255, 64, 64], [235, 232, 210], [4, 3, 12]], false],
  ['miku', [[5, 13, 19], [14, 38, 46], [9, 23, 31], [15, 46, 54], [232, 248, 246], [112, 160, 164], [34, 70, 78], [57, 214, 200], [255, 82, 160], [255, 226, 120], [134, 230, 120], [47, 168, 168], [255, 80, 80], [235, 232, 210], [2, 6, 9]], false],
  ['vapor', [[24, 16, 48], [48, 32, 88], [36, 24, 68], [64, 44, 112], [255, 240, 250], [168, 148, 204], [80, 60, 128], [128, 236, 224], [255, 140, 200], [255, 220, 150], [170, 240, 170], [176, 140, 255], [255, 120, 120], [245, 236, 220], [12, 8, 24]], false],
  ['amber', [[12, 6, 0], [40, 22, 0], [24, 13, 0], [56, 32, 0], [255, 200, 96], [176, 112, 24], [88, 52, 0], [255, 176, 0], [255, 120, 0], [255, 224, 140], [255, 176, 0], [255, 148, 0], [255, 80, 32], [255, 224, 140], [6, 3, 0]], false],
  ['phosphor', [[2, 10, 4], [8, 34, 14], [4, 18, 8], [10, 46, 20], [150, 255, 170], [60, 164, 84], [20, 72, 34], [80, 255, 120], [190, 255, 110], [225, 255, 205], [80, 255, 120], [135, 255, 115], [255, 120, 60], [225, 255, 205], [1, 5, 2]], false],
  ['gameboy', [[155, 188, 15], [139, 172, 15], [139, 172, 15], [124, 160, 20], [6, 34, 8], [30, 68, 30], [60, 100, 20], [15, 56, 15], [48, 98, 48], [15, 56, 15], [15, 56, 15], [48, 98, 48], [15, 56, 15], [48, 98, 48], [139, 172, 15]], true],
  ['paper', [[240, 232, 214], [224, 214, 192], [250, 244, 230], [228, 216, 190], [40, 32, 28], [120, 108, 96], [150, 138, 118], [190, 48, 40], [28, 72, 120], [200, 140, 20], [60, 130, 70], [109, 60, 80], [190, 48, 40], [200, 140, 20], [224, 214, 192]], true],
  // Solarized dark: the colours are Ethan Schoonover's (MIT).
  ['solar', [[0, 43, 54], [16, 70, 84], [7, 54, 66], [16, 70, 84], [238, 232, 213], [131, 148, 150], [88, 110, 117], [42, 161, 152], [211, 54, 130], [181, 137, 0], [133, 153, 0], [108, 113, 196], [220, 50, 47], [253, 246, 227], [0, 30, 38]], false],
  // Gruvbox dark: the colours are Pavel Pertsev's (MIT).
  ['gruv', [[40, 40, 40], [60, 56, 54], [50, 48, 47], [80, 73, 69], [235, 219, 178], [168, 153, 132], [102, 92, 84], [254, 128, 25], [131, 165, 152], [250, 189, 47], [184, 187, 38], [211, 134, 155], [251, 73, 52], [235, 219, 178], [29, 32, 33]], false],
  ['unit', [[14, 8, 24], [44, 26, 72], [24, 14, 42], [48, 28, 80], [232, 240, 224], [140, 120, 176], [64, 44, 100], [140, 230, 60], [255, 140, 40], [255, 220, 80], [140, 230, 60], [150, 90, 230], [255, 60, 60], [235, 232, 210], [6, 3, 12]], false],
  ['ink', [[250, 250, 248], [220, 220, 218], [255, 255, 255], [232, 232, 230], [16, 16, 16], [96, 96, 96], [140, 140, 138], [16, 16, 16], [96, 96, 96], [16, 16, 16], [16, 16, 16], [96, 96, 96], [160, 0, 0], [96, 96, 96], [220, 220, 218]], true],
  ['snes', [[36, 36, 48], [52, 52, 68], [56, 56, 72], [74, 74, 94], [236, 236, 244], [178, 178, 194], [120, 120, 138], [138, 127, 214], [224, 58, 62], [247, 201, 72], [59, 170, 90], [75, 63, 158], [224, 58, 62], [46, 119, 208], [18, 18, 24]], false],
  ['sfc', [[30, 32, 40], [46, 48, 60], [54, 56, 70], [72, 74, 92], [240, 240, 244], [180, 180, 192], [120, 120, 136], [200, 16, 46], [0, 112, 192], [250, 200, 0], [0, 160, 80], [120, 80, 180], [200, 16, 46], [0, 112, 192], [16, 16, 22]], false],
  ['caution', [[22, 16, 34], [40, 30, 60], [36, 28, 54], [52, 40, 76], [240, 236, 248], [170, 160, 190], [110, 100, 130], [250, 140, 0], [140, 255, 80], [255, 220, 0], [140, 255, 80], [120, 60, 200], [255, 60, 60], [180, 120, 255], [10, 6, 16]], false],
  ['aqua', [[244, 250, 250], [224, 236, 236], [255, 255, 255], [236, 247, 246], [30, 40, 44], [110, 124, 128], [170, 186, 186], [30, 150, 142], [255, 100, 160], [255, 200, 40], [60, 170, 90], [140, 100, 200], [220, 60, 60], [80, 120, 200], [220, 232, 232]], true],
  ['cozy', [[250, 242, 226], [236, 222, 200], [255, 252, 246], [242, 230, 206], [70, 50, 40], [140, 116, 100], [190, 170, 150], [120, 84, 60], [240, 150, 150], [255, 204, 80], [120, 160, 90], [160, 120, 160], [210, 90, 80], [110, 130, 170], [236, 222, 200]], true],
  ['mono', [[255, 255, 255], [230, 230, 230], [255, 255, 255], [240, 240, 240], [10, 10, 10], [90, 90, 90], [160, 160, 160], [10, 10, 10], [230, 30, 30], [230, 30, 30], [40, 140, 80], [100, 80, 160], [230, 30, 30], [60, 90, 160], [230, 230, 230]], true],
  ['carbon', [[12, 12, 14], [30, 30, 36], [24, 24, 28], [34, 34, 40], [235, 235, 240], [160, 160, 170], [100, 100, 110], [255, 120, 0], [0, 200, 230], [255, 200, 0], [80, 200, 120], [160, 100, 255], [255, 70, 70], [120, 140, 200], [6, 6, 8]], false],
];

// The data table: { name, colours: 15 x [r, g, b], light }, frozen.
export const THEMES = Object.freeze(RAW.map(([name, colours, light]) => Object.freeze({ name, colours: Object.freeze(colours.map((c) => Object.freeze(c))), light })));
export const THEME_NAMES = Object.freeze(THEMES.map((t) => t.name));

function build(name, colours, light) {
  const t = { name, light };
  FIELDS.forEach((f, i) => { t[f] = rgb(colours[i][0], colours[i][1], colours[i][2]); });
  return Object.freeze(t);
}
const BY_NAME = new Map(THEMES.map((t) => [t.name, build(t.name, t.colours, t.light)]));

// The theme a page draws with: { name, bg, grid, …, black (packed colours), light }; null if unknown.
export function themeNamed(name) {
  return BY_NAME.get(name) ?? null;
}

// '#rgb', '#rrggbb' or 'rgb(r, g, b)' / 'rgba(…)' -> [r, g, b]; null for anything else.
function parse(s) {
  if (typeof s !== 'string') return null;
  s = s.trim().toLowerCase();
  let m = /^#([0-9a-f]{3})$/.exec(s);
  if (m) return [...m[1]].map((h) => parseInt(h + h, 16));
  m = /^#([0-9a-f]{6})$/.exec(s);
  if (m) return [0, 2, 4].map((i) => parseInt(m[1].slice(i, i + 2), 16));
  m = /^rgba?\(\s*(\d+)[\s,]+(\d+)[\s,]+(\d+)/.exec(s);
  if (m) return [1, 2, 3].map((i) => Math.min(255, +m[i]));
  return null;
}

function hsvOf([r, g, b]) {
  const mx = Math.max(r, g, b), mn = Math.min(r, g, b), d = mx - mn;
  let h = 0;
  if (d) {
    if (mx === r) h = 60 * (((g - b) / d) % 6);
    else if (mx === g) h = 60 * ((b - r) / d + 2);
    else h = 60 * ((r - g) / d + 4);
  }
  return [(h + 360) % 360, mx ? d / mx : 0, mx / 255];
}

// The `app` choice: a palette from the app's current CSS custom properties. `tokens` is a
// getComputedStyle()-like object (getPropertyValue) or a function name -> value.
// Mapping (the app has one accent, the palettes have five):
//   bg = --bg, panel = --card, grid = --line, panel2 = --card and --line half and half,
//   fg = --fg, moon = --fg, dim = --muted, faint = --bg 57% towards --muted,
//   cyan (accent 1) = --accent; purp, mag (accent 2) and yel (the highlight) are the accent's hue
//   turned as neon's are from its cyan (+73°, +140°, +222°) at the accent's saturation and value,
//   so the palette ramp runs accent -> purple-ish -> clash -> highlight as rgxx's does;
//   lime ("good", the left channel's start) is a green and warn a red at the same s/v;
//   black = --bg halfway to black on a dark page, --line on a light one.
//   light = the background is light (lum > 128): the app's tokens follow prefers-color-scheme.
// Tokens that are missing or unreadable keep the value of neon (dark) or paper (light); no --bg
// at all gives the neon palette itself.
export function themeFromTokens(tokens) {
  const get = (k) => {
    try {
      if (typeof tokens === 'function') return parse(tokens(k));
      if (tokens && typeof tokens.getPropertyValue === 'function') return parse(tokens.getPropertyValue(k));
    } catch { /* unreadable: as missing */ }
    return null;
  };
  const bg = get('--bg');
  if (!bg) return themeNamed('neon');
  const c = (v) => rgb(v[0], v[1], v[2]);
  const light = lum(c(bg)) > 128;
  const base = themeNamed(light ? 'paper' : 'neon');
  const t = { name: 'app', light };
  for (const f of FIELDS) t[f] = base[f];
  t.bg = c(bg);
  const card = get('--card'), line = get('--line'), fg = get('--fg'), muted = get('--muted'), accent = get('--accent');
  if (card) t.panel = c(card);
  if (line) t.grid = c(line);
  t.panel2 = mix(t.panel, t.grid, 128);
  if (fg) { t.fg = c(fg); t.moon = t.fg; }
  if (muted) t.dim = c(muted);
  t.faint = mix(t.bg, t.dim, 146);
  if (accent) {
    t.cyan = c(accent);
    const [h, s0, v] = hsvOf(accent);
    const s = Math.max(s0, 0.5);
    t.purp = hsv(Math.round(h + 73), s, v);
    t.mag = hsv(Math.round(h + 140), s, v);
    t.yel = hsv(Math.round(h + 222), s, v);
    t.lime = hsv(140, s, v);
    t.warn = hsv(0, s, v);
  }
  t.black = light ? t.grid : mix(t.bg, rgb(0, 0, 0), 128);
  return Object.freeze(t);
}
