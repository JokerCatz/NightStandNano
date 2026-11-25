#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-or-later
"""Synthetic register streams for the second stage of the FM backend: YM2608 (rhythm, DELTA-T), YM2610 / YM2610B (ADPCM-A, ADPCM-B), YM2612 (with the PCM bank and the 0x8n DAC commands),
AY8910 (SSG alone), SN76489, OKIM6295, and mixes (YM2151 + SN76489, YM2151 + OKIM6295, YM2612 + YM2151 ...). No third-party content: patches, notes and the "sample ROM" bytes are random
(seeded). They are test signals for timing, bit-exactness, memory handling and "does sound come out", not music (the ADPCM data is noise).
   python3 -I gen_fm2_vgm.py OUTDIR [--card] [--big]      --card: the small demo files for the TF card (/sdcard/nsn_chip/fm/); --big: also an 8 MB-class Neo Geo style file and its .vgz
Writes VGM 1.71 files; `--s98` files are written for the devices S98 v3 defines (YM2608, YM2149 / AY8910, SN76489). Chip kinds: opm opn opna opnb opn2 ssg sn oki."""
import sys, struct, random, os, gzip
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_fm_vgm as g1

class Song:
    def __init__(self):
        self.ev = []; self.t = 0; self.marks = {}; self.blocks = []
    def w(self, chip, reg, val, port=0): self.ev.append(('w', chip, port, reg, val))
    def wait(self, n): self.ev.append(('t', n)); self.t += n
    def mark(self, name): self.ev.append(('m', name)); self.marks[name] = self.t
    def block(self, typ, total, start, data, second=False): self.blocks.append((typ, total, start, data, second))
    def pcm(self, data): self.blocks.append((0x00, 0, 0, data, False))   # YM2612 PCM bank: raw bytes, no header

CLK = dict(opm=3579545, opn=3993600, opna=7987200, opnb=8000000, opn2=7670454, ssg=1789773, sn=3579545, oki=1056000)
HDR = dict(sn=0x0C, opn2=0x2C, opm=0x30, opn=0x44, opna=0x48, opnb=0x4C, ssg=0x74, oki=0x98)
CMD = dict(opm=(0x54,), opn=(0x55,), opna=(0x56, 0x57), opnb=(0x58, 0x59), opn2=(0x52, 0x53))
S98DEV = dict(opn=2, opn2=3, opna=4, opm=5, ssg=15, sn=16)

# ---------------------------------------------------------------------------------------------- FM channels of the OPN family (same register layout in YM2203 / 2608 / 2610 / 2612)
def fm_patch(e, chip, port, c, r, pan=True):
    alg = r.choice([2, 4, 4, 5, 7, 7, 3, 0]); fb = r.randint(0, 6)
    w = lambda reg, val: e.w(chip, reg, val, port)
    w(0xB0 + c, (fb << 3) | alg)
    if pan: w(0xB4 + c, 0xC0 | (r.randint(0, 3) << 4) | r.randint(0, 3))        # both outputs on (the YM2203 has no such bit but ignores it)
    car = {0: [3], 1: [3], 2: [3], 3: [3], 4: [1, 3], 5: [1, 2, 3], 6: [1, 2, 3], 7: [0, 1, 2, 3]}[alg]
    for m in range(4):
        o = [0, 8, 4, 12][m] + c; iscar = m in car
        w(0x30 + o, (r.randint(0, 3) << 4) | r.choice([0, 1, 1, 2, 3, 4, 7]))
        w(0x40 + o, r.randint(0, 12) if iscar else r.randint(15, 50))
        w(0x50 + o, (r.randint(0, 2) << 6) | r.choice([31, 31, 28, 24]))
        w(0x60 + o, r.randint(4, 14)); w(0x70 + o, r.randint(0, 8)); w(0x80 + o, (r.randint(2, 8) << 4) | (r.randint(3, 8) if iscar else 15))

def fm_channels(kind):
    """(port, channel-in-port, key-on channel number): the channels each chip really has"""
    if kind == 'opn': return [(0, 0, 0), (0, 1, 1), (0, 2, 2)]
    if kind == 'opnb': return [(0, 1, 1), (0, 2, 2), (1, 1, 5), (1, 2, 6)]            # YM2610: four channels (the YM2610B has six)
    if kind == 'opnb2': return [(0, 0, 0), (0, 1, 1), (0, 2, 2), (1, 0, 4), (1, 1, 5), (1, 2, 6)]
    return [(0, 0, 0), (0, 1, 1), (0, 2, 2), (1, 0, 4), (1, 1, 5), (1, 2, 6)]

