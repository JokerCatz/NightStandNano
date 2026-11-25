// SPDX-License-Identifier: LGPL-2.1-or-later
// NightStand Nano: the Radio page's station finder and the favourites list, built on tunetap's modules in vendor/tunetap/radio/ (copied unmodified):
//   radio/rb.js          radio-browser client (host rotation), streamKey()
//   radio/find-model.js  paging constants, Enter-key check
//   radio/regions.js     country / genre tables
//   radio/route.js       stationId() (tunetap's "u-<hash>" ids, used for the JSON export)
// tunetap's UI files (ui-find.js, ui-fav.js) are tied to its router, player, i18n and icon sprite, so this file is a small
// Nano version of them. Differences: the box does the playing (http and https both fine, HLS not supported; codec and bitrate
// are shown), and favourites are NOT kept in the browser: the card's radio.txt is the list (`radio list` / `radio add` / `radio del`).
// Text comes from the page's string table (globalThis.NSN_I18N, strings.js + i18n.js): the DOM built here carries data-i18n* attributes, so
// a language change is one applyDom pass; only the country / genre names (radio/regions.js has tw / cn / en) are re-set here.
import { createRb, toItem, streamKey } from '../../vendor/tunetap/radio/rb.js';
import { hasMore, PAGE, isSubmitKey } from '../../vendor/tunetap/radio/find-model.js';
import { COUNTRIES, GENRES } from '../../vendor/tunetap/radio/regions.js';
import { stationId } from '../../vendor/tunetap/radio/route.js';

// rb.js remembers its last good host in "storage"; nothing is kept in the browser, so this one lives in memory only.
const mem = new Map();
const memStorage = { getItem: (k) => mem.get(k) ?? null, setItem: (k, v) => { mem.set(k, v); } };

// Codecs the box decodes: MP3, AAC (ADTS), Ogg Vorbis / Opus. radio-browser says "MP3", "AAC", "AAC+", "OGG", "FLAC", "UNKNOWN", ...
const CODEC_OK = /^(mp3|aac\+?|ogg|vorbis|opus|unknown|)$/i;
const isM3u8 = (u) => { try { return new URL(u).pathname.toLowerCase().endsWith('.m3u8'); } catch { return false; } };

// Which of a station's urls to send to the box: not HLS, and plain http first when there is one (no TLS on the box).
export function boxUrl(urls) {
  const ok = (urls || []).filter((u) => typeof u === 'string' && /^https?:\/\/[^\s|]+$/i.test(u) && !isM3u8(u));
  return ok.find((u) => u.startsWith('http:')) || ok[0] || null;
}

// One result row. hls: the box cannot play it at all; maybe: a soft warning (codec it may not decode, a playlist file).
export function toRow(station) {
  const item = toItem(station);
  if (!item) return null;
  const url = boxUrl(item.urls);
  const codec = typeof station.codec === 'string' ? station.codec.replace(/[^A-Za-z0-9.+ -]/g, '').slice(0, 12) : '';
  const bitrate = Number.isInteger(station.bitrate) && station.bitrate > 0 && station.bitrate < 100000 ? station.bitrate : 0;
  const hls = station.hls === 1 || station.hls === true || !url;
  const pl = url && /\.(pls|m3u)$/i.test(new URL(url).pathname);
  return { item, url, codec, bitrate, hls, maybe: !hls && (!CODEC_OK.test(codec) || pl), scheme: url ? url.split(':')[0] : '' };
}

// the card's limits: name <= 63 UTF-8 bytes without | or control characters; url http(s)://, <= 255 bytes, no |
const bytes = (s) => new TextEncoder().encode(s).length;
export function cleanName(name, fallback) {
  let s = String(name || '').replace(/[|\u0000-\u001f\u007f-\u009f]/g, ' ').trim();
  let out = '', n = 0;
  for (const ch of s) { const b = bytes(ch); if (n + b > 63) break; out += ch; n += b; }
  out = out.trim();
  return out || (fallback ? cleanName(fallback) : '');
}
export const urlOk = (u) => /^https?:\/\/[^\s|\u0000-\u001f]+$/i.test(u) && bytes(u) <= 255;

const el = (tag, props = {}, ...kids) => {
  const e = document.createElement(tag);
  for (const [k, v] of Object.entries(props)) { if (k === "class") e.className = v; else if (k in e) e[k] = v; else e.setAttribute(k, v); }
  for (const c of kids) if (c != null) e.append(c);
  return e;
};

