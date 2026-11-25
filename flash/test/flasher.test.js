// SPDX-License-Identifier: LGPL-2.1-or-later
import test from "node:test";
import assert from "node:assert/strict";
import { existsSync } from "node:fs";
import { beginSession, romMd5, ESPTOOL_PATH, FLASH_BAUD, ROM_BAUD } from "../src/js/core/flasher.js";
import { createConn } from "../src/js/core/conn.js";
import { createMockSerial, createMockEsp } from "../src/js/demo/mock.js";
import { analyze, planWrite } from "../src/js/core/release.js";
import { releaseSet } from "../testlib/images.js";
import { md5Hex } from "../src/js/core/md5.js";

const setup = async (espOpts = {}) => {
  const serial = createMockSerial({ bootMs: 30, noise: false });
  const conn = createConn({ serial, sleep: () => Promise.resolve() });
  await conn.open();
  const esp = createMockEsp({ speed: 0, steps: 3, ...espOpts });
  return { serial, conn, esp };
};
const plan = async () => { const s = releaseSet(); const rel = await analyze([{ name: "b", data: s.bootloader }, { name: "p", data: s["partition-table"] }, { name: "a", data: s.app }]); return { rel, plan: planWrite(rel) }; };

test("session: ROM loader only (the stub is never uploaded), 115200 then 460800, chip and flash size read", async () => {
  const { conn, esp } = await setup();
  const logs = [];
  const s = await beginSession({ conn, esp, log: (l) => logs.push(l) });
  assert.equal(esp.state.stubCalls, 0);
  assert.equal(esp.state.loader.IS_STUB, false);
  assert.equal(esp.state.loader.romBaudrate, ROM_BAUD); assert.equal(esp.state.loader.baudrate, FLASH_BAUD);
  assert.deepEqual([ROM_BAUD, FLASH_BAUD], [115200, 460800]);
  assert.deepEqual(s.info, { chip: "ESP32-S31", flashSize: "16MB", flashBytes: 16 << 20, baud: 460800 });
  assert.ok(logs.some((l) => /no stub/.test(l)));
  assert.equal(conn.state, "flashing");
  assert.equal(await s.end(), false);
  assert.equal(conn.state, "console");
  await conn.close();
});
test("write: the regions go to 0x2000 / 0x8000 / 0x10000, progress runs 0..1, verification uses the chip's MD5", async () => {
  const { conn, esp } = await setup();
  const { plan: p } = await plan();
  const s = await beginSession({ conn, esp });
  const prog = [], phases = new Set();
  await s.write(p.parts, { onProgress: (f, m) => { prog.push(f); phases.add(m.phase); } });
  assert.deepEqual(esp.state.writeCalls[0].map((x) => x.address), [0x2000, 0x8000, 0x10000]);
  assert.deepEqual([...phases], ["write", "verify"]);
  assert.ok(prog.every((v, i) => i === 0 || v >= prog[i - 1]) && prog.at(-1) === 1);
  await s.end(); await conn.close();
});
test("write: a flash that differs from the file fails the check", async () => {
  const { conn, esp } = await setup({ corrupt: true });
  const { plan: p } = await plan();
  const s = await beginSession({ conn, esp });
  await assert.rejects(s.write(p.parts), (e) => e.code === "verify" && e.extra === "bootloader");
  await s.end(); await conn.close();
});
test("write: a failing write is retried, then reported; the console comes back", async () => {
  const { conn, esp } = await setup({ failAt: 0.2 });
  const { plan: p } = await plan();
  const logs = [], s = await beginSession({ conn, esp, log: (l) => logs.push(l), baud: ROM_BAUD });   // already at the slowest speed: nothing to fall back to
  await assert.rejects(s.write(p.parts, { retries: 1 }), /write failed/);
  assert.equal(logs.filter((l) => /^retry/.test(l)).length, 2);
  assert.equal(await s.end(), false);
  assert.equal(conn.state, "console");
  await conn.close();
});
test("a failed connect releases the port and resumes the console", async () => {
  const { conn, esp } = await setup();
  esp.ESPLoader.prototype.main = async function () { throw new Error("Failed to connect with the device"); };
  await assert.rejects(beginSession({ conn, esp }), /Failed to connect/);
  assert.equal(conn.state, "console");
  await conn.close();
});
test("MD5 answers: 16 raw bytes' hex (stub) and 32 ASCII characters hex-encoded again (ROM) both come out as the MD5", async () => {
  const want = md5Hex(new Uint8Array([1, 2, 3]));
  assert.equal(await romMd5({ flashMd5sum: async () => want }, 0, 3), want);
  assert.equal(await romMd5({ flashMd5sum: async () => [...want].map((c) => c.charCodeAt(0).toString(16)).join("") }, 0, 3), want);
});

