// SPDX-License-Identifier: LGPL-2.1-or-later
import test from "node:test";
import assert from "node:assert/strict";
import { createFramer, parseVer, describeVersion, parseHw, parseAck, parseReason, isHwReadLine, redact, sdCommand, ledCommand, validateSd, validateLed, stepGpio, pinReason } from "../src/js/core/proto.js";

// the real output of the firmware on the stock board (app/main/hw.c), as captured on the board
const HW = [
  "hw sd clk=24 cmd=25 d0=20 d1=21 d2=22 d3=23 width=4 src=default state=mounted mode=sdmmc",
  "hw sdmmc routing=iomux clk=24 cmd=25 d0=20 d1=21 d2=22 d3=23 spi=any",
  "hw led gpio=60 order=grb on=1 src=default",
  "hw reserved 26,27,28,29,30,31,32,33,34,36,37,38,39,40,41,58,59,60,61",
  "hw gpios 0-61",
  "hw pending 0",
];
const REAL_VER = "ver 0.1.0-ee14d98-dirty idf=v6.1 chip=esp32s31 built=874f52d83";
const hwObj = () => parseHw(HW);

test("ver line: the real one, and the version forms git gives", () => {
  assert.deepEqual(parseVer(REAL_VER), { version: "0.1.0-ee14d98-dirty", base: "0.1.0", commit: "ee14d98", dirty: true, idf: "v6.1", chip: "esp32s31", built: "874f52d83" });
  assert.deepEqual(parseVer("ver v0.1.0 idf=v6.1 chip=esp32s31 built=874f52d83"), { version: "v0.1.0", base: "0.1.0", commit: "", dirty: false, idf: "v6.1", chip: "esp32s31", built: "874f52d83" });
  assert.deepEqual(describeVersion("v1.2.3-4-gabc1234"), { base: "1.2.3", commit: "abc1234", dirty: false });
  assert.deepEqual(describeVersion("1.0.0-rc1"), { base: "1.0.0-rc1", commit: "", dirty: false });
  assert.deepEqual(describeVersion("2026.11.25"), { base: "2026.11.25", commit: "", dirty: false });
  assert.deepEqual(describeVersion("0.1.0-dirty"), { base: "0.1.0", commit: "", dirty: true });
  assert.equal(parseVer("I (12) ver 1.0"), null);
  assert.equal(parseVer("version 1"), null);
});

test("hw reply parsed: the real stock-board output", () => {
  const h = hwObj();
  assert.deepEqual(h.sd, { mode: "sdmmc", clk: 24, cmd: 25, d0: 20, d1: 21, d2: 22, d3: 23, width: 4, src: "default", state: "mounted" });
  assert.deepEqual(h.sdmmc, { routing: "iomux", clk: 24, cmd: 25, d0: 20, d1: 21, d2: 22, d3: 23, spi: "any" });
  assert.deepEqual(h.led, { gpio: 60, order: "grb", on: true, src: "default" });
  assert.deepEqual(h.reserved, [26, 27, 28, 29, 30, 31, 32, 33, 34, 36, 37, 38, 39, 40, 41, 58, 59, 60, 61]);
  assert.deepEqual(h.gpios, { min: 0, max: 61 });
  assert.equal(h.pending, false); assert.equal(h.complete, true);
});
test("hw: SPI line (clk mosi miso cs, no width), width 1, LED off, pending 1", () => {
  const h = parseHw(["hw sd clk=10 mosi=11 miso=12 cs=13 src=custom state=none mode=spi", HW[1], "hw led gpio=-1 order=grb on=0 src=custom", "hw reserved ", "hw gpios 0-48", "hw pending 1"]);
  assert.deepEqual(h.sd, { mode: "spi", clk: 10, mosi: 11, miso: 12, cs: 13, src: "custom", state: "none" });
  assert.equal(h.led.on, false); assert.equal(h.led.gpio, -1);
  assert.deepEqual(h.reserved, []); assert.equal(h.gpios.max, 48); assert.equal(h.pending, true);
  const w1 = parseHw(["hw sd clk=24 cmd=25 d0=20 d1=-1 d2=-1 d3=-1 width=1 src=custom state=mounted mode=sdmmc", HW[5]]);
  assert.deepEqual([w1.sd.mode, w1.sd.width, w1.sd.d1], ["sdmmc", 1, -1]);
});
test("hw: incomplete reply is flagged", () => {
  assert.equal(parseHw(HW.slice(0, 3)).complete, false);
});

