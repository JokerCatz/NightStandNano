#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-or-later
"""Tiny synthetic VGM files for the Nano VGM gate (everything written here is made from nothing: no third-party music).
   python3 gen_vgm_cases.py OUTDIR
ok_*      must play (audible: SN76489 / YM2413 / YM2612), refuse_* must be refused with the chip names in their file name, bad_* are damaged files."""
import gzip, os, struct, sys

def hdr(ver, size, clocks, data_off=0x100, total=0, garbage=False):
    h = bytearray(size)
    if garbage:
        for i in range(0x38, size): h[i] = (0xA5 ^ (i * 7)) & 0xFF or 0x5A
    h[0:4] = b"Vgm "; struct.pack_into("<I", h, 8, ver); struct.pack_into("<I", h, 0x18, total)
    for o in (0x0C, 0x10, 0x2C, 0x30): struct.pack_into("<I", h, o, 0)
    if ver >= 0x101: struct.pack_into("<I", h, 0x24, 60)
    if ver >= 0x150: struct.pack_into("<I", h, 0x34, (data_off - 0x34) if data_off != 0x40 else 0)
    for o, v in clocks.items(): struct.pack_into("<I", h, o, v)
    return h

def build(ver, clocks, cmds, size=0x100, garbage=False, total=0):
    h = hdr(ver, size, clocks, data_off=size, total=total, garbage=garbage)
    out = h + cmds
    struct.pack_into("<I", out, 4, len(out) - 4)
    return bytes(out)

def wait(n):
    o = b""
    while n > 0:
        m = min(n, 65535); o += b"\x61" + struct.pack("<H", m); n -= m
    return o

def psg_tune(secs=4):
    c = bytearray([0x50, 0x9F, 0x50, 0xBF, 0x50, 0xDF, 0x50, 0xFF])     # all silent
    notes = [262, 330, 392, 523]
    for k in range(secs * 4):
        n = notes[k % 4]; d = int(3579545 / (32 * n))
        c += bytes([0x50, 0x80 | (d & 15), 0x50, d >> 4, 0x50, 0x90 | 2, 0x61]) + struct.pack("<H", 11025)
    return bytes(c + b"\x66"), secs * 44100

def opm_tune():     # a few YM2151 register writes and waits: refused before it is ever played
    c = bytearray()
    for r, v in ((0x20, 0xC7), (0x60, 0x10), (0x80, 0x1F), (0x28, 0x4A), (0x08, 0x78)): c += bytes([0x54, r, v])
    c += b"\x61" + struct.pack("<H", 44100) + b"\x66"
    return bytes(c), 44100

def opn_cmds(cmd):
    c = bytearray()
    for r, v in ((0xB0, 0x07), (0x40, 0x10), (0x50, 0x1F), (0xA4, 0x22), (0xA0, 0x69), (0x28, 0xF0)): c += bytes([cmd, r, v])
    c += b"\x61" + struct.pack("<H", 44100) + b"\x66"
    return bytes(c), 44100

def ym2413_tune():
    c = bytearray()
    for r, v in ((0x30, 0x13), (0x10, 0x80), (0x20, 0x16)): c += bytes([0x51, r, v])      # instrument 1, volume 3, f-number, key on
    c += wait(44100 * 3) + b"\x66"
    return bytes(c), 44100 * 3

def ym2612_tune():
    c = bytearray()
    for g in (0x30, 0x34, 0x38, 0x3C):
        c += bytes([0x52, g, 0x01, 0x52, g + 0x10, 0x08, 0x52, g + 0x20, 0x1F, 0x52, g + 0x50, 0x0F])
    c += bytes([0x52, 0xB0, 0x07, 0x52, 0xB4, 0xC0, 0x52, 0xA4, 0x22, 0x52, 0xA0, 0x69, 0x52, 0x28, 0xF0])
    c += wait(44100 * 3) + b"\x66"
    return bytes(c), 44100 * 3

