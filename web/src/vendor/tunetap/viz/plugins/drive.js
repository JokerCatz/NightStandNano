// SPDX-License-Identifier: LGPL-2.1-or-later
// 夜行, the synthwave night drive, on the shared pixel canvas. A pixel port of chipchill
// crates/chipchill-viz/src/pages/drive.rs (rgxx drive.c), expression for expression (f32 steps
// through Math.fround, f64 where chipchill has f64, integer division truncating), checked against
// chipchill's own pictures (tests/viz-drive.test.mjs, tests/fixtures/viz-drive.json).
//
// Five routes, each with its own thing to look at: the setting sun, a neon tower, a lighthouse over
// the sea, the northern lights, a stage. The hour tints the sky (drive-env.js), the weather is a
// layer over the finished scene, and the music drives all of it: the road runs faster when it is
// loud, the stars listen to one band each, the tail lights flare on the kick, and the weather hears
// the music (drive-react.js).
//
// tunetap differences (see the drive report):
// - no forecast: the weather is the owner's or the station's (隨電台), never "follow the forecast";
//   the hour may follow the local clock (an injected Date);
// - with no audio data (audio: null) the scene is drawn from silent bands and the weather does not
//   hear (天氣隨音樂 off): the road runs at its quiet speed, nothing flares, and status() says
//   'no-data' — the picture never pretends to react;
// - the screens (CITY's billboard, STAGE's LED wall) follow cfg.ramp when one is given (chipchill
//   shares the instrument pages' ramp); tunetap keeps a cfg per visual and the drive has no ramp
//   row, so they show the scene's own colours, as chipchill does on its palette ramp;
// - the song position (meteor places) is the time this scene has been playing.
import { mix, rgb } from '../pixel.js';
import { palOf, ramp as rampColour, rampAt, RAMP } from '../colour.js';
import { Fit } from '../page.js';
import { steps, decay, STEP_S, STEP_CAP } from '../anim.js';
import { DRIVE, keyDef, valueLabelKey, ROUTE, HOUR_KEY, CAR, WEATHER_REACT } from '../cfg.js';
import { BANDS, createBands, resetBands, BandAnalyser } from '../bands.js';
import { HOUR, WEATHER, hourAt, tint as hourTint, minuteOfDay } from '../drive-env.js';
import { React } from '../drive-react.js';

const F = Math.fround;
const d = (a, b) => Math.trunc(a / b); // Rust's integer division
const lcg = (s) => (Math.imul(s, 1103515245) + 12345) >>> 0;
const u32 = (v) => (v > 0 ? (v < 4294967295 ? Math.trunc(v) : 4294967295) : 0); // f64 as u32 (saturating)
const PI = Math.PI;
const SUNSET = 0, CITY = 1, COAST = 2, AURORA = 3, STAGE = 4;
const WHITE = rgb(255, 255, 255), BLACK = rgb(0, 0, 0);
const F045 = F(0.45), F055 = F(0.55), F06 = F(0.6), F04 = F(0.4), F09 = F(0.9);

// The drive needs the 16 bands' levels averaged over the two channels (View.band and
// View.loudness are both channels' mean) and, for the city's windows, the left channel: a mono
// analyser gives exactly that (L = R) without the stereo splitter.
export const needs = Object.freeze({ freq: 'mono', time: 'none' });

// The scene's layout on a canvas of any size (chipchill's Lay): rgxx's 640 x 404 page scaled to fit
// the rows (exactly 1 at rgxx's size) and centred; what runs across the whole width runs across the
// canvas's; on a canvas taller than the scene (a phone held upright) the scene stands on the ground
// and the rows above it are more sky.
class Lay {
  constructor(fit, cx, top, sky) {
    this.fit = fit;
    this.s = fit.f();
    this.cx = cx;
    this.top = top;
    this.sky = sky;
  }
  x(x) { return this.cx + this.fit.i(x - 320); }
  l(v) { return this.fit.i(v); }
  dx(x) { return 320 + this.fit.inv(x - this.cx); }
  dxf(x) { return 320 + (x - this.cx) / this.s; }
}

// The average level of bands lo..=hi, both channels (View.loudness).
function loudness(lv, lo, hi) {
  let s = 0;
  for (let b = lo; b <= hi; b++) s = F(s + F(lv[0][b] + lv[1][b]));
  return F(s / (2 * (hi - lo + 1)));
}
const bandOf = (lv, b) => F(F(lv[0][b] + lv[1][b]) / 2);

// A roadside thing at depth z (near 2 .. far 16), on one side: lamps, palms, pines.
function roadside(c, lay, route, side, z, hz, y1, glow) {
  const k = 1 / z;
  const y = hz + Math.trunc((y1 - hz) * 1.2 * k) - lay.l(6);
  if (y <= hz + lay.l(2) || y > y1 + lay.l(40)) return;
  const x = lay.cx + d(side * lay.l(330) * (y - hz), y1 - hz); // just outside the grid's fourth line
  const h = Math.trunc(120 * lay.s * k);
  const w = d(h, 12) + 1;
  const dark = rgb(14, 6, 30);
  if (route === COAST) { // palms
    c.fill(x - d(w, 2), y - h, w, h, dark);
    for (let f = 0; f < 5; f++) {
      const a = (f - 2) * 0.55;
      const lean = f < 2 ? -1 : 1;
      c.line(x, y - h, x + Math.trunc(h * 0.45 * Math.sin(a + (PI / 2) * lean * 0.8)), y - h + Math.trunc(h * 0.18 * Math.abs(Math.cos(a))) + d(h, 10), w, dark);
    }
  } else if (route === AURORA) { // pines, with snow on their tips
    for (let r = 0; r < h; r++) {
      const half = (r % (d(h, 3) + 1)) * w + d(r, 4);
      c.fill(x - d(half, 2), y - h + r, half + 1, 1, r < 2 ? rgb(56, 76, 92) : rgb(10, 24, 30));
    }
  } else { // lamps
    const arm = d(side * h, 5);
    c.fill(x - d(w, 2), y - h, w, h, dark);
    c.fill(x - arm, y - h, d(h, 5) + 1, w, dark);
    c.fill(x - arm - w, y - h + w, w * 2 + 1, w + 1, route === CITY ? rgb(120, 255, 255) : rgb(255, 200, 120));
    if (glow) {
      const r2 = w * w * 6;
      for (let gy = -w * 2; gy <= w * 3; gy++) {
        for (let gx = -w * 2; gx <= w * 3; gx++) if (gx * gx + gy * gy < r2) c.add(x - arm + gx, y - h + w + gy, 24, 20, 8);
      }
    }
  }
}

