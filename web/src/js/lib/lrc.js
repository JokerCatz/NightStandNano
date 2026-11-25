// SPDX-License-Identifier: LGPL-2.1-or-later
// NightStand Nano: LRC lyrics. The box sends the raw .lrc once (`lrc` -> `lrc begin <n>`, `lrc+ <base64>` ..., `lrc end`, or `lrc none`);
// this file decodes it and parses it with the rules of the owner's other player (ESP32S31_round components/player_core pc_lrc.h):
//   "[mm:ss.xx]text", any number of time tags before one text ([mm:ss] [mm:ss.x] [mm:ss.xx] [mm:ss.xxx] [mm:ss:xx], mm 1-3 digits),
//   "[offset:+-ms]" (positive = lyrics show that much earlier), other tags ignored, "<mm:ss.xx>" word times removed, CR LF / LF / CR end a line,
//   a time tag with no text is kept (a pause), lines without a time tag are ignored, output sorted by time (stable).
//   Limits as the firmware's struct: 64 KB read, 400 lines, 24576 bytes of text, 160 bytes per line (cut at a character).

export const MAX_FILE = 65536, MAX_LINES = 400, TEXT_BYTES = 24576, LINE_BYTES = 160;
const TIME = /^(\d{1,3}):(\d{2})(?:[.:](\d{1,3}))?$/;
const OFFSET = /^offset:\s*([+-]?\d+)\s*$/i;
const WORD = /<\d{1,3}:\d{2}(?:[.:]\d{1,3})?>/g;
const utf8Len = (s) => new TextEncoder().encode(s).length;

function cutBytes(s, max) {
  if (utf8Len(s) <= max) return s;
  let out = "", n = 0;
  for (const ch of s) { const b = utf8Len(ch); if (n + b > max) break; out += ch; n += b; }
  return out;
}

// text -> { lines: [{ ms, text }], truncated }
export function parseLrc(text) {
  const raw = [];
  let offset = 0;
  for (const src of text.split(/\r\n|\n|\r/)) {
    let rest = src, times = [];
    for (;;) {
      const m = /^\s*\[([^\]]*)\]/.exec(rest);
      if (!m) break;
      rest = rest.slice(m[0].length);
      const t = TIME.exec(m[1]);
      if (t) {
        const f = t[3] === undefined ? 0 : t[3].length === 1 ? +t[3] * 100 : t[3].length === 2 ? +t[3] * 10 : +t[3];
        times.push((+t[1] * 60 + +t[2]) * 1000 + f);
      } else { const o = OFFSET.exec(m[1]); if (o) offset = parseInt(o[1], 10); }
    }
    if (!times.length) continue;
    const txt = rest.replace(WORD, "").trim();
    for (const ms of times) raw.push({ ms, text: txt });
  }
  const lines = raw.map((l) => ({ ms: l.ms - offset, text: l.text })).sort((a, b) => a.ms - b.ms);   // Array.sort is stable
  let truncated = false, bytes = 0;
  const out = [];
  for (const l of lines) {
    if (out.length >= MAX_LINES) { truncated = true; break; }
    let t = cutBytes(l.text, LINE_BYTES); if (t !== l.text) truncated = true;
    const b = utf8Len(t);
    if (bytes + b > TEXT_BYTES) { truncated = true; break; }
    bytes += b; out.push({ ms: l.ms, text: t });
  }
  return { lines: out, truncated };
}

// index of the last line whose time <= posMs, -1 before the first (or no lines)
export function findLine(lines, posMs) {
  let lo = 0, hi = lines.length;
  while (lo < hi) { const mid = (lo + hi) >> 1; if (lines[mid].ms <= posMs) lo = mid + 1; else hi = mid; }
  return lo - 1;
}

// bytes -> string: UTF-8 (BOM skipped), else Big5, else GBK
export function decodeLrc(bytes) {
  if (bytes.length >= 3 && bytes[0] === 0xEF && bytes[1] === 0xBB && bytes[2] === 0xBF) bytes = bytes.subarray(3);
  for (const enc of ["utf-8", "big5", "gbk"]) {
    try { return new TextDecoder(enc, { fatal: true }).decode(bytes); } catch { /* next */ }
  }
  return new TextDecoder("gbk").decode(bytes);   // nothing decodes cleanly: show what we can
}

// reply lines of the `lrc` command -> null (no lyrics file / unreadable) or { lines, truncated, enc bytes }
export function parseReply(replyLines) {
  if (replyLines.some((l) => l.startsWith("lrc none"))) return null;
  const b = replyLines.find((l) => l.startsWith("lrc begin"));
  if (!b) return null;
  const want = parseInt(b.slice(9), 10) || 0;
  const chunks = [];
  for (const l of replyLines) if (l.startsWith("lrc+ ")) {
    try { const s = atob(l.slice(5).trim()); const u = new Uint8Array(s.length); for (let i = 0; i < s.length; i++) u[i] = s.charCodeAt(i); chunks.push(u); } catch { return null; }
  }
  const all = new Uint8Array(chunks.reduce((n, c) => n + c.length, 0));
  let o = 0; for (const c of chunks) { all.set(c, o); o += c.length; }
  if (!all.length || (want && all.length < want && !replyLines.includes("lrc end"))) return null;   // cut short
  const r = parseLrc(decodeLrc(all.subarray(0, MAX_FILE)));
  return r.lines.length ? r : null;
}