def init_fm(e, chip, kind, seed):
    r = random.Random(seed)
    chans = fm_channels(kind)
    if kind in ('opna',): e.w(chip, 0x29, 0x80)                                         # OPNA mode (6 channels, ADPCM, rhythm)
    for (port, c, k) in chans: fm_patch(e, chip, port, c, r, pan=kind != 'opn')
    return dict(r=r, chans=chans, free=[0] * len(chans), state=[None] * len(chans))

def step_fm(e, chip, st, k, gate):
    r = st['r']
    for i, (port, c, kc) in enumerate(st['chans']):
        if st['state'][i] is not None and k >= st['state'][i]:
            e.w(chip, 0x28, kc); st['state'][i] = None
        if st['state'][i] is None and k >= st['free'][i] and r.random() < 0.9:
            blk = r.randint(2, 5); fn = r.randint(617, 1155)
            e.w(chip, 0xA4 + c, (blk << 3) | (fn >> 8), port); e.w(chip, 0xA0 + c, fn & 0xFF, port)
            e.w(chip, 0x28, 0xF0 | kc)
            d = r.choice([6, 9, 12, 15, 24]); st['state'][i] = k + max(1, int(d * gate)); st['free'][i] = k + d

# ---------------------------------------------------------------------------------------------- SSG (inside the OPN family on port 0 regs 0..15, or alone as AY8910)
def init_ssg(e, chip, seed, via=lambda e, chip, reg, val: e.w(chip, reg, val)):
    r = random.Random(seed ^ 0x5500)
    via(e, chip, 7, 0b00111000)
    for i in range(3): via(e, chip, 8 + i, 0x0A)
    return dict(r=r, via=via, free=[0] * 3, st=[None] * 3)
def step_ssg(e, chip, st, k, gate):
    r, via = st['r'], st['via']
    for s in range(3):
        if st['st'][s] is not None and k >= st['st'][s]: via(e, chip, 8 + s, 0); st['st'][s] = None
        if st['st'][s] is None and k >= st['free'][s] and r.random() < 0.8:
            per = r.randint(60, 600); via(e, chip, 2 * s, per & 255); via(e, chip, 2 * s + 1, per >> 8); via(e, chip, 8 + s, r.randint(8, 15))
            if r.random() < 0.15: via(e, chip, 6, r.randint(1, 31)); via(e, chip, 7, 0b00110000 | (7 & ~(1 << s) if False else 0))
            d = r.choice([6, 9, 12]); st['st'][s] = k + max(1, int(d * gate)); st['free'][s] = k + d

# ---------------------------------------------------------------------------------------------- YM2608 rhythm and ADPCM-B (DELTA-T)
def init_opna_extra(e, chip, seed, deltat=True):
    r = random.Random(seed ^ 0x2608)
    e.w(chip, 0x11, 0x3F)                                                               # rhythm total level
    for i in range(6): e.w(chip, 0x18 + i, 0xC0 | r.randint(14, 31))                  # per instrument: pan + level
    st = dict(r=r, free=0, dt_free=0)
    if deltat:
        e.w(chip, 0x01, 0xC0, 1)                                                        # DELTA-T: L + R
        e.w(chip, 0x0B, 0xC0, 1)                                                        # level
        e.w(chip, 0x0C, 0xFF, 1); e.w(chip, 0x0D, 0xFF, 1)                              # limit address (at 0 playback wraps to address 0 at once)
    return st