// Stars that twinkle with the music: each listens to one band.
function musicStars(c, v, x, y, w, h, n, seed) {
  if (w <= 0 || h <= 0) return;
  const t = v.theme, lv = v.bands.level;
  // The night sky is dark on every palette: a star is a light on it, never the paper's ink.
  const dim = t.light ? mix(t.black, WHITE, 90) : t.faint;
  const white = t.light ? WHITE : t.fg;
  for (let i = 0; i < n; i++) {
    seed = lcg(seed);
    const sx = x + ((seed >>> 8) % w);
    seed = lcg(seed);
    const sy = y + ((seed >>> 8) % h);
    const b = (seed >>> 16) % BANDS;
    const l = bandOf(lv, b);
    const bright = F(l * 255) > 255 ? 255 : F(60 + F(l * 195));
    const col = mix(dim, (seed >>> 12) % 2 ? t.cyan : white, Math.trunc(bright));
    if (l > F055) {
      c.fill(sx - 1, sy, 3, 1, col);
      c.fill(sx, sy - 1, 1, 3, col);
    } else {
      c.px(sx, sy, col);
    }
  }
}

// The sun, striped in its lower half: one gap per band, wider as it gets louder. rd is its radius
// in rgxx's pixels, which the stripes are measured in.
function stripedSun(c, v, lay, hz, rd) {
  const r = lay.l(rd);
  const lv = v.bands.level, t = v.theme;
  for (let y = -r; y < 0; y++) {
    const h = Math.trunc(Math.sqrt(r * r - y * y));
    const yd = lay.fit.inv(y);
    if (yd > d(-rd, 2) && (-yd) % 12 < d(d(rd, 2) + yd, 8) + Math.trunc(F(bandOf(lv, d(-yd, 12) % BANDS) * 5))) continue;
    c.fill(lay.cx - h, hz + y, 2 * h, 1, mix(t.yel, t.mag, d((y + r) * 255, r)));
  }
}

// The concert's crowd, in front of the stage: heads, and a glow stick above most of them.
function stageCrowd(c, lay, w, hz, clockS, kick) {
  const teal = rgb(57, 214, 200), pink = rgb(232, 48, 140);
  // rgxx's 90 across its 640 pixels, as many again for every 640 more of the canvas
  const left = Math.min(lay.dx(0), 0), span = Math.max(lay.dx(w), 640) - left;
  const people = d(90 * span, 640);
  const up = Math.trunc(F(kick * F(6 * lay.s)));
  const l6 = lay.l(6), thick = lay.fit.px(2);
  for (let i = 0; i < people; i++) {
    const r = Math.imul(i, 2654435761) >>> 0;
    const px0 = lay.x(left + (r % span)), row = (r >>> 12) % 3;
    const py0 = hz + lay.l(8 + row * 7);
    c.disc(px0, py0, lay.l(4 + row), rgb(8, 6, 16));
    if ((r >>> 20) % 5 === 0) continue;
    const ang = 0.5 * Math.sin(clockS * 3 + i) + ((r >>> 8) % 2 ? 0.2 : -0.2);
    const len = lay.l(10 + row * 2);
    const col = (r >>> 16) % 2 ? teal : pink;
    const x1 = px0 + Math.trunc(Math.sin(ang) * len);
    const y1 = py0 - l6 - up - Math.trunc(Math.cos(ang) * len);
    c.line(px0, py0 - l6 - up, x1, y1, thick, col);
    c.add(x1, y1, 60, 60, 60);
  }
}

// The screens inside the scenes show a spectrum in the chosen ramp, or, while that is the palette's,
// in the scene's own colours. A screen gives light of its own on a black backing, whatever the
// palette: always the dark ramp.
function screenColour(v, f, own) {
  const r = rampAt(v.cfg.ramp);
  if (r === RAMP.PALETTE) return own;
  const pal = palOf(v.theme, v.cfg.duo1 ?? 32, v.cfg.duo2 ?? 5);
  pal.light = false;
  return rampColour(pal, r, 1, f);
}

