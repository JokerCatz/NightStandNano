// SPDX-License-Identifier: LGPL-2.1-or-later
// #/music: the file browser, play folder, play mode, gapless and ReplayGain.
// A name that ends in "/" is a folder; a .zip archive is listed that way too (`ls /sdcard/x.zip` lists its entries, which play like files), with its own icon.
import { app, $, on, setT, segSet, segBind, emptyLi, MODES, RGS } from "../core/app.js";
import { send } from "../core/link.js";
import { refreshStatus } from "../core/status.js";
import { playError, inZip, isZipEntry } from "../lib/fw.js";

// a failed `play` is shown for a few seconds under the list (the box answers `play: <esp error name>`; `play: ESP_OK` is success)
let msgTimer = 0;
function playMsg(err) {
  const m = $("playMsg"); clearTimeout(msgTimer);
  if (!err) { m.textContent = ""; m.removeAttribute("data-i18n"); m.removeAttribute("data-i18n-p"); return; }
  setT(m, err.key, { code: err.code });
  msgTimer = setTimeout(() => playMsg(null), 8000);
}
async function play(path) {
  playMsg(null);
  const r = await send("play " + path);
  playMsg(playError(r));
  refreshStatus();
}
// the "play this folder" button reads "play this archive" at or inside a zip
function playDirLabel() { setT($("btnPlayDir").lastElementChild, inZip(app.cwd) ? "files.playZip" : "files.playDir"); }

export async function ls(dir) {
  const prev = app.cwd;
  app.cwd = dir.replace(/\/+$/, "") || "/";
  $("cwd").textContent = app.cwd; playDirLabel(); playMsg(null);
  const lines = await send("ls " + app.cwd);
  const ul = $("list"); if (app.link !== "on" && !lines.length) return;   // dropped meanwhile: keep the list as it is
  const keep = app.cwd === prev ? ul.scrollTop : 0; ul.textContent = "";
  const items = lines.filter(l => l && l !== "ls: cannot open" && l !== "?").sort((a, b) => (b.endsWith("/") - a.endsWith("/")) || a.localeCompare(b));
  for (const name of items) {
    const li = document.createElement("li"), isDir = name.endsWith("/"), zip = isZipEntry(name);
    li.tabIndex = 0; li.dataset.name = name; if (zip) li.classList.add("zip");
    li.textContent = (zip ? "📦 " : isDir ? "📁 " : "♪ ") + (isDir ? name.slice(0, -1) : name);
    li.onclick = () => isDir ? ls(app.cwd + "/" + name.slice(0, -1)) : play(app.cwd + "/" + name);
    ul.appendChild(li);
  }
  if (!items.length) ul.appendChild(emptyLi("files.empty"));
  ul.scrollTop = keep;
}

export function initMusic() {
  $("btnUp").onclick = () => { const p = app.cwd.split("/"); if (p.length > 2) { p.pop(); ls(p.join("/")); } };
  $("btnPlayDir").onclick = () => play(app.cwd);
  segBind("mode", v => send("mode " + v));
  segBind("rg", v => send("rg " + v));
  $("gapless").onchange = async () => { app.gapUntil = Date.now() + 3000; await send("gapless " + ($("gapless").checked ? "on" : "off")); app.gapUntil = 0; refreshStatus(); };
  on("status", state => {
    segSet("mode", MODES[state.mode] || "folder");
    if (typeof state.rg === "number") segSet("rg", RGS[state.rg] || "off");
    for (const li of $("list").children) li.classList.toggle("cur", !!state.file && li.dataset.name === state.file && state.st !== "idle");
  });
}
