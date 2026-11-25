// SPDX-License-Identifier: LGPL-2.1-or-later
import test from "node:test";
import assert from "node:assert/strict";
import { createConn, FILTERS } from "../src/js/core/conn.js";
import { createMockSerial } from "../src/js/demo/mock.js";
import { isVerLine, parseVer, isHwReadLine, parseHw, isAckLine, parseAck, parseReason, redact } from "../src/js/core/proto.js";

const quick = () => Promise.resolve();
const mk = (opts = {}, cOpts = {}) => { const serial = createMockSerial({ bootMs: 40, ...opts }); const conn = createConn({ serial, sleep: quick, ...cOpts }); return { serial, conn }; };
const wait = (ms) => new Promise((r) => setTimeout(r, ms));
const hwReq = (conn) => conn.request("hw", { accept: isHwReadLine, until: (ls) => ls.some((l) => /^hw pending/.test(l)), timeout: 1500 });

test("open: port chooser filters (CH340, CP210x, FTDI, Espressif), any port, cancel", async () => {
  const { serial, conn } = mk();
  assert.equal(await conn.open(), true);
  assert.deepEqual(serial.requestCalls[0], { filters: FILTERS });
  assert.deepEqual(FILTERS.map((f) => f.usbVendorId), [0x1a86, 0x10c4, 0x0403, 0x303a]);
  assert.match(conn.label, /CH340-family 1a86:7523/);
  await conn.close();
  assert.equal(conn.state, "idle");
  await conn.open({ any: true });
  assert.deepEqual(serial.requestCalls[1], {});
  await conn.close();
  const cancelled = { requestPort: async () => { const e = new Error("none"); e.name = "NotFoundError"; throw e; }, addEventListener() {} };
  assert.equal(await createConn({ serial: cancelled }).open(), false);
  await assert.rejects(createConn({ serial: null }).open(), (e) => e.code === "nosupport");
});

test("open resets the board the classic way (RTS pulse, DTR off) and the console is at 115200", async () => {
  const { serial, conn } = mk();
  await conn.open();
  const p = serial.ports[0];
  assert.equal(p.baud, 115200);
  assert.deepEqual(p.signals, [{ dataTerminalReady: false, requestToSend: true }, { dataTerminalReady: false, requestToSend: false }]);
  await conn.close();
});

test("ver and hw over a noisy, chunked console", async () => {
  for (const split of [0, 1, 4, 9]) {
    const { conn } = mk({ split, noise: true });
    await conn.open();
    const v = await conn.poll("ver", { accept: isVerLine, until: () => true, every: 100, total: 3000 });
    assert.equal(parseVer(v[0]).chip, "esp32s31");
    const h = parseHw(await hwReq(conn));
    assert.equal(h.sd.state, "mounted"); assert.equal(h.led.gpio, 60); assert.equal(h.pending, false);
    await conn.close();
  }
});

test("setters: ok, refusals with reasons, restore defaults, pending", async () => {
  const { conn } = mk({ noise: false });
  await conn.open(); await wait(60);
  const ack = async (cmd, kind) => parseAck((await conn.request(cmd, { accept: isAckLine(kind), until: () => true, timeout: 1000 }))[0]);
  assert.deepEqual(await ack("hw sd spi 10 11 12 13", "sd"), { kind: "sd", ok: true, text: "reboot to apply" });
  assert.equal(parseHw(await hwReq(conn)).pending, true);
  assert.equal((await ack("hw sd 24 25 20 21 22 23 3", "sd")).text, "width");
  assert.equal((await ack("hw sd spi 10 11 12 63", "sd")).text, "range");
  assert.equal((await ack("hw sd spi 10 11 12 59", "sd")).text, "reserved 59");
  assert.equal((await ack("hw sd spi 10 11 12 10", "sd")).text, "duplicate 10");
  assert.equal((await ack("hw led 60 xyz", "led")).text, "order");
  assert.equal((await ack("hw led 10 rgb", "led")).text, "sd/led conflict 10");
  assert.deepEqual(await ack("hw led 5 rgb", "led"), { kind: "led", ok: true, text: "reboot to apply" });
  assert.equal(parseHw(await hwReq(conn)).led.gpio, 5);
  assert.deepEqual(await ack("hw led off", "led"), { kind: "led", ok: true, text: "reboot to apply" });
  assert.equal(parseHw(await hwReq(conn)).led.on, false);
  assert.equal((await ack("hw reset", "reset")).ok, true);
  const h = parseHw(await hwReq(conn));
  assert.equal(h.sd.clk, 24); assert.equal(h.led.gpio, 60); assert.equal(h.sd.src, "default");
  assert.deepEqual(parseReason((await ack("hw sd 10 11 12 13 14 15 4", "sd")).text), { code: "notsup", why: "sdmmc-pins-fixed" });
  await conn.close();
});

