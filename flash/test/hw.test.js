// SPDX-License-Identifier: LGPL-2.1-or-later
// The pretend board against the real firmware (app/main/hw.c): same lines, same refusals, same order; and the page's validation against the pretend board.
import test from "node:test";
import assert from "node:assert/strict";
import { createMockBoard } from "../src/js/demo/mock.js";
import { parseHw, parseVer, parseAck, validateSd, validateLed, sdCommand, ledCommand } from "../src/js/core/proto.js";

// what the firmware printed on the stock board
const REAL_HW = [
  "hw sd clk=24 cmd=25 d0=20 d1=21 d2=22 d3=23 width=4 src=default state=mounted mode=sdmmc",
  "hw sdmmc routing=iomux clk=24 cmd=25 d0=20 d1=21 d2=22 d3=23 spi=any",
  "hw led gpio=60 order=grb on=1 src=default",
  "hw reserved 26,27,28,29,30,31,32,33,34,36,37,38,39,40,41,58,59,60,61",
  "hw gpios 0-61",
  "hw pending 0",
];
const REAL_VER = "ver 0.1.0-ee14d98-dirty idf=v6.1 chip=esp32s31 built=874f52d83";

const dev = (opts = {}) => {
  const b = createMockBoard({ noise: false, bootMs: 0, ...opts }), lines = [];
  b.attach((t) => lines.push(...t.split("\r\n").filter(Boolean)));
  const say = (c) => { lines.length = 0; b.receive(c); return [...lines]; };
  const reboot = () => { b.boot(); return new Promise((r) => setTimeout(r, 15)).then(() => { lines.length = 0; }); };
  return { b, say, reboot, hw: () => parseHw(say("hw")) };
};

test("mock: ver and hw print exactly what the real firmware printed", () => {
  const d = dev();
  assert.deepEqual(d.say("ver"), [REAL_VER]);
  assert.deepEqual(d.say("hw"), REAL_HW);
  assert.deepEqual(parseVer(d.say("ver")[0]).built, "874f52d83");
});

test("mock: SDMMC refuses every pin set but slot 0's (not supported sdmmc-pins-fixed), whatever else is right", () => {
  const d = dev();
  for (const c of ["hw sd 10 11 12 13 14 15 4", "hw sd 25 24 20 21 22 23 4", "hw sd 24 25 20 22 21 23 4", "hw sd 24 25 21 -1 -1 -1 1", "hw sd 24 25 20 21 22 5 4"])
    assert.deepEqual(d.say(c), ["hw sd: refused not supported sdmmc-pins-fixed"], c);
  assert.equal(d.hw().pending, false);
});
test("mock: the refusals of hw.c, in its order", () => {
  const d = dev();
  const r = (c) => d.say(c)[0];
  assert.equal(r("hw sd 24 25 20 21 22 23 3"), "hw sd: refused width");
  assert.equal(r("hw sd 24 25 20 21 22 23 1"), "hw sd: refused width");           // width 1 needs d1..d3 = -1
  assert.equal(r("hw sd 24 25 20 -1 -1 -1 4"), "hw sd: refused range");           // -1 is no GPIO at width 4
  assert.equal(r("hw sd 24 25 20 21 22 29 4"), "hw sd: refused range");           // GPIO 29 does not exist (range before reserved)
  assert.equal(r("hw sd 24 25 20 21 22 62 4"), "hw sd: refused range");
  assert.equal(r("hw sd 24 25 20 21 22 128 4"), "hw sd: refused range");          // does not fit the stored int8
  assert.equal(r("hw sd 24 25 20 21 22 5000 4"), "hw sd: refused syntax");        // not a believable number
  assert.equal(r("hw sd 24 25 20 21 22 x 4"), "hw sd: refused syntax");
  assert.equal(r("hw sd 24 25 20 21 22 23"), "hw sd: refused syntax");
  assert.equal(r("hw sd 24 25 20 21 22 23 4 5"), "hw sd: refused syntax");
  assert.equal(r("hw sd 24 25 20 21 22 26 4"), "hw sd: refused reserved 26");
  assert.equal(r("hw sd 24 25 20 21 22 22 4"), "hw sd: refused duplicate 22");
  assert.equal(r("hw sd 24 26 26 21 22 23 4"), "hw sd: refused reserved 26");     // reserved before duplicate
  assert.equal(r("hw sd spi 60 11 12 13"), "hw sd: refused reserved 60");         // 60 is the stock LED's only
  assert.equal(r("hw sd spi 41 11 12 13"), "hw sd: refused range");
  assert.equal(r("hw sd spi 10 11 12 10"), "hw sd: refused duplicate 10");
  assert.equal(r("hw sd spi 10 11 12"), "hw sd: refused syntax");
  assert.equal(r("hw sd spi 10 11 12 13 14"), "hw sd: refused syntax");
  assert.equal(r("hw sd spi 10 11 12 -1"), "hw sd: refused range");
  assert.equal(r("hw led 61"), "hw led: refused reserved 61");
  assert.equal(r("hw led 40"), "hw led: refused reserved 40");
  assert.equal(r("hw led 29"), "hw led: refused range");
  assert.equal(r("hw led 128"), "hw led: refused range");
  assert.equal(r("hw led -1"), "hw led: refused range");
  assert.equal(r("hw led x"), "hw led: refused syntax");
  assert.equal(r("hw led 5 6 7"), "hw led: refused syntax");
  assert.equal(r("hw led off grb"), "hw led: refused syntax");
  assert.equal(r("hw led 5 xyz"), "hw led: refused order");
  assert.equal(r("hw led 5 rgbx"), "hw led: refused order");
  assert.equal(r("hw led 24"), "hw led: refused sd/led conflict 24");
  assert.deepEqual(["hw", "hw sd", "hw led", "hw reset x", "hw bogus"].slice(1).map((c) => d.say(c)), [["?"], ["?"], ["?"], ["?"]]);
  assert.equal(d.hw().pending, false);   // nothing above stored anything
});