test("framer: CRLF, LF, lone CR, empty lines, ANSI colours", () => {
  const f = createFramer();
  assert.deepEqual(f.push("a\r\nb\nc\rd\r\n\r\n"), ["a", "b", "c", "d"]);
  assert.deepEqual(f.push("\x1b[0;32mI (1) x: ok\x1b[0m\r\n"), ["I (1) x: ok"]);
});
test("framer: partial lines split across chunks (text and bytes)", () => {
  const f = createFramer();
  assert.deepEqual(f.push("hw sd clk=24 cm"), []);
  assert.deepEqual(f.push("d=25 d0=20\r"), ["hw sd clk=24 cmd=25 d0=20"]);
  assert.deepEqual(f.push("\nhw pend"), []);
  const enc = new TextEncoder().encode("ing 1\r\n");
  assert.deepEqual(f.push(enc.subarray(0, 3)), []);
  assert.deepEqual(f.push(enc.subarray(3)), ["hw pending 1"]);
  // a multi-byte character cut in the middle of its bytes
  const g = createFramer(), u = new TextEncoder().encode("é\n");
  assert.deepEqual([...g.push(u.subarray(0, 1)), ...g.push(u.subarray(1))], ["é"]);
});

test("a noisy log: reply lines are found by prefix, in any chunking", () => {
  const stream = "I (100) boot: hello\r\n" + HW[0] + "\r\nW (101) wifi: scan\r\n" + HW[1] + "\r\n" + HW[2] + "\r\n\x1b[0;33mI (3) x: y\x1b[0m\r\n" + HW[3] + "\r\n" + HW[4] + "\r\nI (9) night: tick\r\n" + HW[5] + "\r\nD (10) z\r\n";
  for (const step of [1, 2, 3, 5, 7, 64, stream.length]) {
    const f = createFramer(), got = [];
    for (let i = 0; i < stream.length; i += step) for (const l of f.push(stream.slice(i, i + step))) if (isHwReadLine(l)) got.push(l);
    assert.deepEqual(got, HW, "step " + step);
    assert.equal(parseHw(got).sd.clk, 24);
  }
  // log lines that merely contain the words are not replies
  assert.equal(isHwReadLine("I (5) hw: hw sd clk=1"), false);
  assert.equal(isHwReadLine("hw sd: ok, reboot to apply"), false);
  assert.equal(isHwReadLine("hw sd 24 25 20 21 22 23 4"), false);   // an echo of our own command
  assert.equal(isHwReadLine("hw sd spi 1 2 3 4"), false);
  assert.equal(isHwReadLine("hw sd: refused not supported sdmmc-pins-fixed"), false);
  assert.equal(isHwReadLine(HW[1]), true);
});

test("acks and refusals", () => {
  assert.deepEqual(parseAck("hw sd: ok, reboot to apply"), { kind: "sd", ok: true, text: "reboot to apply" });
  assert.deepEqual(parseAck("hw led: refused reserved 59"), { kind: "led", ok: false, text: "reserved 59" });
  assert.deepEqual(parseAck("hw reset: ok, reboot to apply"), { kind: "reset", ok: true, text: "reboot to apply" });
  assert.equal(parseAck("hw sd clk=1"), null);
  assert.deepEqual(parseReason("range"), { code: "range" });
  assert.deepEqual(parseReason("reserved 59"), { code: "reserved", n: 59 });
  assert.deepEqual(parseReason("duplicate 21"), { code: "duplicate", n: 21 });
  assert.deepEqual(parseReason("width"), { code: "width" });
  assert.deepEqual(parseReason("order"), { code: "order" });
  assert.deepEqual(parseReason("sd/led conflict 60"), { code: "conflict", n: 60 });
  assert.deepEqual(parseReason("not supported sdmmc-pins-fixed"), { code: "notsup", why: "sdmmc-pins-fixed" });
  assert.deepEqual(parseReason("syntax"), { code: "syntax" });
  assert.deepEqual(parseReason("storage"), { code: "storage" });
  assert.deepEqual(parseAck("hw sd: refused not supported sdmmc-pins-fixed"), { kind: "sd", ok: false, text: "not supported sdmmc-pins-fixed" });
  assert.deepEqual(parseReason("nvs write failed 0x105"), { code: "other", raw: "nvs write failed 0x105" });
});

