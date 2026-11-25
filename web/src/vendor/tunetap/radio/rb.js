// SPDX-License-Identifier: LGPL-2.1-or-later
// radio-browser.info client (host rotation, last-good memory) and stream picking.
// fetch, storage and timers are injected. API data is untrusted: toItem only emits valid Items.
import { isUuid } from './route.js';

const BUILTIN = [
  'all.api.radio-browser.info', 'de1.api.radio-browser.info', 'de2.api.radio-browser.info',
  'nl1.api.radio-browser.info', 'at1.api.radio-browser.info', 'fi1.api.radio-browser.info',
];
const HOST_RE = /^[a-z0-9-]+(\.[a-z0-9-]+)*\.radio-browser\.info$/i;
// Namespaced: other apps on the same github.io origin share this localStorage.
const LAST_KEY = 'tunetap.rb.last';
const TIMEOUT_MS = 8000;
// Per call: at most MAX_TRIES hosts, and give up after DEADLINE_MS overall.
export const MAX_TRIES = 3;
const DEADLINE_MS = 15000;
// Caps mirror the sanitizer in fav.js.
const MAX_URLS = 3;
const MAX_URL_LEN = 500;
const ORDERS = new Set(['name', 'votes', 'clickcount', 'clicktrend', 'bitrate', 'lastcheckok', 'random']);
const REVERSE = new Set(['votes', 'clickcount', 'clicktrend', 'bitrate']);
const LIST_KINDS = new Set(['tags', 'countries', 'languages']);

const isHost = (h) => typeof h === 'string' && HOST_RE.test(h);
const str = (v, max) => (typeof v === 'string' ? v.slice(0, max) : '');

function httpUrl(s) {
  if (typeof s !== 'string' || s.length > MAX_URL_LEN || /[\u0000- \u007f]/.test(s)) return null;
  try {
    const u = new URL(s);
    return (u.protocol === 'https:' || u.protocol === 'http:') && u.hostname ? s : null;
  } catch {
    return null;
  }
}

