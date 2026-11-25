// SPDX-License-Identifier: LGPL-2.1-or-later
// The visualizer host (design amendments A + D): owns the low-res <canvas>, the shared pixel canvas,
// the requestAnimationFrame loop and ALL its run conditions, reads the audio tap once a frame for
// the active plugin's needs, paints the chosen palette's backdrop (chipchill pages/mod.rs backdrop)
// before each frame, switches plugins (dispose old, create new), keeps plugin errors away from the
// player, and persists the choices (mode, cfg, theme; never on/off). chipchill's platform layer plays this part there; this is
// tunetap's own.
//
// deps: { doc, storage, raf, caf, now, getTheme, getAudio, visible, playing, station, reducedMotion,
//         dpr, registry, title, stationName, playTime }
//   getTheme()  -> the `app` palette (themes.js themeFromTokens on the app's computed style)
//   getAudio()  -> the tap (viz-audio.js createVizTap) or null; player.vizTap() in the app
//   visible()   -> the Play page is the current route; playing() -> the player is playing
//   station()   -> the station id (follow-the-station seed); reducedMotion() -> bool; dpr -> number|fn
//   title()     -> the ICY now-playing title or null; stationName() -> the station's name;
//   playTime()  -> ms this station has played (player.playTime(): 0 for a new one, frozen while paused)
// Call update() whenever the route, the player's state or prefers-color-scheme change; the host
// listens to visibilitychange itself. It never uses timers: the injected raf/caf are its only clock.
//
// A plugin module exports create({ canvas, ctx2d, theme, seed }) -> instance:
//   needs { freq: 'mono'|'stereo', time: 'none'|'mono'|'stereo' }, schema [...], presets [{ id, labelKey, cfg }],
//   defaults(), validate(obj), applyPreset(cfg, id), presetOf(cfg), resolveFollow(cfg, seed),
//   groups() -> [{ id, labelKey, count, pos(cfg), step(cfg, shown) -> { cfg, pos } }],
//   resize(w, h, dpr), frame({ dt, now, audio, theme, cfg, playing, pixels, full, seed, title, artist,
//   elapsedMs, measure }) -> undefined | { labels }, status(), dispose()
//   (seed: the station's follow-the-station seed, which the drive also uses for its gust and meteors)
//   (full: the canvas fills the screen, chipchill's View.full; the spectrum then always shows caps)
//   (title / artist: chipchill's View.title / artist for a stream: the ICY title when there is one,
//   else the station's name; the artist is the station's name when a title is shown, else '')
//   (elapsedMs: the play time, the cassette's counter; measure: the labels' text widths, labels.js)
//   (labels: chipchill's Labels, js/viz/labels.js: the host draws them over the picture on a second
//   canvas, class viz-labels, aria-hidden, at the screen's resolution, and only when they change)
import { Canvas, blit } from './pixel.js';
import { backdrop } from './page.js';
import { themeNamed, THEME_NAMES } from './themes.js';
import { songHash } from './cfg.js';
import { registry as defaultRegistry } from './registry.js';
import { createLabelLayer } from './labels.js';

export const CANVAS_W = 480;
const FPS = 30;
const FPS_REDUCED = 15;
const REST_S = 10; // the dt of a resting frame: every level has long fallen
const SLACK_MS = 2; // vsync jitter: a 33.3 ms interval may arrive as 33.0
// on/off is not stored: it is the Play page's 頻譜 view, which always starts off (design amendment H).
const KEY = { mode: 'tunetap.viz.mode', theme: 'tunetap.viz.theme', cfg: (id) => `tunetap.viz.cfg.${id}` };

const call = (fn, fallback) => { try { return fn(); } catch { return fallback; } };