test("command lines", () => {
  const h = hwObj();
  assert.equal(sdCommand({ clk: 24, cmd: 25, d0: 20, d1: 21, d2: 22, d3: 23, width: 4 }), "hw sd 24 25 20 21 22 23 4");
  assert.equal(sdCommand({ clk: 24, cmd: 25, d0: 20, d1: 21, d2: 22, d3: 23, width: 1 }), "hw sd 24 25 20 -1 -1 -1 1");
  assert.equal(ledCommand({ on: true, gpio: 60, order: "rgb" }), "hw led 60 rgb");
  assert.equal(ledCommand({ on: false, gpio: 60, order: "rgb" }), "hw led off");
  assert.equal(sdCommand({ mode: "spi", clk: 10, mosi: 11, miso: 12, cs: 13 }), "hw sd spi 10 11 12 13");
  assert.ok(h);
});

test("wifi set is redacted", () => {
  assert.equal(redact("wifi set MyNet hunter2"), "wifi set ***");
  assert.equal(redact("  WIFI   SET  a b"), "  WIFI   SET ***");
  assert.equal(redact("wifi set"), "wifi set ***");
  assert.equal(redact("wifi status"), "wifi status");
  assert.equal(redact("hw"), "hw");
  assert.equal(redact("echo wifi set x"), "echo wifi set x");
});

