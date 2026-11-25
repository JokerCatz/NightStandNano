#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-or-later
"""Short stress / leak / load checks on the test board, one serial session (opening the port resets the board once).
   python tools/stress.py leak [ROUNDS]     cycle every source and feature ROUNDS times (default 20), compare free memory before / after
   python tools/stress.py load              CPU per core for the heavy combinations, 8 s each
Run with the IDF python (it has pyserial). Reads `st` JSON: mem (internal free), big (largest free internal block), psram, cpu, und."""
import sys, os, time, json, serial
from nsn_port import find_port
PORT = find_port()
s = serial.Serial(); s.port = PORT; s.baudrate = 115200; s.timeout = 0.05; s.dtr = False; s.rts = False; s.open()
for d, r in ((False, True), (False, False)): s.dtr = d; s.rts = r; time.sleep(0.15)
buf = b""
def pump(t):
    global buf
    end = time.time() + t
    while time.time() < end: buf += s.read(4096)
def cmd(c, wait=0.4):
    global buf
    buf = b""; s.write((c + "\n").encode()); pump(wait)
    return buf.decode(errors="replace")
def st():
    for _ in range(4):
        for l in cmd("st", 0.5).splitlines():
            if l.startswith("{"):
                try: return json.loads(l)
                except ValueError: pass
    raise SystemExit("no st reply")
def crashed(): return b"Guru Meditation" in buf or b"rst:0x" in buf
pump(14)                                   # boot, Wi-Fi, NTP
F = "/sdcard/nsn_fmt/"
if sys.argv[1] == "leak":
    rounds = int(sys.argv[2]) if len(sys.argv) > 2 else 20
    cmd("stop"); pump(1); a = st(); und = 0; bad = 0
    print(f"start  mem {a['mem']}  big {a['big']}  psram {a['psram']}")
    steps = ["play " + F + x for x in ("t.mp3", "t.flac", "t.m4a", "t.ogg", "t.opus", "t.aac", "t.aiff")] + \
            ["play " + F, "next", "prev", "scene 1", "scene 2", "noise pink", "noise brown", "radio 1", "play /sdcard/nsn_tag/tag.flac",
             "fx on equalizer 1", "fx on compressor 1", "fx on freeverb 1", "play " + F + "t.flac", "lrc", "fx off", "bedtime 1", "wake", "timer 0", "mode shuffle", "play " + F, "next", "mode folder", "stop", "resume", "stop"]
    for r in range(rounds):
        for c in steps:
            out = cmd(c, 1.2 if c.startswith(("play", "scene", "noise", "resume")) else 3.0 if c.startswith("radio") else 0.4)
            if crashed(): raise SystemExit(f"CRASH in round {r} after '{c}'\n{out[-600:]}")
        x = st(); und += x["und"]
        cmd("stop"); pump(0.8); x = st()
        print(f"round {r + 1:2d}  mem {x['mem']}  big {x['big']}  psram {x['psram']}", flush=True)
    b = x
    print(f"end    mem {b['mem']} ({b['mem'] - a['mem']:+d})  big {b['big']}  psram {b['psram']} ({b['psram'] - a['psram']:+d})")
else:
    def measure(name, setup, secs=8):
        for c in setup: cmd(c, 0.5)
        pump(3); st(); pump(secs); x = st()
        print(f"{name:44s} cpu {x['cpu'][0]:3d}% {x['cpu'][1]:3d}%  mem {x['mem']:6d} big {x['big']:6d}  und {x['und']}  {x['st']} {x['fmt']} -> {x['out']}", flush=True)
        if crashed(): raise SystemExit("CRASH")
    HEAVY = ["fx on equalizer 1", "fx set equalizer count 10", "fx on tone 1", "fx on bass 1", "fx on compressor 1", "fx on freeverb 1", "fx on tube 1", "fx on velvet_widener 1", "fx on limiter 1", "fx on loudness 1", "fx on crossfeed 1"]
    cmd("fx off"); cmd("mode one"); cmd("out dac")
    measure("idle", ["stop"], 5)
    measure("FLAC 48k -> DAC", ["play " + F + "t.flac"])
    measure("MP3 -> DAC", ["play " + F + "t.mp3"])
    measure("Opus -> DAC", ["play " + F + "t.opus"])
    measure("scene (2 Ogg layers) -> DAC", ["scene 1"])
    measure("radio AAC -> DAC", ["radio 1"], 10)
    measure("FLAC + all 11 fx -> DAC", HEAVY + ["play " + F + "t.flac"])
    measure("scene + all 11 fx -> DAC", ["scene 1"])
    measure("radio AAC + all 11 fx -> DAC", ["radio 1"], 10)
    cmd("fx off"); cmd("fx set equalizer count 0"); cmd("mode folder"); cmd("stop")
