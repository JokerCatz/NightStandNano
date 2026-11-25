// SPDX-License-Identifier: LGPL-2.1-or-later
// Pure helpers for the Find screen (no DOM): rb query per view, station -> row, filtering, paging.
import { pickStream, toItem } from './rb.js';

export const PAGE = 30;
const MAY_NOT = new Set(['http-only', 'playlist', 'hls']);

// state: {view:'popular'|'genre'|'region'|'language'|'search', value}. Returns rb.search params or null.
export function buildQuery(state, offset) {
  const value = typeof state?.value === 'string' ? state.value.trim() : '';
  const base = { order: 'clickcount', limit: PAGE, offset };
  switch (state?.view) {
    case 'popular': return value ? { country: value, ...base } : base;
    case 'genre': return value ? { tag: value, ...base } : null;
    case 'region': return value ? { country: value, ...base } : null;
    case 'language': return value ? { language: value, ...base } : null;
    case 'search': return value ? { name: value, ...base } : null;
    default: return null;
  }
}

// One result row: the validated Item plus display-only fields; null when the station is unusable.
export function toRow(station) {
  const item = toItem(station);
  if (!item) return null;
  const pick = pickStream(station);
  const codec = typeof station.codec === 'string' ? station.codec.replace(/[^A-Za-z0-9.+ -]/g, '').slice(0, 12) : '';
  const bitrate = Number.isInteger(station.bitrate) && station.bitrate > 0 && station.bitrate < 100000 ? station.bitrate : 0;
  // Soft warning only, never a verdict: http-only is tried over https, playlist and HLS (native in Safari
  // only) are just tried. Rows stay tappable.
  return { item, kind: pick.kind, mayNotPlay: MAY_NOT.has(pick.kind), codec, bitrate };
}

export const visibleRows = (rows, hideMayNot) => (hideMayNot ? rows.filter((r) => !r.mayNotPlay) : rows);

// More pages may exist when the raw (unfiltered) page was full.
export const hasMore = (rawCount) => rawCount >= PAGE;

export function metaText(row) {
  return [row.item.country, row.codec, row.bitrate ? `${row.bitrate} kbps` : ''].filter(Boolean).join(' · ');
}

// Enter submits only when it is a real Enter, not the one that confirms an IME candidate.
export const isSubmitKey = (e) => e?.key === 'Enter' && !e.isComposing && e.keyCode !== 229;
