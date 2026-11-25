#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-or-later
"""Synthetic register streams for the FM backend (components/fmvgm): VGM (YM2151, YM2203, dual chips, loop, GD3) and S98 (v3, YM2203 / YM2151). No third-party content:
the patches are random (seeded) and the notes are random; they are test signals for timing, bit-exactness and "does sound come out", not music.
   python3 -I gen_fm_vgm.py OUTDIR [--card]      --card: only the small files meant for the TF card (/sdcard/nsn_chip/fm/), 20 s each
Files (full set): ym2151_typical / _dense, ym2203_typical / _dense / _nossg, ym2151_dual, ym2203_dual, *_loop (intro + loop, GD3), ym2203_typical.s98, ym2151_typical.s98, ym2203_loop.s98 ..."""
import sys, struct, random, os

class Ev:
    """register / wait events; serialised to VGM or S98 afterwards"""
    def __init__(self): self.ev = []; self.t = 0; self.marks = {}
    def w(self, chip, reg, val): self.ev.append(('w', chip, reg, val))
    def wait(self, n):
        self.ev.append(('t', n)); self.t += n
    def mark(self, name): self.ev.append(('m', name)); self.marks[name] = self.t

OPM_NOTES = [0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14]

def gen_opm(e, chip, secs, gate, seed, lfo=True):
    r = random.Random(seed)
    wr = lambda a, d: e.w(chip, a, d)
    wr(0x01, 0x02); wr(0x01, 0x00)
    wr(0x18, 0xC0); wr(0x19, 0x80 | 20); wr(0x19, 0x00 | 12); wr(0x1B, 0x02)
    for ch in range(8):
        alg = r.choice([2, 4, 4, 5, 7, 7, 3]); fb = r.randint(0, 6)
        wr(0x20 + ch, 0xC0 | (fb << 3) | alg)
        wr(0x38 + ch, ((r.randint(0, 5) << 4) | r.randint(0, 2)) if lfo else 0)
        car = {0: [3], 1: [3], 2: [3], 3: [3], 4: [1, 3], 5: [1, 2, 3], 6: [1, 2, 3], 7: [0, 1, 2, 3]}[alg]
        for m in range(4):
            o = ch + 8 * m; iscar = m in car
            wr(0x40 + o, (r.randint(0, 3) << 4) | r.choice([0, 1, 1, 2, 3, 4, 7]))
            wr(0x60 + o, r.randint(0, 12) if iscar else r.randint(15, 50))
            wr(0x80 + o, (r.randint(0, 2) << 6) | r.choice([31, 31, 28, 24]))
            wr(0xA0 + o, ((0x80 if r.random() < .3 else 0) if lfo else 0) | r.randint(4, 14))
            wr(0xC0 + o, (r.randint(0, 3) << 6) | r.randint(0, 8))
            wr(0xE0 + o, (r.randint(2, 8) << 4) | (r.randint(3, 8) if iscar else 15))
    free = [0] * 8; state = [None] * 8
    return ('opm', r, free, state, gate, secs)

def step_opm(e, chip, st, k):
    _, r, free, state, gate, _s = st
    wr = lambda a, d: e.w(chip, a, d)
    for ch in range(8):
        if state[ch] is not None and k >= state[ch]:
            wr(0x08, ch); state[ch] = None
        if state[ch] is None and k >= free[ch] and r.random() < 0.9:
            if ch == 7 and r.random() < .15: wr(0x0F, 0x80 | r.randint(0, 31))
            wr(0x28 + ch, (r.randint(3, 6) << 4) | r.choice(OPM_NOTES)); wr(0x30 + ch, r.randint(0, 255) & 0xFC)
            wr(0x08, 0x78 | ch)
            d = r.choice([6, 9, 12, 15, 24]); state[ch] = k + max(1, int(d * gate)); free[ch] = k + d

