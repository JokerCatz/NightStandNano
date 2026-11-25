// SPDX-License-Identifier: LGPL-2.1-or-later
// What the owner chose for the visuals: every choice is a key with a few values, kept by INDEX,
// every list ending in 隨電台 (follow the station; chipchill's 隨歌曲) where the key has one.
// Ported from chipchill crates/chipchill-viz/src/cfg.rs (rgxx viz_cfg.c): the key table, the
// follow-the-station resolver (FNV-1a seed + salted finisher), presets, the per-combination rows
// and the two tours per page (odometers over the plain values with one more stop where every key of
// the group follows the station).
//
// A page's cfg is a plain object { key: index } holding only that page's keys. Everything here is
// pure: functions return new objects and never change what they are given. A page (definePage)
// declares its keys, its rows, its presets and its groups; later plugins add their own page here
// (and their keys to KEYS) without touching the host.

// Every key there is, in chipchill's order: a key's place + 1 is the salt its follow-the-station
// roll uses, so the same station gets the same look as in chipchill/rgxx.
export const KEY_ORDER = Object.freeze([
  'ramp', 'duo1', 'duo2', 'shape', 'map', 'peaks', 'reflect', 'wf_layout', 'wf_dir', 'sc_chan', 'sc_draw',
  'sc_decay', 'xy_draw', 'xy_layout', 'route', 'hour', 'weather', 'weather_react', 'car', 'tape_shell',
  'tape_label', 'tape_alpha', 'mini_style', 'mini_color', 'voice_view', 'cover_over', 'voice_scope',
]);

export const SHAPE = Object.freeze({ SEG: 0, SOLID: 1, LINE: 2, RADIAL: 3, SONG: 4 });
export const MAPPING = Object.freeze({ BOTTOM: 0, TIP: 1, BAND: 2, LEVEL: 3, SONG: 4 });
export const RAMPS = Object.freeze({ PALETTE: 0, FIRE: 1, ICE: 2, WINAMP: 3, RAINBOW: 4, DUO: 5, MONO: 6, SONG: 7 });
export const OFF = Object.freeze({ NO: 0, YES: 1, SONG: 2 });
// 瀑布 (chipchill cfg.rs wf_layout / wf_dir).
export const WF_LAYOUT = Object.freeze({ SPLIT: 0, SUM: 1, STACKED: 2, SONG: 3 });
export const WF_DIR = Object.freeze({ DOWN: 0, UP: 1, ACROSS: 2, SONG: 3 });
// 波形 (chipchill cfg.rs sc_chan / sc_draw; 殘影 sc_decay is a two-way key, OFF).
export const SC_CHAN = Object.freeze({ OVERLAY: 0, STACKED: 1, MIDSIDE: 2, SONG: 3 });
export const SC_DRAW = Object.freeze({ LINE: 0, FILL: 1, DOTS: 2, SONG: 3 });
// 相位 (chipchill cfg.rs xy_draw / xy_layout).
export const XY_DRAW = Object.freeze({ DOTS: 0, LINES: 1, HEAT: 2, SONG: 3 });
export const XY_LAYOUT = Object.freeze({ PLAIN: 0, METERS: 1, SONG: 2 });
// 夜行 (chipchill cfg.rs hour / weather / car / weather_react / route). The hour's AUTO is the local
// clock (跟著時鐘). The weather has no AUTO: chipchill's 跟著預報 needs a forecast and a location,
// which tunetap never asks for, so its 隨電台 is index 4 (chipchill's 隨歌曲 is 5); the four plain
// values, their weights and so every station's roll are chipchill's.
export const ROUTE = Object.freeze({ SONG: 0, SUNSET: 1, CITY: 2, COAST: 3, AURORA: 4, STAGE: 5 });
export const HOUR_KEY = Object.freeze({ NIGHT: 0, DUSK: 1, DAWN: 2, DAY: 3, AUTO: 4, SONG: 5 });
export const WEATHER_KEY = Object.freeze({ CLEAR: 0, RAIN: 1, SNOW: 2, FOG: 3, SONG: 4 });
export const CAR = Object.freeze({ OFF: 0, ON: 1, SONG: 2 });
export const WEATHER_REACT = Object.freeze({ OFF: 0, SOFT: 1, STRONG: 2 });
// 錄音帶 (chipchill cfg.rs tape_shell / tape_label / tape_alpha). The label has no 角色 (chipchill's
// CHARACTER, the 初音 label): character art is out of tunetap's scope, so its 隨電台 is index 2
// (chipchill's 隨歌曲 is 3) and a station's label is rolled over the two labels there are (its shell
// and opacity rolls are chipchill's).
export const TAPE_SHELL = Object.freeze({ BLACK: 0, CLEAR: 1, WHITE: 2, PALETTE: 3, SONG: 4 });
export const TAPE_LABEL = Object.freeze({ CREAM: 0, PALETTE: 1, SONG: 2 });
export const TAPE_ALPHA = Object.freeze({ FULL: 0, A75: 1, A50: 2, A25: 3, SONG: 4 });
const HUE_STEPS = 36;
// A page called 夜行 that is daylight a quarter of the time is not 夜行, and rain, snow and fog are
// each worth less than a clear sky (parts per thousand over the plain values).
const W_HOUR = Object.freeze([400, 250, 200, 150]); // 夜, 黃昏, 黎明, 白天
const W_WEATHER = Object.freeze([400, 250, 200, 150]); // 晴, 雨, 雪, 霧