// 天氣隨音樂: what the music does to the weather layer, worked out once a frame. With 關 every field
// is the constant the layer was written with.
const AMP_RAIN = F(0.58);
const AMP_GENTLE = F(0.4);
const HEAR_SPAN = F(0.2);
function hearAmp(cfg, soft) {
  if (cfg.weather_react === WEATHER_REACT.STRONG) return 1;
  if (cfg.weather_react === WEATHER_REACT.SOFT) return soft;
  return 0;
}
function hearSwing(slow) {
  const s = F(F(slow - 0.5) / HEAR_SPAN);
  return s < -1 ? -1 : s > 1 ? 1 : s;
}
function weatherHear(cfg, r, kick, seed) {
  const w = { n: 300, lr: 60, lg: 70, lb: 95, gust: 0, glint: 0, grow: false, fogA: 150 };
  const amp = hearAmp(cfg, AMP_RAIN), gentle = hearAmp(cfg, AMP_GENTLE);
  if (amp <= 0) return w;
  const s = hearSwing(r.slow);
  // quiet passages rain lightly, a chorus pours, and the kick throws up to 120 more
  const swing = s > 0 ? F(F(0.4) * s) : F(F(0.6) * s);
  const count = F(F(300 * F(1 + F(amp * swing))) + F(F(amp * 120) * kick));
  w.n = Math.min(Math.max(Math.trunc(count), 120), 420);
  // the streaks' light rises with the kick, like lightning caught in them: never the sky
  const flare = F(F(amp * F(0.55)) * kick);
  w.lr += Math.trunc(F(60 * flare));
  w.lg += Math.trunc(F(70 * flare));
  w.lb += Math.trunc(F(95 * flare));
  // one push for the whole layer, its direction this song's, so it never flip-flops
  w.gust = Math.trunc(F(F(gentle * 14) * r.gust)) * ((seed >>> 3) & 1 ? 1 : -1);
  w.glint = Math.trunc(F(F(255 * amp) * r.glint));
  w.grow = cfg.weather_react === WEATHER_REACT.STRONG && r.glint > F06;
  w.fogA = 150 + Math.trunc(F(F(F(150 * gentle) * F(0.625)) * s));
  return w;
}

// The weather as a layer over the finished scene: particles that depend on their number, the
// scene's clock (a paused song stops the rain too) and what the music is doing. rgxx's counts are
// for its 640 x 404 page; a bigger canvas gets as many more as its area holds.
function weatherLayer(c, lay, kind, clockS, y0, hz, y1, w) {
  const width = c.width, h = y1 - y0;
  if (h <= 0 || width <= 0) return;
  const more = (n) => (width === 640 && h === 404 ? n : Math.trunc((n * width * h) / (640 * 404)));
  if (kind === WEATHER.RAIN) {
    const n = more(w.n);
    for (let i = 0; i < n; i++) {
      const r = Math.imul(i + 1, 2654435761) >>> 0;
      const len = 6 + (r >>> 28);
      const x = r % (width + 40);
      const fall = u32(clockS * ((420 + ((r >>> 24) & 63)) * lay.s));
      const y = y0 + (((((r >>> 8) % h) + fall) | 0) % h);
      for (let k = 0; k < len && y + k < y1; k++) c.add(x - d(k, 3), y + k, w.lr, w.lg, w.lb); // slanting
    }
  } else if (kind === WEATHER.SNOW) {
    const flake = rgb(240, 244, 255);
    // how white the big flakes are just now: one mix for the layer, not one per flake
    const glinting = w.glint !== 0 ? mix(flake, WHITE, w.glint) : flake;
    const gust = lay.l(w.gust);
    const n = more(260);
    for (let i = 0; i < n; i++) {
      const r = Math.imul(i + 1, 2654435761) >>> 0;
      const big = ((r >>> 27) & 3) === 0 ? 1 : 0;
      const fall = u32(clockS * ((28 + ((r >>> 24) & 31)) * lay.s));
      const y = y0 + (((((r >>> 8) % h) + fall) | 0) % h);
      let x = (r % width) + Math.trunc(6 * lay.s * Math.sin(clockS * 0.9 + i));
      const lit = big !== 0 && w.glint !== 0; // only the big ones catch the light
      const grow = big !== 0 && w.grow ? 1 : 0;
      x += big !== 0 ? d(gust * 3, 5) : gust; // and the big ones are pushed less
      if (y + big < y1) {
        const s = lay.fit.px(1 + big + grow);
        c.fill(x, y, s, s, lit ? glinting : flake);
      }
    }
  } else if (kind === WEATHER.FOG) {
    // a band of mist on the horizon, thickest on it
    const reach = lay.l(50);
    for (let y = hz - reach; y < hz + lay.l(70) && y < y1; y++) {
      const dd = y < hz ? hz - y : d((y - hz) * 5, 7);
      const a = reach > 0 ? d(w.fogA * (reach - dd), reach) : 0;
      if (a <= 0 || y < y0) continue;
      const row = c.rowMut(y);
      if (!row) continue;
      for (let i = 0; i < row.length; i++) {
        const p = row[i];
        let r = p & 255, g = (p >>> 8) & 255, b = (p >>> 16) & 255;
        r += ((120 - r) * a) >> 8;
        g += ((130 - g) * a) >> 8;
        b += ((160 - b) * a) >> 8;
        row[i] = rgb(r, g, b);
      }
    }
  }
}

// The hour of the sky: the owner's, or the local clock's (night without one).
function envHour(cfg, v) {
  if (cfg.hour !== HOUR.AUTO) return cfg.hour;
  return v.minuteOfDay === null || v.minuteOfDay === undefined ? HOUR.NIGHT : hourAt(v.minuteOfDay);
}
// The weather: the owner's or the station's; there is no forecast to follow.
function envWeather(cfg) {
  const w = cfg.weather;
  return w === WEATHER.RAIN || w === WEATHER.SNOW || w === WEATHER.FOG ? w : WEATHER.CLEAR;
}

// The drive and what it remembers: how far the road has run, the scene's clock (both stand still
// while paused), the kick and its fall, the shooting stars, the lighthouse's beam, and what the
// weather hears. view = { bands, cfg (resolved), theme, dt, moving, seed, posMs, minuteOfDay }.
export class DrivePainter {
  constructor() {
    this.road = 0;
    this.clockS = 0;
    this.kick = 0;
    this.kickAcc = { v: 0 };
    this.react = new React();
    this.meteorList = Array.from({ length: 4 }, () => ({ x: 0, y: 0, t: 0 }));
    this.lastBass = 0;
    this.beam = 0;
  }

