// SPDX-License-Identifier: LGPL-2.1-or-later
import test from "node:test";
import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { STRINGS as S } from "../src/js/core/strings.js";
import { I18N } from "../src/js/core/i18n.js";
import { TRAD } from "../testlib/trad.js";

const LANGS = ["en", "zh-TW", "zh-CN"], keys = Object.keys(S);
const ph = (s) => (s.match(/\{\w+\}/g) || []).sort().join(",");

test("string table: three languages for every key, same placeholders", () => {
  assert.ok(keys.length > 100);
  for (const k of keys) {
    for (const l of LANGS) assert.ok(typeof S[k][l] === "string" && S[k][l].length > 0, k + " lacks " + l);
    assert.equal(new Set(LANGS.map((l) => ph(S[k][l]))).size, 1, "placeholders differ: " + k);
  }
});
test("no Traditional characters left in zh-CN, no CJK in en, zh-CN differs from zh-TW where it should", () => {
  for (const k of keys) {
    const hit = [...S[k]["zh-CN"]].filter((c) => TRAD.includes(c));
    assert.deepEqual(hit, [], "zh-CN has Traditional characters: " + k);
    assert.ok(!/[一-鿿]/.test(S[k].en), "en contains CJK: " + k);
    if ([...S[k]["zh-TW"]].some((c) => TRAD.includes(c))) assert.notEqual(S[k]["zh-CN"], S[k]["zh-TW"], "zh-CN == zh-TW: " + k);
  }
});
test("every key the sources use exists", () => {
  const files = ["src/index.html", "src/js/ui/app.js", "src/js/ui/flashtab.js", "src/js/ui/hwtab.js"];
  const dyn = ["fl.part.", "fl.err.", "hw.sd.state.", "hw.src.", "hw.why.", "hw.field.", "hw.iss.", "hw.other.", "hw.width.", "hw.res.", "fl.speed.fb."];
  for (const f of files) {
    const src = readFileSync(new URL("../" + f, import.meta.url), "utf8");
    for (const m of src.matchAll(/(?:data-i18n(?:-aria|-placeholder|-title)?="|\bt\(\s*")([\w.]+)"/g)) assert.ok(S[m[1]] || dyn.some((p) => m[1].startsWith(p)), "missing key " + m[1] + " in " + f);
  }
});

test("language rules: ?lang > nsn.lang > browser; browser mapping", () => {
  const store = new Map();
  globalThis.localStorage = { getItem: (k) => (store.has(k) ? store.get(k) : null), setItem: (k, v) => store.set(k, v), removeItem: (k) => store.delete(k) };
  Object.defineProperty(globalThis, "navigator", { value: { languages: ["fr-FR", "zh-HK", "en"] }, configurable: true });
  globalThis.location = { search: "" };
  assert.equal(I18N.init(), "zh-TW");                                   // first listed language that is English or Chinese
  navigatorLangs(["zh-CN"]); assert.equal(I18N.init(), "zh-CN");
  navigatorLangs(["zh"]); assert.equal(I18N.init(), "zh-CN");           // plain zh -> simplified, as on the control page
  navigatorLangs(["zh-Hant-TW"]); assert.equal(I18N.init(), "zh-TW");
  navigatorLangs(["de"]); assert.equal(I18N.init(), "en");
  I18N.setLang("zh-TW"); assert.equal(store.get("nsn.lang"), "zh-TW");  // the same key as the control page
  navigatorLangs(["en"]); assert.equal(I18N.init(), "zh-TW");           // the saved choice beats the browser
  globalThis.location = { search: "?lang=en" }; assert.equal(I18N.init(), "en");   // the URL beats the saved choice
  assert.equal(store.get("nsn.lang"), "zh-TW");                         // ... for this load only
  globalThis.location = { search: "" };
  I18N.setLang("auto"); assert.equal(store.has("nsn.lang"), false);
  assert.equal(I18N.t("btn.connect"), "Connect");
  I18N.setLang("zh-CN"); assert.equal(I18N.t("btn.connect"), "连接");
  assert.equal(I18N.t("conn.on", { port: "X" }), "已连接：X");
  assert.equal(I18N.t("no.such.key"), "no.such.key");
  I18N.setLang("auto");
  function navigatorLangs(l) { Object.defineProperty(globalThis, "navigator", { value: { languages: l }, configurable: true }); }
});