export const FOLLOW_LABEL = 'viz.follow';
const ONOFF = ['viz.off', 'viz.on', FOLLOW_LABEL];
const vals = (prefix, n) => [...Array.from({ length: n }, (_, i) => `${prefix}.${i}`), FOLLOW_LABEL];

// A key: how many values (n), how many of them are PLAIN (what a tour steps through and what
// following can become), which index is 隨電台 (-1: none), the value it had before following
// existed (was), its label and its values' labels (null: a hue in steps of 10 degrees), weights
// over the plain values (null: uniform).
const KEYS = {
  ramp: { n: 8, plain: 7, song: RAMPS.SONG, was: RAMPS.PALETTE, labelKey: 'viz.ramp', values: vals('viz.ramp', 7), weights: null },
  duo1: { n: HUE_STEPS, plain: HUE_STEPS, song: -1, was: 32, labelKey: 'viz.hue1', values: null, weights: null },
  duo2: { n: HUE_STEPS, plain: HUE_STEPS, song: -1, was: 5, labelKey: 'viz.hue2', values: null, weights: null },
  shape: { n: 5, plain: 4, song: SHAPE.SONG, was: SHAPE.SEG, labelKey: 'viz.shape', values: vals('viz.shape', 4), weights: null },
  map: { n: 5, plain: 4, song: MAPPING.SONG, was: MAPPING.BOTTOM, labelKey: 'viz.map', values: vals('viz.map', 4), weights: null },
  peaks: { n: 3, plain: 2, song: OFF.SONG, was: OFF.YES, labelKey: 'viz.peaks', values: ONOFF, weights: null },
  reflect: { n: 3, plain: 2, song: OFF.SONG, was: OFF.NO, labelKey: 'viz.reflect', values: ONOFF, weights: null },
  wf_layout: { n: 4, plain: 3, song: WF_LAYOUT.SONG, was: WF_LAYOUT.SPLIT, labelKey: 'viz.wf_layout', values: vals('viz.wf_layout', 3), weights: null },
  wf_dir: { n: 4, plain: 3, song: WF_DIR.SONG, was: WF_DIR.DOWN, labelKey: 'viz.wf_dir', values: vals('viz.wf_dir', 3), weights: null },
  sc_chan: { n: 4, plain: 3, song: SC_CHAN.SONG, was: SC_CHAN.OVERLAY, labelKey: 'viz.sc_chan', values: vals('viz.sc_chan', 3), weights: null },
  sc_draw: { n: 4, plain: 3, song: SC_DRAW.SONG, was: SC_DRAW.LINE, labelKey: 'viz.sc_draw', values: vals('viz.sc_draw', 3), weights: null },
  sc_decay: { n: 3, plain: 2, song: OFF.SONG, was: OFF.YES, labelKey: 'viz.sc_decay', values: ONOFF, weights: null },
  xy_draw: { n: 4, plain: 3, song: XY_DRAW.SONG, was: XY_DRAW.DOTS, labelKey: 'viz.xy_draw', values: vals('viz.xy_draw', 3), weights: null },
  xy_layout: { n: 3, plain: 2, song: XY_LAYOUT.SONG, was: XY_LAYOUT.PLAIN, labelKey: 'viz.xy_layout', values: vals('viz.xy_layout', 2), weights: null },
  // 路線's 隨電台 is at 0, where chipchill's 隨歌曲 has been since before the idea had a name.
  route: { n: 6, plain: 5, song: ROUTE.SONG, was: ROUTE.SUNSET, labelKey: 'viz.route', values: [FOLLOW_LABEL, 'viz.route.1', 'viz.route.2', 'viz.route.3', 'viz.route.4', 'viz.route.5'], weights: null },
  hour: { n: 6, plain: 4, song: HOUR_KEY.SONG, was: HOUR_KEY.NIGHT, labelKey: 'viz.hour', values: vals('viz.hour', 5), weights: W_HOUR },
  weather: { n: 5, plain: 4, song: WEATHER_KEY.SONG, was: WEATHER_KEY.CLEAR, labelKey: 'viz.weather', values: vals('viz.weather', 4), weights: W_WEATHER },
  // 天氣隨音樂 is the owner's taste, not the station's: it has no 隨電台.
  weather_react: { n: 3, plain: 3, song: -1, was: WEATHER_REACT.SOFT, labelKey: 'viz.weather_react', values: ['viz.weather_react.0', 'viz.weather_react.1', 'viz.weather_react.2'], weights: null },
  car: { n: 3, plain: 2, song: CAR.SONG, was: CAR.OFF, labelKey: 'viz.car', values: ONOFF, weights: null },
  tape_shell: { n: 5, plain: 4, song: TAPE_SHELL.SONG, was: TAPE_SHELL.BLACK, labelKey: 'viz.tape_shell', values: vals('viz.tape_shell', 4), weights: null },
  tape_label: { n: 3, plain: 2, song: TAPE_LABEL.SONG, was: TAPE_LABEL.CREAM, labelKey: 'viz.tape_label', values: vals('viz.tape_label', 2), weights: null },
  // The percentages are how much of the plate is left: 不透明 is the label exactly as it was.
  tape_alpha: { n: 5, plain: 4, song: TAPE_ALPHA.SONG, was: TAPE_ALPHA.FULL, labelKey: 'viz.tape_alpha', values: vals('viz.tape_alpha', 4), weights: null },
};
for (const k of Object.keys(KEYS)) Object.freeze(KEYS[k]);

