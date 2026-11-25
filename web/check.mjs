// SPDX-License-Identifier: LGPL-2.1-or-later
// npm run check: (1) the string table (three languages, placeholders, no Traditional characters in zh-CN, every key the sources use exists,
// index.html's own zh-TW text equals the table), (2) syntax of every source file (node --check), (3) dist/ is what `npm run build` makes from src/,
// and uses relative URLs only. Exit code 1 on any problem.
import { readFileSync, readdirSync, statSync, existsSync } from "node:fs";
import { join, dirname } from "node:path";
import { fileURLToPath } from "node:url";
import { spawnSync } from "node:child_process";
import { STRINGS as S } from "./src/js/core/strings.js";
import { bundle } from "./build.mjs";

const ROOT = dirname(fileURLToPath(import.meta.url)), SRC = join(ROOT, "src");
const walk = d => readdirSync(d).flatMap(n => { const p = join(d, n); return statSync(p).isDirectory() ? walk(p) : [p]; });
let bad = 0; const say = m => { bad++; console.log("  " + m); };
const keys = Object.keys(S), LANGS = ["en", "zh-TW", "zh-CN"];

console.log("string table: " + keys.length + " keys");
for (const k of keys) for (const l of LANGS) if (typeof S[k][l] !== "string" || !S[k][l]) say(k + " lacks " + l);
const ph = s => (s.match(/\{\w+\}/g) || []).sort().join(",");
for (const k of keys) if (new Set(LANGS.map(l => ph(S[k][l]))).size > 1) say("placeholders differ: " + k);
// characters that exist only in Traditional writing (a zh-CN text that is identical to zh-TW or contains one is a missed conversion)
const TRAD = [...new Set("檔設訊藍連網體開關電選單頻譜視覺記憶儲靜預輸聲無讀寫匯場時間鐘燈詞樂請這個為與類數種顯態狀轉換後從裡還邊總錄覽應來點將稱處舊該話語說讓動區環線經結圖雜離參層廣釋據礎認錯樣機臨補償衝壓縮諧濃隨載擇優質齒銳溫權閉歡暫續終發報導號貓")];
for (const k of keys) {
  const a = S[k]["zh-TW"], b = S[k]["zh-CN"], hit = [...b].filter(c => TRAD.includes(c));
  if (hit.length) say("zh-CN has Traditional characters: " + k + " [" + [...new Set(hit)].join("") + "]");
  else if (a === b && [...a].some(c => TRAD.includes(c))) say("zh-CN == zh-TW: " + k);
  if (/[一-鿿]/.test(S[k].en)) say("en contains CJK: " + k);
}
// keys used by the sources must exist (dynamic families are checked by prefix)
const DYN = ["fx.app.", "fx.p.", "noise.", "st.", "rs.", "rn.add.", "vol.for.", "keys.preset.", "keys.btn.", "keys.g.", "keys.a.", "wifi.state.", "wifi.sig.", "wifi.src.", "time.src.", "card.reason.", "card.grade.", "hw.src.", "hw.sdstate.", "rt.state.", "rt.v.", "rt.why.", "rt.refused.", "play.err."];
const used = new Map(), add = (k, f) => { if (!used.has(k)) used.set(k, f); };
const files = walk(SRC).filter(f => /\.(js|html)$/.test(f) && !f.includes("/vendor/") && !f.endsWith("strings.js"));
for (const f of files) {
  const src = readFileSync(f, "utf8");
  for (const m of src.matchAll(/data-i18n(?:-aria|-placeholder|-title)?="([\w.]+)"/g)) add(m[1], f);
  for (const m of src.matchAll(/["'`]((?:nav|help|set|ent|cb|tr|lyr|files|amb|timer|night|radio|output|diag|eq|rn|url|fx|viz|sys|lang|mode|scenes|noise|bed|out|conn|btn|meta|now|warn|vol|vmax|fav|find|common|led|card|wifi|time|keys|cast|rg|gapless|rt|hw|about|flash|rate|link|play)\.[\w.]+)["'`]/g)) add(m[1], f);
}
for (const [k, f] of used) if (!S[k] && !DYN.some(p => k === p.slice(0, -1) || k.startsWith(p))) say("used but not in the table: " + k + " (" + f.slice(ROOT.length + 1) + ")");
const html = readFileSync(join(SRC, "index.html"), "utf8").replace(/<script[\s\S]*?<\/script>/g, "");
for (const m of html.matchAll(/<(\w+)[^>]*\sdata-i18n="([\w.]+)"[^>]*>([^<]*)</g)) { const want = S[m[2]] && S[m[2]]["zh-TW"]; if (want !== undefined && want !== m[3].trim()) say("index.html text != table: " + m[2] + ' "' + m[3].trim() + '"'); }
for (const m of html.matchAll(/<[^>]*\sdata-i18n-(aria|placeholder|title)="([\w.]+)"[^>]*>/g)) {
  const attr = m[1] === "aria" ? "aria-label" : m[1], v = new RegExp("\\s" + attr + '="([^"]*)"').exec(m[0]), want = S[m[2]] && S[m[2]]["zh-TW"];
  if (v && want !== undefined && v[1] !== want) say("index.html " + attr + " != table: " + m[2] + ' "' + v[1] + '"');
}
const dynUsed = k => DYN.some(p => k.startsWith(p)) || k.startsWith("viz.") || k.startsWith("rn.") || k.startsWith("eq.");
const unused = keys.filter(k => !used.has(k) && !dynUsed(k));
if (unused.length) console.log("  (info) not referenced by a literal: " + unused.join(" "));

console.log("syntax: node --check");
for (const f of walk(SRC).filter(f => f.endsWith(".js")).concat(["build.mjs", "check.mjs"].map(n => join(ROOT, n)))) {
  const r = spawnSync(process.execPath, ["--check", f], { encoding: "utf8" });
  if (r.status !== 0) say(f.slice(ROOT.length + 1) + ": " + r.stderr.split("\n").slice(0, 3).join(" "));
}

console.log("dist/ up to date and relative");
const want = await bundle();
for (const [name, data] of want) {
  const p = join(ROOT, "dist", name);
  if (!existsSync(p)) say("dist/" + name + " is missing: run npm run build");
  else if (!readFileSync(p).equals(Buffer.from(data))) say("dist/" + name + " differs from a fresh build: run npm run build");
}
const idx = String(want.get("index.html"));
// (the <a> links to the repository and the releases are navigation, not resources the page loads)
for (const m of idx.replace(/<a\s[^>]*>/g, "").matchAll(/(?:href|src)="([^"]+)"/g)) if (/^\/(?!\/)/.test(m[1]) || /^https?:/.test(m[1]) && !m[1].includes("w3.org")) say("dist/index.html has a non-relative URL: " + m[1]);
if (/(?:fetch|import)\(\s*["'`]\//.test(String(want.get("app.js")))) say("dist/app.js has an absolute path");
console.log(bad ? "PROBLEMS: " + bad : "OK: no problems");
process.exit(bad ? 1 : 0);