test("mock: SDMMC 1-bit, then SPI, restart, back to SDMMC (the flow of the page)", async () => {
  const d = dev();
  assert.deepEqual(d.say("hw sd 24 25 20 -1 -1 -1 1"), ["hw sd: ok, reboot to apply"]);
  let h = d.say("hw");
  assert.equal(h[0], "hw sd clk=24 cmd=25 d0=20 d1=-1 d2=-1 d3=-1 width=1 src=custom state=mounted mode=sdmmc");
  assert.equal(h.at(-1), "hw pending 1");
  assert.deepEqual(d.say("hw sd spi 10 11 12 13"), ["hw sd: ok, reboot to apply"]);
  h = d.say("hw");
  assert.equal(h[0], "hw sd clk=10 mosi=11 miso=12 cs=13 src=custom state=mounted mode=spi"); // state is the RUNNING mount: still the SDMMC card
  assert.equal(h.at(-1), "hw pending 1");
  assert.deepEqual(d.say("hw led 10"), ["hw led: refused sd/led conflict 10"]);   // the stored SPI pins count
  await d.reboot();
  h = d.say("hw");
  assert.equal(h[0], "hw sd clk=10 mosi=11 miso=12 cs=13 src=custom state=none mode=spi");   // nothing is wired to those pins in the mock
  assert.equal(h[1], REAL_HW[1]); assert.equal(h.at(-1), "hw pending 0");
  assert.deepEqual(d.say("hw sd 24 25 20 21 22 23 4"), ["hw sd: ok, reboot to apply"]);
  h = d.say("hw");
  assert.equal(h[0], "hw sd clk=24 cmd=25 d0=20 d1=21 d2=22 d3=23 width=4 src=default state=none mode=sdmmc");   // the default again: nothing stored
  assert.equal(h.at(-1), "hw pending 1");
  await d.reboot();
  assert.deepEqual(d.say("hw"), REAL_HW);
  // a card that is wired to the SPI pins
  const w = dev({ spiCard: "mounted" });
  w.say("hw sd spi 10 11 12 13"); await w.reboot();
  assert.match(w.say("hw")[0], /state=mounted mode=spi$/);
  // hw reset
  assert.deepEqual(w.say("hw reset"), ["hw reset: ok, reboot to apply"]);
  assert.equal(w.say("hw").at(-1), "hw pending 1");
});