export function keyDef(k) {
  return KEYS[k] ?? null;
}
const saltOf = (k) => KEY_ORDER.indexOf(k) + 1;
const defaultValue = (k) => (KEYS[k].song >= 0 ? KEYS[k].song : KEYS[k].was);

// The i-th plain value of a key (its values without 隨電台), or null.
export function plainAt(k, i) {
  const d = KEYS[k];
  if (!d || !Number.isInteger(i) || i < 0 || i >= d.plain) return null;
  let seen = 0;
  for (let v = 0; v < d.n; v++) {
    if (v === d.song) continue;
    if (seen++ === i) return v;
  }
  return null;
}

// The label key of a value (null for a hue: the UI writes it as `${i * 10}°`).
export function valueLabelKey(k, i) {
  const d = KEYS[k];
  return d?.values?.[i] ?? null;
}

// ---- following the station ------------------------------------------------------------------------
// A station's seed, from something that does not move (its id / url). FNV-1a over the UTF-8 bytes,
// never 0 for a real name: 0 is "nothing is playing", where every 隨電台 takes its plain value.
export function songHash(text) {
  if (text === null || text === undefined || text === '') return 0;
  let h = 2166136261;
  for (const b of new TextEncoder().encode(String(text))) h = Math.imul((h ^ b) >>> 0, 16777619) >>> 0;
  return h || 1;
}

