#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-or-later
"""Temporary A2DP SBC sink on this PC's BlueZ, for testing the NightStand Nano source without headphones.
Registers a MediaEndpoint1 (SBC sink) for the lifetime of the script, accepts whatever the remote configures, counts the bytes
that arrive on the transport fd and prints a line per second. Nothing is played and nothing is changed permanently.
  python3 tools/a2dp_sink_probe.py [seconds]
"""
import sys, os, time, struct
import dbus, dbus.service, dbus.mainloop.glib
from gi.repository import GLib

SBC_UUID = "0000110b-0000-1000-8000-00805f9b34fb"      # A2DP sink
EP_PATH = "/test/nsn_sink"
# SBC capabilities: 16/32/44.1/48 kHz, all channel modes, 4/8/12/16 blocks, 4/8 subbands, loudness+SNR, bitpool 2..53
CAPS = dbus.Array([dbus.Byte(0xFF), dbus.Byte(0xFF), dbus.Byte(2), dbus.Byte(53)], signature="y")
stats = {"bytes": 0, "pkts": 0, "t0": None}


class Endpoint(dbus.service.Object):
    @dbus.service.method("org.bluez.MediaEndpoint1", in_signature="a{sv}", out_signature="ay")
    def SelectConfiguration(self, caps):
        print("SelectConfiguration", flush=True)
        return dbus.Array([dbus.Byte(0x21), dbus.Byte(0x15), dbus.Byte(2), dbus.Byte(53)], signature="y")   # 44.1k, joint stereo, 16 blocks, 8 sub, loudness

    @dbus.service.method("org.bluez.MediaEndpoint1", in_signature="oa{sv}", out_signature="")
    def SetConfiguration(self, transport, props):
        print("SetConfiguration", transport, flush=True)
        bus.get_object("org.bluez", transport)
        self.transport = transport

    @dbus.service.method("org.bluez.MediaEndpoint1", in_signature="o", out_signature="")
    def ClearConfiguration(self, transport):
        print("ClearConfiguration", flush=True)

    @dbus.service.method("org.bluez.MediaEndpoint1", in_signature="", out_signature="")
    def Release(self):
        print("Release", flush=True)


def on_props(iface, changed, inval, path=None):
    if iface != "org.bluez.MediaTransport1": return
    st = changed.get("State")
    if st is not None:
        print("transport state:", st, flush=True)
        if st == "pending":                    # remote started streaming: acquire the fd so BlueZ reads it
            try:
                tr = dbus.Interface(bus.get_object("org.bluez", path), "org.bluez.MediaTransport1")
                fd, rmtu, wmtu = tr.TryAcquire()
                fd = fd.take()
                print("acquired fd, read mtu", rmtu, flush=True)
                GLib.io_add_watch(fd, GLib.IO_IN, lambda f, c: read(f))
            except Exception as e:
                print("acquire failed", e, flush=True)


def read(fd):
    try:
        d = os.read(fd, 4096)
    except OSError:
        return False
    if not d: return False
    if stats["t0"] is None: stats["t0"] = time.time()
    stats["bytes"] += len(d); stats["pkts"] += 1
    return True


def tick():
    print("rx pkts=%d bytes=%d" % (stats["pkts"], stats["bytes"]), flush=True)
    return True


dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
bus = dbus.SystemBus()
ep = Endpoint(bus, EP_PATH)
media = dbus.Interface(bus.get_object("org.bluez", "/org/bluez/hci0"), "org.bluez.Media1")
media.RegisterEndpoint(EP_PATH, {"UUID": SBC_UUID, "Codec": dbus.Byte(0), "Capabilities": CAPS})
bus.add_signal_receiver(on_props, "PropertiesChanged", "org.freedesktop.DBus.Properties", "org.bluez", path_keyword="path")
print("endpoint registered", flush=True)
loop = GLib.MainLoop()
GLib.timeout_add_seconds(1, tick)
GLib.timeout_add_seconds(int(sys.argv[1]) if len(sys.argv) > 1 else 60, loop.quit)
try:
    loop.run()
finally:
    try: media.UnregisterEndpoint(EP_PATH)
    except Exception: pass
    print("done: pkts=%d bytes=%d" % (stats["pkts"], stats["bytes"]), flush=True)