def main(out):
    os.makedirs(out, exist_ok=True)
    F = {}
    psg, tp = psg_tune(); opm, to = opm_tune()
    F["ok_psg_v171.vgm"] = build(0x171, {0x0C: 3579545}, psg, total=tp)
    F["ok_psg_v150_garbage.vgm"] = build(0x150, {0x0C: 3579545}, psg, size=0x100, garbage=True, total=tp)   # bytes after the 0x40 header are junk, version 1.50
    F["ok_psg_v150_hdr40.vgm"] = build(0x150, {0x0C: 3579545}, psg, size=0x40, total=tp)                      # data right behind the old header
    F["ok_psg_v101.vgm"] = build(0x101, {0x0C: 3579545}, psg, size=0x40, total=tp)
    F["ok_declared_unused.vgm"] = build(0x171, {0x0C: 3579545, 0x30: 3579545, 0x98: 1056000}, psg, total=tp)  # YM2151 + OKIM6295 declared, never written
    F["ok_psg_dual_t6w28.vgm"] = build(0x171, {0x0C: 0xC0000000 | 3579545}, psg, total=tp)
    c, t = ym2413_tune(); F["ok_ym2413.vgm"] = build(0x171, {0x10: 3579545}, c, total=t)
    c, t = ym2612_tune(); F["ok_ym2612.vgm"] = build(0x171, {0x2C: 7670454}, c, total=t)
    F["refuse_YM2151.vgm"] = build(0x171, {0x30: 3579545}, opm, total=to)
    c, t = opn_cmds(0x55); F["refuse_YM2203.vgm"] = build(0x171, {0x44: 3993600}, c, total=t)
    F["refuse_SN76489_YM2151.vgm"] = build(0x171, {0x0C: 3579545, 0x30: 3579545}, psg[:-1] + opm, total=tp)
    F["refuse_YM2151_OKIM6295.vgm"] = build(0x171, {0x30: 3579545, 0x98: 1056000}, opm[:-1] + bytes([0xB8, 0x00, 0x10, 0x66]), total=to)
    F["refuse_v100_shared_clock_YM2151.vgm"] = build(0x100, {0x10: 3579545}, opm, size=0x40, total=to)     # pre-1.10: the YM2413 field is shared, the command says YM2151
    c, t = ym2612_tune(); F["refuse_YM2612_YM2413.vgm"] = build(0x171, {0x2C: 7670454, 0x10: 3579545}, c[:-1] + bytes([0x51, 0x30, 0x13, 0x66]), total=t)   # gme plays only the YM2612
    F["refuse_YM2608_YM2610B.vgm"] = build(0x171, {0x48: 7987200, 0x4C: 0x80000000 | 8000000}, bytes([0x56, 0x28, 0xF0, 0x58, 0x28, 0xF0, 0x66]), total=1)
    # damaged
    full = F["ok_psg_v171.vgm"]
    F["bad_header_only.vgm"] = full[:0x40]
    F["bad_header_0x100_only.vgm"] = full[:0x100]
    F["bad_10_bytes.vgm"] = full[:10]
    F["bad_empty.vgm"] = b""
    F["bad_not_vgm.vgm"] = b"hello, this is not a vgm file, just text\n" * 4
    F["bad_cut_in_command.vgm"] = full[:0x100 + 9]
    F["bad_not_gzip.vgz"] = b"Not gzip data at all, 0123456789" * 3
    gz = gzip.compress(full, mtime=0)
    F["bad_gzip_cut.vgz"] = gz[:len(gz) // 2]
    # same files in other wrappers
    F["ok_psg_plain_named.vgz"] = full                                           # not gzip'd although named .vgz
    F["ok_psg_gz_named.vgm"] = gz                                                # gzip'd although named .vgm
    F["ok_psg_v171.vgz"] = gz
    F["refuse_YM2151.vgz"] = gzip.compress(F["refuse_YM2151.vgm"], mtime=0)
    for k, v in F.items(): open(os.path.join(out, k), "wb").write(v)
    print(len(F), "files,", sum(map(len, F.values())), "bytes ->", out)

if __name__ == "__main__": main(sys.argv[1] if len(sys.argv) > 1 else "/tmp/nsn_vgm_cases")