// 0..n-1 from a seed and a salt: the salt goes in before a multiply-xorshift finisher.
export function songRoll(seed, salt, n) {
  let h = ((seed >>> 0) ^ Math.imul(salt >>> 0, 0x9e3779b9)) >>> 0;
  h = Math.imul(h, 0x85ebca6b) >>> 0;
  h = (h ^ (h >>> 13)) >>> 0;
  h = Math.imul(h, 0xc2b2ae35) >>> 0;
  h = (h ^ (h >>> 16)) >>> 0;
  return n ? h % n : 0;
}

function pick(k, seed) {
  const d = KEYS[k];
  const salt = saltOf(k);
  if (!d.weights) return songRoll(seed, salt, d.plain);
  const w = d.weights.slice(0, d.plain);
  const roll = songRoll(seed, salt, w.reduce((a, b) => a + b, 0));
  let at = 0, p = 0;
  while (p < d.plain - 1) {
    at += w[p];
    if (roll < at) break;
    p++;
  }
  return p;
}

// ---- pages ----------------------------------------------------------------------------------------
// spec: { keys, rows(cfg) -> keys that are rows in this combination, presets: { keys, rows: [[labelKey,
// values]] }, groups: [{ id, labelKey, keys (fastest first) }] }
export function definePage(spec) {
  const keys = Object.freeze([...spec.keys]);
  const groups = Object.freeze(spec.groups.map((g) => Object.freeze({ ...g, keys: Object.freeze([...g.keys]) })));
  const presets = spec.presets;
  const copy = (c) => { const o = {}; for (const k of keys) o[k] = c[k]; return o; };
  const isIdx = (k, v) => Number.isInteger(v) && v >= 0 && v < KEYS[k].n;

  const page = {
    id: spec.id,
    keys,
    defaults() { const o = {}; for (const k of keys) o[k] = defaultValue(k); return o; },
    plain() { const o = {}; for (const k of keys) o[k] = KEYS[k].was; return o; },
    validate(obj) {
      const src = obj && typeof obj === 'object' && !Array.isArray(obj) ? obj : {};
      const o = {};
      for (const k of keys) o[k] = isIdx(k, src[k]) ? src[k] : defaultValue(k);
      return o;
    },
    rows(c) { return spec.rows(c); },
    // A row that does not exist in this combination carries no value of its own: it takes its
    // first plain value, the one the tour walks it at.
    snapAbsent(c) {
      const o = copy(c);
      const rows = spec.rows(o);
      for (const g of groups) for (const k of g.keys) if (!rows.includes(k)) o[k] = plainAt(k, 0) ?? 0;
      return o;
    },
    // The effective choices for the station with this seed: every key at 隨電台 takes one of its
    // own plain values, a salted hash of the seed per key; seed 0 takes the plain look.
    resolve(c, seed) {
      const o = page.plain();
      for (const k of keys) {
        const d = KEYS[k];
        let v = c?.[k];
        if (!isIdx(k, v)) v = defaultValue(k);
        if (d.song < 0 || v !== d.song) { o[k] = v; continue; }
        o[k] = seed ? plainAt(k, pick(k, seed >>> 0)) ?? d.was : d.was;
      }
      return page.snapAbsent(o);
    },

    // presets: the named ones and one more, 隨電台
    presetCount() { return presets.rows.length ? presets.rows.length + 1 : 0; },
    presetLabelKey(i) {
      const n = presets.rows.length;
      if (!n || !Number.isInteger(i) || i < 0 || i > n) return null;
      return i === n ? 'viz.preset.follow' : presets.rows[i][0];
    },
    applyPreset(c, i) {
      if (page.presetLabelKey(i) === null) return copy(c);
      if (i === presets.rows.length) {
        let o = copy(c);
        for (let g = 0; g < groups.length; g++) o = page.tourSet(o, g, page.tourCount(g));
        return o;
      }
      const o = copy(c);
      presets.keys.forEach((k, j) => { o[k] = presets.rows[i][1][j]; });
      return o;
    },
    // The preset these choices are, or -1 (自訂).
    preset(c) {
      if (!presets.rows.length) return -1;
      let any = false, all = true;
      for (const g of groups) for (const k of g.keys) {
        if (KEYS[k].song < 0) continue;
        any = true;
        if (c[k] !== KEYS[k].song) all = false;
      }
      if (any && all) return presets.rows.length;
      for (let i = 0; i < presets.rows.length; i++) if (presets.keys.every((k, j) => c[k] === presets.rows[i][1][j])) return i;
      return -1;
    },
    // The next preset (from 自訂, the first): { cfg, index }.
    stepPreset(c, dir) {
      const n = page.presetCount();
      if (!n) return { cfg: copy(c), index: -1 };
      const cur = page.preset(c);
      const next = cur < 0 ? 0 : (cur + (dir < 0 ? -1 : 1) + n) % n;
      return { cfg: page.applyPreset(c, next), index: next };
    },

    // tours
    groupCount() { return groups.length; },
    groupId(g) { return groups[g]?.id ?? null; },
    groupKeys(g) { return groups[g]?.keys ?? []; },
    groupLabelKey(g) { return groups[g]?.labelKey ?? null; },
    // How many combinations the tour has, not counting its 隨電台 stop (which is this index).
    tourCount(g) { return walk(page.plain(), g, -1).found; },
    // Combination pos of the group's tour into the choices, every other key left alone; pos ===
    // the count sets the group's keys to 隨電台. Out of range wraps.
    tourSet(c, g, pos) {
      if (!groups[g]) return copy(c);
      const n = page.tourCount(g);
      pos = ((pos % (n + 1)) + n + 1) % (n + 1);
      if (pos === n) {
        const o = copy(c);
        for (const k of groups[g].keys) if (KEYS[k].song >= 0) o[k] = KEYS[k].song;
        return o;
      }
      return walk(c, g, pos).out ?? copy(c);
    },
    // Where these choices stand on the tour: the count when every key of the group follows, the
    // combination's index when all are plain and it exists, -1 for a mixed group.
    tourPos(c, g) {
      if (!groups[g]) return -1;
      const ks = groups[g].keys;
      const song = ks.filter((k) => KEYS[k].song >= 0 && c[k] === KEYS[k].song).length;
      if (song === ks.length) return page.tourCount(g);
      if (song > 0) return -1;
      const n = page.tourCount(g);
      for (let pos = 0; pos < n; pos++) {
        const got = walk(c, g, pos).out;
        if (got && ks.every((k) => got[k] === c[k])) return pos;
      }
      return -1;
    },
    // One press, from these choices with `shown` being what the page actually draws (the resolved
    // cfg), to the next stop: { cfg, pos }. A mixed group first takes the values that are showing.
    tourStep(c, shown, g) {
      if (!groups[g]) return { cfg: copy(c), pos: -1 };
      let o = copy(c);
      let pos = page.tourPos(o, g);
      if (pos < 0) {
        for (const k of groups[g].keys) o[k] = shown[k];
        o = page.snapAbsent(o);
        pos = page.tourPos(o, g);
      }
      o = page.tourSet(o, g, pos + 1);
      return { cfg: o, pos: page.tourPos(o, g) };
    },
  };

  // Does this combination exist? A key the page leaves out in it does nothing, so only its first
  // plain value is toured: otherwise one picture would come round twice.
  function real(c, g, digit) {
    const rows = spec.rows(c);
    return groups[g].keys.every((k, i) => rows.includes(k) || digit[i] === 0);
  }

  // The group's odometer from base, fastest key first: want -1 counts the combinations, otherwise
  // stops at that one and returns it as out.
  function walk(base, g, want) {
    const ks = groups[g]?.keys ?? [];
    if (!ks.length) return { found: 0, out: null };
    const size = ks.map((k) => KEYS[k].plain);
    const raw = size.reduce((a, b) => a * b, 1);
    let found = 0;
    for (let r = 0; r < raw; r++) {
      const c = copy(base);
      let rest = r;
      const digit = size.map((s) => { const d = rest % s; rest = Math.trunc(rest / s); return d; });
      ks.forEach((k, i) => { c[k] = plainAt(k, digit[i]) ?? 0; });
      if (!real(c, g, digit)) continue;
      if (want === found) return { found, out: c };
      found++;
    }
    return { found, out: null };
  }

  return Object.freeze(page);
}

