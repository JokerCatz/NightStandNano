#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-or-later
"""Type console lines on the JukeDial (round) board on the desk and print what it says: the Bluetooth peer for Nano's tests.
   python tools/round_cmd.py SECONDS [LINE...]        e.g.  round_cmd.py 5 "btmode 1"   /   round_cmd.py 20 "btmode 4"
Answers a pairing prompt with "pair yes". Put round back with "btmode 0" afterwards.

OPTIONAL helper, only for people who own a JukeDial as well (it is the Bluetooth peer in Nano's tests); the firmware does not need it.
  JUKEDIAL_TOOLS   the tools/ folder of a JukeDial (ESP32S31_round) checkout; it provides boardport.open_port (required)
  JUKEDIAL_PORT    the JukeDial's serial device (required; never Nano's own port)"""
import os, sys, time
tools, PORT = os.environ.get("JUKEDIAL_TOOLS"), os.environ.get("JUKEDIAL_PORT")
if not tools or not PORT: sys.exit("round_cmd: set JUKEDIAL_TOOLS (JukeDial checkout's tools/ folder) and JUKEDIAL_PORT (its serial device)")
sys.path.insert(0, tools)
from boardport import open_port
s = open_port(PORT); secs = float(sys.argv[1]); cmds = sys.argv[2:]
for c in cmds: s.write((c + "\n").encode()); time.sleep(0.4)
end = time.time() + secs; b = b""; said = False
while time.time() < end:
    b += s.read(4096)
    if not said and b"pair request" in b: s.write(b"pair yes\n"); said = True
sys.stdout.write(b.decode(errors="replace")[-6000:])
