// SPDX-License-Identifier: LGPL-2.1-or-later
// Release files: what the user picks, what is accepted, what gets written where. Pure functions (WebCrypto for SHA-256 only), no UI, no text:
// errors carry .code (and .extra) and the page turns them into words.
// Derived from the owner's JukeDial web tool (tools/web/flasher.js: espImage, partitionTable, planWrite) and adapted: no board choice, no fonts
// partition, three separate files as an alternative to one merged image, the content (not the name) decides.
//
// Release names (the release script writes exactly these; the page also takes the raw build names in parentheses):
//   NightStandNano-<version>-full.bin              ONE merged image, flash offset 0x0 (bootloader at 0x2000, table at 0x8000, app at 0x10000, 0xFF between)
//   NightStandNano-<version>-bootloader.bin        (bootloader.bin)           -> 0x2000
//   NightStandNano-<version>-partition-table.bin   (partition-table.bin)      -> 0x8000
//   NightStandNano-<version>-app.bin               (nightstand_nano.bin)      -> 0x10000
// A merged image is NOT written as one block from 0: only the three regions are written, so the settings partition (NVS) in between survives.
import { md5Hex, sha256Hex } from "./md5.js";

export const LAYOUT = {
  bootloader: { off: 0x2000, max: 0x6000 },   // up to the partition table
  "partition-table": { off: 0x8000, size: 0xc00 },
  app: { off: 0x10000 },
  flashBytes: 16 << 20,
};
export const DEFAULT_NVS = { off: 0x9000, size: 0x6000 };   // app/partitions.csv; the table inside the image wins when it has an nvs entry
export const CHIP_ID = 32;                                  // IMAGE_CHIP_ID of the ESP32-S31 (esptool-js S31 target: 32)
export const APP_DESC_MAGIC = 0xabcd5432;
export const PROJECT_NAME = "nightstand_nano";
export const EXPECT_FLASH = { mode: "dio", freq: "80m", size: "16MB" };
export const ROLES = ["bootloader", "partition-table", "app"];

const err = (code, extra) => { const e = new Error(code + (extra !== undefined ? ": " + extra : "")); e.code = code; e.extra = extra; return e; };
export const hex = (n) => "0x" + n.toString(16);
const u32 = (d, o) => (d[o] | (d[o + 1] << 8) | (d[o + 2] << 16) | (d[o + 3] << 24)) >>> 0;
const cstr = (d, o, n) => { let e = o; while (e < o + n && d[e]) e++; return String.fromCharCode(...d.subarray(o, e)); };
const allFF = (d, a, b) => { for (let i = a; i < b; i++) if (d[i] !== 0xff) return false; return true; };
const hexOf = (d) => [...d].map((b) => b.toString(16).padStart(2, "0")).join("");

// ---- names -------------------------------------------------------------------------------------------------------------------
const VER = "([0-9A-Za-z][0-9A-Za-z._+]*(?:-[0-9A-Za-z._+]+)*?)";
// -> {kind:"full", version} | {kind:"part", role, version} | {kind:"raw", role} | null. Only a hint: the content decides.
export function parseName(name) {
  const n = String(name || "").split(/[\\/]/).pop();
  let m = new RegExp("^NightStandNano-" + VER + "-full\\.bin$").exec(n);
  if (m) return { kind: "full", version: m[1] };
  m = new RegExp("^NightStandNano-" + VER + "-(bootloader|partition-table|app)\\.bin$").exec(n);
  if (m) return { kind: "part", role: m[2], version: m[1] };
  if (n === "bootloader.bin") return { kind: "raw", role: "bootloader" };
  if (n === "partition-table.bin") return { kind: "raw", role: "partition-table" };
  if (n === "nightstand_nano.bin") return { kind: "raw", role: "app" };
  return null;
}

// ---- flash header ------------------------------------------------------------------------------------------------------------
const MODES = ["qio", "qout", "dio", "dout"], FREQS = { 0xf: "80m", 0: "40m", 1: "26m", 2: "20m" }, SIZES = ["1MB", "2MB", "4MB", "8MB", "16MB", "32MB", "64MB", "128MB"];
export function flashParams(d, off) {
  const fp = { mode: MODES[d[off + 2]] || "?", freq: FREQS[d[off + 3] & 15] || "?", size: SIZES[d[off + 3] >> 4] || "?" };
  fp.matches = fp.mode === EXPECT_FLASH.mode && fp.freq === EXPECT_FLASH.freq && fp.size === EXPECT_FLASH.size;
  return fp;
}

