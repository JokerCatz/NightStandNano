// SPDX-License-Identifier: LGPL-2.1-or-later
// npm run check: (1) the string table (three languages, placeholders, no Traditional characters in zh-CN, every key the sources use exists),
// (2) syntax of every source file (node --check), (3) dist/ is what `npm run build` makes from src/ and uses relative URLs only,
// (4) the vendored esptool-js matches its SHA256SUMS and ships its licence, (5) page safety: a CSP, no inline script, no eval / innerHTML in our code,
// (6) the privacy scan of everything that gets published (modelled on JukeDial's tools/web_publish.py): no home-directory paths, no MAC addresses,
// no USB serial numbers, no private-key markers, no private IP addresses, (7) the flash layout constants equal app/partitions.csv.
// Exit code 1 on any problem.
import { readFileSync, readdirSync, statSync, existsSync } from "node:fs";
import { join, dirname, relative } from "node:path";
import { fileURLToPath } from "node:url";
import { createHash } from "node:crypto";
import { spawnSync } from "node:child_process";
import { STRINGS as S } from "./src/js/core/strings.js";
import { LAYOUT, DEFAULT_NVS } from "./src/js/core/release.js";
import { bundle, ESPTOOL, VENDOR_FILES } from "./build.mjs";
import { TRAD } from "./testlib/trad.js";
import { PRIVATE } from "./testlib/privacy.js";

const ROOT = dirname(fileURLToPath(import.meta.url)), SRC = join(ROOT, "src");
const walk = (d) => readdirSync(d).sort().flatMap((n) => { const p = join(d, n); return statSync(p).isDirectory() ? walk(p) : [p]; });
let bad = 0; const say = (m) => { bad++; console.log("  " + m); };
const keys = Object.keys(S), LANGS = ["en", "zh-TW", "zh-CN"];