  // Shooting stars on the kicks, somewhere in the upper sky.
  meteors(c, v, lay, bass, y0, h, sky, salt) {
    if (v.moving && bass > 0.5 && this.lastBass <= 0.5) {
      const m = this.meteorList.find((q) => q.t <= 0);
      if (m) {
        const r = Math.imul(((salt >>> 0) + (Math.trunc(v.posMs / 97) >>> 0)) >>> 0, 2654435761) >>> 0;
        m.x = F(lay.x(40 + ((r >>> 8) % 480)));
        const span = h > 16 ? h - 16 : 1;
        m.y = F(((((y0 + 8) >>> 0) + ((r >>> 20) % span)) >>> 0));
        m.t = 1;
      }
    }
    this.lastBass = bass;
    const run = F(180 * lay.s), tail = Math.max(lay.l(46), 2);
    const head = v.theme.light ? WHITE : v.theme.fg;
    for (const m of this.meteorList) {
      if (!(m.t > 0)) continue;
      const dd = F(F(1 - m.t) * run);
      for (let k = 0; k < tail; k += 2) {
        c.px(Math.trunc(F(F(m.x + dd) - k)), Math.trunc(F(m.y + F(F(dd - k) * F04))), mix(head, sky, d(k * 255, tail)));
      }
      m.t = F(m.t - F(v.dt * 1.5));
    }
  }