// ---- one ESP image at `off` in d: header (0xE9, segment count, chip id at 12..13), segments, checksum byte, SHA-256 when the header says so ------
// Returns {start, end, desc?}; throws no_magic / chip / hash. `who` goes into .extra so the page can name the file.
async function espImage(d, off, who) {
  if (off + 32 > d.length || d[off] !== 0xe9) throw err("no_magic", who);
  const chip = d[off + 12] | (d[off + 13] << 8);
  if (chip !== CHIP_ID) throw err("chip", who + " (" + chip + ")");
  let p = off + 24;
  for (let i = 0; i < d[off + 1]; i++) {
    if (p + 8 > d.length) throw err("no_magic", who);
    p += 8 + u32(d, p + 4);
    if (p > d.length) throw err("size", who);
  }
  p = ((p - off) | 15) + off + 1;   // the checksum byte is the last of a 16-byte block
  let end = p;
  if (d[off + 23] === 1) {
    if (end + 32 > d.length) throw err("hash", who);
    if ((await sha256Hex(d.subarray(off, end))) !== hexOf(d.subarray(end, end + 32))) throw err("hash", who);
    end += 32;
  }
  const hasDesc = u32(d, off + 0x20) === APP_DESC_MAGIC;
  return { start: off, end, hasDesc, flash: flashParams(d, off) };
}
function appDesc(d, off) {
  return { version: cstr(d, off + 0x30, 32), projectName: cstr(d, off + 0x50, 32), buildTime: cstr(d, off + 0x70, 16), buildDate: cstr(d, off + 0x80, 16), idf: cstr(d, off + 0x90, 32) };
}

// ---- partition table: 32-byte entries (AA 50, type, subtype, offset, size, name[16], flags), up to the 0xEBEB line holding the MD5 of the entries ----
export function partitionTable(d, who = "partition-table") {
  const entries = [];
  let p = 0, md5ok = false;
  for (; p + 32 <= d.length && p < LAYOUT["partition-table"].size; p += 32) {
    const m = d[p] | (d[p + 1] << 8);
    if (m === 0x50aa) entries.push({ type: d[p + 2], subtype: d[p + 3], offset: u32(d, p + 4), size: u32(d, p + 8), name: cstr(d, p + 12, 16) });
    else if (m === 0xebeb) { md5ok = md5Hex(d.subarray(0, p)) === hexOf(d.subarray(p + 16, p + 32)); break; }
    else break;
  }
  if (!entries.length) throw err("no_pt", who);
  if (!md5ok) throw err("pt_md5", who);
  return { entries, usedBytes: p + 32 };
}
function checkTable(entries) {
  const app = entries.find((e) => e.type === 0 && e.offset === LAYOUT.app.off);
  if (!app) throw err("pt_layout", hex(LAYOUT.app.off));
  for (const e of entries) if (e.offset + e.size > LAYOUT.flashBytes || e.offset < 0x9000) throw err("pt_layout", e.name);
  const nvsE = entries.find((e) => e.type === 1 && e.subtype === 2);
  return { app, nvs: nvsE ? { off: nvsE.offset, size: nvsE.size, fromTable: true } : { ...DEFAULT_NVS, fromTable: false } };
}

// ---- classification of one picked file ----------------------------------------------------------------------------------------
// "merged" (0xFF up to 0x2000, an ESP image at 0x2000) | "partition-table" | "app" | "bootloader" | throws no_magic
async function classify(f) {
  const d = f.data;
  if (d.length >= 0x2000 + 32 && d[0x2000] === 0xe9 && allFF(d, 0, 0x2000)) return "merged";
  if (d[0] === 0xaa && d[1] === 0x50) return "partition-table";
  if (d[0] === 0xe9 && d.length >= 0x24) return u32(d, 0x20) === APP_DESC_MAGIC ? "app" : "bootloader";
  throw err("no_magic", f.name);
}

// ---- analysis ---------------------------------------------------------------------------------------------------------------
// files: [{name, data: Uint8Array}] (1 merged or 3 separate). Returns the release description or throws .code.
export async function analyze(files) {
  if (!files || !files.length) throw err("none");
  if (files.length !== 1 && files.length !== 3) throw err("count", String(files.length));
  const kinds = [];
  for (const f of files) kinds.push(await classify(f));
  let rel;
  if (files.length === 1) {
    const ni = parseName(files[0].name);
    if (kinds[0] !== "merged" && ni && ni.kind === "full") throw err("not_merged", files[0].name);   // the name promises a merged image, the content is not one
    if (kinds[0] !== "merged") { if (ROLES.includes(kinds[0])) throw err("need_all", kinds[0]); throw err("no_magic", files[0].name); }
    rel = await analyzeMerged(files[0]);
  } else {
    if (kinds.includes("merged")) throw err("mixed", files.find((f, i) => kinds[i] === "merged").name);
    for (const r of ROLES) if (kinds.filter((k) => k === r).length !== 1) throw err("roles", r);
    rel = await analyzeThree(Object.fromEntries(files.map((f, i) => [kinds[i], f])));
  }
  rel.sha256 = Object.fromEntries(await Promise.all(files.map(async (f) => [f.name, await sha256Hex(f.data)])));
  return rel;
}