// Same stream, ignoring scheme, trailing slash and host case.
export function streamKey(s) {
  return String(s).trim().replace(/^[a-z][a-z0-9+.-]*:\/\//i, '').replace(/\/+$/, '').replace(/^[^/?#]+/, (h) => h.toLowerCase());
}

function streamKind(url) {
  let p;
  try { p = new URL(url).pathname.toLowerCase(); } catch { return 'ok'; }
  if (p.endsWith('.pls') || p.endsWith('.m3u')) return 'playlist';
  if (p.endsWith('.m3u8')) return 'hls';
  return 'ok';
}

// Candidate URLs in preference order (url_resolved first), valid http(s) only, de-duplicated.
function candidates(st) {
  if (!st || typeof st !== 'object') return [];
  const out = [];
  for (const c of [st.url_resolved, st.url]) {
    const u = httpUrl(c);
    if (u && !out.includes(u)) out.push(u);
  }
  return out;
}

// An HTTPS page cannot play http audio, so https wins; http-only stations report 'http-only'.
export function pickStream(station) {
  const all = candidates(station);
  const https = all.filter((u) => u.startsWith('https:'));
  if (!all.length) return { url: null, kind: 'none', alts: [] };
  if (!https.length) return { url: null, kind: 'http-only', alts: all };
  const kinds = https.map(streamKind);
  let i = kinds.indexOf('ok');
  if (i < 0) i = 0;
  return { url: https[i], kind: kinds[i], alts: https.filter((_, j) => j !== i) };
}

// https candidates for http-only streams (an HTTPS page cannot play http audio). Best effort: per http URL,
// the same host and port over https, then (explicit non-443 port only) the default port. Never throws.
const MAX_UPGRADES = 4;
export function httpsUpgrades(urls) {
  const out = [];
  if (!Array.isArray(urls)) return out;
  for (const s of urls) {
    if (typeof s !== 'string' || s.length > MAX_URL_LEN) continue;
    let u;
    try { u = new URL(s); } catch { continue; }
    if (u.protocol !== 'http:' || !u.hostname) continue;
    const same = new URL(u.href);
    same.protocol = 'https:';
    const cands = [same.href];
    if (same.port && same.port !== '443') {
      const dflt = new URL(same.href);
      dflt.port = '';
      cands.push(dflt.href);
    }
    for (const c of cands) if (!out.includes(c)) out.push(c);
  }
  return out.slice(0, MAX_UPGRADES);
}

// Returns a valid Item, or null when the station is unusable (bad uuid, no name, no http(s) URL).
export function toItem(station) {
  if (!station || typeof station !== 'object' || Array.isArray(station)) return null;
  if (!isUuid(station.stationuuid)) return null;
  const name = str(station.name, 120).trim();
  if (!name) return null;
  const all = candidates(station);
  const urls = [...all.filter((u) => u.startsWith('https:')), ...all.filter((u) => !u.startsWith('https:'))].slice(0, MAX_URLS);
  if (!urls.length) return null;
  const fav = httpUrl(station.favicon);
  return {
    id: station.stationuuid,
    name,
    urls,
    icon: fav && fav.startsWith('https:') ? fav : '',
    tags: str(station.tags, 200),
    country: str(station.country, 60),
    added: 0,
  };
}

export function createRb({ fetch, storage, setTimeout: setT = globalThis.setTimeout, clearTimeout: clearT = globalThis.clearTimeout } = {}) {
  const read = (k) => { try { return storage.getItem(k) || ''; } catch { return ''; } };
  const write = (k, v) => { try { storage.setItem(k, v); } catch { /* ignore */ } };

  let last = read(LAST_KEY);
  if (!isHost(last)) last = '';
  let hosts = BUILTIN.slice();
  let discovered = false;
  // Hosts that failed since the last success, oldest first. They go to the back of the
  // order so consecutive failed calls rotate through every host. Reset on success.
  let failed = [];

  // The timer covers the body read too, so a stalled body cannot hang rotation.
  // `outer` (optional) is the call-wide deadline signal.
  async function getJson(host, path, outer) {
    const ctl = new AbortController();
    const t = setT(() => ctl.abort(), TIMEOUT_MS);
    const onOuter = () => ctl.abort();
    if (outer) { if (outer.aborted) ctl.abort(); else outer.addEventListener('abort', onOuter); }
    try {
      const r = await fetch(`https://${host}/json/${path}`, { signal: ctl.signal });
      if (!r || !r.ok) throw new Error('http');
      return await r.json();
    } finally {
      clearT(t);
      if (outer) outer.removeEventListener('abort', onOuter);
    }
  }

  async function discover(host) {
    if (discovered) return;
    discovered = true;
    try {
      const list = await getJson(host, 'servers');
      if (!Array.isArray(list)) return;
      const names = [...new Set(list.map((x) => (x && typeof x.name === 'string' ? x.name.toLowerCase() : '')).filter(isHost))];
      if (!names.length) return;
      hosts = ['all.api.radio-browser.info', ...names.filter((h) => h !== 'all.api.radio-browser.info')];
    } catch { /* keep the built-in list */ }
  }

  // persist:false keeps the last-good-host memory out of storage (the on-demand details lookup writes nothing).
  async function call(path, { persist = true } = {}) {
    const fresh = [...(last ? [last] : []), ...hosts].filter((h, i, a) => a.indexOf(h) === i && !failed.includes(h));
    const order = [...fresh, ...failed].slice(0, MAX_TRIES);
    const dl = new AbortController();
    const d = setT(() => dl.abort(), DEADLINE_MS);
    try {
      for (const h of order) {
        if (dl.signal.aborted) break;
        try {
          const j = await getJson(h, path, dl.signal);
          failed = [];
          if (h !== last) { last = h; if (persist) write(LAST_KEY, h); }
          discover(h);
          return j;
        } catch {
          failed = [...failed.filter((x) => x !== h), h];
          // A failing remembered host must not pin the order.
          if (h === last) { last = ''; if (persist) write(LAST_KEY, ''); }
        }
      }
      throw new Error('rb-unreachable');
    } finally {
      clearT(d);
    }
  }

  return {
    async search(p = {}) {
      const q = new URLSearchParams();
      for (const k of ['name', 'tag', 'language']) {
        if (typeof p[k] === 'string' && p[k]) q.set(k, p[k].slice(0, 100));
      }
      if (typeof p.country === 'string' && p.country) {
        if (/^[a-z]{2}$/i.test(p.country)) q.set('countrycode', p.country.toUpperCase());
        else q.set('country', p.country.slice(0, 100));
      }
      const order = ORDERS.has(p.order) ? p.order : 'clickcount';
      q.set('order', order);
      if (REVERSE.has(order)) q.set('reverse', 'true');
      const off = Number.isInteger(p.offset) && p.offset > 0 ? p.offset : 0;
      const lim = Number.isInteger(p.limit) && p.limit > 0 ? Math.min(p.limit, 200) : 50;
      q.set('offset', String(off));
      q.set('limit', String(lim));
      q.set('hidebroken', 'true');
      const r = await call('stations/search?' + q);
      return Array.isArray(r) ? r.filter((x) => x && typeof x === 'object') : [];
    },
    async byUuid(uuid) {
      if (!isUuid(uuid)) return null;
      const r = await call('stations/byuuid/' + uuid);
      const s = Array.isArray(r) ? r[0] : null;
      return s && typeof s === 'object' ? s : null;
    },
    // Lookup by stream URL (the NFC page; user-initiated only). One request, no filters. Exact match
    // (trailing slash and http/https ignored) wins, else the first answer. null: nothing found or bad URL.
    async byUrl(url) {
      if (!httpUrl(url)) return null;
      const r = await call('stations/byurl?' + new URLSearchParams({ url }), { persist: false });
      const list = Array.isArray(r) ? r.filter((x) => x && typeof x === 'object' && !Array.isArray(x)) : [];
      const key = streamKey(url);
      return list.find((s) => [s.url_resolved, s.url].some((c) => typeof c === 'string' && streamKey(c) === key)) || list[0] || null;
    },
    async lists(kind) {
      if (!LIST_KINDS.has(kind)) throw new Error('bad-kind');
      const r = await call(`${kind}?hidebroken=true&order=stationcount&reverse=true`);
      if (!Array.isArray(r)) return [];
      return r
        .filter((x) => x && typeof x.name === 'string' && x.name)
        .map((x) => ({ name: x.name, stationcount: Number.isFinite(x.stationcount) ? x.stationcount : 0 }));
    },
    hosts: () => hosts.slice(),
    last: () => last,
  };
}
