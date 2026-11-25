# SPDX-License-Identifier: LGPL-2.1-or-later
"""Which serial port is the board? Shared by the helper scripts in this folder.

Order: $NSN_PORT, then the first line of <repo>/.local/nsn_port (git-ignored, for a workstation with several boards), then the only
serial device under /dev/serial/by-id. With none or several, stop with a message: the helpers never guess between boards."""
import glob, os, sys

def find_port():
    p = os.environ.get("NSN_PORT")
    if p: return p
    f = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".local", "nsn_port")
    try:
        with open(f) as fh:
            p = fh.readline().strip()
        if p: return p
    except OSError:
        pass
    found = sorted(glob.glob("/dev/serial/by-id/*"))
    if len(found) == 1: return found[0]
    sys.exit("nsn_port: set NSN_PORT to the board's serial device, e.g. NSN_PORT=/dev/ttyUSB0 "
             "(found %d device(s) under /dev/serial/by-id; a helper never picks one of several)" % len(found))