// ---- 頻譜, the spectrum -------------------------------------------------------------------------
export const SPECTRUM = definePage({
  id: 'spectrum',
  keys: ['shape', 'map', 'ramp', 'peaks', 'reflect', 'duo1', 'duo2'],
  // The keys that are rows in this combination (cfg.rs page_keys): a tube has no caps, a ring no
  // floor, the hues only for the duo ramp.
  rows(c) {
    const l = ['shape', 'map'];
    if (c.shape !== SHAPE.LINE) l.push('peaks');
    if (c.shape !== SHAPE.RADIAL) l.push('reflect');
    l.push('ramp');
    if (c.ramp === RAMPS.DUO) l.push('duo1', 'duo2');
    return l;
  },
  // The first row is the page as it was before there were choices.
  presets: {
    keys: ['shape', 'map', 'ramp', 'peaks', 'reflect'],
    rows: [
      ['viz.spectrum.preset.0', [SHAPE.SEG, MAPPING.BOTTOM, RAMPS.PALETTE, 1, 0]], // 經典霓虹
      ['viz.spectrum.preset.1', [SHAPE.SOLID, MAPPING.TIP, RAMPS.FIRE, 1, 1]], // 火焰
      ['viz.spectrum.preset.2', [SHAPE.SOLID, MAPPING.TIP, RAMPS.ICE, 1, 1]], // 冰焰
      ['viz.spectrum.preset.3', [SHAPE.SEG, MAPPING.BOTTOM, RAMPS.WINAMP, 1, 1]], // Winamp
      ['viz.spectrum.preset.4', [SHAPE.SOLID, MAPPING.BAND, RAMPS.RAINBOW, 1, 1]], // 彩虹頻段
      ['viz.spectrum.preset.5', [SHAPE.RADIAL, MAPPING.LEVEL, RAMPS.PALETTE, 1, 0]], // 脈動
      ['viz.spectrum.preset.6', [SHAPE.LINE, MAPPING.BAND, RAMPS.PALETTE, 0, 1]], // 霓虹燈管
      ['viz.spectrum.preset.7', [SHAPE.LINE, MAPPING.BAND, RAMPS.ICE, 0, 1]], // 極光
      ['viz.spectrum.preset.8', [SHAPE.SOLID, MAPPING.BAND, RAMPS.PALETTE, 1, 1]], // 純色＋倒影
    ],
  },
  // Two groups, each a thing a person can name: the body against its colour. Fastest key first.
  groups: [
    { id: 'form', labelKey: 'viz.group.form', keys: ['shape', 'peaks', 'reflect'] }, // 形體
    { id: 'colour', labelKey: 'viz.group.colour', keys: ['ramp', 'map'] }, // 顏色
  ],
});