  draw(c, v) {
    const y0 = c.y0, y1 = c.y1, width = c.width;
    const t = v.theme, cfg = v.cfg, lv = v.bands.level;
    const dt = F(v.dt > 0 ? v.dt : 0);
    const vv = { ...v, dt };
    const moving = !!v.moving;
    const bass = loudness(lv, 0, 3);
    // Up with the bass at once, then a tenth away every 1/30 s. As in rgxx, the fall's steps are
    // only counted on a frame where the bass does not rise.
    this.kick = bass > this.kick ? bass : F(this.kick * decay(F09, steps(this.kickAcc, dt, STEP_S, STEP_CAP)));
    const loud = loudness(lv, 0, 15);
    if (moving) {
      this.road += dt * (0.8 + 2.5 * loud);
      this.clockS += dt;
    }
    // the bass itself, not kick: the gust wants the onset, and kick is a peak-hold
    this.react.step(dt, moving, bass, loud, loudness(lv, 12, 15));
    const route = Math.min(Math.max((cfg.route | 0) - 1, 0), 4);
    const kick = this.kick;
    // rgxx's page is 640 x 404; the scene fits the rows it is given, and a canvas taller than that
    // shape puts its extra rows into the sky
    const ph = y1 - y0;
    const fit = Fit.min(Fit.new(width, 640), Fit.new(ph, 404));
    const fh = fit.i(404);
    const hz = y0 + ph - (fh - d(fh * 16, 25));
    const sky = hz - y0;
    if (sky <= 0 || width <= 0) return; // too few rows for a sky: nothing to draw
    const lay = new Lay(fit, d(width, 2), hz - d(fh * 16, 25), d(fh * 16, 25));
    const cx = lay.cx, l = (n) => lay.l(n);
    const band = (b) => bandOf(lv, b);

    // the sky
    let top, bottom;
    if (route === CITY) { top = rgb(4, 2, 16); bottom = rgb(46, 12, 80); }
    else if (route === COAST) { top = rgb(3, 5, 20); bottom = rgb(28, 40, 92); }
    else if (route === AURORA) { top = rgb(2, 4, 14); bottom = rgb(10, 26, 46); }
    else if (route === STAGE) { top = rgb(4, 2, 12); bottom = rgb(34, 10, 54); }
    else { top = rgb(8, 4, 24); bottom = rgb(130, 24, 110); }
    // the hour is a tint of the sky's rows, the weather dims them; the routes' art is not redrawn
    const weather = envWeather(cfg);
    const carOn = cfg.car === CAR.ON;
    const tint = hourTint(envHour(cfg, v));
    if (tint.amount !== 0) {
      const tt = tint.top, th = tint.horizon;
      top = mix(top, rgb(tt[0], tt[1], tt[2]), tint.amount);
      bottom = mix(bottom, rgb(th[0], th[1], th[2]), tint.amount);
    }
    if (weather === WEATHER.RAIN || weather === WEATHER.FOG) {
      top = mix(top, BLACK, 64);
      bottom = mix(bottom, rgb(40, 44, 60), 64);
    }
    for (let y = y0; y < hz; y++) c.fill(0, y, width, 1, mix(top, bottom, d((y - y0) * 255, sky)));
    const wideSky = route === AURORA || route === COAST;
    const starRows = wideSky ? d(sky * 3, 4) : d(sky, 2), starBase = wideSky ? 110 : 50;
    const clearSky = weather === WEATHER.CLEAR ? 2 : 1;
    const stars = d(d(d(starBase * tint.stars, 100) * clearSky, 2) * width, 640);
    musicStars(c, vv, 0, y0, width, starRows, stars, 5 + route);
    if (route === SUNSET) this.meteors(c, vv, lay, bass, y0, d(sky, 3), top, v.seed >>> 0);
    let grid = t.mag;
    if (route === SUNSET) {
      // the sun, big in the middle; the spectrum as an aurora behind two ranges
      const step = Math.max(l(4), 1), bar = Math.max(l(3), 1);
      for (let x = 0; x < width; x += step) {
        const b = Math.min(Math.max(d(lay.dx(x) * BANDS, 640), 0), BANDS - 1);
        const gh = l(Math.trunc(F(band(b) * 80)));
        for (let k = 0; k < gh; k += 2) c.fill(x, hz - l(30) - k, bar, 1, mix(t.mag, top, 60 + d(lay.fit.inv(k) * 190, 80)));
      }
      stripedSun(c, vv, lay, hz, 96 + Math.trunc(F(kick * 16)));
      for (let x = 0; x < width; x += 2) {
        const fx = lay.dxf(x) + this.road * 6;
        let m = Math.trunc(26 + 12 * Math.sin(fx * 0.013) + 7 * Math.sin(fx * 0.041 + 2));
        const dx = lay.dx(x);
        if (dx > 200 && dx < 440) m = d(m, 2); // the valley the sun sets into
        c.fill(x, hz - l(m), 2, l(m), rgb(24, 8, 44));
      }
      for (let x = 0; x < width; x += 2) {
        const fx = lay.dxf(x) + this.road * 16;
        let m = Math.trunc(18 + 14 * Math.sin(fx * 0.021) + 8 * Math.sin(fx * 0.057 + 1));
        const dx = lay.dx(x);
        if (dx > 240 && dx < 400) m = d(m, 3);
        c.fill(x, hz - l(m), 2, l(m), rgb(36, 12, 60));
      }
    } else if (route === CITY) {
      // the moon up right; a neon tower on the left with the spectrum on it
      const my = lay.top + d(lay.sky, 4);
      c.disc(lay.x(500), my, l(30), mix(bottom, t.moon, 40));
      c.disc(lay.x(500), my, l(24), t.moon);
      c.disc(lay.x(508), my - l(6), l(5), mix(t.moon, BLACK, 30));
      // the skyline: rgxx's from its left edge rightwards, and on a wider canvas more to the left
      // of it, from a sequence of its own
      const l0 = lv[0], win = Math.max(l(3), 1);
      const block = (rs, x) => {
        rs.r = lcg(rs.r);
        const bw = 24 + ((rs.r >>> 16) % 40);
        let bh = 40 + ((rs.r >>> 8) % 90);
        if (x > 260 && x < 380) bh = d(bh, 2);
        c.fill(lay.x(x), hz - l(bh), lay.x(x + bw - 2) - lay.x(x), l(bh), rgb(16, 8, 36));
        for (let up = bh - 6; up > 4; up -= 7) { // a row of windows, as far above the horizon
          for (let wx = x + 4; wx < x + bw - 6; wx += 6) {
            rs.r = lcg(rs.r);
            const lit = (rs.r >>> 20) % 5 === 0;
            const flick = (rs.r >>> 12) % 16 < Math.trunc(F(l0[(rs.r >>> 8) % 16] * 12));
            if (lit || flick) c.fill(lay.x(wx), hz - l(up), win, win, flick ? t.cyan : rgb(255, 190, 90));
          }
        }
        return bw;
      };
      let rs = { r: 12345 };
      for (let x = 0; lay.x(x) < width;) x += block(rs, x);
      rs = { r: 54321 };
      for (let x = 0; lay.x(x) > 0;) {
        const probe = lcg(rs.r);
        x -= 24 + ((probe >>> 16) % 40);
        block(rs, x);
      }
      // the tower: its tip and the kick's halo end at the scene's top row
      const tx = lay.x(150), tw = l(56), th = lay.sky - l(58);
      c.fill(tx, hz - th, tw, th, rgb(20, 10, 44));
      c.fill(tx + d(tw, 2) - l(1), hz - th - l(40), l(3), l(40), rgb(40, 30, 70));
      const tip = kick > F045 ? t.warn : mix(t.warn, BLACK, 150);
      c.fill(tx + d(tw, 2) - l(3), hz - th - l(44), l(7), l(5), tip);
      if (kick > F045) {
        const reach = l(12), num = lay.fit.i(1000), den = 1000;
        const lim = 140 * num * num;
        for (let gy = -reach; gy <= reach; gy++) {
          for (let gx = -reach; gx <= reach; gx++) {
            // inside 140 of rgxx's pixels squared
            if ((gx * gx + gy * gy) * den * den < lim) c.add(tx + d(tw, 2) + gx, hz - th - l(42) + gy, 40, 6, 10);
          }
        }
      }
      for (let b = 0; b < BANDS; b++) { // the billboard: sixteen neon bars
        const bx = tx + l(4 + b * 3), bh = l(6 + Math.trunc(F(band(b) * 70)));
        const own = mix(t.cyan, t.mag, d(b * 255, BANDS));
        c.fill(bx, hz - th + l(90) - bh, Math.max(l(2), 1), bh, screenColour(vv, F(b / (BANDS - 1)), own));
      }
      c.frame(tx + l(2), hz - th + l(14), tw - l(4), l(80), t.mag);
      for (let wy = 104; hz - th + l(wy) < hz - l(6); wy += 9) {
        c.fill(tx + l(8), hz - th + l(wy), tw - l(16), Math.max(l(2), 1), mix(t.cyan, BLACK, 120));
      }
      grid = t.cyan;
    } else if (route === COAST) {
      // a moonlit sea: the moon up left, its road of light on the water (one band a ripple), a
      // lighthouse sweeping on the right
      const mx = lay.x(220), my = lay.top + d(lay.sky, 3);
      const reach = l(60);
      for (let gy = -reach; gy <= reach; gy++) { // the moon's halo, measured in rgxx's pixels
        const dy = lay.fit.inv(gy);
        for (let gx = -reach; gx <= reach; gx += 2) {
          const dx = lay.fit.inv(gx);
          const d2 = dx * dx + dy * dy;
          if (d2 < 3600 && d2 > 900) {
            const f = d(3600 - d2, 200);
            c.add(mx + gx, my + gy, d(f, 2), d(f, 2), f);
            c.add(mx + gx + 1, my + gy, d(f, 2), d(f, 2), f);
          }
        }
      }
      c.disc(mx, my, l(28), t.moon);
      c.disc(mx - l(8), my - l(6), l(6), mix(t.moon, BLACK, 25));
      c.disc(mx + l(9), my + l(8), l(4), mix(t.moon, BLACK, 25));
      for (let k = 0; k < 20; k++) {
        const y = hz - l(40 - 2 * k), yn = hz - l(38 - 2 * k);
        c.fill(0, y, width, yn - y, mix(rgb(8, 12, 40), rgb(20, 30, 70), d(k * 255, 20)));
        const b = band(k % BANDS);
        const w = l(16 + (40 - 2 * k) * 3 + Math.trunc(F(b * 120)));
        c.fill(mx - d(w, 2) + Math.trunc(6 * lay.s * Math.sin(this.road * 3 + y)), y, w, 1, mix(rgb(150, 160, 200), t.fg, Math.trunc(F(b * 255))));
      }
      const lx = lay.x(548), ly = hz - l(40);
      for (let y = ly - l(6); y < hz; y++) c.fill(lx - l(44) + (y - ly) * 2, y, l(150), 1, rgb(14, 6, 30)); // the point
      c.fill(lx - l(5), ly - l(52), l(11), l(46), rgb(230, 220, 230));
      c.fill(lx - l(5), ly - l(40), l(11), l(6), rgb(255, 60, 90));
      c.fill(lx - l(5), ly - l(24), l(11), l(6), rgb(255, 60, 90));
      c.fill(lx - l(7), ly - l(60), l(15), l(8), rgb(40, 20, 60));
      this.beam += dt * (moving ? 0.9 + kick * 2.5 : 0);
      // the beam: a few rays of light added to the sky, fading with distance, brightest when it
      // sweeps across towards us
      const a = this.beam;
      const facing = Math.abs(Math.sin(a));
      const bright = 40 + Math.trunc(facing * 70 + kick * 40);
      const lamp = ly - l(57);
      const reachB = l(330), from = l(6);
      for (let r = -3; r <= 3; r++) {
        const ang = a + r * 0.025;
        const dx = Math.cos(ang), dy = Math.sin(ang) * 0.12; // flattened: we see it from the side
        const side = 4 - Math.abs(r);
        for (let dd = from; dd < reachB; dd++) {
          const f = d(d(bright * (reachB - dd), reachB) * side, 4);
          if (f > 2) c.add(lx + Math.trunc(dx * dd), lamp + Math.trunc(dy * dd), f, d(f * 9, 10), d(f * 6, 10));
        }
      }
      const glow = l(6);
      for (let gy = -glow; gy <= glow; gy++) {
        const dy = lay.fit.inv(gy);
        for (let gx = -glow; gx <= glow; gx++) {
          const dx = lay.fit.inv(gx);
          if (dx * dx + dy * dy < 40) c.add(lx + gx, lamp + gy, 90, 80, 50);
        }
      }
    } else if (route === AURORA) {
      // curtains across the sky, each column one band; snow on the mountains
      const tt = this.clockS * 0.35;
      const turn = Math.trunc(tt * 3);
      for (let x = 0; x < width; x += 2) {
        const fx = lay.dxf(x);
        const wave = 18 * Math.sin(fx * 0.011 + tt) + 10 * Math.sin(fx * 0.027 - tt * 1.7);
        const base = lay.top + d(lay.sky, 2) + Math.trunc(wave * lay.s);
        const bi = (((d(lay.dx(x) * BANDS, 640) + turn) % BANDS) + BANDS) % BANDS;
        const h = Math.max(l(30 + Math.trunc(F(band(bi) * 110))), 1);
        for (let k = 0; k < h; k++) {
          let f = d((h - k) * 120, h); // bright at the foot, fading upwards
          if (k < 6) f = d(f * (k + 1), 7); // soft at the very foot
          const fk = d(f * k, h + 1);
          const g = f, r = d(fk * 2, 3), b = d(f, 2) + d(fk, 2); // green, purple above
          c.add(x, base - k, r, g, b);
          c.add(x + 1, base - k, d(r, 2), d(g, 2), d(b, 2));
        }
      }
      for (let x = 0; x < width; x += 2) {
        const fx = lay.dxf(x) + this.road * 5;
        const a = fx / 190 - Math.floor(fx / 190);
        const b = fx / 53 + 0.3 - Math.floor(fx / 53 + 0.3);
        const t1 = 1 - Math.abs(a * 2 - 1), t2 = 1 - Math.abs(b * 2 - 1); // sharp ridges
        const m = Math.trunc(18 + 62 * t1 * t1 + 12 * t2);
        c.fill(x, hz - l(m), 2, l(m), rgb(18, 26, 40));
        if (m > 56) c.fill(x, hz - l(m), 2, l(d(m - 56, 2) + 1), rgb(74, 96, 122)); // snow on the high ones
      }
      grid = rgb(80, 200, 220);
    } else {
      // a stage on the horizon: lights sweeping from the truss, the spectrum on an LED wall, a
      // singer with long twin tails; glow sticks all over the crowd
      const teal = rgb(57, 214, 200), pink = rgb(232, 48, 140);
      const sx0 = lay.x(150), sx1 = lay.x(490), floorY = hz - l(18), tr = lay.top + d(lay.sky, 5);
      // the beams first, under everything: six lights, pairs crossing, brighter on the kick
      const reach = l(420), from = l(4), stop = hz + l(30), down = tr + l(8);
      const bright = 26 + Math.trunc(F(kick * 40));
      for (let li = 0; li < 6; li++) {
        const lx = sx0 + l(20) + d(li * (sx1 - sx0 - l(40)), 5);
        const sign = li % 2 ? 1 : -1;
        const a = PI / 2 + 0.55 * Math.sin(this.clockS * (0.7 + 0.13 * li) + li * 1.3) * sign;
        const cr = li % 3 === 0 ? 57 : li % 3 === 1 ? 232 : 150;
        const cg = li % 3 === 0 ? 214 : li % 3 === 1 ? 48 : 80;
        const cb = li % 3 === 0 ? 200 : li % 3 === 1 ? 140 : 255;
        const ca = Math.cos(a), sa = Math.sin(a);
        for (let dd = from; dd < reach; dd++) {
          const half = d(dd, 14); // a cone
          const bx = lx + Math.trunc(ca * dd);
          const by = down + Math.trunc(sa * dd);
          if (by > stop) break;
          const f = d(bright * (reach - dd), reach);
          const ar = d(cr * f, 255), ag = d(cg * f, 255), ab = d(cb * f, 255);
          for (let w = -half; w <= half; w += 2) c.add(bx + w, by, ar, ag, ab);
        }
      }
      // the truss
      c.fill(sx0 - l(10), tr, sx1 - sx0 + l(20), l(4), rgb(60, 60, 80));
      for (let x = sx0 - l(10); x < sx1 + l(10); x += Math.max(l(8), 1)) c.line(x, tr, x + l(8), tr + l(4), 1, rgb(40, 40, 56));
      c.fill(sx0 - l(10), tr, l(4), floorY - tr, rgb(50, 50, 70));
      c.fill(sx1 + l(6), tr, l(4), floorY - tr, rgb(50, 50, 70));
      for (let li = 0; li < 6; li++) {
        c.fill(sx0 + l(16) + d(li * (sx1 - sx0 - l(40)), 5), tr + l(4), l(9), l(6), kick > F045 ? t.fg : rgb(120, 120, 140));
      }
      // the LED wall: sixteen bars in her colours
      const wx = lay.x(210), ww = l(220), wy = tr + l(20);
      const wh = floorY - l(62) - wy;
      const whn = wh !== 0 ? wh : 1;
      const step = Math.max(l(3), 1), seg = Math.max(l(2), 1);
      c.fill(wx - l(3), wy - l(3), ww + l(6), wh + l(6), rgb(10, 8, 20));
      const barW = d(ww - l(4), BANDS) - l(2);
      for (let b = 0; b < BANDS; b++) {
        const bh = Math.trunc(F(band(b) * wh));
        const bx = wx + l(2) + d(b * (ww - l(4)), BANDS);
        for (let yy = 0; yy + seg <= bh; yy += step) {
          const own = mix(teal, pink, d(yy * 255, whn));
          c.fill(bx, wy + wh - yy - seg, barW, seg, screenColour(vv, F(yy / whn), own));
        }
      }
      // the stage and the singer, swaying with the beat
      c.fill(sx0 - l(20), floorY, sx1 - sx0 + l(40), l(18), rgb(24, 18, 40));
      c.fill(sx0 - l(20), floorY, sx1 - sx0 + l(40), l(2), mix(teal, BLACK, 60));
      const sway = Math.trunc(3 * lay.s * Math.sin(this.clockS * 4));
      const hop = Math.trunc(F(kick * F(3 * lay.s)));
      const cx0 = cx + sway, fy = floorY - hop;
      for (const side of [-1, 1]) { // the twin tails, swinging
        for (let k = 0; k < 34; k++) {
          const tx = cx0 + side * l(6 + d(k, 5)) + Math.trunc(side * 2 * lay.s * Math.sin(this.clockS * 5 + k * 0.2));
          c.fill(tx - l(1), fy - l(52 - k), l(3), Math.max(l(1), 1), teal);
        }
      }
      const body = rgb(20, 16, 30);
      c.disc(cx0, fy - l(50), l(7), body); // the head
      c.fill(cx0 - l(9), fy - l(55), l(3), l(3), pink); // the ties
      c.fill(cx0 + l(7), fy - l(55), l(3), l(3), pink);
      c.fill(cx0 - l(6), fy - l(42), l(13), l(18), body);
      c.fill(cx0 - l(5), fy - l(24), l(4), l(24), body);
      c.fill(cx0 + l(2), fy - l(24), l(4), l(24), body);
      c.line(cx0 + l(6), fy - l(38), cx0 + l(14), fy - l(48) - hop * 3, l(3), body); // an arm up, higher on the kick
      grid = pink;
    }
    c.fill(0, hz, width, y1 - hz, route === AURORA ? rgb(6, 14, 22) : rgb(10, 3, 24));
    // the grid, running towards us
    const fr = this.road - Math.floor(this.road);
    for (let i = 0; i < 14; i++) {
      const z = i + 1 - fr;
      const y = hz + Math.trunc(((y1 - hz) * 1.2) / z) - l(6);
      if (y > hz && y < y1) c.fill(0, y, width, 1, mix(grid, t.bg, 60 + Math.trunc(z * 10)));
    }
    const rule = mix(grid, t.bg, 80);
    // rgxx's 25 lines reach its edges at the bottom row; a wider canvas has more of them
    const spread = 80 * lay.s;
    const lines = Math.max(12, Math.trunc(width / 2 / spread) + 1);
    const span = y1 - hz;
    for (let i = -lines; i <= lines; i++) {
      for (let y = hz + 2; y < y1; y += 2) c.px(cx + Math.trunc((((i * 50 * (y - hz)) / span) * 1.6) * lay.s), y, rule);
    }
    if (route === STAGE) stageCrowd(c, lay, width, hz, this.clockS, kick);
    // the roadside, far ones first
    const sfr = this.road * 0.5 - Math.floor(this.road * 0.5);
    for (let i = 7; i >= 0; i--) {
      const z = (i + 1 - sfr) * 2;
      roadside(c, lay, route, -1, z, hz, y1, route === CITY);
      roadside(c, lay, route, 1, z, hz, y1, route === CITY);
    }
    // the car, from behind, bouncing a little; the tail lights flare on the kick
    const by = y1 - l(56) + (moving ? Math.trunc(2 * lay.s * Math.sin(this.road * 12)) : 0);
    c.fill(cx - l(50), by, l(100), l(24), rgb(30, 20, 50));
    c.fill(cx - l(42), by - l(18), l(84), l(20), rgb(50, 30, 80));
    const glass = carOn ? (route === CITY ? t.mag : t.cyan) : route === CITY ? rgb(200, 60, 200) : rgb(20, 180, 220);
    c.fill(cx - l(35), by - l(15), l(70), l(11), glass);
    const kk = Math.min(Math.trunc(F(kick * 255)), 255);
    const tail = carOn ? mix(mix(t.mag, BLACK, 110), t.mag, kk) : mix(rgb(160, 20, 50), rgb(255, 90, 120), kk);
    if (kick > F045) {
      for (let gy = -l(6); gy <= l(14); gy++) {
        for (let gx = -l(8); gx <= l(30); gx++) {
          c.add(cx - l(48) + gx, by + l(4) + gy, 30, 0, 8);
          c.add(cx + l(26) + gx, by + l(4) + gy, 30, 0, 8);
        }
      }
    }
    c.fill(cx - l(48), by + l(6), l(22), l(7), tail);
    c.fill(cx + l(26), by + l(6), l(22), l(7), tail);
    c.fill(cx - l(58), by + l(20), l(22), l(14), rgb(8, 6, 12));
    c.fill(cx + l(36), by + l(20), l(22), l(14), rgb(8, 6, 12));
    const hear = weatherHear(cfg, this.react, kick, v.seed >>> 0);
    weatherLayer(c, lay, weather, this.clockS, y0, hz, y1, hear);
    // No words on the road (chipchill, the owner 2026-10-06): the page is the picture.
  }
}

