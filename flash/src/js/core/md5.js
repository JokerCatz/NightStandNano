// SPDX-License-Identifier: LGPL-2.1-or-later
// MD5 (RFC 1321), synchronous: the flash chip answers a "flash MD5" command and the page compares it with the MD5 of what it wrote
// (crypto.subtle has no MD5). Derived from the owner's JukeDial web tool (tools/web/flasher.js md5Hex), unchanged apart from being a module.
// Also sha256Hex (WebCrypto) for showing a file's hash.
/* RFC 1321 */
export function md5Hex(u8) {
  const K = new Int32Array(64), R = [7, 12, 17, 22, 5, 9, 14, 20, 4, 11, 16, 23, 6, 10, 15, 21];
  for (let i = 0; i < 64; i++) K[i] = Math.floor(Math.abs(Math.sin(i + 1)) * 4294967296) | 0;
  const n = u8.length, total = ((n + 8) >> 6) * 64 + 64, b = new Uint8Array(total); b.set(u8); b[n] = 0x80;
  const dv = new DataView(b.buffer); dv.setUint32(total - 8, (n << 3) >>> 0, true); dv.setUint32(total - 4, Math.floor(n / 536870912), true);
  let a0 = 0x67452301 | 0, b0 = 0xefcdab89 | 0, c0 = 0x98badcfe | 0, d0 = 0x10325476 | 0; const M = new Int32Array(16);
  for (let o = 0; o < total; o += 64) {
    for (let i = 0; i < 16; i++) M[i] = dv.getInt32(o + 4 * i, true);
    let A = a0, B = b0, C = c0, D = d0;
    for (let i = 0; i < 64; i++) {
      let F, g;
      if (i < 16) { F = (B & C) | (~B & D); g = i } else if (i < 32) { F = (D & B) | (~D & C); g = (5 * i + 1) & 15 } else if (i < 48) { F = B ^ C ^ D; g = (3 * i + 5) & 15 } else { F = C ^ (B | ~D); g = (7 * i) & 15 }
      F = (F + A + K[i] + M[g]) | 0; A = D; D = C; C = B; const r = R[(i >> 4) * 4 + (i & 3)]; B = (B + ((F << r) | (F >>> (32 - r)))) | 0;
    }
    a0 = (a0 + A) | 0; b0 = (b0 + B) | 0; c0 = (c0 + C) | 0; d0 = (d0 + D) | 0;
  }
  const out = new DataView(new ArrayBuffer(16)); out.setInt32(0, a0, true); out.setInt32(4, b0, true); out.setInt32(8, c0, true); out.setInt32(12, d0, true);
  return [...new Uint8Array(out.buffer)].map(x => x.toString(16).padStart(2, '0')).join('');
}

export async function sha256Hex(bytes) {
  const d = await globalThis.crypto.subtle.digest("SHA-256", bytes);
  return [...new Uint8Array(d)].map((b) => b.toString(16).padStart(2, "0")).join("");
}
