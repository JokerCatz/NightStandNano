#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-or-later
"""BLE control-service test client (what the web page / app does): connect, subscribe to EVT and SPEC, send commands, print replies.
   .local/venv/bin/python tools/ble_probe.py [--name 'NightStand Nano'] [--spec SECONDS] CMD [CMD ...]
Commands are sent one by one; a command 'sleep N' waits N seconds."""
import argparse, asyncio, time
from bleak import BleakClient, BleakScanner

SVC = "7f2a0001-a1c4-4e53-9a31-6e73616e6f00"
CMD = "7f2a0002-a1c4-4e53-9a31-6e73616e6f00"
EVT = "7f2a0003-a1c4-4e53-9a31-6e73616e6f00"
SPEC = "7f2a0004-a1c4-4e53-9a31-6e73616e6f00"


async def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--name", default="NightStand Nano")
    ap.add_argument("--spec", type=float, default=0, help="seconds of spectrum frames to count")
    ap.add_argument("cmds", nargs="*")
    a = ap.parse_args()
    t0 = time.time()
    dev = await BleakScanner.find_device_by_filter(lambda d, ad: SVC in [u.lower() for u in (ad.service_uuids or [])] or d.name == a.name, timeout=15)
    if not dev:
        print("device not found"); return 2
    print("found", dev.address, dev.name, "in %.1fs" % (time.time() - t0), flush=True)
    buf = {"evt": b"", "spec": 0, "first": None, "last": None, "len": 0}

    def on_evt(_, data):
        buf["evt"] += bytes(data)
        while b"\n" in buf["evt"]:
            line, buf["evt"] = buf["evt"].split(b"\n", 1)
            print("<", line.decode(errors="replace"), flush=True)

    def on_spec(_, data):
        buf["spec"] += 1; buf["len"] = len(data)
        if buf["first"] is None: buf["first"] = time.time()
        buf["last"] = time.time(); buf["frame"] = bytes(data)

    async with BleakClient(dev, timeout=20) as c:
        print("connected, mtu", c.mtu_size, flush=True)
        await c.start_notify(EVT, on_evt)
        if a.spec: await c.start_notify(SPEC, on_spec)
        for cmd in a.cmds:
            if cmd.startswith("sleep "):
                await asyncio.sleep(float(cmd[6:])); continue
            print(">", cmd, flush=True)
            await c.write_gatt_char(CMD, cmd.encode(), response=True)
            await asyncio.sleep(0.8)
        if a.spec: await asyncio.sleep(a.spec)
        if a.spec:
            n = buf["spec"]; dt = (buf["last"] - buf["first"]) if n > 1 else 0
            print("spectrum frames: %d (%.1f/s), frame bytes %d" % (n, (n - 1) / dt if dt else 0, buf["len"]))
            if "frame" in buf: print("last frame:", " ".join("%02x" % b for b in buf["frame"]))
    return 0

raise SystemExit(asyncio.run(main()))
