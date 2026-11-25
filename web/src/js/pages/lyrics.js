// SPDX-License-Identifier: LGPL-2.1-or-later
// Lyrics (lib/lrc.js): the box sends the whole .lrc once per track; the page keeps time between the 1 s polls.
// Three lines sit under the title on the stage (a link to #/now/lyrics); the full list is the page #/now/lyrics.
import { app, $, on, setT, setRaw } from "../core/app.js";
import { send } from "../core/link.js";
import { findLine, parseReply } from "../lib/lrc.js";

let lrcKey = "", posRef = { pos: 0, at: 0, playing: false }, lyrLines = [], lyrCur = -2;
async function fetchLrc(key) {
  const reply = await send("lrc");
  if (app.link !== "on" && !reply.length) { if (key === lrcKey) lrcKey = ""; return; }   // dropped meanwhile: fetch again after the reconnect
  if (key !== lrcKey) return;                    // the track changed meanwhile
  setLyrics(parseReply(reply));
}
function setLyrics(res) {
  lyrLines = res ? res.lines : []; lyrCur = -2;
  const ul = $("lyrList"); ul.textContent = "";
  for (const l of lyrLines) { const li = document.createElement("li"); li.textContent = l.text || " "; ul.appendChild(li); }
  $("lyr3").hidden = !lyrLines.length;
  if (!lyrLines.length) setT($("lyrNote"), "lyr.none"); else if (res.truncated) setT($("lyrNote"), "lyr.truncated"); else setRaw($("lyrNote"), "");
  tickLyrics();
}
export function lyrScroll() {
  const ul = $("lyrList"), li = ul.children[lyrCur];
  if (li && $("p-lyrics").getClientRects().length) ul.scrollTop = li.offsetTop - ul.clientHeight / 2 + li.clientHeight / 2;
}
function tickLyrics() {
  if (!lyrLines.length) return;
  let pos = posRef.pos + (posRef.playing ? performance.now() - posRef.at : 0);
  if (app.state && app.state.len > 0) pos = Math.min(pos, app.state.len);
  const i = findLine(lyrLines, pos);
  if (i === lyrCur) return;
  const ul = $("lyrList"); if (ul.children[lyrCur]) ul.children[lyrCur].classList.remove("cur");
  lyrCur = i; if (ul.children[i]) ul.children[i].classList.add("cur");
  const t = k => (lyrLines[k] ? lyrLines[k].text : "");
  const d = $("lyr3").children; d[0].textContent = t(i - 1); d[1].textContent = t(i); d[2].textContent = t(i + 1);
  lyrScroll();
}

export function initLyrics() {
  setInterval(tickLyrics, 200);
  on("status", state => {
    posRef = { pos: state.pos || 0, at: performance.now(), playing: state.st === "playing" };
    // lyrics: only for a file with a length (radio, noise, scenes and phone audio have no .lrc); refetch when the file changes
    const key = state.len > 0 && state.st !== "idle" ? (state.file || "") + "#" + state.trk : "";
    if (key !== lrcKey) { lrcKey = key; setLyrics(null); if (key) fetchLrc(key); }
  });
  on("endlink", () => { lrcKey = ""; setLyrics(null); });
}
