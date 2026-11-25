// SPDX-License-Identifier: LGPL-2.1-or-later
import test from "node:test";
import assert from "node:assert/strict";
import { existsSync, readFileSync } from "node:fs";
import { analyze, planWrite, parseName, chipCheck, flashParams, LAYOUT, DEFAULT_NVS } from "../src/js/core/release.js";
import { releaseSet, mergedImage, espImage, appDesc, partitionTable, DEFAULT_ENTRIES } from "../testlib/images.js";

const f = (name, data) => ({ name, data });
const three = (s = releaseSet()) => [f("NightStandNano-1.2.3-bootloader.bin", s.bootloader), f("NightStandNano-1.2.3-partition-table.bin", s["partition-table"]), f("NightStandNano-1.2.3-app.bin", s.app)];
const fails = async (files, code) => { await assert.rejects(analyze(files), (e) => e.code === code, "expected " + code); };

test("names", () => {
  assert.deepEqual(parseName("NightStandNano-1.2.3-full.bin"), { kind: "full", version: "1.2.3" });
  assert.deepEqual(parseName("NightStandNano-2026.11.25-rc1-full.bin"), { kind: "full", version: "2026.11.25-rc1" });
  assert.deepEqual(parseName("NightStandNano-1.2.3-app.bin"), { kind: "part", role: "app", version: "1.2.3" });
  assert.deepEqual(parseName("NightStandNano-1.2.3-partition-table.bin"), { kind: "part", role: "partition-table", version: "1.2.3" });
  assert.deepEqual(parseName("nightstand_nano.bin"), { kind: "raw", role: "app" });
  assert.deepEqual(parseName("C:\\dl\\bootloader.bin"), { kind: "raw", role: "bootloader" });
  assert.equal(parseName("jukedial-1-round.bin"), null);
  assert.equal(parseName("NightStandNano-full.bin"), null);
});

test("merged image: accepted, three regions, settings area untouched", async () => {
  const s = releaseSet(), img = mergedImage(s);
  const r = await analyze([f("NightStandNano-1.2.3-full.bin", img)]);
  assert.equal(r.kind, "merged"); assert.equal(r.version, "1.2.3"); assert.equal(r.projectName, "nightstand_nano");
  assert.deepEqual(r.parts.map((p) => [p.role, p.offset]), [["bootloader", 0x2000], ["partition-table", 0x8000], ["app", 0x10000]]);
  assert.deepEqual(r.parts[0].data, s.bootloader);
  assert.deepEqual(r.parts[2].data, s.app);
  assert.deepEqual(r.nvs, { off: 0x9000, size: 0x6000, fromTable: true });
  assert.equal(r.flashNeeded, 0x10000 + 0xf00000);
  assert.equal(r.flash.matches, true);
  assert.match(r.sha256["NightStandNano-1.2.3-full.bin"], /^[0-9a-f]{64}$/);
});
test("three files: accepted in any order, by content", async () => {
  const s = releaseSet(), t = three(s);
  for (const order of [[0, 1, 2], [2, 1, 0], [1, 2, 0]]) {
    const r = await analyze(order.map((i) => t[i]));
    assert.equal(r.kind, "three");
    assert.deepEqual(r.parts.map((p) => p.role), ["bootloader", "partition-table", "app"]);
    assert.equal(r.version, "1.2.3");
  }
  // raw build names work, wrong names do not matter
  const r = await analyze([f("x.bin", s.app), f("y.bin", s.bootloader), f("z.bin", s["partition-table"])]);
  assert.equal(r.kind, "three");
});
test("write plan: offsets and sizes, settings kept", async () => {
  const s = releaseSet(), r = await analyze(three(s)), p = planWrite(r);
  assert.deepEqual(p.parts.map((x) => [x.name, x.offset]), [["bootloader", 0x2000], ["partition-table", 0x8000], ["app", 0x10000]]);
  assert.ok(p.parts.every((x) => x.data.length % 4 === 0));
  assert.equal(p.parts[0].data.length, s.bootloader.length);
  assert.deepEqual(p.erased, []);
  assert.deepEqual(p.kept.map((e) => e.name), ["nvs", "phy_init"]);
  // no part overlaps the settings area
  for (const x of p.parts) assert.ok(x.offset + x.data.length <= 0x9000 || x.offset >= 0x10000, x.name);
  assert.equal(p.totalBytes, p.parts.reduce((a, x) => a + x.data.length, 0));
});
test("write plan with erase settings: the nvs region from the table, blank", async () => {
  const r = await analyze(three()), p = planWrite(r, { eraseSettings: true });
  assert.deepEqual(p.erased, [{ name: "nvs", offset: 0x9000, size: 0x6000 }]);
  const e = p.parts.at(-1);
  assert.equal(e.name, "nvs"); assert.equal(e.offset, 0x9000); assert.equal(e.data.length, 0x6000); assert.ok(e.data.every((b) => b === 0xff));
  assert.deepEqual(p.kept.map((k) => k.name), ["phy_init"]);
});
test("erase uses the nvs entry of the image's own table, and falls back to partitions.csv values", async () => {
  const moved = [["nvs", 1, 2, 0xa000, 0x8000], ["phy_init", 1, 1, 0x12000, 0x1000], ["factory", 0, 0, 0x20000, 0xe00000]];
  // app must stay at 0x10000 (it is written there): a table that puts the app elsewhere is refused
  await fails(three(releaseSet({ entries: moved })), "pt_layout");
  const ok = [["nvs", 1, 2, 0x9000, 0x4000], ["factory", 0, 0, 0x10000, 0x400000]];
  const r = await analyze(three(releaseSet({ entries: ok })));
  assert.deepEqual(r.nvs, { off: 0x9000, size: 0x4000, fromTable: true });
  const none = await analyze(three(releaseSet({ entries: [["factory", 0, 0, 0x10000, 0x400000]] })));
  assert.deepEqual({ off: none.nvs.off, size: none.nvs.size, fromTable: none.nvs.fromTable }, { ...DEFAULT_NVS, fromTable: false });
});