def gen_opn(e, chip, secs, gate, seed, ssg=True):
    r = random.Random(seed)
    p0 = lambda a, d: e.w(chip, a, d)
    chans = [0, 1, 2]
    patches = []
    for c in chans:
        alg = r.choice([2, 4, 4, 5, 7, 7, 3, 0]); fb = r.randint(0, 6)
        p0(0xB0 + c, (fb << 3) | alg)
        car = {0: [3], 1: [3], 2: [3], 3: [3], 4: [1, 3], 5: [1, 2, 3], 6: [1, 2, 3], 7: [0, 1, 2, 3]}[alg]
        for m in range(4):
            o = [0, 8, 4, 12][m] + c; iscar = m in car
            p0(0x30 + o, (r.randint(0, 3) << 4) | r.choice([0, 1, 1, 2, 3, 4, 7]))
            p0(0x40 + o, r.randint(0, 12) if iscar else r.randint(15, 50))
            p0(0x50 + o, (r.randint(0, 2) << 6) | r.choice([31, 31, 28, 24]))
            p0(0x60 + o, r.randint(4, 14))
            p0(0x70 + o, r.randint(0, 8))
            p0(0x80 + o, (r.randint(2, 8) << 4) | (r.randint(3, 8) if iscar else 15))
    if ssg:
        p0(7, 0b00111000)
        for i in range(3): p0(8 + i, 0x0A)
    return ('opn', r, [0] * 3, [None] * 3, gate, ssg, [0, 0, 0], [None] * 3)

def step_opn(e, chip, st, k):
    _, r, free, state, gate, ssg, ssgfree, ssgst = st
    p0 = lambda a, d: e.w(chip, a, d)
    for c in range(3):
        if state[c] is not None and k >= state[c]:
            p0(0x28, c); state[c] = None
        if state[c] is None and k >= free[c] and r.random() < 0.9:
            blk = r.randint(2, 5); fn = r.randint(617, 1155)
            p0(0xA4 + c, (blk << 3) | (fn >> 8)); p0(0xA0 + c, fn & 0xFF)
            p0(0x28, 0xF0 | c)
            d = r.choice([6, 9, 12, 15, 24]); state[c] = k + max(1, int(d * gate)); free[c] = k + d
    if ssg:
        for s in range(3):
            if ssgst[s] is not None and k >= ssgst[s]: p0(8 + s, 0); ssgst[s] = None
            if ssgst[s] is None and k >= ssgfree[s] and r.random() < 0.8:
                per = r.randint(60, 600); p0(2 * s, per & 255); p0(2 * s + 1, per >> 8); p0(8 + s, r.randint(8, 15))
                d = r.choice([6, 9, 12]); ssgst[s] = k + max(1, int(d * gate)); ssgfree[s] = k + d

def song(kinds, secs, gate, seeds, loop_at=None, ssg=True, lfo=True):
    """kinds: list of 'opm' / 'opn' (one per chip); loop_at: second at which the loop point lies"""
    e = Ev(); sts = []
    for i, k in enumerate(kinds):
        sts.append(gen_opm(e, i, secs, gate, seeds[i], lfo) if k == 'opm' else gen_opn(e, i, secs, gate, seeds[i], ssg))
    for k in range(int(secs * 60)):
        if loop_at is not None and k == int(loop_at * 60): e.mark('loop')
        for i, st in enumerate(sts):
            (step_opm if st[0] == 'opm' else step_opn)(e, i, st, k)
        e.wait(735)
    return e

# ---- serialisers ----
def gd3(title, game, system, author):
    def s(x): return x.encode('utf-16-le') + b'\0\0'
    body = s(title) + s('') + s(game) + s('') + s(system) + s('') + s(author) + s('') + s('2026/11/25') + s('fm test generator') + s('synthetic register stream, not music')
    return b'Gd3 ' + struct.pack('<II', 0x100, len(body)) + body

def to_vgm(e, kinds, clocks, tags=None, version=0x171):
    cmd = {'opm': 0x54, 'opn': 0x55}
    dual = {}
    body = bytearray(); loop_off = None; seen = {}
    for x in e.ev:
        if x[0] == 'm' and x[1] == 'loop': loop_off = len(body)
        elif x[0] == 't':
            n = x[1]
            while n > 0:
                m = min(n, 65535)
                body += (b'\x62' if m == 735 else b'\x63' if m == 882 else b'\x61' + struct.pack('<H', m)); n -= m
        elif x[0] == 'w':
            _, chip, reg, val = x
            k = kinds[chip]; second = chip > 0 and kinds[chip - 1] == k
            body += bytes([cmd[k] + (0x50 if second else 0), reg, val])
    body += b'\x66'
    data_off = 0x100
    h = bytearray(data_off); h[0:4] = b'Vgm '
    struct.pack_into('<I', h, 8, version); struct.pack_into('<I', h, 0x18, e.t)
    if loop_off is not None:
        struct.pack_into('<I', h, 0x1C, data_off + loop_off - 0x1C); struct.pack_into('<I', h, 0x20, e.t - e.marks['loop'])
    struct.pack_into('<I', h, 0x24, 60); struct.pack_into('<I', h, 0x34, data_off - 0x34)
    for k, c in zip(kinds, clocks):
        off = 0x30 if k == 'opm' else 0x44
        v = struct.unpack_from('<I', h, off)[0]
        if v == 0: struct.pack_into('<I', h, off, c)
        else: struct.pack_into('<I', h, off, v | 0x40000000)           # a second chip of the same kind
    out = bytearray(h) + body
    if tags:
        struct.pack_into('<I', out, 0x14, len(out) - 0x14)
        out += gd3(*tags)
    struct.pack_into('<I', out, 4, len(out) - 4)
    return bytes(out)