// ---- the vendored esptool-js itself, executed (no hardware): what its code does for this chip and this kind of port ----
const bundle = new URL("../src/" + ESPTOOL_PATH, import.meta.url);
test("vendored esptool-js 0.7.1: ESP32-S31 ROM class, classic DTR/RTS reset for a CH340, USB reset only for the chip's own USB", async (t) => {
  if (!existsSync(bundle)) return t.skip("no vendored bundle");
  const E = await import(bundle.href);
  const fake = (vid, pid) => { const calls = []; return { calls, getInfo: () => "fake", getVid: () => vid, getPid: () => pid, setSignals: async (d, r) => { calls.push(["sig", !!d, !!r]); }, setRTS: async (r) => { calls.push(["rts", !!r]); }, setDTR: async (d) => { calls.push(["dtr", !!d]); }, tracing: false, trace() {}, connect: async () => {}, disconnect: async () => {} }; };
  const mkLoader = (tr) => new E.ESPLoader({ transport: tr, baudrate: 460800, romBaudrate: 115200, terminal: { clean() {}, writeLine() {}, write() {} } });
  // chip table: image chip id 32 is the S31, with the bootloader at 0x2000
  const l = mkLoader(fake(0x1a86, 0x7523));
  const rom = l.romFromChipId(32);
  assert.equal(rom.CHIP_NAME, "ESP32-S31"); assert.equal(rom.IMAGE_CHIP_ID, 32); assert.equal(rom.BOOTLOADER_FLASH_OFFSET, 0x2000);
  // the stub exists for this chip, which is why flasher.js must stop main() from uploading it
  assert.ok(await E.getStubJsonByChipName("ESP32-S31"));
  assert.equal(typeof E.ESPLoader.prototype.runStub, "function");
  // CH340 (0x1a86): the classic reset sequence, twice (50 ms and 550 ms strap delays), as esptool does for UART bridges
  const seq = l.constructResetSequence("default_reset");
  assert.equal(seq.length, 2);
  const tr = fake(0x1a86, 0x7523), l2 = mkLoader(tr), s2 = l2.constructResetSequence("default_reset");
  assert.deepEqual(s2.map((r) => r.resetDelay), [50, 550]);
  const t0 = Date.now(); await s2[0].reset();
  assert.deepEqual(tr.calls, [["sig", false, true], ["sig", true, false], ["sig", false, false]]);   // EN low (RTS), then IO0 low (DTR) with EN high, then released
  assert.ok(Date.now() - t0 >= 150);
  // the chip's own USB-Serial-JTAG (303a:1001) gets its own sequence instead
  const usb = mkLoader(fake(0x303a, 0x1001)).constructResetSequence("default_reset");
  assert.equal(usb.length, 1); assert.equal(usb[0].resetDelay, undefined);
  // after a write, esptool-js's hard reset only RELEASES RTS (no pulse): that is why the page resets through Conn.resume() instead
  const tr3 = fake(0x1a86, 0x7523), l3 = mkLoader(tr3);
  await l3.resetConstructors.hardReset(tr3, false).reset();
  assert.deepEqual(tr3.calls, [["rts", false]]);
});
test("vendored esptool-js 0.7.1: in ROM mode flashMd5sum() hex-encodes the ASCII answer (64 characters): the page's own check handles it", async (t) => {
  if (!existsSync(bundle)) return t.skip("no vendored bundle");
  const E = await import(bundle.href);
  const tr = { getInfo: () => "", getVid: () => 0x1a86, getPid: () => 0x7523, tracing: false, trace() {} };
  const l = new E.ESPLoader({ transport: tr, baudrate: 460800, romBaudrate: 115200, terminal: { clean() {}, writeLine() {}, write() {} } });
  const digest = md5Hex(new Uint8Array([9, 9, 9]));
  l.command = async () => [0, Uint8Array.from([...Buffer.from(digest, "ascii"), 0, 0])];   // the ROM's answer: 32 ASCII digits + 2 status bytes
  const raw = await l.flashMd5sum(0x2000, 3);
  assert.equal(raw.length, 64);
  assert.notEqual(raw, digest);   // esptool-js's own comparison would always fail in ROM mode
  assert.equal(await romMd5(l, 0x2000, 3), digest);
});

