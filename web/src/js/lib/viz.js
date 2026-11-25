// SPDX-License-Identifier: LGPL-2.1-or-later
// NightStand Nano: glue between the page and the visualizer ported from tunetap (viz/).
// Everything Nano-specific lives here and in viz-nano-tap.js / viz-nano-ui.js; viz/ stays as close to tunetap's
// js/viz/ as it can (see the "nano:" comments there, if any).
//
//   createNanoViz({ getBytes })  ->  { host, t, setState(state), update() }
//     getBytes: () => Uint8Array(32), the latest spectrum bands from the box
//     setState(state): the page calls it with every parsed `st` reply (and null when the box is gone)
import { createVizHost } from '../../vendor/tunetap/viz/host.js';
import { registry } from '../../vendor/tunetap/viz/registry.js';
import { themeFromTokens } from '../../vendor/tunetap/viz/themes.js';
import { createBandTap } from './viz-tap.js';

// The four visuals, in tunetap's order (js/viz/index.js minus 波形 scope and 相位 xy: the box sends no raw samples).
if (!registry.get('spectrum')) {
  registry.register({ id: 'spectrum', labelKey: 'viz.mode.spectrum', needsAudio: true, stillWithoutAudio: true, words: 'spectrum', load: () => import('../../vendor/tunetap/viz/plugins/spectrum.js') });
  registry.register({ id: 'drive', labelKey: 'viz.mode.drive', needsAudio: true, stillWithoutAudio: false, load: () => import('../../vendor/tunetap/viz/plugins/drive.js') });
  registry.register({ id: 'waterfall', labelKey: 'viz.mode.waterfall', needsAudio: true, stillWithoutAudio: true, load: () => import('../../vendor/tunetap/viz/plugins/waterfall.js') });
  registry.register({ id: 'cassette', labelKey: 'viz.mode.cassette', needsAudio: false, stillWithoutAudio: false, words: 'none', load: () => import('../../vendor/tunetap/viz/plugins/cassette.js') });
}

// Text: the page's string table (strings.js, viz.* keys = tunetap's, with Nano's "follow the track" wording), in the language
// i18n.js has chosen. {0} {1} placeholders as in tunetap's t(). The global is set by the classic scripts strings.js + i18n.js.
import { I18N } from '../core/i18n.js';
export function t(key, ...args) { return I18N.t(key, args); }

// The page's CSS variables under the names themes.js themeFromTokens expects (--bg --card --line --fg --muted --accent).
// Nano's --line is a translucent colour; themes.js reads rgba() without its alpha, so it is laid over --card first.
const NUM = /^\s*(?:#([0-9a-f]{3}|[0-9a-f]{6})|rgba?\(\s*([\d.]+)[\s,]+([\d.]+)[\s,]+([\d.]+)(?:[\s,/]+([\d.]+))?)/i;
function rgbaOf(s) {
  const m = NUM.exec(s || '');
  if (!m) return null;
  if (m[1]) { const h = m[1].length === 3 ? [...m[1]].map((c) => c + c).join('') : m[1]; return [0, 2, 4].map((i) => parseInt(h.slice(i, i + 2), 16)).concat(1); }
  return [+m[2], +m[3], +m[4], m[5] === undefined ? 1 : +m[5]];
}
function nanoTheme(root) {
  const cs = getComputedStyle(root);
  const css = (k) => cs.getPropertyValue(k).trim();
  const map = { '--muted': '--mut', '--accent': '--acc' };
  return themeFromTokens((k) => {
    if (k === '--line') {
      const l = rgbaOf(css('--line')), c = rgbaOf(css('--card'));
      if (!l || !c) return '';
      const a = l[3];
      return `rgb(${[0, 1, 2].map((i) => Math.round(l[i] * a + c[i] * (1 - a))).join(',')})`;
    }
    return css(map[k] || k);
  });
}

const baseName = (s) => String(s || '').replace(/\.[A-Za-z0-9]{1,5}$/, '');

export function createNanoViz({ getBytes }) {
  let state = null, polledAt = 0;
  const mq = (q) => { try { return window.matchMedia(q); } catch { return null; } };
  const reducedMq = mq('(prefers-reduced-motion: reduce)');
  const tap = createBandTap(getBytes);
  const store = {
    // the host's keys are tunetap.viz.*; Nano keeps its own under nsn.viz.*
    getItem: (k) => { try { return localStorage.getItem(k.replace(/^tunetap\.viz\./, 'nsn.viz.')); } catch { return null; } },
    setItem: (k, v) => { try { localStorage.setItem(k.replace(/^tunetap\.viz\./, 'nsn.viz.'), v); } catch { /* not kept */ } },
  };
  const host = createVizHost({
    doc: document,
    storage: store,
    raf: (fn) => window.requestAnimationFrame(fn),
    caf: (id) => window.cancelAnimationFrame(id),
    now: () => performance.now(),
    getTheme: () => nanoTheme(document.documentElement),
    getAudio: () => tap,
    visible: () => !document.hidden,
    playing: () => state?.st === 'playing',
    station: () => state ? (state.file || state.title || '') : '',
    title: () => (state?.title ? String(state.title) : null),
    stationName: () => baseName(state?.file || state?.title),
    // the box reports the position once a second; the cassette's counter runs on between the reports
    playTime: () => state ? (+state.pos || 0) + (state.st === 'playing' ? Math.min(2000, performance.now() - polledAt) : 0) : 0,
    reducedMotion: () => !!reducedMq?.matches,
    dpr: () => window.devicePixelRatio || 1,
    registry,
  });
  mq('(prefers-color-scheme: dark)')?.addEventListener?.('change', () => host.update());
  return {
    host, t,
    setState(s) { state = s; polledAt = performance.now(); host.update(); },
  };
}