def to_s98(e, kinds, clocks, title=None, tags=None):
    """S98 version 3: timer 1/60 s, device table, [S98] tag block. device types: 2 = YM2203, 5 = YM2151"""
    body = bytearray(); loop_off = None; pend = 0
    def flush():
        nonlocal pend
        while pend > 0:
            if pend == 1: body.append(0xFF); pend = 0
            else:
                n = min(pend, 2 + 0x3FFF) - 2; pend -= n + 2; v = n; out = bytearray()
                while True:
                    b = v & 0x7F; v >>= 7
                    out.append(b | (0x80 if v else 0))
                    if not v: break
                body.append(0xFE); body.extend(out)
    for x in e.ev:
        if x[0] == 'm' and x[1] == 'loop': flush(); loop_off = len(body)
        elif x[0] == 't': pend += 1
        elif x[0] == 'w':
            flush(); _, chip, reg, val = x
            body += bytes([chip * 2, reg, val])
    flush(); body.append(0xFD)
    dev = bytearray()
    for k, c in zip(kinds, clocks): dev += struct.pack('<IIII', 5 if k == 'opm' else 2, c, 0, 0)
    tag = b''
    if tags:
        t, g, a = tags
        tag = b'[S98]' + (f'title={t}\ngame={g}\nartist={a}\nsystem=synthetic\n').encode() + b'\0'
    hdr_len = 0x20 + len(dev)
    tag_off = hdr_len if tag else 0
    data_off = hdr_len + len(tag)
    h = bytearray(0x20); h[0:4] = b'S983'
    struct.pack_into('<IIIIIII', h, 4, 1, 60, 0, tag_off, data_off, (data_off + loop_off) if loop_off is not None else 0, len(kinds))
    return bytes(h) + bytes(dev) + tag + bytes(body)

def stress(kind, secs, seed):
    """random writes to the whole register file with note on / off in odd orders: SSG-EG modes, LFO / PM / AM, CSM and channel-3 modes, rate changes in mid note. Not music: a probe for patches that
    must not change the output by a single bit."""
    r = random.Random(seed); e = Ev()
    if kind == 'opm': lo, hi, keyreg = 0x20, 0xFF, 0x08
    else: lo, hi, keyreg = 0x30, 0xB6, 0x28
    for k in range(int(secs * 60)):
        for _ in range(r.randint(0, 14)):
            reg = r.randint(lo, hi)
            if reg in (0x08, 0x28): continue
            if kind == 'opm' and reg in (0x0F, 0x10, 0x11, 0x12, 0x14): continue
            if kind == 'opn' and 0x24 <= reg <= 0x2F: continue
            val = r.randint(0, 255)
            if kind == 'opn' and 0x90 <= reg <= 0x9F: val = r.choice([0, 8, 9, 10, 11, 12, 13, 14, 15])       # SSG-EG on in all eight modes, or off
            e.w(0, reg, val)
        for _ in range(r.randint(0, 3)):
            ch = r.randint(0, 7) if kind == 'opm' else r.choice([0, 1, 2])
            e.w(0, keyreg, (r.choice([0, 0, 0xF, 0x7, 0x3, 0x5]) << (3 if kind == 'opm' else 4)) | ch)
        if kind == 'opn' and r.random() < 0.05: e.w(0, 0x27, r.choice([0x00, 0x40, 0x80]))
        e.wait(735)
    return e