def step_opna_extra(e, chip, st, k, deltat=True, dt_len=0x300):
    r = st['r']
    if k >= st['free'] and r.random() < 0.5:
        e.w(chip, 0x10, r.randint(1, 63)); st['free'] = k + r.choice([8, 12, 20, 30])   # key on (bit 7 = 0) of some rhythm instruments
    if deltat and k >= st['dt_free'] and r.random() < 0.3:
        start = r.randint(0, 16) * 4                                                      # register units: the chip's address is this << 2 bytes
        e.w(chip, 0x00, 0x01, 1)                                                        # reset
        e.w(chip, 0x02, start & 0xFF, 1); e.w(chip, 0x03, start >> 8, 1)
        e.w(chip, 0x04, (start + dt_len) & 0xFF, 1); e.w(chip, 0x05, (start + dt_len) >> 8, 1)
        dn = r.randint(0x2000, 0x8000); e.w(chip, 0x09, dn & 255, 1); e.w(chip, 0x0A, dn >> 8, 1)
        e.w(chip, 0x00, 0xA0, 1)                                                        # start, external memory
        st['dt_free'] = k + r.choice([20, 40, 60])

# ---------------------------------------------------------------------------------------------- YM2610: ADPCM-A (port 1) and ADPCM-B (port 0)
def init_opnb_extra(e, chip, seed, adpcm_a_len, adpcm_b_len):
    r = random.Random(seed ^ 0x2610)
    e.w(chip, 0x01, 0x3F, 1)                                                            # ADPCM-A total level
    for i in range(6): e.w(chip, 0x08 + i, 0xC0 | r.randint(14, 31), 1)
    e.w(chip, 0x11, 0xC0)                                                               # ADPCM-B pan
    e.w(chip, 0x1B, 0xC0)                                                               # ADPCM-B level
    return dict(r=r, free=[0] * 6, bfree=0, alen=adpcm_a_len, blen=adpcm_b_len)
