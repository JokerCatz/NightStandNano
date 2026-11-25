#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-or-later
"""Push a local file to the board's TF card through the UART console (putx). The file content is NEVER printed.
   python3 tools/put_file.py LOCAL_FILE /sdcard/remote/path
Port: $NSN_PORT, see nsn_port.py. No reset."""
import sys, os, time, serial
from nsn_port import find_port
PORT = find_port()
src, dst = sys.argv[1], sys.argv[2]
data = open(src, "rb").read()
s = serial.Serial(); s.port = PORT; s.baudrate = 115200; s.timeout = 0.02; s.dtr = False; s.rts = False; s.open()
time.sleep(5.5)          # opening the port may reset the board: let it boot and the console come up
s.reset_input_buffer()
ok = True
for i in range(0, max(len(data), 1), 150):
    chunk = data[i:i + 150].hex()
    s.write((("putx! " if i == 0 else "putx ") + dst + " " + chunk + "\n").encode())
    t = time.time() + 3; got = b""
    while time.time() < t and b"ok " not in got and b"putx:" not in got:
        got += s.read(512)
    if b"ok " not in got:
        ok = False; print("chunk", i // 150, "failed:", got.decode(errors="replace").strip()[-60:]); break
print("put", len(data), "bytes ->", dst if ok else "FAILED")
sys.exit(0 if ok else 1)