def main(out, card_only=False):
    os.makedirs(out, exist_ok=True)
    W = lambda name, data: open(os.path.join(out, name), 'wb').write(data)
    OPM, OPN = 3579545, 3993600
    if '--short' in sys.argv:                      # 3 s files for gapless / next / prev tests
        for i, (k, c, sd) in enumerate((('opn', OPN, 11), ('opm', OPM, 12), ('opn', OPN, 13), ('opm', OPM, 14))):
            W(f'short{i + 1}_{k}.vgm', to_vgm(song([k], 3, 0.7, [sd]), [k], [c]))
        return
    if card_only:
        for name, kinds, clocks, seeds, tg in (('ym2203_demo', ['opn'], [OPN], [2], ('YM2203 test stream', 'fmvgm test', 'PC-88 style', 'generator')),
                                              ('ym2151_demo', ['opm'], [OPM], [1], ('YM2151 test stream', 'fmvgm test', 'X68000 style', 'generator'))):
            e = song(kinds, 20, 0.7, seeds, loop_at=4)
            W(name + '.vgm', to_vgm(e, kinds, clocks, tg))
            W(name + '.s98', to_s98(e, kinds, clocks, tags=(tg[0], tg[1], tg[3])))
        e = song(['opn', 'opn'], 20, 0.7, [2, 5]); W('ym2203_dual_demo.vgm', to_vgm(e, ['opn', 'opn'], [OPN, OPN], ('YM2203 dual test', 'fmvgm test', 'dual chip', 'generator')))
        return
    S = 60
    for kind, clk in (('opm', OPM), ('opn', OPN)):
        for sd in range(1, 5):
            W(f'stress_{kind}_{sd}.vgm', to_vgm(stress(kind, 20, 100 + sd), [kind], [clk]))
    for nm, gate in (('dense', 1.0), ('typical', 0.6)):
        W(f'ym2151_{nm}.vgm', to_vgm(song(['opm'], S, gate, [1]), ['opm'], [OPM]))
        W(f'ym2203_{nm}.vgm', to_vgm(song(['opn'], S, gate, [2]), ['opn'], [OPN]))
    W('ym2203_nossg.vgm', to_vgm(song(['opn'], S, 1.0, [2], ssg=False), ['opn'], [OPN]))
    W('ym2151_nolfo.vgm', to_vgm(song(['opm'], S, 1.0, [1], lfo=False), ['opm'], [OPM]))
    W('ym2151_dual.vgm', to_vgm(song(['opm', 'opm'], 30, 0.7, [1, 7]), ['opm', 'opm'], [OPM, OPM]))
    W('ym2151_dual_dense.vgm', to_vgm(song(['opm', 'opm'], 60, 1.0, [1, 7]), ['opm', 'opm'], [OPM, OPM]))
    W('ym2203_dual_dense.vgm', to_vgm(song(['opn', 'opn'], 60, 1.0, [2, 8]), ['opn', 'opn'], [OPN, OPN]))
    e = song(['opm'], 60, 0.6, [1]); e.ev = [x for x in e.ev if not (x[0] == 'w' and x[2] in (0x08, 0x28, 0x30))]          # chip programmed, no note ever played: the "silent chip" case
    W('ym2151_idle.vgm', to_vgm(e, ['opm'], [OPM]))
    W('ym2203_dual.vgm', to_vgm(song(['opn', 'opn'], 30, 0.7, [2, 8]), ['opn', 'opn'], [OPN, OPN]))
    W('mix_opm_opn.vgm', to_vgm(song(['opm', 'opn'], 30, 0.7, [1, 2]), ['opm', 'opn'], [OPM, OPN]))
    tg = ('Loop test', 'fmvgm tests', 'synthetic', 'generator é日本')
    e = song(['opn'], 20, 0.7, [2], loop_at=6); W('ym2203_loop.vgm', to_vgm(e, ['opn'], [OPN], tg)); W('ym2203_loop.s98', to_s98(e, ['opn'], [OPN], tags=(tg[0], tg[1], tg[3])))
    e = song(['opm'], 20, 0.7, [1], loop_at=6); W('ym2151_loop.vgm', to_vgm(e, ['opm'], [OPM], tg)); W('ym2151_loop.s98', to_s98(e, ['opm'], [OPM], tags=(tg[0], tg[1], tg[3])))
    e = song(['opn'], S, 0.6, [2]); W('ym2203_typical.s98', to_s98(e, ['opn'], [OPN]))
    e = song(['opm'], S, 0.6, [1]); W('ym2151_typical.s98', to_s98(e, ['opm'], [OPM]))

if __name__ == '__main__':
    main(sys.argv[1], '--card' in sys.argv)