def step_opnb_extra(e, chip, st, k):
    r = st['r']
    for ch in range(6):
        if k >= st['free'][ch] and r.random() < 0.35:
            start = r.randint(0, max(0, st['alen'] // 256 - 8)); end = start + r.randint(1, 8)
            e.w(chip, 0x10 + ch, start & 255, 1); e.w(chip, 0x18 + ch, start >> 8, 1); e.w(chip, 0x20 + ch, end & 255, 1); e.w(chip, 0x28 + ch, end >> 8, 1)
            e.w(chip, 0x00, 1 << ch, 1)                                                  # key on
            st['free'][ch] = k + r.choice([6, 9, 14, 20])
    if k >= st['bfree'] and r.random() < 0.4:
        start = r.randint(0, max(0, st['blen'] // 256 - 8)); end = start + r.randint(2, 8)
        e.w(chip, 0x10, 0x01); e.w(chip, 0x12, start & 255); e.w(chip, 0x13, start >> 8); e.w(chip, 0x14, end & 255); e.w(chip, 0x15, end >> 8)
        dn = r.randint(0x2000, 0x8000); e.w(chip, 0x19, dn & 255); e.w(chip, 0x1A, dn >> 8)
        e.w(chip, 0x10, 0x80)                                                           # start
        st['bfree'] = k + r.choice([20, 40])

# ---------------------------------------------------------------------------------------------- SN76489
def init_sn(e, chip, seed):
    r = random.Random(seed ^ 0x7648)
    for ch in range(3): e.w(chip, 0, 0x90 | (ch << 5) | 0x0F)                           # attenuation 15 (off)
    e.w(chip, 0, 0xFF)
    return dict(r=r, free=[0] * 4, st=[None] * 4)
def step_sn(e, chip, st, k, gate):
    r = st['r']
    for ch in range(4):
        if st['st'][ch] is not None and k >= st['st'][ch]: e.w(chip, 0, 0x90 | (ch << 5) | 0x0F); st['st'][ch] = None
        if st['st'][ch] is None and k >= st['free'][ch] and r.random() < 0.8:
            if ch < 3:
                per = r.randint(40, 900); e.w(chip, 0, 0x80 | (ch << 5) | (per & 15)); e.w(chip, 0, per >> 4)
            else:
                e.w(chip, 0, 0xE0 | r.randint(0, 7))
            e.w(chip, 0, 0x90 | (ch << 5) | r.randint(0, 6))
            d = r.choice([6, 9, 12, 18]); st['st'][ch] = k + max(1, int(d * gate)); st['free'][ch] = k + d

# ---------------------------------------------------------------------------------------------- OKIM6295: phrase table + noise "ADPCM"
def oki_rom(seed, nphr=24, size=0x20000):
    r = random.Random(seed ^ 0x6295)
    rom = bytearray(size)
    pos = 0x400                                                                          # phrase table in the first 0x400 bytes: 8 bytes per phrase (start, end: 3 bytes each)
    for p in range(1, nphr + 1):
        ln = r.randint(0x400, 0x1800)
        if pos + ln >= size: break
        rom[p * 8:p * 8 + 3] = pos.to_bytes(3, 'big'); rom[p * 8 + 3:p * 8 + 6] = (pos + ln - 1).to_bytes(3, 'big')
        for i in range(ln): rom[pos + i] = r.randint(0, 255) & 0xEE if r.random() < 0.8 else r.randint(0, 255)
        pos += ln
    return bytes(rom), nphr
def init_oki(e, chip, seed, nphr):
    return dict(r=random.Random(seed ^ 0x1295), free=[0] * 4, nphr=nphr)
def step_oki(e, chip, st, k):
    r = st['r']
    for v in range(4):
        if k >= st['free'][v] and r.random() < 0.3:
            e.w(chip, 0, 0x80 | r.randint(1, st['nphr']))                                # first byte: the phrase
            e.w(chip, 0, (1 << (4 + v)) | r.randint(0, 5))                               # second byte: the voice and the attenuation
            st['free'][v] = k + r.choice([10, 20, 40])

# ---------------------------------------------------------------------------------------------- YM2612 PCM bank
def pcm_bank(seed, n=0x6000):
    r = random.Random(seed ^ 0x2612)
    ph = 0.0; out = bytearray()
    for i in range(n): ph += 0.07 + 0.03 * (i // 2048 % 5); out.append(128 + int(90 * ((ph % 1.0) * 2 - 1)) if i % 7 else r.randint(100, 156))
    return bytes(out)

# ---------------------------------------------------------------------------------------------- the song builder
def build(chips, secs, gate, seeds, loop_at=None, oki_nphr=24, adpcm_a=0x20000, adpcm_b=0x10000, dac=False, deltat=True, pieces=1, resend=False, regload=False):
    """chips: list of (kind, variant): kind in opm opn opna opnb opn2 ssg sn oki; variant 'b' for the YM2610B, 'dual' for a second chip of the kind just before it"""
    e = Song(); sts = []; blob = {}
    for i, (kind, var) in enumerate(chips):
        sd = seeds[i]
        if kind == 'opm': sts.append(('opm', g1.gen_opm(e, i, secs, gate, sd, True)))
        elif kind == 'opn': sts.append(('opn', g1.gen_opn(e, i, secs, gate, sd, True)))
        elif kind == 'opna':
            fm = init_fm(e, i, 'opna', sd); ssg = init_ssg(e, i, sd); ex = init_opna_extra(e, i, sd, deltat)
            sts.append(('opna', (fm, ssg, ex)))
            if deltat and not regload:
                rnd = random.Random(sd); data = bytes(rnd.randint(0, 255) for _ in range(0x300 + 64 + 0x300))
                e.block(0x81, 0x40000, 0, data)
            elif deltat:                                                                # S98 has no data blocks: the DELTA-T RAM is written through the chip's own memory-write register 0x08
                rnd = random.Random(sd)
                e.w(i, 0x00, 0x01, 1); e.w(i, 0x00, 0x60, 1); e.w(i, 0x02, 0, 1); e.w(i, 0x03, 0, 1); e.w(i, 0x04, 0x00, 1); e.w(i, 0x05, 0x04, 1)
                for _ in range(0x1000): e.w(i, 0x08, rnd.randint(0, 255), 1)
                e.w(i, 0x00, 0x01, 1)
        elif kind == 'opnb':
            fm = init_fm(e, i, 'opnb2' if var == 'b' else 'opnb', sd); ssg = init_ssg(e, i, sd); ex = init_opnb_extra(e, i, sd, adpcm_a, adpcm_b)
            sts.append(('opnb', (fm, ssg, ex)))
            rnd = random.Random(sd)
            def noise(n):                                                                # random bytes; above 1 MB one 16 KB random block repeated (inside deflate's 32 KB window) (compresses well, so a 14 MB file is a small .vgz to put on the card)
                if n <= (1 << 20): return bytes(rnd.randint(0, 255) for _ in range(n))
                blk = bytes(rnd.randint(0, 255) for _ in range(16384)); return (blk * (n // 16384 + 1))[:n]
            da = noise(adpcm_a); db = noise(adpcm_b)
            if pieces > 1:                                                              # the ROM sent in pieces (and the first piece sent again at the end of the header block list)
                step = adpcm_a // pieces
                for q in range(pieces): e.block(0x82, adpcm_a, q * step, da[q * step:(q + 1) * step] if q < pieces - 1 else da[q * step:])
                if resend: e.block(0x82, adpcm_a, 0, da[:step])
            else: e.block(0x82, adpcm_a, 0, da)
            e.block(0x83, adpcm_b, 0, db)
        elif kind == 'opn2':
            fm = init_fm(e, i, 'opn2', sd); sts.append(('opn2', (fm, dac)))
            if dac: e.pcm(pcm_bank(sd)); e.w(i, 0x2B, 0x80)                              # DAC on
        elif kind == 'ssg': sts.append(('ssg', init_ssg(e, i, sd, via=lambda e, c, reg, val: e.w(c, reg, val))))
        elif kind == 'sn': sts.append(('sn', init_sn(e, i, sd)))
        elif kind == 'oki':
            rom, n = oki_rom(sd, oki_nphr); e.block(0x8B, len(rom), 0, rom); sts.append(('oki', init_oki(e, i, sd, n)))
    dacpos = 0
    for k in range(int(secs * 60)):
        if loop_at is not None and k == int(loop_at * 60): e.mark('loop')
        for i, (kind, st) in enumerate(sts):
            if kind == 'opm': g1.step_opm(e, i, st, k)
            elif kind == 'opn': g1.step_opn(e, i, st, k)
            elif kind == 'opna': step_fm(e, i, st[0], k, gate); step_ssg(e, i, st[1], k, gate); step_opna_extra(e, i, st[2], k, deltat)
            elif kind == 'opnb': step_fm(e, i, st[0], k, gate); step_ssg(e, i, st[1], k, gate); step_opnb_extra(e, i, st[2], k)
            elif kind == 'opn2': step_fm(e, i, st[0], k, gate)
            elif kind == 'ssg': step_ssg(e, i, st, k, gate)
            elif kind == 'sn': step_sn(e, i, st, k, gate)
            elif kind == 'oki': step_oki(e, i, st, k)
        if dac and any(c[0] == 'opn2' for c in chips):
            for j in range(735 // 5):                                                   # the PCM bank played back at 8.8 kHz: 0x80 + n commands, each followed by a wait of n (5 samples)
                e.ev.append(('dac', 5)); e.t += 5
            e.t += 735 - 5 * (735 // 5)
            e.ev.append(('t_nocount', 735 - 5 * (735 // 5)))
        else: e.wait(735)
    return e

# ---------------------------------------------------------------------------------------------- serialisers
def gd3(title, game, system, author):
    return g1.gd3(title, game, system, author)

def block_bytes(typ, total, start, data, second):
    if typ == 0x00: payload = data
    else: payload = struct.pack('<II', total, start) + data
    size = len(payload) | (0x80000000 if second else 0)
    return b'\x67\x66' + bytes([typ]) + struct.pack('<I', size) + payload

def to_vgm(e, chips, tags=None, version=0x171, flags=None):
    seen = {}; kinds = []
    for kind, var in chips:
        n = seen.get(kind, 0); seen[kind] = n + 1; kinds.append((kind, n))
    body = bytearray(); loop_off = None
    for b in e.blocks: body += block_bytes(*b)
    for x in e.ev:
        if x[0] == 'm' and x[1] == 'loop': loop_off = len(body)
        elif x[0] == 't_nocount': pass
        elif x[0] == 'dac':
            body += bytes([0x80 | x[1]])
        elif x[0] == 't':
            n = x[1]
            while n > 0:
                m = min(n, 65535); body += (b'\x62' if m == 735 else b'\x63' if m == 882 else b'\x61' + struct.pack('<H', m)); n -= m
        elif x[0] == 'w':
            _, chip, port, reg, val = x
            kind, n = kinds[chip]
            if kind in CMD: body += bytes([CMD[kind][port] + (0x50 if n else 0), reg, val])
            elif kind == 'ssg': body += bytes([0xA0, reg | (0x80 if n else 0), val])
            elif kind == 'sn': body += bytes([0x30 if n else 0x50, val])
            elif kind == 'oki': body += bytes([0xB8, reg | (0x80 if n else 0), val])
    body += b'\x66'
    data_off = 0x100
    h = bytearray(data_off); h[0:4] = b'Vgm '
    struct.pack_into('<I', h, 8, version); struct.pack_into('<I', h, 0x18, e.t)
    if loop_off is not None:
        struct.pack_into('<I', h, 0x1C, data_off + loop_off - 0x1C); struct.pack_into('<I', h, 0x20, e.t - e.marks['loop'])
    struct.pack_into('<I', h, 0x24, 60); struct.pack_into('<I', h, 0x34, data_off - 0x34)
    seen = {}
    for kind, var in chips:
        off = HDR[kind]; n = seen.get(kind, 0); seen[kind] = n + 1
        clk = CLK[kind]
        if n == 0:
            if (kind == 'opnb' and var == 'b') or (kind == 'opn2' and var == 'b'): clk |= 0x80000000
            struct.pack_into('<I', h, off, clk)
        else: struct.pack_into('<I', h, off, struct.unpack_from('<I', h, off)[0] | 0x40000000)
    if any(k == 'sn' for k, _ in chips): struct.pack_into('<HBB', h, 0x28, 0x0009, 16, 0)       # SN76489: Sega PSG taps, 16-bit register, no flags
    out = bytearray(h) + body
    if tags:
        struct.pack_into('<I', out, 0x14, len(out) - 0x14); out += gd3(*tags)
    struct.pack_into('<I', out, 4, len(out) - 4)
    return bytes(out)

def to_s98(e, chips, tags=None):
    """S98 v3, timer 1/60 s. Devices: opn 2, opn2 3, opna 4, opm 5, ssg 15, sn 16. DELTA-T / rhythm data is not carried (the rhythm ROM is the host's)."""
    body = bytearray(); loop_off = None; pend = 0
    def flush():
        nonlocal pend
        while pend > 0:
            if pend == 1: body.append(0xFF); pend = 0
            else:
                n = min(pend, 2 + 0x3FFF) - 2; pend -= n + 2; v = n; out = bytearray()
                while True:
                    b = v & 0x7F; v >>= 7; out.append(b | (0x80 if v else 0))
                    if not v: break
                body.append(0xFE); body.extend(out)
    for x in e.ev:
        if x[0] == 'm' and x[1] == 'loop': flush(); loop_off = len(body)
        elif x[0] == 't': pend += 1
        elif x[0] == 'w':
            flush(); _, chip, port, reg, val = x
            body += bytes([chip * 2 + port, reg, val])
    flush(); body.append(0xFD)
    dev = bytearray()
    for kind, var in chips: dev += struct.pack('<IIII', S98DEV[kind], CLK[kind], 0, 0)
    tag = b''
    if tags: tag = b'[S98]' + (f'title={tags[0]}\ngame={tags[1]}\nartist={tags[2]}\nsystem=synthetic\n').encode() + b'\0'
    hdr_len = 0x20 + len(dev); tag_off = hdr_len if tag else 0; data_off = hdr_len + len(tag)
    h = bytearray(0x20); h[0:4] = b'S983'
    struct.pack_into('<IIIIIII', h, 4, 1, 60, 0, tag_off, data_off, (data_off + loop_off) if loop_off is not None else 0, len(chips))
    return bytes(h) + bytes(dev) + tag + bytes(body)

def stress2(kind, var, secs, seed):
    """random writes over the whole register space of a YM2608 / YM2610 / YM2612 (both ports), with ADPCM data present: SSG-EG, LFO, CSM, channel-3 modes, the rhythm / ADPCM-A / DELTA-T registers
    in any order. Not music: a probe for patches and for the memory handling, which must not change one bit against the unpatched ymfm."""
    r = random.Random(seed); e = Song()
    rnd = random.Random(seed + 1)
    if kind == 'opna': e.block(0x81, 0x40000, 0x100, bytes(rnd.randint(0, 255) for _ in range(0x2000)))
    if kind == 'opnb':
        e.block(0x82, 0x8000, 0, bytes(rnd.randint(0, 255) for _ in range(0x8000))); e.block(0x83, 0x4000, 0, bytes(rnd.randint(0, 255) for _ in range(0x4000)))
    for k in range(int(secs * 60)):
        for _ in range(r.randint(0, 16)):
            port = r.randint(0, 1)
            if r.random() < 0.15:                                                       # the chip-specific areas
                if kind == 'opna' and port == 0: reg = r.randint(0x00, 0x1D)
                elif kind == 'opna': reg = r.randint(0x00, 0x10)
                elif kind == 'opnb' and port == 0: reg = r.choice(list(range(0x00, 0x10)) + list(range(0x10, 0x1D)))
                elif kind == 'opnb': reg = r.randint(0x00, 0x2D)
                else: reg = r.randint(0x20, 0x2F)
            else: reg = r.randint(0x30, 0xB6)
            if port == 0 and reg in (0x28,) or (kind == 'opn2' and port == 0 and reg == 0x2A): continue
            val = r.randint(0, 255)
            if 0x90 <= reg <= 0x9F and port in (0, 1) and reg >= 0x30: val = r.choice([0, 8, 9, 10, 11, 12, 13, 14, 15])
            e.w(0, reg, val, port)
        for _ in range(r.randint(0, 3)):
            chans = {'opna': [0, 1, 2, 4, 5, 6], 'opnb': [1, 2, 5, 6], 'opn2': [0, 1, 2, 4, 5, 6]}[kind]
            e.w(0, 0x28, (r.choice([0, 0, 0xF, 0x7, 0x3, 0x5]) << 4) | r.choice(chans))
        if r.random() < 0.05: e.w(0, 0x27, r.choice([0x00, 0x40, 0x80]))
        e.wait(735)
    return e

def main(out, card=False, big=False):
    os.makedirs(out, exist_ok=True)
    W = lambda name, data: open(os.path.join(out, name), 'wb').write(data)
    def V(name, chips, secs, gate, seeds, tg=None, **kw):
        e = build(chips, secs, gate, seeds, **kw); W(name, to_vgm(e, chips, tg)); return e
    if card:
        T = lambda t: (t, 'fmvgm test', 'synthetic', 'generator')
        V('ym2608_demo.vgm', [('opna', '')], 20, 0.7, [3], T('YM2608 test stream (rhythm needs the optional ROM)'), loop_at=4)
        e = build([('opna', '')], 20, 0.7, [3], regload=True, loop_at=4); W('ym2608_demo.s98', to_s98(e, [('opna', '')], ('YM2608 S98 test stream', 'fmvgm test', 'generator')))
        V('ym2610_demo.vgm', [('opnb', '')], 20, 0.7, [4], T('YM2610 ADPCM-A/B test stream'), loop_at=4)
        V('ym2610b_demo.vgm', [('opnb', 'b')], 20, 0.7, [5], T('YM2610B test stream'))
        V('ym2612_dac_demo.vgm', [('opn2', '')], 20, 0.7, [6], T('YM2612 + PCM bank test stream'), dac=True)
        V('ay8910_demo.vgm', [('ssg', '')], 20, 0.7, [7], T('AY8910 test stream'))
        V('ym2151_sn76489_demo.vgm', [('opm', ''), ('sn', '')], 20, 0.7, [1, 8], T('YM2151 + SN76489 test stream'))
        V('ym2151_oki6295_demo.vgm', [('opm', ''), ('oki', '')], 20, 0.7, [1, 9], T('YM2151 + OKIM6295 test stream'))
        V('oki6295_demo.vgm', [('oki', '')], 20, 0.7, [9], T('OKIM6295 test stream'))
        V('ym2151_dual_split_demo.vgm', [('opm', ''), ('opm', '')], 20, 0.7, [1, 7], T('dual YM2151 test stream (split over both cores)'))
        V('ym2203_ym2151_demo.vgm', [('opm', ''), ('opn', '')], 20, 0.7, [1, 2], T('YM2151 + YM2203 test stream'))
        return
    for kind, var in (('opna', ''), ('opnb', ''), ('opnb', 'b'), ('opn2', '')):
        for sd in range(1, 5):
            W(f'stress2_{kind}{var}_{sd}.vgm', to_vgm(stress2(kind, var, 20, 200 + sd), [(kind, var)]))
    S = 30
    V('ym2608_typical.vgm', [('opna', '')], S, 0.6, [3]); V('ym2608_dense.vgm', [('opna', '')], S, 1.0, [3])
    e = build([('opna', '')], S, 0.6, [3], regload=True); W('ym2608_typical.s98', to_s98(e, [('opna', '')]))
    V('ym2610_typical.vgm', [('opnb', '')], S, 0.6, [4]); V('ym2610_dense.vgm', [('opnb', '')], S, 1.0, [4])
    V('ym2610b_typical.vgm', [('opnb', 'b')], S, 0.6, [5]); V('ym2610b_dense.vgm', [('opnb', 'b')], S, 1.0, [5])
    V('ym2610_pieces.vgm', [('opnb', '')], S, 0.6, [4], pieces=5, resend=True)
    V('ym2612_typical.vgm', [('opn2', '')], S, 0.6, [6]); V('ym2612_dense.vgm', [('opn2', '')], S, 1.0, [6]); V('ym2612_dac.vgm', [('opn2', '')], S, 0.6, [6], dac=True)
    V('ay8910_typical.vgm', [('ssg', '')], S, 0.6, [7]); V('ay8910_dual.vgm', [('ssg', ''), ('ssg', '')], S, 0.6, [7, 17])
    V('sn76489_only.vgm', [('sn', '')], S, 0.6, [8])
    V('oki6295_typical.vgm', [('oki', '')], S, 0.6, [9]); V('oki6295_dense.vgm', [('oki', '')], S, 1.0, [9], oki_nphr=24)
    V('ym2151_sn76489.vgm', [('opm', ''), ('sn', '')], S, 0.6, [1, 8]); V('ym2151_oki6295.vgm', [('opm', ''), ('oki', '')], S, 0.6, [1, 9])
    V('ym2151_oki6295_dense.vgm', [('opm', ''), ('oki', '')], S, 1.0, [1, 9])
    V('ym2612_ym2151.vgm', [('opn2', ''), ('opm', '')], S, 0.6, [6, 1]); V('ym2612_ym2151_dac.vgm', [('opn2', ''), ('opm', '')], S, 0.6, [6, 1], dac=True)
    V('ym2203_ay8910.vgm', [('opn', ''), ('ssg', '')], S, 0.6, [2, 7])
    V('ym2608_dual.vgm', [('opna', ''), ('opna', '')], S, 0.6, [3, 13]); V('ym2610_dual.vgm', [('opnb', ''), ('opnb', '')], S, 0.6, [4, 14])
    V('ym2151_ym2203.vgm', [('opm', ''), ('opn', '')], S, 0.6, [1, 2]); V('ym2151_ym2203_dense.vgm', [('opm', ''), ('opn', '')], S, 1.0, [1, 2])
    V('ym2151_dual_typical.vgm', [('opm', ''), ('opm', '')], S, 0.6, [1, 7])
    V('ym2610_loop.vgm', [('opnb', '')], 20, 0.6, [4], ('Loop test', 'fmvgm tests', 'synthetic', 'generator'), loop_at=6)
    V('oki6295_loop.vgm', [('oki', '')], 20, 0.6, [9], ('Loop test', 'fmvgm tests', 'synthetic', 'generator'), loop_at=6)
    if big:                                                                             # a Neo Geo sized file: 6 MB of ADPCM-A data, 2 MB of ADPCM-B
        V('ym2610_big.vgm', [('opnb', '')], 60, 0.6, [4], ('Big ADPCM', 'fmvgm tests', 'synthetic', 'generator'), adpcm_a=6 * 1024 * 1024, adpcm_b=2 * 1024 * 1024)
        with open(os.path.join(out, 'ym2610_big.vgm'), 'rb') as f: raw = f.read()
        with gzip.GzipFile(os.path.join(out, 'ym2610_big.vgz'), 'wb', 9, mtime=0) as g: g.write(raw)
        V('ym2610_huge.vgm', [('opnb', '')], 30, 0.6, [4], ('Huge ADPCM', 'fmvgm tests', 'synthetic', 'generator'), adpcm_a=14 * 1024 * 1024, adpcm_b=4 * 1024 * 1024)
        with open(os.path.join(out, 'ym2610_huge.vgm'), 'rb') as f: raw = f.read()
        with gzip.GzipFile(os.path.join(out, 'ym2610_huge.vgz'), 'wb', 9, mtime=0) as g: g.write(raw)

if __name__ == '__main__':
    main(sys.argv[1], '--card' in sys.argv, '--big' in sys.argv)