const MMC = { mode: "sdmmc", clk: 24, cmd: 25, d0: 20, d1: 21, d2: 22, d3: 23, width: 4 };
const SPI = { mode: "spi", clk: 10, mosi: 11, miso: 12, cs: 13 };
test("SD validation, SDMMC: only the fixed pins and width 4 or 1; reserved default pins are fine", () => {
  const hw = hwObj();
  assert.deepEqual(validateSd(MMC, hw), []);
  assert.deepEqual(validateSd({ ...MMC, width: 1, d1: -1, d2: -1, d3: -1 }, hw), []);
  assert.deepEqual(validateSd({ ...MMC, width: 1 }, hw), [{ field: "width", code: "width" }]);   // d1..d3 must be -1 at width 1
  assert.deepEqual(validateSd({ ...MMC, width: 2 }, hw), [{ field: "width", code: "width" }]);
  assert.deepEqual(validateSd({ ...MMC, clk: 10 }, hw), [{ field: "mode", code: "notsup" }]);          // free pin, but SDMMC cannot move
  assert.deepEqual(validateSd({ ...MMC, d3: 23, d2: 21, d1: 22 }, hw), [{ field: "mode", code: "notsup" }]);
  assert.deepEqual(validateSd({ ...MMC, d3: 59 }, hw), [{ field: "d3", code: "reserved", n: 59 }]);
  assert.deepEqual(validateSd(MMC, { ...hw, reserved: [...hw.reserved, 24, 23] }), []);   // a board default is always accepted for its own function, even when the table lists it
  assert.deepEqual(validateSd(MMC, hw, { ...hw.led, gpio: 24, on: true }), [{ field: "clk", code: "conflict", n: 24 }]);
  assert.deepEqual(validateSd({ ...MMC, width: 1, d1: -1, d2: -1, d3: -1 }, hw, { ...hw.led, gpio: 23, on: true }), []);   // d3 is not used at width 1
});
test("SD validation, SPI: int, range, nopin (29, 41), reserved, duplicate, LED conflict; a default SD pin is allowed in SPI mode", () => {
  const hw = hwObj();
  assert.deepEqual(validateSd(SPI, hw), []);
  assert.deepEqual(validateSd({ ...SPI, cs: NaN }, hw), [{ field: "cs", code: "int" }]);
  assert.deepEqual(validateSd({ ...SPI, cs: 1.5 }, hw), [{ field: "cs", code: "int" }]);
  assert.deepEqual(validateSd({ ...SPI, cs: 62 }, hw), [{ field: "cs", code: "range", n: 62 }]);
  assert.deepEqual(validateSd({ ...SPI, cs: -1 }, hw), [{ field: "cs", code: "range", n: -1 }]);
  assert.deepEqual(validateSd({ ...SPI, cs: 29 }, hw), [{ field: "cs", code: "nopin", n: 29 }]);
  assert.deepEqual(validateSd({ ...SPI, mosi: 41 }, hw), [{ field: "mosi", code: "nopin", n: 41 }]);
  assert.deepEqual(validateSd({ ...SPI, miso: 26 }, hw), [{ field: "miso", code: "reserved", n: 26 }]);
  assert.deepEqual(validateSd({ ...SPI, clk: 60 }, hw), [{ field: "clk", code: "reserved", n: 60 }]);   // 60 is only the stock LED's
  const dup = validateSd({ ...SPI, miso: 10 }, hw);
  assert.deepEqual(dup.map((i) => i.field).sort(), ["clk", "miso"]); assert.ok(dup.every((i) => i.code === "duplicate" && i.n === 10));
  assert.deepEqual(validateSd({ ...SPI, cs: 60 }, { ...hw, led: { ...hw.led, on: false } }), [{ field: "cs", code: "reserved", n: 60 }]);
  assert.deepEqual(validateSd({ ...SPI, cs: 21 }, hw), []);                                            // the stock SD pins stay allowed (not reserved anyway)
  assert.deepEqual(validateSd({ ...SPI, cs: 60 }, hw, { ...hw.led, gpio: 60, on: true })[0].code, "reserved");   // reserved comes before the LED conflict
  assert.deepEqual(validateSd({ ...SPI, cs: 5 }, hw, { ...hw.led, gpio: 5, on: true }), [{ field: "cs", code: "conflict", n: 5 }]);
  assert.deepEqual(validateSd({ ...SPI, cs: 5 }, hw, { ...hw.led, gpio: 5, on: false }), []);
});
test("LED validation: the stock LED pin is allowed although reserved, other reserved pins are not", () => {
  const hw = { ...hwObj(), ledDefault: 60 };
  assert.deepEqual(validateLed({ on: true, gpio: 60, order: "grb" }, hw), []);
  assert.deepEqual(validateLed({ on: true, gpio: 60, order: "grb" }, hwObj()), [{ field: "led", code: "reserved", n: 60 }]);   // default not known (LED was changed before this page started)
  assert.deepEqual(validateLed({ on: false, gpio: 9999, order: "x" }, hw), []);   // off: nothing to check
  assert.deepEqual(validateLed({ on: true, gpio: 61, order: "grb" }, hw), [{ field: "led", code: "reserved", n: 61 }]);
  assert.deepEqual(validateLed({ on: true, gpio: 41, order: "grb" }, hw), [{ field: "led", code: "nopin", n: 41 }]);
  assert.deepEqual(validateLed({ on: true, gpio: 70, order: "grb" }, hw), [{ field: "led", code: "range", n: 70 }]);
  assert.deepEqual(validateLed({ on: true, gpio: 24, order: "grb" }, hw), [{ field: "led", code: "conflict", n: 24 }]);
  assert.deepEqual(validateLed({ on: true, gpio: 21, order: "grb" }, { ...hw, sd: { ...hw.sd, width: 1 } }), []);
  assert.deepEqual(validateLed({ on: true, gpio: 11, order: "grb" }, { ...hw, sd: { mode: "spi", clk: 10, mosi: 11, miso: 12, cs: 13 } }), [{ field: "led", code: "conflict", n: 11 }]);
  assert.deepEqual(validateLed({ on: true, gpio: 30, order: "zzz" }, hw).map((i) => i.field), ["order", "led"]);   // the firmware checks the order first
});
test("reasons for the reserved pins (docs/porting.md)", () => {
  const g = (n) => pinReason(n);
  assert.deepEqual([26, 27, 28, 30, 31, 32].map(g), Array(6).fill("flash"));
  assert.deepEqual([33, 34].map(g), ["usb", "usb"]);
  assert.deepEqual([36, 37, 38, 39, 40, 61].map(g), Array(6).fill("strap"));
  assert.deepEqual([58, 59].map(g), ["console", "console"]);
  assert.equal(g(60), "strap60"); assert.deepEqual([29, 41].map(g), ["nopin", "nopin"]);
  // every pin of the real list has a reason that is not the generic one
  for (const n of hwObj().reserved) assert.notEqual(g(n), "other", "GPIO " + n);
});
test("stepGpio skips reserved numbers and stops at the ends", () => {
  const hw = hwObj();
  assert.equal(stepGpio(57, 1, hw), null);   // 58..61 are reserved and 61 is the last pin
  assert.equal(stepGpio(25, 1, hw), 35);
  assert.equal(stepGpio(35, -1, hw), 25);
  assert.equal(stepGpio(40, 1, hw), 42);
  assert.equal(stepGpio(0, -1, hw), null);
  assert.equal(stepGpio(10, 1, hw), 11);
});
