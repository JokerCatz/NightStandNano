// SPDX-License-Identifier: LGPL-2.1-or-later
// Synthetic firmware images for the tests and the screenshots (NOT firmware): valid ESP image structure (header, segments, checksum byte, appended
// SHA-256), an esp_app_desc_t, a partition table with its MD5 line, and the merged layout (bootloader 0x2000, table 0x8000, app 0x10000, 0xFF between).
import { createHash } from "node:crypto";

export const DEFAULT_ENTRIES = [["nvs", 1, 2, 0x9000, 0x6000], ["phy_init", 1, 1, 0xf000, 0x1000], ["factory", 0, 0, 0x10000, 0xf00000]];
const u32 = (n) => { const b = Buffer.alloc(4); b.writeUInt32LE(n >>> 0); return b; };
const pad = (s, n) => { const b = Buffer.alloc(n); Buffer.from(s).copy(b); return b; };

export function espImage(payload, { chip = 32, desc = null, segments = 1, hash = true, mode = 2, freqSize = 0x4f } = {}) {
  const hdr = Buffer.alloc(24); hdr[0] = 0xe9; hdr[1] = segments; hdr[2] = mode; hdr[3] = freqSize; hdr.writeUInt16LE(chip, 12); hdr[23] = hash ? 1 : 0;
  const body = [];
  const per = Math.max(16, Math.floor(payload.length / segments));
  let pos = 0;
  for (let i = 0; i < segments; i++) {
    let chunk = Buffer.from(i < segments - 1 ? payload.subarray(pos, pos + per) : payload.subarray(pos));
    pos += chunk.length;
    if (i === 0 && desc) chunk = Buffer.concat([desc, chunk]);
    if (chunk.length % 4) chunk = Buffer.concat([chunk, Buffer.alloc(4 - (chunk.length % 4))]);
    body.push(u32(0x42000000 + i * 0x100000), u32(chunk.length), chunk);
  }
  let img = Buffer.concat([hdr, ...body]);
  img = Buffer.concat([img, Buffer.alloc((16 - ((img.length + 1) % 16)) % 16), Buffer.from([0x5a])]);
  return new Uint8Array(hash ? Buffer.concat([img, createHash("sha256").update(img).digest()]) : img);
}
export function appDesc({ version = "1.2.3", project = "nightstand_nano", time = "12:00:00", date = "Nov 25 2026", idf = "v6.1" } = {}) {
  return Buffer.concat([u32(0xabcd5432), Buffer.alloc(12), pad(version, 32), pad(project, 32), pad(time, 16), pad(date, 16), pad(idf, 32), Buffer.alloc(256 - 144)]);
}
export function partitionTable(entries = DEFAULT_ENTRIES, { padTo = 0xc00 } = {}) {
  const e = Buffer.concat(entries.map(([n, ty, sub, off, size]) => Buffer.concat([Buffer.from([0xaa, 0x50, ty, sub]), u32(off), u32(size), pad(n, 16), Buffer.alloc(4)])));
  const t = Buffer.concat([e, Buffer.from([0xeb, 0xeb]), Buffer.alloc(14, 0xff), createHash("md5").update(e).digest()]);
  return new Uint8Array(Buffer.concat([t, Buffer.alloc(Math.max(0, padTo - t.length), 0xff)]));
}
const fill = (n, k) => Uint8Array.from({ length: n }, (_, i) => (i * k + 5) & 255);

// the three files of a release
export function releaseSet({ version = "1.2.3", appLen = 60000, bootLen = 3000, project = "nightstand_nano", entries = DEFAULT_ENTRIES, appOpts = {}, bootOpts = {} } = {}) {
  return {
    bootloader: espImage(fill(bootLen, 7), bootOpts),
    "partition-table": partitionTable(entries),
    app: espImage(fill(appLen, 13), { desc: appDesc({ version, project }), segments: 2, ...appOpts }),
  };
}
export function mergedImage(set) {
  const out = new Uint8Array(0x10000 + set.app.length).fill(0xff);
  out.set(set.bootloader, 0x2000);
  out.set(set["partition-table"], 0x8000);
  out.set(set.app, 0x10000);
  return out;
}