// ---- 瀑布, the waterfall ------------------------------------------------------------------------
// The ramp (and its hues for the duo) is a row as on every instrument page (cfg.rs page_keys);
// chipchill shares one ramp across the instrument pages, tunetap keeps it per visual.
const rampRows = (c, l) => {
  l.push('ramp');
  if (c.ramp === RAMPS.DUO) l.push('duo1', 'duo2');
  return l;
};
export const WATERFALL = definePage({
  id: 'waterfall',
  keys: ['wf_layout', 'wf_dir', 'ramp', 'duo1', 'duo2'],
  rows(c) { return rampRows(c, ['wf_layout', 'wf_dir']); },
  presets: {
    keys: ['wf_layout', 'wf_dir'],
    rows: [
      ['viz.waterfall.preset.0', [WF_LAYOUT.SPLIT, WF_DIR.DOWN]], // 鏡像瀑布
      ['viz.waterfall.preset.1', [WF_LAYOUT.SUM, WF_DIR.DOWN]], // 全寬瀑布
      ['viz.waterfall.preset.2', [WF_LAYOUT.STACKED, WF_DIR.DOWN]], // 雙層瀑布
      ['viz.waterfall.preset.3', [WF_LAYOUT.SUM, WF_DIR.UP]], // 上升
      ['viz.waterfall.preset.4', [WF_LAYOUT.SUM, WF_DIR.ACROSS]], // 頻譜圖
      ['viz.waterfall.preset.5', [WF_LAYOUT.STACKED, WF_DIR.ACROSS]], // 頻譜圖 L / R
    ],
  },
  // cfg.rs TOURS: the layout against the ramp, fastest key first.
  groups: [
    { id: 'layout', labelKey: 'viz.group.layout', keys: ['wf_layout', 'wf_dir'] }, // 版面
    { id: 'ramp', labelKey: 'viz.group.ramp', keys: ['ramp'] }, // 色帶
  ],
});

