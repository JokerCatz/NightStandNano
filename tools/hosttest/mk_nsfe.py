#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-or-later
"""Makes a synthetic NSFE from an NSF (there is no real NSFE in the test library): INFO, BANK, DATA, auth, time (every tune MS long), fade, tlbl ("Track N"), NEND.
   python3 mk_nsfe.py IN.nsf OUT.nsfe [MS=3000] [TITLE] [ARTIST]"""
import struct, sys
src, dst = sys.argv[1], sys.argv[2]
ms = int(sys.argv[3]) if len(sys.argv) > 3 else 3000
title = (sys.argv[4] if len(sys.argv) > 4 else "NSFE test").encode(); artist = (sys.argv[5] if len(sys.argv) > 5 else "synthetic").encode()
b = open(src, "rb").read()
assert b[:5] == b"NESM\x1a", "not an NSF"
n, start = b[6], b[7]
load, init, play = struct.unpack("<HHH", b[8:14]); bank = b[0x70:0x78]; pal, ext = b[0x7a], b[0x7b]
def ch(i, d): return struct.pack("<I", len(d)) + i + d
o = b"NSFE" + ch(b"INFO", struct.pack("<HHHBBBB", load, init, play, pal, ext, n, start - 1 if start else 0))
if any(bank): o += ch(b"BANK", bank)
o += ch(b"DATA", b[0x80:]) + ch(b"auth", title + b"\0" + artist + b"\0(c) none\0nobody\0")
o += ch(b"time", b"".join(struct.pack("<i", ms) for _ in range(n))) + ch(b"fade", b"".join(struct.pack("<i", 300) for _ in range(n)))
o += ch(b"tlbl", b"".join(b"Track %d\0" % (i + 1) for i in range(n))) + ch(b"NEND", b"")
open(dst, "wb").write(o)
