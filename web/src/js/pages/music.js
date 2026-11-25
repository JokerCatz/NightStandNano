// SPDX-License-Identifier: LGPL-2.1-or-later
// #/music: the file browser, play folder, play mode, gapless and ReplayGain.
// A name that ends in "/" is a folder; a .zip archive is listed that way too (`ls /sdcard/x.zip` lists its entries, which play like files), with its own icon.
import { app, $, on, t, setT, segSet, segBind, emptyLi, MODES, RGS } from "../core/app.js";
import { send } from "../core/link.js";
import { refreshStatus } from "../core/status.js";
import { playError, playErrView, inZip, isZipEntry } from "../lib/fw.js";
import { describeEvent, skipView } from "../lib/events.js";
import { fetchEvents } from "../core/evfeed.js";

// A failed `play` (the box answers `play: <esp error name>[ <detail>]`; `play: ESP_OK` is success) or a track the box skipped (event `skip ...`) is shown
// for a few seconds under the list. view(sep) -> { key, p }; kept, so the text can be rebuilt on a language switch (the chip list separator is per language).
let msgTimer = 0, msgView = null;
function showMsg() { const v = msgView(t("list.sep")); setT($("playMsg"), v.key, v.p); }
function playMsg(view) {
  const m = $("playMsg"); clearTimeout(msgTimer); msgView = view;
  if (!view) { m.textContent = ""; m.removeAttribute("data-i18n"); m.removeAttribute("data-i18n-p"); return; }
  showMsg();
  msgTimer = setTimeout(() => playMsg(null), 8000);
}
async function play(path) {
  playMsg(null);
  const r = await send("play " + path);
  const err = playError(r);
  playMsg(err && (sep => playErrView(err, sep)));
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
  // skips while this page is open: events are fetched (quietly, incrementally) after the playing file changed and every ~5 s while something plays
  const watching = () => app.link === "on" && app.route === "music" && !!app.state && app.state.st === "playing";
  let lastFile = null;
  setInterval(() => { if (watching()) fetchEvents(); }, 5000);
  on("events", ({ added }) => {
    if (app.route !== "music") return;
    for (const e of added) {   // newest first: the latest skip of this reply (the first fetch may bring old ones: only recent ones count)
      const d = describeEvent(e.text);
      if (d.kind === "skip" && e.at >= Date.now() - 20000) { playMsg(sep => skipView(d, sep)); break; }
    }
  });
  on("lang", () => { if (msgView) showMsg(); });
  on("status", state => {
    if (state.file !== lastFile) { const was = lastFile; lastFile = state.file; if (was !== null && watching()) fetchEvents(); }
    segSet("mode", MODES[state.mode] || "folder");
    if (typeof state.rg === "number") segSet("rg", RGS[state.rg] || "off");
    for (const li of $("list").children) li.classList.toggle("cur", !!state.file && li.dataset.name === state.file && state.st !== "idle");
  });
}