// ---- 波形, the scope ------------------------------------------------------------------------------
export const SCOPE = definePage({
  id: 'scope',
  keys: ['sc_chan', 'sc_draw', 'sc_decay', 'ramp', 'duo1', 'duo2'],
  rows(c) { return rampRows(c, ['sc_chan', 'sc_draw', 'sc_decay']); },
  presets: {
    keys: ['sc_chan', 'sc_draw', 'sc_decay'],
    rows: [
      ['viz.scope.preset.0', [SC_CHAN.OVERLAY, SC_DRAW.LINE, 1]], // 疊加
      ['viz.scope.preset.1', [SC_CHAN.STACKED, SC_DRAW.LINE, 1]], // 雙軌
      ['viz.scope.preset.2', [SC_CHAN.MIDSIDE, SC_DRAW.LINE, 1]], // Mid / Side
      ['viz.scope.preset.3', [SC_CHAN.OVERLAY, SC_DRAW.FILL, 0]], // 填色
      ['viz.scope.preset.4', [SC_CHAN.OVERLAY, SC_DRAW.DOTS, 0]], // 點陣
    ],
  },
  // cfg.rs TOURS: how it is drawn against the ramp, fastest key first.
  groups: [
    { id: 'draw', labelKey: 'viz.group.draw', keys: ['sc_chan', 'sc_draw', 'sc_decay'] }, // 畫法
    { id: 'ramp', labelKey: 'viz.group.ramp', keys: ['ramp'] }, // 色帶
  ],
});

// ---- 相位, the goniometer -------------------------------------------------------------------------
export const XY = definePage({
  id: 'xy',
  keys: ['xy_draw', 'xy_layout', 'ramp', 'duo1', 'duo2'],
  rows(c) { return rampRows(c, ['xy_draw', 'xy_layout']); },
  presets: {
    keys: ['xy_draw', 'xy_layout'],
    rows: [
      ['viz.xy.preset.0', [XY_DRAW.DOTS, XY_LAYOUT.PLAIN]], // 經典
      ['viz.xy.preset.1', [XY_DRAW.DOTS, XY_LAYOUT.METERS]], // 儀表
      ['viz.xy.preset.2', [XY_DRAW.LINES, XY_LAYOUT.METERS]], // 向量
      ['viz.xy.preset.3', [XY_DRAW.HEAT, XY_LAYOUT.METERS]], // 熱圖
    ],
  },
  // cfg.rs TOURS: how it is drawn against the ramp, fastest key first.
  groups: [
    { id: 'draw', labelKey: 'viz.group.draw', keys: ['xy_draw', 'xy_layout'] }, // 畫法
    { id: 'ramp', labelKey: 'viz.group.ramp', keys: ['ramp'] }, // 色帶
  ],
});