test("mock: the LED setters (default order grb when none is given, off, the stock pin 60 is allowed, back to default is not custom)", () => {
  const d = dev();
  assert.deepEqual(d.say("hw led 5 rgb"), ["hw led: ok, reboot to apply"]);
  assert.equal(d.say("hw")[2], "hw led gpio=5 order=rgb on=1 src=custom");
  assert.deepEqual(d.say("hw led 6"), ["hw led: ok, reboot to apply"]);
  assert.equal(d.say("hw")[2], "hw led gpio=6 order=grb on=1 src=custom");
  assert.deepEqual(d.say("hw led off"), ["hw led: ok, reboot to apply"]);
  assert.equal(d.say("hw")[2], "hw led gpio=-1 order=grb on=0 src=custom");
  assert.deepEqual(d.say("hw led 60"), ["hw led: ok, reboot to apply"]);          // reserved, but the board's own
  assert.equal(d.say("hw")[2], REAL_HW[2]);
  assert.equal(d.say("hw")[5], "hw pending 0");                                   // stored == default == what runs (nothing was rebooted)
});

test("page validation = firmware: the first issue is the firmware's reason, over many random settings", () => {
  const d = dev();
  let seed = 12345;
  const rnd = (n) => { seed = (seed * 1103515245 + 12345) & 0x7fffffff; return seed % n; };
  const pool = [24, 25, 20, 21, 22, 23, 10, 11, 12, 13, 60, 26, 29, 41, 5, -1, 61, 62, 40, 58, 35, 0, 1, 33, 99, 127, 128];
  const pick = () => { const k = rnd(8); return k < 3 ? 3 + rnd(20) : k === 3 ? rnd(135) - 3 : pool[rnd(pool.length)]; };
  const reason = (i) => ({ range: "range", nopin: "range", reserved: `reserved ${i.n}`, duplicate: `duplicate ${i.n}`, conflict: `sd/led conflict ${i.n}`, width: "width", notsup: "not supported sdmmc-pins-fixed", order: "order" })[i.code];
  let accepted = 0, refused = 0;
  for (let k = 0; k < 4000; k++) {
    d.say(rnd(3) === 0 ? "hw reset" : `hw led ${pick()} ${["grb", "rgb"][rnd(2)]}`);
    const hw = { ...parseHw(d.say("hw")) };
    if (hw.led.src === "default") hw.ledDefault = hw.led.gpio; else hw.ledDefault = 60;   // the page knows the default once it has seen it; the test knows it
    let sd;
    const m = rnd(4);
    if (m === 0) sd = { mode: "spi", clk: pick(), mosi: pick(), miso: pick(), cs: pick() };
    else if (m === 1) sd = { mode: "sdmmc", clk: 24, cmd: 25, d0: 20, d1: 21, d2: 22, d3: 23, width: 4 };
    else if (m === 2) sd = { mode: "sdmmc", clk: 24, cmd: 25, d0: 20, d1: -1, d2: -1, d3: -1, width: 1 };
    else sd = { mode: "sdmmc", clk: pick(), cmd: pick(), d0: pick(), d1: pick(), d2: pick(), d3: pick(), width: [4, 4, 1, 2][rnd(4)] };
    const got = d.say(sdCommand(sd))[0], issues = validateSd(sd, hw);
    if (/ok,/.test(got)) { accepted++; assert.deepEqual(issues, [], sdCommand(sd)); } else { refused++; assert.equal(got, "hw sd: refused " + reason(issues[0]), sdCommand(sd) + " " + JSON.stringify(issues)); }
    // the LED against the stored SD setting
    const led = { on: rnd(8) !== 0, gpio: pick(), order: ["grb", "rgb", "xyz"][rnd(3)] };
    const lgot = d.say(ledCommand(led))[0], li = validateLed(led, { ...parseHw(d.say("hw")), ledDefault: 60 });
    if (/ok,/.test(lgot)) assert.deepEqual(li, [], ledCommand(led)); else assert.equal(lgot, "hw led: refused " + reason(li[0]), ledCommand(led) + " " + JSON.stringify(li));
  }
  assert.ok(accepted > 200 && refused > 500, `${accepted} accepted, ${refused} refused`);
});