async function finish(kind, parts, tableEntries, appImgDesc, bootFlash, names) {
  const { app, nvs } = checkTable(tableEntries);
  const appPart = parts.find((p) => p.role === "app");
  if (appPart.data.length > app.size) throw err("app_big", hex(app.size));
  const desc = appDesc(appPart.data, 0);
  const name0 = names.find(Boolean) || null;
  return {
    kind, parts, entries: tableEntries, nvs,
    version: desc.version || (name0 && name0.version) || "", projectName: desc.projectName, buildTime: desc.buildTime, buildDate: desc.buildDate, idf: desc.idf,
    flash: bootFlash,
    flashNeeded: Math.max(...tableEntries.map((e) => e.offset + e.size)),
    nameInfo: name0,
  };
}
const part = (role, file, off, data) => ({ role, file, offset: off, data, size: data.length });

async function analyzeMerged(f) {
  const d = f.data;
  if (d.length < LAYOUT.app.off + 64 || d.length > LAYOUT.flashBytes) throw err("size", f.name);
  const boot = await espImage(d, LAYOUT.bootloader.off, f.name + " @0x2000");
  if (boot.hasDesc) throw err("no_magic", f.name + " @0x2000");
  if (boot.end > LAYOUT["partition-table"].off) throw err("size", "bootloader");
  const ptEnd = LAYOUT["partition-table"].off + LAYOUT["partition-table"].size;
  const pt = partitionTable(d.subarray(LAYOUT["partition-table"].off, ptEnd), f.name + " @0x8000");
  const app = await espImage(d, LAYOUT.app.off, f.name + " @0x10000");
  if (!app.hasDesc) throw err("no_desc", f.name);
  // what the build made is what gets written: the gaps (and so the settings area between table and app) must be blank
  if (!allFF(d, boot.end, LAYOUT["partition-table"].off)) throw err("gap", hex(boot.end));
  if (!allFF(d, ptEnd, LAYOUT.app.off)) throw err("gap", hex(ptEnd));
  if (!allFF(d, app.end, d.length)) throw err("gap", hex(app.end));
  const parts = [
    part("bootloader", f.name, LAYOUT.bootloader.off, d.subarray(LAYOUT.bootloader.off, boot.end)),
    part("partition-table", f.name, LAYOUT["partition-table"].off, d.subarray(LAYOUT["partition-table"].off, LAYOUT["partition-table"].off + pt.usedBytes)),
    part("app", f.name, LAYOUT.app.off, d.subarray(LAYOUT.app.off, app.end)),
  ];
  return finish("merged", parts, pt.entries, null, boot.flash, [parseName(f.name)]);
}

async function analyzeThree(by) {
  const b = by.bootloader, t = by["partition-table"], a = by.app;
  const boot = await espImage(b.data, 0, b.name);
  if (b.data.length > LAYOUT.bootloader.max) throw err("size", b.name);
  if (!allFF(b.data, boot.end, b.data.length)) throw err("gap", b.name);
  if (t.data.length > LAYOUT["partition-table"].size) throw err("size", t.name);
  const pt = partitionTable(t.data, t.name);
  const app = await espImage(a.data, 0, a.name);
  if (!app.hasDesc) throw err("no_desc", a.name);
  if (!allFF(a.data, app.end, a.data.length)) throw err("gap", a.name);
  const parts = [
    part("bootloader", b.name, LAYOUT.bootloader.off, b.data.subarray(0, boot.end)),
    part("partition-table", t.name, LAYOUT["partition-table"].off, t.data.subarray(0, pt.usedBytes)),
    part("app", a.name, LAYOUT.app.off, a.data.subarray(0, app.end)),
  ];
  return finish("three", parts, pt.entries, null, boot.flash, [parseName(a.name), parseName(b.name), parseName(t.name)]);
}

// ---- the write plan ---------------------------------------------------------------------------------------------------------
// parts to write, in order; erased = the settings partition when asked; kept = the data partitions that are left alone.
// Every block is padded to a multiple of 4 with 0xFF (what esptool does, so the flash MD5 below is over the same bytes).
const pad4 = (d) => { if (d.length % 4 === 0) return d; const o = new Uint8Array(d.length + 4 - (d.length % 4)).fill(0xff); o.set(d); return o; };
export function planWrite(rel, { eraseSettings = false } = {}) {
  const parts = rel.parts.map((p) => ({ name: p.role, offset: p.offset, data: pad4(p.data) }));
  const dataParts = rel.entries.filter((e) => e.type === 1).map((e) => ({ name: e.name, offset: e.offset, size: e.size }));
  const nvs = rel.nvs;
  const erased = eraseSettings ? [{ name: "nvs", offset: nvs.off, size: nvs.size }] : [];
  for (const e of erased) parts.push({ name: e.name, offset: e.offset, data: new Uint8Array(e.size).fill(0xff) });
  const kept = dataParts.filter((e) => !(eraseSettings && e.offset === nvs.off));
  return { parts, erased, kept, totalBytes: parts.reduce((a, p) => a + p.data.length, 0) };
}

// the chip and the flash against what the image needs; returns the codes of what is wrong
export function chipCheck(info, rel) {
  const errors = [];
  if (info.chip !== "ESP32-S31") errors.push("chip");
  if (info.flashBytes && info.flashBytes < rel.flashNeeded) errors.push("flash_small");
  return errors;
}