test("reboot: no reply, the board comes back, the new settings run, card state follows the pins", async () => {
  const { conn } = mk({ bootMs: 60, noise: false });
  await conn.open();
  const ack = async (cmd, kind) => parseAck((await conn.request(cmd, { accept: isAckLine(kind), until: () => true, timeout: 1000 }))[0]);
  await wait(80);
  assert.equal(parseHw(await hwReq(conn)).sd.state, "mounted");
  await ack("hw sd spi 10 11 12 13", "sd");
  await conn.send("reboot");
  const lines = []; conn.on("line", (l) => lines.push(l));
  const r = await conn.poll("hw", { accept: isHwReadLine, until: (ls) => ls.some((l) => /^hw pending/.test(l)), every: 30, total: 3000 });
  const h = parseHw(r);
  assert.equal(h.pending, false);              // what is stored is what runs now
  assert.equal(h.sd.state, "none");            // nothing is wired to those pins
  assert.equal(h.sd.mode, "spi");
  assert.ok(lines.some((l) => /nsn: ready/.test(l)), "boot log reached the console");
  await ack("hw sd 24 25 20 21 22 23 4", "sd"); await conn.send("reboot");
  const r2 = parseHw(await conn.poll("hw", { accept: isHwReadLine, until: (ls) => ls.some((l) => /^hw pending/.test(l)), every: 30, total: 3000 }));
  assert.equal(r2.sd.state, "mounted"); assert.equal(r2.sd.mode, "sdmmc"); assert.equal(r2.sd.src, "default");
  await conn.close();
});

test("requests: timeout, one at a time, tx events", async () => {
  const { conn } = mk({ silent: true });
  await conn.open();
  const tx = []; conn.on("tx", (c) => tx.push(c));
  await assert.rejects(conn.request("ver", { accept: isVerLine, timeout: 60 }), (e) => e.code === "timeout");
  await assert.rejects(conn.poll("ver", { accept: isVerLine, every: 30, total: 100 }), (e) => e.code === "timeout");
  assert.ok(tx.length >= 3);
  await conn.close();
  await assert.rejects(conn.request("ver"), (e) => e.code === "notconn");
});

test("pause / resume lends the port to esptool and resets the board on the way back", async () => {
  const { conn, serial } = mk({ noise: false });
  await conn.open();
  const states = []; conn.on("state", (s) => states.push(s));
  const pending = conn.request("ver", { accept: () => false, timeout: 2000 });
  const port = await conn.pause("flash");
  assert.equal(port, serial.ports[0]);
  await assert.rejects(pending, (e) => e.code === "busy");
  await assert.rejects(conn.request("ver"), (e) => e.code === "busy");
  assert.equal(port.isOpen, false);
  assert.equal(conn.state, "flashing");
  await port.open({ baudRate: 460800 }); await port.close();   // what esptool-js does with it
  const before = port.signals.length;
  await conn.resume();
  assert.equal(conn.state, "console");
  assert.ok(port.signals.length > before, "reset on reopen");
  assert.deepEqual(states, ["flashing", "console"]);
  const v = await conn.poll("ver", { accept: isVerLine, until: () => true, every: 100, total: 3000 });
  assert.ok(parseVer(v[0]));
  await conn.close();
});

test("unplugging closes the connection and says it was not the user", async () => {
  const { conn, serial } = mk();
  await conn.open();
  let closed = null; conn.on("close", (byUser) => { closed = byUser; });
  const pending = conn.request("ver", { accept: () => false, timeout: 2000 });
  await wait(10);
  serial.ports[0].unplug();
  await assert.rejects(pending, (e) => e.code === "lost");
  assert.equal(conn.state, "idle"); assert.equal(closed, false);
});

test("wifi set typed by hand never shows its argument in the log pane (redact on rx and tx)", async () => {
  const { conn, serial } = mk({ noise: false });
  await conn.open();
  const shown = [];
  conn.on("tx", (c) => shown.push(redact(c))); conn.on("line", (l) => shown.push(redact(l)));
  await conn.send("wifi set MyNet hunter2");
  await wait(40);
  assert.ok(serial.ports[0].board.log.includes("wifi set MyNet hunter2"));   // the board got it
  assert.ok(shown.includes("wifi set ***"));
  assert.ok(!shown.join("\n").includes("hunter2"));
  await conn.close();
});