// ---- the plugin ---------------------------------------------------------------------------------
const option = (k, i) => ({ value: i, labelKey: valueLabelKey(k, i) });
const selectOf = (k) => ({ key: k, type: 'select', labelKey: keyDef(k).labelKey, options: Array.from({ length: keyDef(k).n }, (_, i) => option(k, i)) });
// The controls in the order of chipchill's menu rows (cfg.rs page_keys).
const SCHEMA = Object.freeze(DRIVE.keys.map(selectOf));
const PRESETS = Object.freeze(Array.from({ length: DRIVE.presetCount() }, (_, i) => Object.freeze({ id: i, labelKey: DRIVE.presetLabelKey(i), cfg: Object.freeze(DRIVE.applyPreset(DRIVE.plain(), i)) })));
const CALM = Object.freeze({ weather_react: WEATHER_REACT.OFF });
export { ROUTE, HOUR_KEY };

// create({ canvas, ctx2d, theme, seed, clock }) -> instance (design amendment A/D). clock() gives the
// local Date for 跟著時鐘 (injected; new Date() by default). The plugin draws on the host's pixel
// canvas (frame's `pixels`); it keeps no timers, fetches nothing and touches no DOM.
export function create(args = {}) {
  const clock = typeof args.clock === 'function' ? args.clock : () => new Date();
  const painter = new DrivePainter();
  const analyser = new BandAnalyser();
  const bands = createBands();
  let seed = args.seed >>> 0;
  let playedS = 0;
  let state = 'no-data'; // until audio data is seen
  let alive = true;
  const groups = Object.freeze(Array.from({ length: DRIVE.groupCount() }, (_, g) => Object.freeze({
    id: DRIVE.groupId(g),
    labelKey: DRIVE.groupLabelKey(g),
    count: DRIVE.tourCount(g),
    pos: (cfg) => DRIVE.tourPos(cfg, g),
    step: (cfg, shown) => DRIVE.tourStep(cfg, shown, g),
  })));
  return {
    needs,
    schema: SCHEMA,
    presets: PRESETS,
    defaults: () => DRIVE.defaults(),
    validate: (obj) => DRIVE.validate(obj),
    applyPreset: (cfg, id) => DRIVE.applyPreset(cfg, id),
    presetOf: (cfg) => DRIVE.preset(cfg),
    resolveFollow: (cfg, s) => DRIVE.resolve(cfg, s),
    groups: () => groups,
    resize() {},
    // audio: { freq: { l, r }, rate, bins } or null; cfg: the resolved choices; playing: the
    // scene's clock and road run only while the player plays.
    frame({ dt, audio, theme, cfg, playing, pixels, seed: s }) {
      if (!alive) return;
      if (s !== undefined) seed = s >>> 0;
      dt = F(Number.isFinite(dt) && dt > 0 ? dt : 0);
      const freq = audio?.freq;
      const live = !!freq?.l;
      state = live ? 'ok' : 'no-data';
      if (live) analyser.run(bands, freq, audio.rate || 48000, (audio.bins || 1024) * 2, dt);
      else resetBands(bands); // silence: nothing to react to, and nothing pretends to
      const moving = !!playing;
      if (moving) playedS += dt;
      const shown = live ? cfg : { ...cfg, ...CALM };
      const minute = shown.hour === HOUR.AUTO ? minuteOfDay((() => { try { return clock(); } catch { return null; } })()) : null;
      painter.draw(pixels, { bands, cfg: shown, theme, dt, moving, seed, posMs: Math.trunc(playedS * 1000), minuteOfDay: minute });
    },
    status: () => (alive ? state : 'off'),
    dispose() { alive = false; },
  };
}