// ---- 夜行, the night drive ------------------------------------------------------------------------
export const DRIVE = definePage({
  id: 'drive',
  keys: ['route', 'hour', 'weather', 'weather_react', 'car'],
  // Every key is always a row (cfg.rs page_keys): 天氣隨音樂 stays one even on 晴, it is how the
  // owner finds out the weather can hear. The drive has no ramp row.
  rows() { return ['route', 'hour', 'weather', 'weather_react', 'car']; },
  presets: {
    keys: ['hour', 'weather', 'car'],
    rows: [
      ['viz.drive.preset.0', [HOUR_KEY.NIGHT, WEATHER_KEY.CLEAR, CAR.OFF]], // 原版夜色
      // chipchill's 跟著現在 (the clock and the forecast); with no forecast here the weather is
      // what chipchill shows when the platform has none: clear
      ['viz.drive.preset.1', [HOUR_KEY.AUTO, WEATHER_KEY.CLEAR, CAR.ON]], // 跟著時鐘
      ['viz.drive.preset.2', [HOUR_KEY.DUSK, WEATHER_KEY.CLEAR, CAR.ON]], // 黃昏
      ['viz.drive.preset.3', [HOUR_KEY.DAWN, WEATHER_KEY.RAIN, CAR.ON]], // 黎明雨
      ['viz.drive.preset.4', [HOUR_KEY.NIGHT, WEATHER_KEY.SNOW, CAR.ON]], // 雪夜
      ['viz.drive.preset.5', [HOUR_KEY.DAWN, WEATHER_KEY.FOG, CAR.ON]], // 晨霧
    ],
  },
  // 夜行's sky against its road (cfg.rs TOURS), fastest key first.
  groups: [
    { id: 'sky', labelKey: 'viz.group.sky', keys: ['weather', 'hour'] }, // 天空
    { id: 'road', labelKey: 'viz.group.road', keys: ['route', 'car'] }, // 路線
  ],
});

// ---- 錄音帶, the cassette -----------------------------------------------------------------------
export const CASSETTE = definePage({
  id: 'cassette',
  keys: ['tape_shell', 'tape_label', 'tape_alpha'],
  // Every key is always a row (cfg.rs page_keys); the cassette has no ramp row.
  rows() { return ['tape_shell', 'tape_label', 'tape_alpha']; },
  presets: {
    keys: ['tape_shell', 'tape_label', 'tape_alpha'],
    rows: [
      ['viz.cassette.preset.0', [TAPE_SHELL.BLACK, TAPE_LABEL.CREAM, TAPE_ALPHA.FULL]], // 經典
      ['viz.cassette.preset.1', [TAPE_SHELL.CLEAR, TAPE_LABEL.CREAM, TAPE_ALPHA.FULL]], // 透明
      ['viz.cassette.preset.2', [TAPE_SHELL.WHITE, TAPE_LABEL.CREAM, TAPE_ALPHA.FULL]], // 白色
      ['viz.cassette.preset.3', [TAPE_SHELL.PALETTE, TAPE_LABEL.PALETTE, TAPE_ALPHA.FULL]], // 配色
      // (chipchill's 初音, the character label, is left out)
      ['viz.cassette.preset.4', [TAPE_SHELL.CLEAR, TAPE_LABEL.PALETTE, TAPE_ALPHA.A50]], // 透視
    ],
  },
  // The shell against the label (cfg.rs TOURS), fastest key first.
  groups: [
    { id: 'shell', labelKey: 'viz.group.shell', keys: ['tape_shell'] }, // 外殼
    { id: 'label', labelKey: 'viz.group.label', keys: ['tape_label', 'tape_alpha'] }, // 標籤
  ],
});
