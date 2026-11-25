// SPDX-License-Identifier: LGPL-2.1-or-later
// NightStand Nano: language choice, t() and the DOM pass. A classic script (loaded after strings.js) that sets
// globalThis.NSN_I18N, so it works from file:// too; the ES modules (viz-nano*.js, radio-nano.js) read the same global.
//
// Language: ?lang=en|zh-TW|zh-CN (this load only) > localStorage "nsn.lang" > navigator.languages > en.
//   Browser: the first listed language that is English or Chinese decides; zh-Hant / zh-TW / zh-HK / zh-MO -> zh-TW,
//   zh-Hans / zh-CN / zh-SG / plain zh -> zh-CN; a list with neither -> en.
// Markup: data-i18n (text), data-i18n-aria, data-i18n-placeholder, data-i18n-title (attributes); data-i18n-p holds a JSON
//   object of {name} parameters for any of them. The HTML carries the zh-TW text as its pre-load default.
// Code: t(key, params) for text, setT(node, key, params) for text that must follow later language changes, setRaw(node, text)
//   for text that must not (it drops the key). A parameter value {t: key, p: params} is itself translated.
import { STRINGS } from "./strings.js";

const g = globalThis;
  const KEY = "nsn.lang";
  const LANGS = ["en", "zh-TW", "zh-CN"];
  const CHAIN = { en: ["en"], "zh-TW": ["zh-TW", "en"], "zh-CN": ["zh-CN", "zh-TW", "en"] };
  let lang = "zh-TW", urlLang = null;
  const listeners = [];
  const table = () => STRINGS;

  // "zh-hk" -> "zh-TW", "zh-Hans-SG" -> "zh-CN", "en-GB" -> "en", anything else -> null
  function normalize(tag) {
    const s = String(tag || "").trim().replace(/_/g, "-").toLowerCase();
    if (s === "en" || s.startsWith("en-")) return "en";
    if (s === "zh" || s.startsWith("zh-")) {
      const p = s.split("-").slice(1);
      if (p.includes("hans")) return "zh-CN";
      if (p.includes("hant")) return "zh-TW";
      return p.some((x) => x === "tw" || x === "hk" || x === "mo") ? "zh-TW" : "zh-CN";
    }
    return null;
  }
  function exact(tag) { const s = String(tag || "").toLowerCase(); return LANGS.find((l) => l.toLowerCase() === s) || null; }
  function detect(list) {
    for (const tag of list || []) { const l = normalize(tag); if (l) return l; }
    return "en";
  }
  const browserLangs = () => (g.navigator && (g.navigator.languages && g.navigator.languages.length ? g.navigator.languages : [g.navigator.language])) || [];
  function saved() { try { return exact(g.localStorage.getItem(KEY)); } catch { return null; } }

  function has(key) { const row = table()[key]; return !!row && CHAIN[lang].some((l) => typeof row[l] === "string"); }
  function t(key, params) {
    const row = table()[key];
    let s = null;
    if (row) for (const l of CHAIN[lang]) if (typeof row[l] === "string") { s = row[l]; break; }
    if (s === null) return String(key);
    if (params === undefined || params === null) return s;
    return s.replace(/\{(\w+)\}/g, (m, name) => {
      if (!Object.prototype.hasOwnProperty.call(params, name)) return m;
      const v = params[name];
      return v && typeof v === "object" && "t" in v ? t(v.t, v.p) : String(v);
    });
  }

  function paramsOf(el) {
    const p = el.getAttribute("data-i18n-p");
    if (!p) return undefined;
    try { return JSON.parse(p); } catch { return undefined; }
  }
  const ATTRS = [["data-i18n-aria", "aria-label"], ["data-i18n-placeholder", "placeholder"], ["data-i18n-title", "title"]];
  function applyDom(root) {
    root = root || g.document;
    const doc = g.document;
    if (root === doc) {
      doc.documentElement.lang = lang;
      doc.title = t("app.title");
    }
    for (const el of root.querySelectorAll("[data-i18n]")) el.textContent = t(el.getAttribute("data-i18n"), paramsOf(el));
    for (const [from, to] of ATTRS) for (const el of root.querySelectorAll("[" + from + "]")) el.setAttribute(to, t(el.getAttribute(from), paramsOf(el)));
  }
  function setT(node, key, params) {
    node.setAttribute("data-i18n", key);
    if (params) node.setAttribute("data-i18n-p", JSON.stringify(params)); else node.removeAttribute("data-i18n-p");
    node.textContent = t(key, params);
  }
  function setRaw(node, text) {
    node.removeAttribute("data-i18n"); node.removeAttribute("data-i18n-p");
    node.textContent = text;
  }

  function init() {
    let u = null;
    try { u = normalize(new URLSearchParams(g.location.search).get("lang")); } catch { /* no URL */ }
    urlLang = u;
    lang = u || saved() || detect(browserLangs());
    if (g.document) { applyDom(g.document); }
    return lang;
  }
  // The select's value: the language in force when it was forced (URL or saved), else "auto".
  const pref = () => urlLang || saved() || "auto";
  // choice: "en" | "zh-TW" | "zh-CN" (remembered), or "auto" (forget it, follow the browser again)
  function setLang(choice) {
    urlLang = null;
    if (choice === "auto") {
      try { g.localStorage.removeItem(KEY); } catch { /* not kept */ }
      lang = detect(browserLangs());
    } else {
      const l = exact(choice);
      if (!l) return;
      lang = l;
      try { g.localStorage.setItem(KEY, l); } catch { /* not kept */ }
    }
    if (g.document) applyDom(g.document);
    for (const fn of listeners.slice()) { try { fn(lang); } catch (e) { if (g.console) g.console.error(e); } }
  }
  function onChange(fn) { listeners.push(fn); }

  export const I18N = g.NSN_I18N = { LANGS, t, has, init, setLang, pref, applyDom, setT, setRaw, onChange, lang: () => lang, detect, normalize };
