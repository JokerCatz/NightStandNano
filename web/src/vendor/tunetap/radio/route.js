// SPDX-License-Identifier: LGPL-2.1-or-later
// Pure hash router and URL validation. Never throws on any input.

const UUID_RE = /^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/i;
const MAX_URL = 2000;
const MAX_TITLE = 100;
// eslint-disable-next-line no-control-regex
const BAD_CHARS = /[\u0000- \u007f]/;
// C0 controls, DEL, C1 controls, LRM/RLM, bidi embeddings/overrides (U+202A-202E) and isolates (U+2066-2069), lone surrogates.
// eslint-disable-next-line no-control-regex
const STRIP = /[\u0000-\u001f\u007f-\u009f\u200e\u200f\u202a-\u202e\u2066-\u2069]|[\uD800-\uDBFF](?![\uDC00-\uDFFF])|(?<![\uD800-\uDBFF])[\uDC00-\uDFFF]/g;

// Untrusted display text (link titles, ICY stream titles): controls and bidi overrides dropped,
// whitespace runs collapsed to one space, trimmed, truncated by code point (never splits a pair).
export function cleanText(s, max) {
  if (typeof s !== 'string') return '';
  return Array.from(s.replace(STRIP, '').replace(/\s+/g, ' ').trim()).slice(0, max).join('').trim();
}

export function isUuid(s) {
  return typeof s === 'string' && UUID_RE.test(s);
}

export function isHttpsUrl(s) {
  if (typeof s !== 'string' || s.length > MAX_URL || BAD_CHARS.test(s)) return false;
  try {
    const u = new URL(s);
    return u.protocol === 'https:' && u.hostname !== '';
  } catch {
    return false;
  }
}

// Scheme-less input gets https://; any explicit non-https scheme is rejected.
export function normalizeStreamUrl(u) {
  if (typeof u !== 'string') return null;
  const s = u.trim();
  if (!s || s.length > MAX_URL || BAD_CHARS.test(s)) return null;
  // "host:8000/x" is scheme-less; "javascript:x" and "http://x" are explicit schemes.
  const explicit = /^[a-z][a-z0-9+.-]*:(?!\d+(?:[/?#]|$))/i.test(s);
  if (explicit && !/^https:\/\//i.test(s)) return null;
  const full = explicit ? s : 'https://' + s.replace(/^\/\//, '');
  try {
    const p = new URL(full);
    if (p.protocol !== 'https:' || !p.hostname || p.username || p.password) return null;
    return p.href;
  } catch {
    return null;
  }
}

// Two 32-bit FNV-1a variants -> 16 hex chars.
export function stationId(url) {
  const s = normalizeStreamUrl(url) ?? String(url);
  let a = 0x811c9dc5;
  let b = 0x01000193;
  for (let i = 0; i < s.length; i++) {
    const c = s.charCodeAt(i);
    a = Math.imul(a ^ c, 0x01000193) >>> 0;
    b = Math.imul(b ^ c, 0x85ebca6b) >>> 0;
    b ^= b >>> 13;
  }
  return 'u-' + a.toString(16).padStart(8, '0') + (b >>> 0).toString(16).padStart(8, '0');
}

const FIND = () => ({ name: 'find' });

export function parseHash(hash) {
  try {
    if (typeof hash !== 'string' || hash.length > 5000) return FIND();
    const h = hash.startsWith('#') ? hash.slice(1) : hash;
    const q = h.indexOf('?');
    const path = q < 0 ? h : h.slice(0, q);
    const query = q < 0 ? '' : h.slice(q + 1);
    const seg = path.split('/').filter((x, i) => i > 0 || x !== '');
    if (seg[0] === 'fav' && seg.length === 1) return { name: 'fav' };
    if (seg[0] === 'play' && seg.length === 2 && isUuid(seg[1])) {
      return { name: 'play', uuid: seg[1] };
    }
    if ((seg[0] === 'play' || seg[0] === 'p') && seg.length === 1) {
      const params = new URLSearchParams(query);
      const url = normalizeStreamUrl(params.get('u') ?? '');
      if (!url) return FIND();
      const out = { name: 'play', url };
      const n = params.get('n');
      if (n) {
        const title = cleanText(n, MAX_TITLE);
        if (title) out.title = title;
      }
      return out;
    }
  } catch {
    // fall through
  }
  return FIND();
}
