// SPDX-License-Identifier: LGPL-2.1-or-later
// Build: src/ -> dist/ (committed; GitHub Pages serves it at /NightStandNano/flash/). `node build.mjs` writes dist/; `node build.mjs --dev` builds into
// .dev/ with source maps, rebuilds on every change in src/ and serves it on http://localhost:8001/ (the control page's dev server uses 8000).
// Same shape as web/build.mjs: dist/app.js is ONE classic script (esbuild iife), all URLs are relative (a project site lives under a sub-path),
// the output is deterministic (no clock, no absolute paths; the build stamp shown in the footer is a hash of the contents of src/).
// Additions: the zh-TW text of every data-i18n element is written into index.html (the pre-load default), and the vendored esptool-js is copied to
// dist/vendor/ (it is loaded on demand, only when flashing starts).
import { build } from "esbuild";
import { createHash } from "node:crypto";
import { readFileSync, writeFileSync, mkdirSync, readdirSync, statSync, existsSync, watch } from "node:fs";
import { createServer } from "node:http";
import { dirname, join, extname } from "node:path";
import { fileURLToPath } from "node:url";
import { STRINGS } from "./src/js/core/strings.js";

const ROOT = dirname(fileURLToPath(import.meta.url)), SRC = join(ROOT, "src");
export const ESPTOOL = "vendor/esptool-js/0.7.1";
export const VENDOR_FILES = ["bundle.js", "LICENSE", "NOTICE"];

function walk(dir) {
  return readdirSync(dir).sort().flatMap((n) => { const p = join(dir, n); return statSync(p).isDirectory() ? walk(p) : [p]; });
}
export function stamp() {   // 8 hex digits over every file in src/ (path + content)
  const h = createHash("sha256");
  for (const f of walk(SRC)) h.update(f.slice(SRC.length) + "\0").update(readFileSync(f)).update("\0");
  return h.digest("hex").slice(0, 8);
}
const esc = (s) => s.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
// <tag ... data-i18n="key" ...>old text</tag>  ->  the zh-TW text of key (elements with data-i18n hold text only)
export function fillText(html) {
  return html.replace(/(<(\w+)\b[^>]*\sdata-i18n="([\w.]+)"[^>]*>)([^<]*)(<\/\2>)/g, (m, open, tag, key, old, close) => {
    const row = STRINGS[key];
    if (!row) throw new Error("index.html uses a key that is not in the table: " + key);
    return open + esc(row["zh-TW"]) + close;
  });
}

// -> Map(file name -> Buffer | string) for the output directory
export async function bundle({ dev = false } = {}) {
  const s = stamp(), out = new Map();
  const common = { bundle: true, write: false, logLevel: "warning", legalComments: "none", sourcemap: dev ? "external" : false, outdir: "out", absWorkingDir: ROOT };
  const js = await build({ ...common, entryPoints: { app: "src/js/main.js" }, format: "iife", target: "es2020", define: { __BUILD__: JSON.stringify(dev ? s + "-dev" : s) } });
  const css = await build({ ...common, entryPoints: { app: "src/css/app.css" } });
  for (const r of [...js.outputFiles, ...css.outputFiles]) out.set(r.path.slice((ROOT + "/out/").length), Buffer.from(r.contents));
  out.set("index.html", fillText(readFileSync(join(SRC, "index.html"), "utf8"))
    .replace('href="css/app.css"', `href="app.css?v=${s}"`).replace('src="js/main.js"', `src="app.js?v=${s}"`));
  for (const f of VENDOR_FILES) out.set(ESPTOOL + "/" + f, readFileSync(join(SRC, ESPTOOL, f)));
  return out;
}
export function writeAll(files, dir) {
  for (const [name, data] of files) {
    const p = join(dir, name);
    mkdirSync(dirname(p), { recursive: true });
    if (!existsSync(p) || !readFileSync(p).equals(Buffer.from(data))) writeFileSync(p, data);
  }
}

if (process.argv[1] === fileURLToPath(import.meta.url)) {
  if (!process.argv.includes("--dev")) {
    const files = await bundle();
    writeAll(files, join(ROOT, "dist"));
    console.log("dist/ built, stamp " + stamp() + ": " + [...files].map(([n, d]) => n + " " + d.length).join(", "));
  } else {
    const DEV = join(ROOT, ".dev");
    const rebuild = async () => { try { writeAll(await bundle({ dev: true }), DEV); console.log("rebuilt " + new Date().toLocaleTimeString()); } catch (e) { console.error(String(e)); } };
    await rebuild();
    let t = 0; watch(SRC, { recursive: true }, () => { clearTimeout(t); t = setTimeout(rebuild, 150); });
    const types = { ".html": "text/html", ".js": "text/javascript", ".css": "text/css", ".map": "application/json" };
    createServer((req, res) => {
      const p = join(DEV, decodeURIComponent(new URL(req.url, "http://x").pathname).replace(/\/$/, "/index.html"));
      if (!p.startsWith(DEV) || !existsSync(p) || statSync(p).isDirectory()) { res.writeHead(404).end("not found"); return; }
      res.writeHead(200, { "content-type": types[extname(p)] || "application/octet-stream", "cache-control": "no-store" }).end(readFileSync(p));
    }).listen(8001, () => console.log("http://localhost:8001/  (?demo for the pretend board)"));
  }
}
