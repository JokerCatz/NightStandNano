#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-or-later
"""Build the NightStandNano scene folder from sleepmixer's library.muk (a zip) + default_scenes.json.
   make_scenes.py LIBRARY.muk default_scenes.json OUTDIR [scene-id-or-name-substring ...] [--max-bytes N]
OUTDIR gets scenes.txt ("name|file.ogg:vol|...", at most 3 layers) and the sound files used; copy it to /sdcard/_player/scenes/.
Without scene filters every scene whose sounds are all in the library (ogg/mp3) is written."""
import sys, json, zipfile, os
a = [x for x in sys.argv[1:]]
mb = 10**9
if "--max-bytes" in a:
    i = a.index("--max-bytes"); mb = int(a[i + 1]); del a[i:i + 2]
lib, js, out, filt = a[0], a[1], a[2], a[3:]
z = zipfile.ZipFile(lib); size = {i.filename: i.file_size for i in z.infolist()}
os.makedirs(out, exist_ok=True)
lines, used = [], set()
for s in json.load(open(js))["scenes"]:
    if filt and not any(f in s["id"] or f.lower() in s["en_name"].lower() for f in filt): continue
    ls = []
    for r in s["resolved_sounds"][:3]:
        f = next((n for n in size if n.startswith(r["sha256"]) and n.endswith((".ogg", ".mp3"))), None)
        if f: ls.append((f, r["volume"]))
    if not ls or sum(size[f] for f, _ in ls) > mb: continue
    lines.append(s["en_name"] + "|" + "|".join(f"{f[:12]}{os.path.splitext(f)[1]}:{v}" for f, v in ls))
    used.update(f for f, _ in ls)
for f in used:
    open(os.path.join(out, f[:12] + os.path.splitext(f)[1]), "wb").write(z.read(f))
open(os.path.join(out, "scenes.txt"), "w").write("\n".join(lines) + "\n")
print(len(lines), "scenes,", len(used), "sound files")
