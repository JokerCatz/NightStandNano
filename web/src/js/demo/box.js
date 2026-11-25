// SPDX-License-Identifier: LGPL-2.1-or-later
// The fake box for ?demo: a device that lives in the page, so the whole UI can be looked at (and tested) without hardware.
// It plugs into the same places a real box does: the GATT setup (setGattSetup) and the notification handler (onEvt), so every
// command goes through the real queue. The query parameters it understands are listed in src/index.html.
import { app } from "../core/app.js";
import { onEvt, enc, dec, connect, setGattSetup } from "../core/link.js";

export function startDemo() {
  const fake = { st: "playing", fmt: "FLAC", rate: 48000, pos: 79000, len: 241000, vol: 42, trk: 3, n: 12, sub: 1, subn: 1, sleep: 1500, mode: 0, und: 0,
    out: "dac", dac: 1, bt: 0, wifi: 1, mem: 101000, title: "", rstate: "", file: "03 - Night Drive.flac", bm: 0, cpu: [-1, -1], artist: "Demo Artist", album: "Harbour Lights", rg: 0, gapless: 0, vmax: 100, dacmax: 0, fxshed: 0 };
  fake.fxshed = new URLSearchParams(location.search).get("shed") === "1" ? 1 : 0;
  fake.title = "Night Drive";
  if (new URLSearchParams(location.search).get("tags") === "long") {   // ?demo&tags=long: very long tag values, to check wrapping
    fake.title = "Variations on a Theme by Somebody Else, Op. 123 No. 4 (Live at the Grand Auditorium, Remastered 2024 Deluxe Edition)";
    fake.artist = "The Extraordinarily Long-Named Philharmonic Orchestra & Chamber Choir featuring Supercalifragilisticexpialidocious_Soloist";
    fake.album = "Complete_Works_Volume_07_1999-2024_Anniversary_Collection"; }
  let tick = 0;
  // the box's output: selected (out) vs playing (st.out); Auracast first start drops the BLE link, like the real one
  const q = new URLSearchParams(location.search);
  let fakeOutSel = "auto", castOn = false, castLoaded = false, castFirst = true, castFrames = 0, castTry = q.get("cast") === "fail" ? 1 : 0;
  const castBad = q.get("cast") === "bad" ? 1 : q.get("cast") === "half" ? 2 : 0;
  // each output remembers its own volume; `st.vol` is the one of the output in use (or of the selected one while nothing plays on it)
  const fakeVols = { dac: 42, bt: 30, cast: 55 };
  const volKey = () => ["dac", "bt", "cast"].includes(fake.out) ? fake.out : fakeOutSel === "bt" ? "bt" : "dac";
  const fakeOut = () => { fake.out = fakeOutSel === "cast" ? "cast" : fakeOutSel === "bt" ? "none" : "dac"; fake.dac = fake.out === "dac" ? 1 : 0; fake.bt = 0; fake.vol = Math.min(fakeVols[volKey()], fake.vmax); };
  fakeOut();
  // a fake device: gatt.connect() succeeds after ~600 ms; fakeDrop() fires "gattserverdisconnected" at the handler the page registered, i.e. the code path a real drop takes
  const dl = [];
  const fakeDev = { addEventListener: (ev, fn) => { if (ev === "gattserverdisconnected" && !dl.includes(fn)) dl.push(fn); },
    gatt: { connected: true,
      connect: () => new Promise(res => setTimeout(() => { fakeDev.gatt.connected = true; res({}); }, 600)),
      disconnect: () => { if (!fakeDev.gatt.connected) return; fakeDev.gatt.connected = false; setTimeout(() => dl.forEach(f => f({ target: fakeDev })), 0); } } };
  function fakeDrop() { if (!fakeDev.gatt.connected) return; fakeDev.gatt.connected = false; dl.forEach(f => f({ target: fakeDev })); }
  const dropAt = +q.get("drop"); if (dropAt > 0) setTimeout(fakeDrop, dropAt * 1000);
  const DEMO_LRC = "[ti:Night Drive]\r\n[ar:Demo]\r\n[00:00.00]\r\n[00:12.50]夜色慢慢落在城市的邊緣\r\n[00:21.00]路燈一盞一盞亮起來\r\n[00:30.40]收音機輕輕唱著老歌\r\n[00:41.00]<00:41.00>我<00:41.50>們<00:42.00>不趕時間\r\n[00:52.00]海風穿過半開的車窗\r\n[01:03.20]把今天的疲倦都帶走\r\n[01:14.00]\r\n[01:19.00]遠方的燈塔閃著微光\r\n[01:30.00]像在說一切都會好的\r\n[01:41.50]再開一段就到家了\r\n[01:52.00]晚安，城市\r\n[02:05.00]";
  const stations = [{ name: "SomaFM Groove Salad", url: "http://ice1.somafm.com/groovesalad-128-mp3" }, { name: "Radio Paradise", url: "http://stream.radioparadise.com/mp3-192" }, { name: "古典音樂電台", url: "https://example.com/classical.aac" }];
  // fx apps in memory: [id, name, on, [param, lo, hi, def, cur]...]; the equalizer also has ten bands [type, fc, gain, q]
  const FX = [["loudness", "Loudness", 1, ["quieter_db", 0, 40, 20, 20]], ["bass", "Bass", 0, ["amount", 0, 1, 0.5, 0.5]],
    ["equalizer", "Equalizer", 1, ["preamp_db", -24, 6, 0, -3], ["count", 0, 10, 0, 3]], ["crossfeed", "Crossfeed", 0, ["feed_db", 0, 15, 4.5, 4.5]],
    ["compressor", "Compressor", 1, ["threshold_db", -60, 0, -24, -24], ["ratio", 1, 20, 2.5, 2.5]], ["limiter", "Limiter", 1, ["ceiling_db", -12, 0, -1, -1]]];
  const EQB = [[1, 80, 4, 0.7], [0, 1000, -3, 1], [2, 8000, 2, 0.7], ...Array.from({ length: 7 }, () => [0, 1000, 0, 1])];
  const fxLines = () => FX.flatMap(([id, name, on, ...ps]) => [`APP fxp ${id} on ${on} ${name}`, ...ps.map(([n, lo, hi, d, c]) => `APP fxp ${id} ${n} ${lo} ${hi} ${d} ${c}`),
    ...(id === "equalizer" ? EQB.map((b, i) => `APP fxb equalizer ${i} ${b.join(" ")}`) : [])]);
  const fxCmd = a => {   // fx on <app> <0|1> / fx off / fx set <app> <param> <value>; empty reply = success
    if (a[1] === "off") { for (const x of FX) x[2] = 0; return []; }
    const app = FX.find(x => x[0] === a[2]); if (!app) return ["fx: unknown app"];
    if (a[1] === "on") { app[2] = +a[3]; return []; }
    if (a[1] === "set") {
      const m = /^b(\d)_(type|fc|gain_db|q)$/.exec(a[3]);
      if (app[0] === "equalizer" && m) { EQB[+m[1]][["type", "fc", "gain_db", "q"].indexOf(m[2])] = +a[4]; return []; }
      const ps = app.slice(3).find(x => x[0] === a[3]); if (ps) { ps[4] = +a[4]; return []; }
    }
    return ["fx: bad command"];
  };
  // the headset buttons, in memory: `keys`, `keys preset N`, `keys set <c|p|m> <1|2|3|h> <action>`; a gesture "arrives" every ~7 s
  const KP = { 1: ["play next prev none", "none none none none", "none none none none"], 2: ["none none none none", "vol+ next none vol+", "vol- prev none vol-"],
    3: ["play next prev none", "vol+ none none vol+", "vol- none none vol-"] };
  const keyMap = { c: {}, p: {}, m: {} }, keyLoad = n => ["c", "p", "m"].forEach((b, i) => KP[n][i].split(" ").forEach((a, j) => { keyMap[b]["123h"[j]] = a; }));
  keyLoad(3);
  let keyLastAt = 0, keyLastG = null;
  setInterval(() => { keyLastG = "cpm"[Math.floor(Math.random() * 3)] + "123h"[Math.floor(Math.random() * 4)]; keyLastAt = Date.now(); }, 7000);
  const keysReply = cmd => {
    let m;
    if ((m = /^keys preset ([123])$/.exec(cmd))) keyLoad(+m[1]);
    else if ((m = /^keys set (\S+) (\S+) (\S+)$/.exec(cmd))) {
      if (!keyMap[m[1]] || !"123h".includes(m[2]) || m[2].length !== 1 || !["none", "play", "next", "prev", "vol+", "vol-", "stop"].includes(m[3])) return ["keys set <c|p|m> <1|2|3|h> <none|play|next|prev|vol+|vol-|stop>"];
      keyMap[m[1]][m[2]] = m[3];
    } else if (cmd !== "keys") return ["keys: unknown"];
    const line = b => `keys ${b} ` + [..."123h"].map(g => `${g}=${keyMap[b][g]}`).join(" ");
    return [line("c"), line("p"), line("m"),
      keyLastG ? `keys last ${keyLastG} ${keyMap[keyLastG[0]][keyLastG[1]]} ${Math.floor((Date.now() - keyLastAt) / 1000)} s ago` : "keys last none"];
  };
  // the memory card: ?demo&card=bad|none starts without a readable card (a successful demo format makes it ok). `cardtest` runs ~6 s with
  // moving percentages; the demo box is slower than the sample so the ladder shows a "marginal" and a "slow" row. ?demo&refuse=<reason> makes
  // `cardtest raw|file` refuse, ?demo&testfail=<reason> makes a run fail. `cardfmt`: three steps with two random codes, 30 s each.
  let cardSt = q.get("card") === "bad" ? "bad" : q.get("card") === "none" ? "none" : "ok", cardName = "SL16G", cardFs = "FAT32";
  const ctest = { run: null, fail: "", raw: null, file: null, basis: "", t0: 0 };
  const CT_FMTS = [["WAV 384k/32", 4.61], ["FLAC 384k/24", 2.60], ["WAV 192k/24", 1.15], ["WAV 96k/24", 0.58], ["FLAC 96k/24", 0.33], ["MP3 320k", 0.04]];
  const ctClock = () => { const d = new Date(), z = n => String(n).padStart(2, "0"); return `${d.getFullYear()}-${z(d.getMonth() + 1)}-${z(d.getDate())}T${z(d.getHours())}:${z(d.getMinutes())}`; };
  const ctFinish = () => {
    const r = ctest.run; ctest.run = null;
    if (q.get("testfail")) { ctest.fail = r + " " + q.get("testfail"); return; }
    if (r === "raw") ctest.raw = { avg16: 3.62, slow16: 3.40, max64: 4.10, worst_us: 5230, time: ctClock() };
    else ctest.file = { write: 2.91, read: 3.05, verify: 1, time: ctClock() };
    ctest.basis = ctest.file ? "file" : "raw";
  };
  const ctReply = cmd => {
    const a = cmd.split(" ");
    if (ctest.run && Date.now() - ctest.t0 >= 6000) ctFinish();
    if (a[1] === "raw" || a[1] === "file") {
      if (cardSt === "none") return ["cardtest: refused no card"];
      if (cardSt === "bad") return ["cardtest: refused card unreadable"];
      if (ctest.run) return ["cardtest: refused already running"];
      if (q.get("refuse")) return ["cardtest: refused " + q.get("refuse")];
      if (fake.st !== "idle") { fake.bm = fake.len > 0 ? 1 : 0; fake.st = "idle"; }
      ctest.run = a[1]; ctest.t0 = Date.now(); ctest.fail = ""; return ["cardtest: started " + a[1]];
    }
    if (a[1] === "cancel") { if (!ctest.run) return ["cardtest: refused not running"]; ctest.run = null; return ["cardtest: cancelled"]; }
    const out = [ctest.run ? `cardtest state running ${ctest.run} ${Math.min(99, Math.floor((Date.now() - ctest.t0) / 60))}%` : "cardtest state idle"];
    if (ctest.fail) out.push("cardtest fail " + ctest.fail);
    const kvs = o => Object.entries(o).map(([k, v]) => `${k}=${v}`).join(" ");
    if (ctest.raw) out.push("cardtest raw " + kvs(ctest.raw));
    if (ctest.file) out.push("cardtest file " + kvs(ctest.file));
    if (ctest.basis) out.push(`cardtest basis ${ctest.basis} speed=${(ctest.basis === "file" ? ctest.file.read : ctest.raw.slow16).toFixed(2)}`);
    const marks = [["raw", ctest.raw ? ctest.raw.slow16 - 0.01 : 0], ["file", ctest.file ? ctest.file.read : 0]].sort((x, y) => y[1] - x[1]);
    const ref = ctest.basis ? (ctest.basis === "file" ? ctest.file.read : ctest.raw.slow16) : 0;
    const rows = CT_FMTS.map(([n, need]) => ({ n, need, v: need }));
    for (const [n, sp] of marks) { const i = rows.findIndex(x => x.need <= sp); rows.splice(i < 0 ? rows.length : i, 0, { mark: n, sp }); }
    for (const x of rows) {
      if (x.mark) out.push(`cardtest mark ${x.mark} speed=${x.sp.toFixed(2)}`);
      else out.push(`cardtest fmt "${x.n}" need=${x.need.toFixed(2)} grade=${!ref ? "unknown" : ref >= x.need * 1.5 ? "ok" : ref >= x.need ? "marginal" : "slow"}`);
    }
    return out;
  };
  let cf = null, cfRun = 0, cfFail = q.get("fmtfail");
  const rnd4 = () => String(1000 + Math.floor(Math.random() * 9000));
  const cfRefuse = why => { cf = null; return ["cardfmt: refused " + why]; };
  const cfReply = cmd => {
    const a = cmd.split(" ");
    if (a[1] === "status") {
      if (cfRun && Date.now() >= cfRun) { cfRun = 0; if (cfFail) return ["cardfmt state failed " + cfFail]; cardSt = "ok"; cardName = "NIGHTSTAND"; cardFs = "exFAT"; return ["cardfmt state done"]; }
      return [cfRun ? "cardfmt state running" : cf ? "cardfmt state pending" : "cardfmt state idle"];
    }
    if (a[1] === "cancel") { cf = null; return ["cardfmt: cancelled"]; }
    if (cardSt === "none") return cfRefuse("no card");
    if (cardSt === "ok") return cfRefuse("the card is readable; formatting is only for a card that cannot be read");
    if (cfRun) return ["cardfmt: refused already running"];
    if (!a[1]) {
      cf = { step: 1, code: rnd4(), exp: Date.now() + 30000 };
      return ['cardfmt plan fs=exFAT label="NIGHTSTAND" size=15931539456', "cardfmt warn everything on the card is erased, including the Wi-Fi settings file", `cardfmt: confirm ${cf.code} within 30 s`];
    }
    if (!cf) return cfRefuse("no request");
    if (Date.now() > cf.exp) return cfRefuse("too late");
    if (a[1] !== cf.code) return cfRefuse("wrong code");
    if (cf.step === 1) { let c2; do c2 = rnd4(); while (c2 === cf.code); cf = { step: 2, code: c2, exp: Date.now() + 30000 }; return [`cardfmt: confirm again ${c2} within 30 s`]; }
    cf = null; cfRun = Date.now() + 5000; return ["cardfmt: started"];
  };
  const cardReply = () => cardSt === "ok" ? [`card state=mounted size=15931539456 free=15908798464 fs=${cardFs} name="${cardName}" serial=78EA244F fmt=idle`]
    : cardSt === "bad" ? [`card state=unreadable size=15931539456 free=? fs=? name="" serial=78EA244F fmt=${cfRun ? "running" : "idle"}`] : ["card state=none fmt=idle"];
  // Wi-Fi: ?demo&wifi=none starts with no credentials. `wifi set`: password wrongpass1 -> wrong password, ssid Nowhere -> not found,
  // anything else connects ~3 s later. `wifi scan` answers after ~2 s, in two pieces (the last line 0.4 s after the others).
  const wf = q.get("wifi") === "none" ? { state: "no credentials", ssid: "", ip: "0.0.0.0", rssi: 0, src: "none", until: 0, next: null }
    : { state: "connected", ssid: "HomeNet-5G", ip: "192.0.2.23", rssi: -58, src: "card", until: 0, next: null };
  const wfCard = wf.src === "card" ? wf.ssid : "";
  const wfTick = () => { if (wf.next && Date.now() >= wf.until) { Object.assign(wf, wf.next); wf.next = null; } fake.wifi = wf.state === "connected" ? 1 : 0; };
  const wfEsc = x => x.replace(/[\\"]/g, "\\$&");
  const SCAN = [[-48, "secured", "HomeNet-5G"], [-61, "open", 'Cafe "Blue" Guest'], [-72, "secured", "Neighbor_2.4G"], [-80, "secured", "訪客專用網路"], [-86, "open", "Office-Guest"]];
  // the clock: ?demo&clock=none starts without time; ?demo&tz=<POSIX TZ> sets the zone (default CST-8)
  const tm = { src: q.get("clock") === "none" ? "none" : "ntp", skew: 0, tz: q.get("tz") || "CST-8", up0: Date.now() };
  const tmUnix = () => tm.src === "none" ? Math.floor((Date.now() - tm.up0) / 1000) + 12 : Math.floor((Date.now() + tm.skew) / 1000);
  const tmOff = () => { const m = /^[A-Za-z]{3,}([+-]?)(\d{1,2})(?::(\d\d))?/.exec(tm.tz); return m ? -((m[1] === "-" ? -1 : 1) * (+m[2] * 60 + +(m[3] || 0))) : 0; };
  const tmLocal = () => new Date((tmUnix() + tmOff() * 60) * 1000).toISOString().slice(0, 19);
  const tmZoneOk = z => /^[\x21-\x7e]{1,47}$/.test(z);
  const timeReply = cmd => {
    let m;
    if (cmd === "time") {
      const o = tmOff(), a = Math.abs(o);
      return [`time ${tmUnix()} ${tmLocal()}${o < 0 ? "-" : "+"}${String(Math.floor(a / 60)).padStart(2, "0")}:${String(a % 60).padStart(2, "0")} src=${tm.src} tz="${tm.tz}"`];
    }
    if ((m = /^time set (\d+)(?: (\S+))?$/.exec(cmd))) {
      if (m[2] && !tmZoneOk(m[2])) return ["time set: refused bad zone"];
      if (m[2]) tm.tz = m[2];
      if (tm.src === "ntp") return ["time set: zone only, clock is from ntp"];
      tm.src = "client"; tm.skew = +m[1] * 1000 - Date.now(); return ["time set: ok"];
    }
    if ((m = /^time tz (\S+)$/.exec(cmd))) { if (!tmZoneOk(m[1])) return ["time tz: refused bad zone"]; tm.tz = m[1]; return ["time tz: ok"]; }
    return ["time: unknown"];
  };
  const wifiReply = cmd => {
    wfTick(); let m;
    if (cmd === "wifi") return [`wifi ${wf.state} ip=${wf.ip} rssi=${wf.rssi} time=${tm.src === "none" ? "(unsynced) " : ""}${tmLocal().replace("T", " ")} src=${wf.src} ssid="${wfEsc(wf.ssid)}"`];
    if (cmd === "wifi scan") {
      const aps = SCAN.map(([r, k, n]) => `wifi ap ${r} ${k} "${wfEsc(n)}"`);
      return { chunks: [[2000, aps], [2400, [`wifi scan: ${aps.length} found`]]] };
    }
    if ((m = /^wifi set (.*)$/s.exec(cmd))) {
      const i = m[1].indexOf("|"); if (i < 0) return ["wifi set: refused usage: wifi set <ssid>|<password>"];
      const ssid = m[1].slice(0, i), pw = m[1].slice(i + 1), sb = enc.encode(ssid).length, pb = enc.encode(pw).length;
      if (/[\x00-\x1f\x7f]/.test(m[1])) return ["wifi set: refused control characters"];
      if (sb < 1 || sb > 32) return ["wifi set: refused ssid must be 1-32 bytes"];
      if (pb && (pb < 8 || pb > 63)) return ["wifi set: refused password must be empty or 8-63 bytes"];
      Object.assign(wf, { state: "connecting", ssid, ip: "0.0.0.0", rssi: 0, src: "nvs", until: Date.now() + 3000,
        next: pw === "wrongpass1" ? { state: "wrong password" } : ssid === "Nowhere" ? { state: "not found" } : { state: "connected", ip: "192.0.2.57", rssi: -52 } });
      return ["wifi set: ok, connecting"];
    }
    if (cmd === "wifi forget") {
      wf.next = null;
      Object.assign(wf, wfCard ? { state: "connected", ssid: wfCard, ip: "192.0.2.23", rssi: -58, src: "card" } : { state: "no credentials", ssid: "", ip: "0.0.0.0", rssi: 0, src: "none" });
      return ["wifi forget: ok"];
    }
    if (cmd === "wifi off") { wf.next = null; wf.state = "off"; wf.ip = "0.0.0.0"; wf.rssi = 0; return ["wifi: off"]; }
    if (cmd === "wifi on") { if (wf.state === "off") Object.assign(wf, wf.ssid ? { state: "connected", ip: "192.0.2.23", rssi: -58 } : { state: "no credentials" }); return ["wifi: on"]; }
    return ["wifi: unknown"];
  };
  // firmware facts: `ver`, `hw` (?demo&hw=spi: the card in SPI mode, &hw=ledoff: the LED disabled, &hw=pending: `hw pending 1`; combine with commas),
  // `dacmax`, and the DAC's sample-rate test `ratetest` (~8 s, one rate bad; ?demo&dac=none refuses it)
  const hwv = (q.get("hw") || "").split(",");
  const hwReply = () => [
    hwv.includes("spi") ? `hw sd clk=18 mosi=19 miso=20 cs=21 src=custom state=${cardSt === "ok" ? "mounted" : cardSt === "none" ? "none" : "failed"} mode=spi`
      : `hw sd clk=24 cmd=25 d0=20 d1=21 d2=22 d3=23 width=4 src=default state=${cardSt === "ok" ? "mounted" : cardSt === "none" ? "none" : "failed"} mode=sdmmc`,
    ...(hwv.includes("spi") ? [] : ["hw sdmmc routing=iomux clk=24 cmd=25 d0=20 d1=21 d2=22 d3=23 spi=any"]),
    `hw led gpio=60 order=grb on=${hwv.includes("ledoff") ? 0 : 1} src=default`, "hw reserved 26,27,28,29,30,31,32,33,34,35,36,37", "hw gpios 0-61", `hw pending ${hwv.includes("pending") ? 1 : 0}`];
  const RT_RATES = [44100, 48000, 96000, 192000, 384000], RT_STEP = 1600;
  const rtb = { state: "idle", t0: 0, n: 0, dac: "06cb:1594" };   // n: rates finished
  const rtTick = () => { if (rtb.state === "running") { rtb.n = Math.min(RT_RATES.length, Math.floor((Date.now() - rtb.t0) / RT_STEP)); if (rtb.n >= RT_RATES.length) rtb.state = "done"; } };
  const rtRow = (r, i) => {
    if (i >= rtb.n) return `ratetest rate=${r} set=na clock=0 xfer_ok=0 xfer_bad=0 fps=0 verdict=untested`;
    if (r === 384000) return `ratetest rate=${r} set=fail clock=192000 xfer_ok=0 xfer_bad=0 fps=0 verdict=bad`;
    if (r === 192000) return `ratetest rate=${r} set=ok clock=${r} xfer_ok=611 xfer_bad=139 fps=${r - 1} verdict=bad`;
    return `ratetest rate=${r} set=ok clock=${r} xfer_ok=${Math.round(r / 128)} xfer_bad=0 fps=${r - 1} verdict=ok`;
  };
  const rtReply = cmd => {
    rtTick();
    if (cmd === "ratetest cancel") { if (rtb.state !== "running") return ["ratetest: refused not running"]; rtb.state = "cancelled"; return ["ratetest: cancelled"]; }
    if (cmd === "ratetest" || cmd === "ratetest start") {
      if (rtb.state !== "running") {
        if (q.get("dac") === "none") return ["ratetest: refused no dac"];
        if (ctest.run) return ["ratetest: refused card test running"];
        if (fake.st !== "idle") { fake.bm = fake.len > 0 ? 1 : 0; fake.st = "idle"; }
        Object.assign(rtb, { state: "running", t0: Date.now(), n: 0 }); return ["ratetest: started"];
      }
    } else if (cmd !== "ratetest status") return ["ratetest [status|cancel]"];
    const out = [`ratetest state=${rtb.state} dac=${rtb.dac}` + (rtb.state === "running" ? ` now=${RT_RATES[Math.min(rtb.n, RT_RATES.length - 1)]}` : "")];
    if (rtb.state !== "idle") {
      RT_RATES.forEach((r, i) => out.push(rtRow(r, i)));
      if (rtb.state !== "running") out.push("ratetest max=" + (rtb.n >= RT_RATES.length ? 96000 : 0));
    }
    return out;
  };
  // the file tree for the Music page: /sdcard/packs has a zip archive (`ls` shows it with a trailing /, and lists its entries like a folder);
  // one entry in it is over 8 MB: listed, but `play` refuses it
  const TREE = { "/sdcard/packs": ["nes.zip/", "Loose Folder/", "08 - Bonus Track.flac"],
    "/sdcard/packs/nes.zip": ["01 - Title Theme.flac", "02 - Overworld.flac", "03 - Dungeon.flac", "99 - Full Soundtrack (huge).wav"] };
  const lsReply = cmd => TREE[cmd.slice(3).trim().replace(/\/+$/, "")] || ["packs/", "Albums/", "Chiptune/", "01 - Intro.flac", "02 - Harbour Lights.flac", "03 - Night Drive.flac", "04 - Slow Rain.mp3"];
  const playReply = cmd => {
    rtTick(); if (rtb.state === "running") return ["play: ESP_ERR_INVALID_STATE"];
    if (/nes\.zip\/.*huge/i.test(cmd)) return ["play: ESP_ERR_INVALID_SIZE"];
    if (/missing/i.test(cmd)) return ["play: ESP_ERR_NOT_FOUND"];
    fake.st = "playing"; fake.bm = 0; return ["play: ESP_OK"];
  };
  const reply = cmd => {
    if (cmd === "ver") return ["ver 0.1.0-ee14d98-dirty idf=v6.1 chip=esp32s31 built=874f52d83"];
    if (cmd === "hw") return hwReply();
    if (cmd === "ratetest" || cmd.startsWith("ratetest ")) return rtReply(cmd);
    if (cmd === "dacmax" || cmd.startsWith("dacmax ")) { const a = cmd.slice(7); if (a === "auto") fake.dacmax = 0; else if ([48000, 96000, 192000, 384000].includes(+a)) fake.dacmax = +a; return [fake.dacmax ? "dacmax " + fake.dacmax : "dacmax auto"]; }
    if (cmd === "wifi" || cmd.startsWith("wifi ")) return wifiReply(cmd);
    if (cmd === "time" || cmd.startsWith("time ")) return timeReply(cmd);
    if (cmd === "gapless" || cmd.startsWith("gapless ")) { if (cmd.endsWith(" on")) fake.gapless = 1; else if (cmd.endsWith(" off")) fake.gapless = 0; return ["gapless " + (fake.gapless ? "on" : "off")]; }
    if (cmd === "vollimit" || cmd.startsWith("vollimit ")) {
      const n = Math.round(+cmd.slice(9)); if (cmd.length > 8 && n >= 0 && n <= 100) { fake.vmax = n; for (const k in fakeVols) fakeVols[k] = Math.min(fakeVols[k], n); fake.vol = Math.min(fake.vol, n); }
      return ["vollimit " + fake.vmax];
    }
    if (cmd.startsWith("vol ")) { const n = Math.max(0, Math.min(fake.vmax, Math.round(+cmd.slice(4)) || 0)); fakeVols[volKey()] = n; fake.vol = n; return ["vol " + n]; }
    if (cmd === "st") {
      wfTick();
      if (fake.st === "playing") fake.pos = (fake.pos + 1000) % fake.len;
      const n = tick++;   // the first reply has no CPU figures yet (-1), like the box; then two wandering loads and a slowly sawing free heap
      fake.cpu = n === 0 ? [-1, -1] : [Math.round(10 + 6 * Math.sin(n / 5) + 4 * Math.random()), Math.round(18 + 10 * Math.sin(n / 3.3 + 1) + 5 * Math.random())];
      fake.mem = 101000 - ((n * 700) % 9000) + Math.round(300 * Math.random());
      if (ctest.run && Date.now() - ctest.t0 >= 6000) ctFinish();
      rtTick(); fake.card = cardSt;
      return [JSON.stringify(fake)];
    }
    if (cmd === "card") return cardReply();
    if (cmd === "cardtest" || cmd.startsWith("cardtest ")) return ctReply(cmd);
    if (cmd === "cardfmt" || cmd.startsWith("cardfmt ")) return cfReply(cmd);
    if (cmd.startsWith("rg ")) { const m = cmd.slice(3); fake.rg = Math.max(0, ["off", "track", "album"].indexOf(m)); return ["rg: " + m]; }
    if (cmd === "pause") { if (fake.st === "playing") fake.st = "paused"; return []; }
    if (cmd === "stop") { if (fake.st !== "idle") { fake.bm = fake.len > 0 ? 1 : 0; fake.st = "idle"; } return []; }
    if (cmd === "resume") {
      if (fake.st === "paused") { fake.st = "playing"; return []; }
      if (fake.st === "idle" && fake.bm) { fake.st = "playing"; fake.bm = 0; return ["resumed /sdcard/" + fake.file]; }
      return ["nothing to resume"];
    }
    if (cmd.startsWith("play ")) return playReply(cmd);
    if (cmd === "fx params") return fxLines();
    if (cmd.startsWith("fx ")) return fxCmd(cmd.split(" "));
    if (cmd.startsWith("ls")) return lsReply(cmd);
    if (cmd === "out" || cmd.startsWith("out ")) {
      const v = cmd.slice(4);
      if (v === "cast") {
        if (castOn) return ["out cast (playing on cast)"];
        if (castTry === 1) { castTry++; return ["out cast: ESP_ERR_NO_MEM"]; }   // ?demo&cast=fail: the first attempt fails
        castOn = true; castLoaded = true; fakeOutSel = "cast"; fakeOut(); fake.dac = 0;
        if (castFirst) { castFirst = false; setTimeout(() => fakeDrop(), 350); return ["out cast: starting, the Bluetooth control link reconnects in a moment"]; }
        return ["out cast (playing on cast)"];
      }
      if (["auto", "dac", "bt"].includes(v)) { castOn = false; fakeOutSel = v; fakeOut(); }
      return ["out " + fakeOutSel + " (playing on " + fake.out + ")"];
    }
    if (cmd === "cast") return [castOn ? `cast: on "NightStand Nano" frames=${castFrames += 100} underflows=${castBad ? 4 : 0} lc3_us_per_ch=2310 send_err=${castBad ? 2 : 0} gap_max_us=1840 streams=${castBad === 2 ? "10" : "11"}` : "cast: off" + (castLoaded ? " (LE Audio stays loaded until reboot)" : "")];
    if (cmd === "keys" || cmd.startsWith("keys ")) return keysReply(cmd);
    if (cmd === "night") return ["night: off, schedule 22:30-06:30"];
    if (cmd === "radio list") return stations.length ? stations.map((x, i) => `${i + 1} ${x.name} ${x.url}`) : ["radio: no stations"];
    if (cmd.startsWith("radio add ")) {
      const [name, url] = cmd.slice(10).split("|");
      if (!/^https?:\/\//.test(url || "")) return ["radio add: refused"];
      let i = stations.findIndex(x => x.url === url);
      if (i < 0) { if (stations.length >= 40) return ["radio add: list full"]; stations.push({ name, url }); i = stations.length - 1; }
      return ["radio add: " + (i + 1)];
    }
    if (cmd.startsWith("radio del ")) { const n = +cmd.slice(10); if (n >= 1 && n <= stations.length) { stations.splice(n - 1, 1); return ["radio del: ok"]; } return ["radio del: failed"]; }
    if (cmd === "lrc") {
      const b = enc.encode(DEMO_LRC), out = ["lrc begin " + b.length];
      for (let i = 0; i < b.length; i += 90) out.push("lrc+ " + btoa(String.fromCharCode(...b.subarray(i, i + 90))));
      return out.concat("lrc end");
    }
    if (/^radio \d+$/.test(cmd) || /^radio https?:/.test(cmd)) { fake.rstate = "buffering"; return ["ok"]; }
    if (cmd === "scene") return ["1 Cooling Summer Nap", "2 Soothing Colored Noise", "3 Campfire Rain"];
    return ["ok"];
  };
  const fakeCh = { writeValueWithResponse: b => {
    if (!fakeDev.gatt.connected) return Promise.reject(new Error("GATT Server is disconnected"));
    const c = dec.decode(b), res = reply(c);
    const emit = (lines, ms) => setTimeout(() => { if (fakeDev.gatt.connected) onEvt({ target: { value: enc.encode(lines.join("\n") + "\n") } }); }, ms);
    if (Array.isArray(res)) { if (res.length) emit(res, 5); } else for (const [ms, lines] of res.chunks) emit(lines, ms);
    return Promise.resolve(); } };
  app.dev = fakeDev;
  setGattSetup(async () => { if (!app.dev.gatt.connected) await app.dev.gatt.connect(); app.chCmd = fakeCh; });
  // a fake spectrum that keeps moving: a tilted floor, a beat on the low bands, a few wandering peaks, some sparkle on top
  setInterval(() => {
    const t = performance.now() / 1000, beat = Math.pow(0.5 + 0.5 * Math.sin(t * 7.5), 3);
    for (let i = 0; i < 32; i++) {
      const tilt = 150 * Math.exp(-i / 14), bump = 70 * Math.exp(-Math.pow((i - (16 + 12 * Math.sin(t * 0.7))) / 3, 2));
      const low = i < 8 ? 60 * beat : 0, spark = i > 20 ? 25 * Math.random() : 15 * Math.random();
      app.target[i] = Math.max(0, Math.min(255, Math.round(55 + tilt * (0.7 + 0.3 * Math.sin(t * 3 + i)) + bump + low + spark)));
    }
  }, 33);
  connect();
}
