// SPDX-License-Identifier: LGPL-2.1-or-later
// Build: src/ -> dist/ (committed; GitHub Pages serves dist/). `node build.mjs` writes dist/; `node build.mjs --dev` builds into .dev/ with source
// maps, rebuilds on every change in src/ and serves it on http://localhost:8000/.
// dist/app.js is ONE classic script (esbuild format iife), so dist/index.html also works from file://. All URLs are relative (GitHub Pages
// serves a project site from a sub-path). Output is deterministic: no clock, no absolute paths; the build stamp shown in Settings > Diagnostics
// is a hash of the contents of src/, so rebuilding unchanged sources changes nothing.
import { build, context } from "esbuild";
import { createHash } from "node:crypto";
import { readFileSync, writeFileSync, mkdirSync, readdirSync, statSync, existsSync, watch } from "node:fs";
import { createServer } from "node:http";
import { dirname, join, extname } from "node:path";
import { fileURLToPath } from "node:url";

const ROOT = dirname(fileURLToPath(import.meta.url)), SRC = join(ROOT, "src");

function walk(dir) {
  return readdirSync(dir).sort().flatMap(n => { const p = join(dir, n); return statSync(p).isDirectory() ? walk(p) : [p]; });
}
export function stamp() {   // 8 hex digits over every file in src/ (path + content)
  const h = createHash("sha256");
  for (const f of walk(SRC)) h.update(f.slice(SRC.length) + "\0").update(readFileSync(f)).update("\0");
  return h.digest("hex").slice(0, 8);
}

// -> Map(file name -> Buffer | string) for the output directory
export async function bundle({ dev = false } = {}) {
  const s = stamp(), out = new Map();
  const common = { bundle: true, write: false, logLevel: "warning", legalComments: "none", sourcemap: dev ? "external" : false, outdir: "out", absWorkingDir: ROOT };
  const js = await build({ ...common, entryPoints: { app: "src/js/main.js" }, format: "iife", target: "es2020", define: { __BUILD__: JSON.stringify(dev ? s + "-dev" : s) } });
  const css = await build({ ...common, entryPoints: { app: "src/css/app.css" } });
  for (const r of [...js.outputFiles, ...css.outputFiles]) out.set(r.path.slice((ROOT + "/out/").length), Buffer.from(r.contents));
  out.set("index.html", readFileSync(join(SRC, "index.html"), "utf8")
    .replace('href="css/app.css"', `href="app.css?v=${s}"`).replace('src="js/main.js"', `src="app.js?v=${s}"`));
  return out;
}
export function writeAll(files, dir) {
  mkdirSync(dir, { recursive: true });
  for (const [name, data] of files) { const p = join(dir, name); if (!existsSync(p) || !readFileSync(p).equals(Buffer.from(data))) writeFileSync(p, data); }
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
    }).listen(8000, () => console.log("http://localhost:8000/  (?demo for the fake box)"));
  }
}
