// SPDX-License-Identifier: LGPL-2.1-or-later
// Parsers for what the firmware reports about itself: `ver`, `hw`, `ratetest status`, the answers of `ratetest` / `ratetest cancel`, and the
// `play: <esp error name>` reply. Pure functions (no DOM), so they are unit-tested in test/fw.test.js.

function kv(s) {   // key=value / key="quoted value" pairs
  const o = {}, re = /(\w+)=(?:"([^"]*)"|(\S+))/g; let m;
  while ((m = re.exec(s))) o[m[1]] = m[2] !== undefined ? m[2] : m[3];
  return o;
}
const numify = o => { for (const k of Object.keys(o)) if (/^-?\d+$/.test(o[k])) o[k] = +o[k]; return o; };

// `ver 0.1.0-ee14d98-dirty idf=v6.1 chip=esp32s31 built=874f52d83` (the version may also be `v0.1.0`) -> { version, idf, chip, built } | null
export function parseVer(lines) {
  for (const l of lines) {
    const m = /^ver\s+(\S+)(.*)$/.exec(l); if (!m) continue;
    const o = kv(m[2]);
    return { version: m[1], idf: o.idf || "", chip: o.chip || "", built: o.built || "" };
  }
  return null;
}

// `hw sd clk=24 cmd=25 d0=20 ... width=4 src=default state=mounted mode=sdmmc`, or (SPI) `hw sd clk=.. mosi=.. miso=.. cs=.. src=custom state=.. mode=spi`,
// `hw sdmmc routing=iomux ...`, `hw led gpio=60 order=grb on=1 src=default`, `hw reserved 26,27,...`, `hw gpios 0-61`, `hw pending 0`
// -> { sd, sdmmc, led, reserved, gpios, pending } (sd / led null when the firmware did not say) | null when there is no hw line at all
export function parseHw(lines) {
  const r = { sd: null, sdmmc: null, led: null, reserved: "", gpios: "", pending: 0 };
  let seen = false;
  for (const l of lines) {
    let m;
    if ((m = /^hw (sd|sdmmc|led)\s+(.*)$/.exec(l))) { r[m[1]] = numify(kv(m[2])); seen = true; }
    else if ((m = /^hw reserved\s*(.*)$/.exec(l))) { r.reserved = m[1].trim(); seen = true; }
    else if ((m = /^hw gpios\s*(.*)$/.exec(l))) { r.gpios = m[1].trim(); seen = true; }
    else if ((m = /^hw pending\s+(\d+)/.exec(l))) { r.pending = +m[1]; seen = true; }
  }
  if (r.led) r.led.on = r.led.on === undefined ? 1 : +r.led.on;
  if (r.sd && !r.sd.mode) r.sd.mode = r.sd.mosi !== undefined ? "spi" : "sdmmc";
  return seen ? r : null;
}

const VERDICTS = ["ok", "bad", "untested"];
// `ratetest state=idle|running|done|cancelled|failed dac=06cb:1594 [now=<Hz>] [reason="..."]`, one `ratetest rate=96000 set=ok|fail|na clock=96000 xfer_ok=750
// xfer_bad=0 fps=95999 verdict=ok|bad|untested` per rate, `ratetest max=<Hz>` -> { state, dac, now, reason, rates: [...], max } | null (no state line)
export function parseRatetest(lines) {
  let r = null;
  for (const l of lines) {
    let m;
    if ((m = /^ratetest state=(\w+)(.*)$/.exec(l))) { const o = kv(m[2]); r = { state: m[1], dac: o.dac || "", now: o.now ? +o.now : 0, reason: o.reason || "", rates: [], max: null }; }
    else if (r && (m = /^ratetest rate=(\d+)(.*)$/.exec(l))) {
      const o = kv(m[2]);
      r.rates.push({ rate: +m[1], set: o.set || "na", clock: +o.clock || 0, xferOk: +o.xfer_ok || 0, xferBad: +o.xfer_bad || 0, fps: +o.fps || 0, verdict: VERDICTS.includes(o.verdict) ? o.verdict : "untested" });
    } else if (r && (m = /^ratetest max=(\d+)/.exec(l))) r.max = +m[1];
  }
  return r;
}
// the answer to `ratetest` / `ratetest cancel`: { ok: true } (started / cancelled) | { refused: "no dac" } | null (nothing recognisable)
export function parseRatetestReply(lines) {
  for (const l of lines) {
    let m;
    if ((m = /^ratetest: refused\s*(.*)$/.exec(l))) return { refused: m[1].trim() || "?" };
    if (/^ratetest: (started|cancelled)/.test(l)) return { ok: true };
    if (/^ratetest state=/.test(l)) return { ok: true, running: true };   // `ratetest` while one runs answers with the status
  }
  return null;
}

// `play: ESP_OK` is success; anything else is `play: <esp error name>`. -> null (played, or no `play:` line, e.g. the link dropped) | { code, key }
// key: the string-table key of the message shown to the user
const PLAY_ERR = { ESP_ERR_INVALID_SIZE: "play.err.size", ESP_ERR_NOT_FOUND: "play.err.notfound", ESP_ERR_TIMEOUT: "play.err.timeout", ESP_ERR_INVALID_STATE: "play.err.busy",
  ESP_ERR_NO_MEM: "play.err.nomem", ESP_ERR_NOT_SUPPORTED: "play.err.notsup" };
export function playError(lines) {
  for (const l of lines) {
    const m = /^play:\s*(\S+)/.exec(l); if (!m) continue;
    if (m[1] === "ESP_OK") return null;
    return { code: m[1], key: PLAY_ERR[m[1]] || "play.err.other" };
  }
  return null;
}

// 96000 -> "96", 44100 -> "44.1", 384000 -> "384" (kHz, no unit)
export const khz = hz => String(+(hz / 1000).toFixed(1));
// is this path inside (or at) a zip archive? "/sdcard/packs/nes.zip" and "/sdcard/packs/nes.zip/sub" -> true
export const inZip = p => /\.zip(\/|$)/i.test(p);
export const isZipEntry = name => /\.zip\/$/i.test(name);