import { I18N } from '../core/i18n.js';
const t = I18N.t;
const NAME_KEY = { "zh-TW": "tw", "zh-CN": "cn", en: "en" };
const nameOf = (o) => o[NAME_KEY[I18N.lang()]] || o.en;
// text that follows the language: the element keeps its key (and {name} params) in data-i18n*
const k = (e, key, params) => { I18N.setT(e, key, params); return e; };
// the same for attributes: kind = "aria" | "placeholder" | "title"
const ka = (e, kind, key, params) => {
  e.setAttribute("data-i18n-" + kind, key);
  if (params) e.setAttribute("data-i18n-p", JSON.stringify(params));
  e.setAttribute(kind === "aria" ? "aria-label" : kind, t(key, params));
  return e;
};
const say = (node, key, params, bad) => { if (key) I18N.setT(node, key, params); else I18N.setRaw(node, ""); node.classList.toggle("bad", !!bad); };
const slug = (s) => String(s).toLowerCase().replace(/[^a-z]+/g, "_").replace(/^_|_$/g, "");

// mountRadio({ favBox, findBox, send, refresh, connected }): builds both sections into the two containers.
export function mountRadio({ favBox, findBox, send, refresh, connected }) {
  const rb = createRb({ fetch: window.fetch.bind(window), storage: memStorage });

  let saved = [];                       // [{ n, name, url }] from `radio list`
  const savedByKey = () => new Map(saved.map((s) => [streamKey(s.url), s]));

  async function play(url, note) {
    if (!connected()) { say(note, "rn.needConn", null, true); return; }
    say(note, "rn.sent");
    await send("radio " + url);
    say(note, null);
    refresh();
  }

  // ---- the card's list (= favourites) ----
  const favNote = el("div", { class: "mut rnote", role: "status" });
  const favUl = el("ul", { class: "rlist" });
  const file = el("input", { type: "file", hidden: true, accept: ".json,application/json" });
  const bExport = k(el("button", { type: "button", disabled: true }), "rn.export");
  const bImport = k(el("button", { type: "button" }), "rn.import");
  favBox.textContent = "";
  favBox.append(favUl, el("div", { class: "row", style: "margin-top:.5rem" }, bExport, bImport, file), favNote);

  // empty: the key of the text shown when the list is empty
  function renderSaved(empty) {
    favUl.textContent = "";
    for (const s of saved) {
      const main = el("button", { type: "button", class: "rmain" }, el("span", { class: "rname" }, s.name), el("span", { class: "rmeta" }, s.url));
      main.onclick = () => { if (!connected()) return say(favNote, "rn.needConn", null, true); play("" + s.n, favNote); };
      const del = el("button", { type: "button", class: "star" }, "✕");
      ka(del, "aria", "rn.removeAria", { name: s.name }); ka(del, "title", "rn.removeTitle");
      del.onclick = () => removeAt(s.n);
      favUl.append(el("li", { class: "rrow" }, main, del));
    }
    if (!saved.length) favUl.append(k(el("li", { class: "mut rempty" }), empty || "rn.empty"));
    bExport.disabled = !saved.length;
    renderResults();
  }
  async function reload() {
    if (!connected()) { saved = []; renderSaved("fav.hint"); return; }
    const r = await send("radio list");
    saved = [];
    for (const l of r) { const m = l.match(/^(\d+) (.*) (https?:\/\/\S+)\s*$/); if (m) saved.push({ n: +m[1], name: m[2], url: m[3] }); }
    renderSaved();
  }
  async function removeAt(n) {
    const r = await send("radio del " + n);
    const ok = r.some((l) => l.startsWith("radio del: ok"));
    await reload();
    const line = r.find((l) => l.startsWith("radio del"));
    say(favNote, ok ? "rn.removed" : "rn.removeFailed", ok ? null : { why: line || { t: "rn.noReply" } }, !ok);
  }
  // returns { ok, key, params, full }: the box's number for the station, or its refusal; key / params are for the page's text
  async function addStation(name, url) {
    if (!connected()) return { ok: false, key: "rn.needConn" };
    if (!urlOk(url)) return { ok: false, key: bytes(url) > 255 ? "rn.urlLong" : "rn.urlBad" };
    let host = url; try { host = new URL(url).hostname; } catch { /* keep */ }
    const nm = cleanName(name, host);
    const r = await send(`radio add ${nm}|${url}`);
    const l = r.find((x) => x.startsWith("radio add:")) || "";
    const m = l.match(/^radio add:\s*(\d+)/);
    if (m) return { ok: true, n: +m[1], key: "rn.saved" };
    if (!l) return { ok: false, key: "rn.devNoReply" };
    const why = l.replace(/^radio add:\s*/, ""), key = "rn.add." + slug(why);   // refused | list full | cannot write the card; else as received
    return { ok: false, key: I18N.has(key) ? key : "rn.add.other", params: { why }, full: why === "list full" };
  }

  bExport.onclick = () => {
    // tunetap's favourites format; the card only has name + url, so the rest is filled like tunetap's own custom stations
    // (id u-<hash of the url>, no icon / tags / country, added = now)
    const items = saved.map((s) => ({ id: stationId(s.url), name: s.name, urls: [s.url], icon: "", tags: "", country: "", added: Date.now() }));
    const url = URL.createObjectURL(new Blob([JSON.stringify(items, null, 2)], { type: "application/json" }));
    const a = el("a", { href: url, hidden: true }); a.download = "tunetap-favorites.json";
    document.body.append(a); a.click(); a.remove(); setTimeout(() => URL.revokeObjectURL(url), 1000);
    say(favNote, "rn.exported", { n: items.length });
  };
  bImport.onclick = () => { if (!connected()) return say(favNote, "rn.needConn", null, true); file.value = ""; file.click(); };
  file.onchange = async () => {
    const f = file.files && file.files[0]; if (!f) return;
    if (f.size > 1024 * 1024) return say(favNote, "rn.fileBig", null, true);
    let arr;
    try { arr = JSON.parse(await f.text()); if (!Array.isArray(arr) || arr.length > 1000) throw 0; } catch { return say(favNote, "rn.badFile", null, true); }
    let added = 0, refused = 0, why = null;
    for (let i = 0; i < arr.length; i++) {
      const it = arr[i], url = it && typeof it === "object" ? boxUrl(Array.isArray(it.urls) ? it.urls : []) : null;
      if (!url || typeof it.name !== "string") { refused++; continue; }
      say(favNote, "rn.importing", { i: i + 1, n: arr.length });
      const r = await addStation(it.name, url);
      if (r.ok) added++; else { refused++; why = { t: r.key, p: r.params }; if (r.full) { refused += arr.length - i - 1; break; } }
    }
    await reload();
    say(favNote, why ? "rn.importDoneWhy" : "rn.importDone", { added, refused, why }, refused > 0 && !added);
  };

  // ---- search ----
  const q = ka(ka(el("input", { type: "search", maxLength: 100, autocomplete: "off" }), "placeholder", "rn.q"), "aria", "rn.q");
  const named = (value, o) => { const e = el("option", { value }, nameOf(o)); e._nm = o; return e; };
  const country = ka(el("select"), "aria", "rn.region");
  country.append(k(el("option", { value: "" }), "rn.regionAll"), ...COUNTRIES.map((c) => named(c.code, c)));
  const tag = ka(el("select"), "aria", "rn.genre");
  tag.append(k(el("option", { value: "" }), "rn.genreAll"), ...GENRES.map((g) => named(g.id, g)));
  const codec = ka(el("select"), "aria", "rn.codec");
  codec.append(k(el("option", { value: "" }), "rn.codecAll"), el("option", { value: "mp3" }, "MP3"), el("option", { value: "aac" }, "AAC"), el("option", { value: "ogg" }, "Ogg"));
  const kbps = ka(el("select"), "aria", "rn.kbps");
  kbps.append(k(el("option", { value: "0" }), "rn.kbpsAll"), el("option", { value: "64" }, "≥ 64 kbps"), el("option", { value: "128" }, "≥ 128 kbps"), el("option", { value: "192" }, "≥ 192 kbps"));
  const go = k(el("button", { type: "button", class: "primary" }), "rn.search");
  const showHls = el("input", { type: "checkbox", id: "showHls" });
  const status = el("div", { class: "mut rnote", role: "status" });
  const ul = el("ul", { class: "rlist" });
  const more = k(el("button", { type: "button", hidden: true }), "rn.more");
  findBox.textContent = "";
  findBox.append(
    el("div", { class: "row" }, el("div", { class: "grow", style: "display:flex" }, q), go),
    el("div", { class: "rfilters" }, country, tag, codec, kbps),
    el("label", { class: "rchk", for: "showHls" }, showHls, k(el("span"), "rn.hlsToo")),
    status, ul, more);
  // a language change: only the region / genre names are looked up here; everything else carries its key
  I18N.onChange(() => { for (const sel of [country, tag]) for (const o of sel.options) if (o._nm) o.textContent = nameOf(o._nm); });

  function rowEl(r) {
    const { item } = r;
    const sv = r.url ? savedByKey().get(streamKey(r.url)) : null;
    const meta = [item.country, r.codec, r.bitrate ? r.bitrate + " kbps" : "", r.scheme].filter(Boolean).join(" · ");
    const main = el("button", { type: "button", class: "rmain", disabled: r.hls },
      el("span", { class: "rname" }, item.name),
      meta ? el("span", { class: "rmeta" }, meta) : null);
    if (r.hls) { ka(main, "title", "rn.hlsTitle"); main.append(k(el("span", { class: "rtag bad" }), "rn.hlsTag")); }
    else { if (r.maybe) main.append(k(el("span", { class: "rtag" }), "rn.maybeTag")); if (sv) main.append(k(el("span", { class: "rtag ok" }), "rn.savedTag")); }
    main.onclick = () => play(r.url, status);
    const star = el("button", { type: "button", class: "star", disabled: r.hls, "aria-pressed": String(!!sv) }, sv ? "★" : "☆");
    ka(star, "aria", sv ? "rn.unsaveAria" : "rn.saveAria", { name: item.name });
    star.onclick = async () => {
      star.disabled = true;
      if (sv) { await removeAt(sv.n); return; }
      const res = await addStation(item.name, r.url);
      await reload();
      if (res.ok) say(status, "rn.savedOne", { name: cleanName(item.name) }); else say(status, res.key, res.params, true);
    };
    return el("li", { class: "rrow" }, main, star);
  }

  let rows = [], rawCount = 0, lastCount = 0, loading = false, failed = false, token = 0, searched = false, hiddenHls = 0;
  const seen = new Set();
  const codecOf = (r) => { const c = r.codec.toLowerCase(); return c.startsWith("aac") ? "aac" : c === "mp3" ? "mp3" : (c === "ogg" || c === "vorbis" || c === "opus") ? "ogg" : c; };
  const passes = (r) => (showHls.checked || !r.hls) && (!codec.value || codecOf(r) === codec.value) && (!r.bitrate ? +kbps.value === 0 : r.bitrate >= +kbps.value);
  function renderResults() {
    const vis = rows.filter(passes);
    ul.textContent = "";
    for (const r of vis) ul.append(rowEl(r));
    hiddenHls = rows.filter((r) => r.hls && !showHls.checked).length;
    more.hidden = !hasMore(lastCount) || failed || !rows.length;
    more.disabled = loading;
    if (loading && !rows.length) say(status, "rn.searching");
    else if (failed) say(status, "rn.rbFail", null, true);
    else if (searched && !vis.length) say(status, rows.length ? "rn.noMatchHid" : "rn.noMatch", { n: rows.length });
    else if (searched) say(status, hiddenHls ? "rn.resultsHls" : "rn.results", { n: vis.length, h: hiddenHls });
    else say(status, null);
  }
  async function load(append) {
    const my = ++token;
    if (!append) { rows = []; rawCount = 0; lastCount = 0; seen.clear(); }
    searched = true; failed = false; loading = true; renderResults();
    const p = { order: "clickcount", limit: PAGE, offset: append ? rawCount : 0 };
    if (q.value.trim()) p.name = q.value.trim();
    if (country.value) p.country = country.value;
    if (tag.value) p.tag = tag.value;
    try {
      const res = await rb.search(p);
      if (my !== token) return;
      rawCount += res.length; lastCount = res.length;
      for (const st of res) { const r = toRow(st); if (r && !seen.has(r.item.id)) { seen.add(r.item.id); rows.push(r); } }
      loading = false;
    } catch { if (my !== token) return; loading = false; failed = true; }
    renderResults();
  }
  go.onclick = () => load(false);
  q.onkeydown = (e) => { if (isSubmitKey(e)) { e.preventDefault(); load(false); } };
  more.onclick = () => { if (!loading) load(true); };
  for (const s of [country, tag]) s.onchange = () => load(false);
  for (const s of [codec, kbps, showHls]) s.onchange = renderResults;

  renderSaved("fav.hint");
  if (connected()) reload();
  return {
    reload,
    // the direct-URL box: save the typed address (name = host) to the card. Returns { ok, key, params } for the page's setT().
    async addUrl(url) { const r = await addStation("", url); if (r.ok) await reload(); return r; },
  };
}