export function createVizHost(deps = {}) {
  const { doc = null, storage = null, raf, caf, now } = deps;
  const registry = deps.registry ?? defaultRegistry;
  const getTheme = deps.getTheme ?? (() => themeNamed('neon'));
  const getAudio = deps.getAudio ?? (() => null);
  const visible = deps.visible ?? (() => true);
  const isPlaying = deps.playing ?? (() => false);
  const station = deps.station ?? (() => '');
  const reduced = deps.reducedMotion ?? (() => false);
  const icyTitle = deps.title ?? (() => null);
  const stationName = deps.stationName ?? (() => '');
  const playTime = deps.playTime ?? (() => 0);
  const dpr = () => call(() => { const d = typeof deps.dpr === 'function' ? deps.dpr() : deps.dpr; return Math.min(Math.max(+d || 1, 1), 2); }, 1);

  const read = (k) => call(() => storage?.getItem(k) ?? null, null);
  const write = (k, v) => call(() => { storage?.setItem(k, v); }, undefined);

  const listeners = new Set();
  let on = false;
  let modeId = registry.get(read(KEY.mode)) ? read(KEY.mode) : registry.list()[0]?.id ?? null;
  let themeName = read(KEY.theme);
  if (themeName !== 'app' && !themeNamed(themeName)) themeName = 'app';
  let appTheme = null;
  let container = null, canvas = null, ctx = null, pixels = null;
  let textCanvas = null, layer = null; // the labels' canvas and its layer (labels.js)
  let inst = null, instMode = null, cfg = null;
  let failedMode = null, loading = null, token = 0;
  let rafId = null, last = null;
  let tapSeen = null;
  let lastStatus = null;
  let full = false;

  function emit() {
    lastStatus = status();
    for (const fn of [...listeners]) call(() => fn(), undefined);
  }
  function emitIfStatusChanged() {
    if (status() !== lastStatus) emit();
  }

  const theme = () => (themeName === 'app' ? (appTheme ??= call(() => getTheme(), null) ?? themeNamed('neon')) : themeNamed(themeName));
  const seed = () => songHash(call(() => station(), '') || '');
  const def = () => registry.get(instMode ?? modeId);
  const hidden = () => !!call(() => doc?.hidden, false);
  const conditions = () => on && !!canvas && !!call(() => visible(), false) && !hidden() && !!call(() => isPlaying(), false);

  function loadCfg(id) {
    let obj = null;
    try { obj = JSON.parse(read(KEY.cfg(id)) ?? 'null'); } catch { obj = null; }
    return call(() => inst.validate(obj), null) ?? call(() => inst.defaults(), {});
  }
  function saveCfg() {
    if (inst) write(KEY.cfg(instMode), JSON.stringify(cfg));
  }

  const clearLabels = () => call(() => layer?.clear(), undefined);

  function dropInstance() {
    clearLabels();
    if (!inst) return;
    const i = inst;
    inst = null; instMode = null; cfg = null; tapSeen = null;
    call(() => i.dispose(), undefined);
  }

  function fail(id) {
    dropInstance();
    failedMode = id;
    stopLoop(true);
    emit();
  }

  // Load and create the current mode's plugin (once; a failed one stays down until chosen again).
  function ensure() {
    const id = modeId;
    if (!id || (inst && instMode === id) || loading?.mode === id || failedMode === id) return;
    const my = ++token;
    const p = registry.load(id)
      .then((mod) => {
        if (my !== token) return;
        const ni = mod.create({ canvas, ctx2d: ctx, theme: theme(), seed: seed() });
        if (!ni || typeof ni.frame !== 'function') throw new Error(`plugin ${id}: no instance`);
        dropInstance();
        inst = ni; instMode = id;
        cfg = loadCfg(id);
        if (pixels) call(() => inst.resize(pixels.width, pixels.height, dpr()), undefined);
      })
      .catch(() => { if (my === token) { failedMode = id; dropInstance(); } })
      .finally(() => {
        if (my !== token) return;
        loading = null;
        update();
        emit();
      });
    loading = { mode: id, p };
  }

  function readAudio() {
    if (!inst) return null;
    const tap = call(() => getAudio(), null);
    if (!tap) return null;
    if (def()?.needsAudio === false) {
      // a visual that does not hear the music (the cassette): its needs (nothing) go to the tap once,
      // so the side analysers an earlier visual asked for are let go, and nothing is read
      if (tap !== tapSeen) { call(() => tap.setNeeds?.(inst.needs), undefined); tapSeen = tap; }
      return null;
    }
    try {
      if (tap !== tapSeen) { tap.setNeeds?.(inst.needs); tapSeen = tap; }
      return tap.read?.(inst.needs) ?? null;
    } catch { return null; }
  }

  const shown = () => call(() => inst.resolveFollow(cfg, seed()), cfg);

  // What a page is told of the stream (chipchill's View.title / artist): the ICY title when there is
  // one, else the station's name; the station's name is the artist when a title is shown.
  function words() {
    const icy = call(() => icyTitle(), null);
    const name = String(call(() => stationName(), '') ?? '');
    const t = typeof icy === 'string' && icy.trim() ? icy.trim() : '';
    return t ? { title: t, artist: name } : { title: name, artist: '' };
  }

  function draw(dt, t, audio) {
    if (!inst || !pixels) return;
    const id = instMode;
    try {
      const th = theme();
      const sd = seed();
      backdrop(pixels, th);
      const { title, artist } = words();
      const elapsedMs = Math.max(0, +call(() => playTime(), 0) || 0);
      const out = inst.frame({ dt, now: t, audio, theme: th, cfg: call(() => inst.resolveFollow(cfg, sd), cfg), playing: !!call(() => isPlaying(), false), pixels, full, seed: sd, title, artist, elapsedMs, measure: layer?.measure ?? null });
      blit(ctx, pixels);
      if (layer) call(() => layer.set(out && Array.isArray(out.labels) ? out.labels : null, pixels.width, pixels.height), undefined);
    } catch {
      fail(id);
      return;
    }
    emitIfStatusChanged();
  }

  function startLoop() {
    if (rafId === null) rafId = raf(tick);
  }
  function stopLoop(clear) {
    if (rafId !== null) { call(() => caf(rafId), undefined); rafId = null; }
    last = null;
    if (clear && ctx && canvas) call(() => ctx.clearRect(0, 0, canvas.width, canvas.height), undefined);
    if (clear) clearLabels();
  }

  function tick() {
    rafId = null;
    if (!inst || !conditions()) { update(); return; }
    rafId = raf(tick);
    const t = now();
    const fps = call(() => reduced(), false) ? FPS_REDUCED : FPS;
    if (last !== null && t - last < 1000 / fps - SLACK_MS) return;
    const dt = last === null ? 1 / fps : (t - last) / 1000;
    last = t;
    const audio = readAudio();
    if (!audio && def()?.stillWithoutAudio) { stopLoop(false); draw(REST_S, t, null); return; } // nothing to react to: rest
    draw(dt, t, audio);
  }

  // A still frame (dt 0) when a choice changes while the loop is not running.
  function repaint() {
    if (rafId !== null || !inst || !canvas || !on || hidden() || !call(() => visible(), false)) return;
    draw(0, now(), readAudio());
  }

  function update() {
    appTheme = null;
    if (on && canvas) ensure();
    if (!inst || !conditions()) {
      stopLoop(true);
    } else if (def()?.stillWithoutAudio && !call(() => getAudio(), null)) {
      // nothing to react to: one resting frame (as after a long silence), no loop
      stopLoop(false);
      draw(REST_S, now(), null);
    } else {
      startLoop();
    }
    emitIfStatusChanged();
  }

  function onVisibility() { update(); }

  function size() {
    const cw = call(() => container.clientWidth, 0) || 0, chh = call(() => container.clientHeight, 0) || 0;
    const w = CANVAS_W;
    const h = cw > 0 && chh > 0 ? Math.min(Math.max(1, Math.round((w * chh) / cw)), 4 * w) : Math.round((w * 9) / 16);
    canvas.width = w;
    canvas.height = h;
    pixels = new Canvas(w, h);
    if (textCanvas) {
      // the labels at the screen's resolution: the box's CSS size times the device pixel ratio
      const k = dpr();
      textCanvas.width = Math.max(1, Math.round((cw > 0 ? cw : w) * k));
      textCanvas.height = Math.max(1, Math.round((chh > 0 ? chh : h) * k));
      clearLabels(); // a new size empties the canvas: the next frame draws them again
    }
    if (inst) call(() => inst.resize(w, h, dpr()), undefined);
  }

  function status() {
    if (!on) return 'off';
    if (failedMode && failedMode === modeId) return 'unavailable';
    if (!inst || loading) return 'loading';
    if (!conditions()) return 'idle';
    return call(() => inst.status(), 'ok') || 'ok';
  }

  function setMode(id) {
    if (!registry.get(id)) return;
    if (id === modeId && failedMode !== id) return;
    modeId = id;
    write(KEY.mode, id);
    failedMode = null;
    token++; loading = null;
    dropInstance();
    stopLoop(false);
    update();
    emit();
  }

  function stepMode(dir) {
    const list = registry.list();
    if (!list.length) return;
    const i = Math.max(0, list.findIndex((p) => p.id === modeId));
    setMode(list[(i + dir + list.length) % list.length].id);
  }

  function whenReady() {
    return loading ? loading.p.then(() => (loading ? whenReady() : undefined)) : Promise.resolve();
  }
  function groups() {
    if (!inst) return [];
    return call(() => inst.groups(), []).map((g) => ({ id: g.id, labelKey: g.labelKey, count: g.count, pos: call(() => g.pos(cfg), -1) }));
  }
  function unmount() {
    stopLoop(false);
    token++; loading = null;
    dropInstance();
    doc?.removeEventListener?.('visibilitychange', onVisibility);
    call(() => canvas?.remove(), undefined);
    call(() => textCanvas?.remove(), undefined);
    container = null; canvas = null; ctx = null; pixels = null; textCanvas = null; layer = null;
    emitIfStatusChanged();
  }

  return {
    // Puts the canvas (class viz-canvas; scaled up by CSS, pixelated) and the labels' canvas over it
    // (class viz-labels) into `el` and starts watching.
    mount(el) {
      if (container) unmount();
      container = el;
      canvas = doc.createElement('canvas');
      canvas.className = 'viz-canvas';
      ctx = call(() => canvas.getContext('2d'), null);
      el.append(canvas);
      // the labels' canvas, stacked over the picture (class viz-labels): text only, never a target
      textCanvas = doc.createElement('canvas');
      textCanvas.className = 'viz-labels';
      call(() => textCanvas.setAttribute('aria-hidden', 'true'), undefined);
      const tctx = call(() => textCanvas.getContext('2d'), null);
      layer = tctx ? createLabelLayer({ canvas: textCanvas, ctx: tctx }) : null;
      el.append(textCanvas);
      size();
      doc.addEventListener?.('visibilitychange', onVisibility);
      update();
    },
    unmount,
    // Re-measure the container (layout / orientation change).
    resize() {
      if (!canvas) return;
      size();
      repaint();
    },
    update,
    // Loads the current plugin even while the visualizer is off (for the controls).
    prepare() { ensure(); },
    // Settles when the plugin being loaded (if any) is ready or has failed.
    whenReady,
    enabled: () => on,
    setEnabled(b) {
      on = !!b;
      if (!on) stopLoop(true);
      update();
      emit();
    },
    mode: () => modeId,
    modes: () => registry.list().map((p) => ({ id: p.id, labelKey: p.labelKey })),
    setMode,
    nextMode() { stepMode(1); },
    prevMode() { stepMode(-1); },
    // The current plugin's controls and choices; null until it is loaded.
    plugin() {
      if (!inst) return null;
      const d = registry.get(instMode);
      return {
        id: instMode, labelKey: d?.labelKey, needsAudio: !!d?.needsAudio,
        // calm: without audio data the plugin keeps animating (the drive), not one resting frame
        calm: !!d && !d.stillWithoutAudio,
        // how the status line speaks of the data (registry.js): 'spectrum' or 'audio'
        words: d?.words ?? 'audio',
        schema: inst.schema ?? [],
        cfg: { ...cfg },
        shown: shown(),
        preset: call(() => inst.presetOf(cfg), -1),
        presets: (inst.presets ?? []).map((p) => ({ id: p.id, labelKey: p.labelKey })),
        groups: groups(),
      };
    },
    setCfg(patch) {
      if (!inst) return;
      cfg = call(() => inst.validate({ ...cfg, ...patch }), cfg);
      saveCfg();
      emit();
      repaint();
    },
    setPreset(id) {
      if (!inst) return;
      cfg = call(() => inst.validate(inst.applyPreset(cfg, id)), cfg);
      saveCfg();
      emit();
      repaint();
    },
    themeName: () => themeName,
    themes: () => [{ name: 'app', labelKey: 'viz.theme.app' }, ...THEME_NAMES.map((n) => ({ name: n, labelKey: `viz.theme.${n}` }))],
    setThemeName(name) {
      if (name !== 'app' && !themeNamed(name)) return;
      themeName = name;
      appTheme = null;
      write(KEY.theme, name);
      emit();
      repaint();
    },
    // The current plugin's two tour groups and where the choices stand on each (count = 隨電台).
    groups,
    // One press of a group's key (the on-page zones): the next stop, persisted. Returns { pos, count,
    // follow, labelKey, key, args } for a toast — t(key, t(labelKey), ...args.slice(1)) — and `text`
    // already translated when t is given. null for no such group.
    stepGroup(i, t) {
      const g = inst ? call(() => inst.groups(), [])[i] : null;
      if (!g) return null;
      const r = call(() => g.step(cfg, shown()), null);
      if (!r) return null;
      cfg = call(() => inst.validate(r.cfg), cfg);
      saveCfg();
      const pos = call(() => g.pos(cfg), r.pos);
      const follow = pos === g.count;
      const out = { pos, count: g.count, follow, labelKey: g.labelKey, key: follow ? 'viz.group.follow' : 'viz.group.pos', args: follow ? [g.labelKey] : [g.labelKey, pos + 1, g.count] };
      if (typeof t === 'function') out.text = follow ? t(out.key, t(g.labelKey)) : t(out.key, t(g.labelKey), pos + 1, g.count);
      emit();
      repaint();
      return out;
    },
    // Full screen on/off (the UI's Fullscreen API state); repaints a still frame when stopped.
    full: () => full,
    setFull(b) {
      if (full === !!b) return;
      full = !!b;
      repaint();
    },
    status,
    // The labels on the screen now (tests and the smoke's hook): [{ kind, text, x, y, … }].
    labels: () => call(() => layer?.shown() ?? [], []),
    onChange(fn) {
      listeners.add(fn);
      return () => listeners.delete(fn);
    },
  };
}