console.log("string table: " + keys.length + " keys");
for (const k of keys) for (const l of LANGS) if (typeof S[k][l] !== "string" || !S[k][l]) say(k + " lacks " + l);
const ph = (s) => (s.match(/\{\w+\}/g) || []).sort().join(",");
for (const k of keys) if (new Set(LANGS.map((l) => ph(S[k][l]))).size > 1) say("placeholders differ: " + k);
for (const k of keys) {
  const a = S[k]["zh-TW"], b = S[k]["zh-CN"], hit = [...b].filter((c) => TRAD.includes(c));
  if (hit.length) say("zh-CN has Traditional characters: " + k + " [" + [...new Set(hit)].join("") + "]");
  else if (a === b && [...a].some((c) => TRAD.includes(c))) say("zh-CN == zh-TW: " + k);
  if (/[一-鿿]/.test(S[k].en)) say("en contains CJK: " + k);
}
const DYN = ["fl.part.", "fl.err.", "hw.sd.state.", "hw.src.", "hw.why.", "hw.field.", "hw.iss.", "hw.other.", "hw.width.", "hw.res.", "fl.speed.fb."];
const used = new Map(), add = (k, f) => { if (!used.has(k)) used.set(k, f); };
const srcFiles = walk(SRC).filter((f) => /\.(js|html)$/.test(f) && !f.includes("/vendor/") && !f.endsWith("strings.js"));
for (const f of srcFiles) {
  const src = readFileSync(f, "utf8");
  for (const m of src.matchAll(/(?:data-i18n(?:-aria|-placeholder|-title)?="|\bt\(\s*")([\w.]+)"/g)) add(m[1], f);
}
for (const [k, f] of used) if (!S[k] && !DYN.some((p) => k.startsWith(p))) say("used but not in the table: " + k + " (" + relative(ROOT, f) + ")");
const allSrc = srcFiles.map((f) => readFileSync(f, "utf8")).join("\n");
const unused = keys.filter((k) => !used.has(k) && !DYN.some((p) => k.startsWith(p)) && !allSrc.includes('"' + k + '"'));
if (unused.length) console.log("  (info) not referenced by a literal: " + unused.join(" "));

console.log("syntax: node --check");
for (const f of walk(SRC).filter((f) => f.endsWith(".js") && !f.includes("/vendor/")).concat(["build.mjs", "check.mjs"].map((n) => join(ROOT, n)))) {
  const r = spawnSync(process.execPath, ["--check", f], { encoding: "utf8" });
  if (r.status !== 0) say(relative(ROOT, f) + ": " + r.stderr.split("\n").slice(0, 3).join(" "));
}

console.log("dist/ up to date and relative");
const want = await bundle();
for (const [name, data] of want) {
  const p = join(ROOT, "dist", name);
  if (!existsSync(p)) say("dist/" + name + " is missing: run npm run build");
  else if (!readFileSync(p).equals(Buffer.from(data))) say("dist/" + name + " differs from a fresh build: run npm run build");
}
if (existsSync(join(ROOT, "dist"))) for (const f of walk(join(ROOT, "dist"))) if (!want.has(relative(join(ROOT, "dist"), f))) say("dist/" + relative(join(ROOT, "dist"), f) + " is not made by the build");
const idx = String(want.get("index.html")), js = String(want.get("app.js"));
for (const m of idx.matchAll(/(?:href|src)="([^"]+)"/g)) if (/^\/(?!\/)/.test(m[1]) || (/^https?:/.test(m[1]) && !/^https:\/\/(github\.com\/JokerCatz\/NightStandNano|www\.w3\.org)/.test(m[1]))) say("dist/index.html has a non-relative URL: " + m[1]);
for (const m of idx.matchAll(/(?:href|src)="(\.\.\/[^"]*)"/g)) if (m[1] !== "../") say("unexpected parent link: " + m[1]);
if (/(?:fetch|import)\(\s*["'`]\//.test(js)) say("dist/app.js has an absolute path");
if (/["'`]\/(?:NightStandNano|flash)\b/.test(js + idx)) say("a hard-coded site path (must work under any sub-path)");

console.log("vendored esptool-js");
const dir = join(SRC, ESPTOOL), sums = readFileSync(join(dir, "SHA256SUMS"), "utf8").trim().split("\n");
for (const line of sums) { const [h, n] = line.split(/\s+/); if (createHash("sha256").update(readFileSync(join(dir, n))).digest("hex") !== h) say("vendor " + n + " differs from SHA256SUMS"); }
for (const n of [...VENDOR_FILES, "VENDOR.md"]) if (!existsSync(join(dir, n))) say("vendor file missing: " + n);
if (!/Apache License/.test(readFileSync(join(dir, "LICENSE"), "utf8"))) say("vendor LICENSE is not the Apache-2.0 text");
const fl = readFileSync(join(SRC, "js/core/flasher.js"), "utf8");
if (!fl.includes('ESPTOOL_VERSION = "0.7.1"') || !/vendor\/esptool-js\/0\.7\.1$/.test(ESPTOOL)) say("flasher.js / build.mjs do not point at the vendored version");
for (const f of walk(SRC).filter((f) => f.endsWith(".js") && !f.includes("/vendor/"))) if (!readFileSync(f, "utf8").startsWith("// SPDX-License-Identifier: LGPL-2.1-or-later")) say("no SPDX header: " + relative(ROOT, f));

console.log("page safety");
if (!/<meta http-equiv="Content-Security-Policy" content="default-src 'none'; script-src 'self'/.test(idx)) say("index.html has no Content-Security-Policy");
if (/<script(?![^>]*\ssrc=)/.test(idx) || /\son[a-z]+="/.test(idx) || /\sstyle="/.test(idx)) say("index.html has an inline script, event attribute or style attribute");
for (const f of walk(SRC).filter((f) => f.endsWith(".js") && !f.includes("/vendor/") && !f.includes("/demo/"))) {
  const s = readFileSync(f, "utf8").replace(/\/\/.*$/gm, "");
  if (/\beval\s*\(|new Function\s*\(|document\.write\s*\(|\.innerHTML\s*=|\.outerHTML\s*=|insertAdjacentHTML/.test(s)) say("unsafe DOM/eval use in " + relative(ROOT, f));
  if (/getPorts\s*\(/.test(s)) say("opens ports without a user choice: " + relative(ROOT, f));
}

console.log("privacy scan of what gets published");
const PRIVATE_NAME = /wifi|secret|passw|\.env|id_rsa|credential|\.local/i;
const scan = (label, name, buf) => {
  const text = buf.toString("latin1");
  for (const [what, rx] of PRIVATE) { const m = rx.exec(text); if (m) say(label + name + " holds " + what + ": " + JSON.stringify(m[0].slice(0, 40))); }
};
for (const [name, data] of want) { if (PRIVATE_NAME.test(name.split("/").pop())) say("dist/" + name + ": private-looking file name"); scan("dist/", name, Buffer.from(data)); }
for (const f of walk(SRC).filter((f) => !f.includes("/vendor/"))) scan("src/", relative(SRC, f), readFileSync(f));
for (const f of ["README.md", "package.json"]) if (existsSync(join(ROOT, f))) scan("", f, readFileSync(join(ROOT, f)));

console.log("flash layout equals app/partitions.csv");
const csv = join(ROOT, "../app/partitions.csv");
if (existsSync(csv)) {
  const rows = Object.fromEntries(readFileSync(csv, "utf8").split("\n").filter((l) => l.trim() && !l.startsWith("#")).map((l) => l.split(",").map((x) => x.trim())).map((c) => [c[0], { off: Number(c[3]), size: Number(c[4]) }]));
  if (!rows.nvs || rows.nvs.off !== DEFAULT_NVS.off || rows.nvs.size !== DEFAULT_NVS.size) say("release.js DEFAULT_NVS differs from partitions.csv nvs " + JSON.stringify(rows.nvs));
  if (!rows.factory || rows.factory.off !== LAYOUT.app.off) say("release.js app offset differs from partitions.csv factory " + JSON.stringify(rows.factory));
  if (rows.factory && rows.factory.off + rows.factory.size > LAYOUT.flashBytes) say("the factory partition does not fit in 16 MB");
} else console.log("  (info) ../app/partitions.csv not found: skipped");
console.log(bad ? "PROBLEMS: " + bad : "OK: no problems");
process.exit(bad ? 1 : 0);