// ---- speed: 460800 by default, one automatic retry at 115200, a manual choice ----
test("speed: a chosen baud is used (230400); 115200 is a session without a switch", async () => {
  for (const b of [230400, ROM_BAUD]) {
    const { conn, esp } = await setup();
    const s = await beginSession({ conn, esp, baud: b });
    assert.equal(esp.state.loader.baudrate, b); assert.equal(s.info.baud, b);
    assert.equal(esp.state.mains, 1);
    await s.end(); await conn.close();
  }
});
test("speed: switching to 460800 fails -> the session is set up again at 115200, the user is told", async () => {
  const { conn, esp } = await setup({ maxBaud: 230400 });
  const notes = [], logs = [];
  const s = await beginSession({ conn, esp, log: (l) => logs.push(l), onFallback: (n) => notes.push(n) });
  assert.equal(esp.state.mains, 2);
  assert.equal(esp.state.loader.baudrate, ROM_BAUD); assert.equal(s.info.baud, ROM_BAUD);
  assert.equal(notes.length, 1); assert.deepEqual([notes[0].from, notes[0].to, notes[0].stage], [460800, 115200, "connect"]);
  assert.ok(logs.some((l) => /at 460800 failed/.test(l)));
  const { plan: p } = await plan();
  await s.write(p.parts);                       // and it flashes, at the slow speed
  assert.equal(esp.state.writeCalls.at(-1)[0].baud, ROM_BAUD);
  assert.equal(await s.end(), false); assert.equal(conn.state, "console");
  await conn.close();
});
test("speed: the first write at 460800 fails -> again at 115200 and the write goes through (only one retry session)", async () => {
  const { conn, esp } = await setup({ writeMaxBaud: 230400 });
  const notes = [];
  const s = await beginSession({ conn, esp, onFallback: (n) => notes.push(n) });
  assert.equal(s.info.baud, 460800); assert.equal(notes.length, 0);
  const { plan: p } = await plan();
  const prog = [];
  await s.write(p.parts, { onProgress: (f) => prog.push(f) });
  assert.equal(notes.length, 1); assert.deepEqual([notes[0].from, notes[0].to, notes[0].stage], [460800, 115200, "write"]);
  assert.deepEqual(esp.state.writeCalls.map((c) => c[0].baud), [460800, 115200]);   // one failed try, one that worked
  assert.equal(esp.state.mains, 2); assert.equal(s.info.baud, 115200);
  assert.equal(prog.at(-1), 1);
  await s.end(); await conn.close();
});
test("speed: when 115200 fails too, the error comes out (no endless retrying), the console comes back", async () => {
  const { conn, esp } = await setup({ failAt: 0.1 });
  const notes = [], { plan: p } = await plan();
  const s = await beginSession({ conn, esp, onFallback: (n) => notes.push(n) });
  await assert.rejects(s.write(p.parts), /write failed/);
  assert.equal(notes.length, 1);
  assert.equal(await s.end(), false); assert.equal(conn.state, "console");
  await conn.close();
  const { conn: c2, esp: e2 } = await setup({ maxBaud: 0 });   // not even 115200 syncs
  e2.ESPLoader.prototype.main = async function () { throw new Error("Failed to connect with the device"); };
  await assert.rejects(beginSession({ conn: c2, esp: e2 }), /Failed to connect/);
  assert.equal(e2.state.mains, 0); assert.equal(c2.state, "console");
  await c2.close();
});