test("refused: not an ESP image, wrong magic byte", async () => {
  await fails([f("a.bin", new Uint8Array(5000).fill(7))], "no_magic");
  const s = releaseSet(); const bad = new Uint8Array(s.app); bad[0] = 0xe8;
  await fails([f("NightStandNano-1.2.3-app.bin", bad)], "no_magic");
  const img = mergedImage(s); img[0x2000] = 0x00;
  await fails([f("NightStandNano-1.2.3-full.bin", img)], "no_magic");
  await fails([f("m.bin", img)], "no_magic");
  // named like a merged image but only an app inside
  await fails([f("NightStandNano-1.2.3-full.bin", s.app)], "not_merged");
});
test("refused: wrong partition-table magic", async () => {
  const s = releaseSet(), img = mergedImage(s); img[0x8000] = 0x00;
  await fails([f("NightStandNano-1.2.3-full.bin", img)], "no_pt");
  const t = three(s); t[1] = f(t[1].name, new Uint8Array(3072).fill(0xff));
  await fails(t, "no_magic");
  const md5 = new Uint8Array(s["partition-table"]); md5[70] ^= 1;   // inside the entries: the MD5 line no longer matches
  await fails([t[0], f("p.bin", md5), t[2]], "pt_md5");
});
test("refused: wrong size / wrong chip / damaged hash / gaps", async () => {
  const s = releaseSet();
  await fails([f("NightStandNano-1.2.3-full.bin", new Uint8Array(0x4000).fill(0xff))], "no_magic");
  const small = mergedImage(s).subarray(0, 0x9000);
  await fails([f("NightStandNano-1.2.3-full.bin", small)], "size");
  const big = new Uint8Array(0x1000001); await fails([f("NightStandNano-1.2.3-full.bin", big)], "no_magic");
  const other = releaseSet({ appOpts: { chip: 9 } });
  await fails(three(other), "chip");
  const dmg = new Uint8Array(s.app); dmg[200] ^= 0xff;
  await fails([three(s)[0], three(s)[1], f("a.bin", dmg)], "hash");
  const gap = mergedImage(s); gap[0x9100] = 0x42;   // data in the settings area of a "release" image
  await fails([f("NightStandNano-1.2.3-full.bin", gap)], "gap");
  const tail = mergedImage(s); const t2 = new Uint8Array(tail.length + 10).fill(0xff); t2.set(tail); t2[tail.length + 3] = 1;
  await fails([f("NightStandNano-1.2.3-full.bin", t2)], "gap");
  const bootBig = releaseSet({ bootLen: 0x7000 });
  await fails(three(bootBig), "size");
});
test("refused: a bootloader where the app should be, no app description", async () => {
  const s = releaseSet();
  const noDesc = espImage(new Uint8Array(500).fill(1), { segments: 2 });
  const merged = mergedImage({ ...s, app: noDesc });
  await fails([f("NightStandNano-1.2.3-full.bin", merged)], "no_desc");
});
test("refused: wrong number / mix of files", async () => {
  const s = releaseSet(), t = three(s);
  await fails([], "none");
  await fails([t[0], t[1]], "count");
  await fails([...t, t[0]], "count");
  await fails([t[0]], "need_all");
  await fails([t[2]], "need_all");
  await fails([t[0], t[0], t[1]], "roles");
  await fails([f("NightStandNano-1.2.3-full.bin", mergedImage(s)), t[1], t[2]], "mixed");
});
test("flash header in the image is read (dio / 80m / 16MB expected)", async () => {
  const ok = await analyze(three());
  assert.deepEqual({ ...ok.flash }, { mode: "dio", freq: "80m", size: "16MB", matches: true });
  const odd = await analyze(three(releaseSet({ bootOpts: { mode: 0, freqSize: 0x20 } })));
  assert.equal(odd.flash.matches, false); assert.equal(odd.flash.mode, "qio"); assert.equal(odd.flash.freq, "40m"); assert.equal(odd.flash.size, "4MB");
  assert.equal(flashParams(new Uint8Array([0xe9, 0, 2, 0x4f]), 0).matches, true);
});
test("an image for a different project is flagged by name, not refused", async () => {
  const r = await analyze(three(releaseSet({ project: "jukedial" })));
  assert.equal(r.projectName, "jukedial");
});
test("chip and flash checks", async () => {
  const r = await analyze(three());
  assert.deepEqual(chipCheck({ chip: "ESP32-S31", flashBytes: 16 << 20 }, r), []);
  assert.deepEqual(chipCheck({ chip: "ESP32-S3", flashBytes: 16 << 20 }, r), ["chip"]);
  assert.deepEqual(chipCheck({ chip: "ESP32-S31", flashBytes: 8 << 20 }, r), ["flash_small"]);
  assert.deepEqual(chipCheck({ chip: "ESP32-S31", flashBytes: 0 }, r), []);   // size not detected: nothing to compare
});
test("layout constants equal app/partitions.csv", () => {
  const p = new URL("../../app/partitions.csv", import.meta.url);
  if (!existsSync(p)) return;
  const rows = Object.fromEntries(readFileSync(p, "utf8").split("\n").filter((l) => l && !l.startsWith("#")).map((l) => l.split(",").map((x) => x.trim())).map((c) => [c[0], { off: Number(c[3]), size: Number(c[4]) }]));
  assert.equal(rows.nvs.off, DEFAULT_NVS.off); assert.equal(rows.nvs.size, DEFAULT_NVS.size);
  assert.equal(rows.factory.off, LAYOUT.app.off);
  assert.equal(rows.factory.off + rows.factory.size, DEFAULT_ENTRIES[2][3] + DEFAULT_ENTRIES[2][4]);
});
test("the real build files (when this checkout has a build) are accepted", async (t) => {
  const b = new URL("../../app/build/", import.meta.url), p = (n) => new URL(n, b);
  if (!existsSync(p("nightstand_nano.bin"))) return t.skip("no app/build here");
  const rd = (n, name) => f(name, new Uint8Array(readFileSync(p(n))));
  const r = await analyze([rd("bootloader/bootloader.bin", "bootloader.bin"), rd("partition_table/partition-table.bin", "partition-table.bin"), rd("nightstand_nano.bin", "nightstand_nano.bin")]);
  assert.equal(r.projectName, "nightstand_nano"); assert.equal(r.flash.matches, true); assert.equal(r.nvs.off, 0x9000);
});
