#!/usr/bin/env bash
# SPDX-License-Identifier: LGPL-2.1-or-later
# Test board helper. The port is ALWAYS explicit: $NSN_PORT, else the first line of .local/nsn_port, else the only device under
# /dev/serial/by-id; with several boards attached it stops and asks (see nsn_port.py).
#   tools/board.sh flash          build output -> the test board
#   tools/board.sh log [SECONDS] [LINE...]   read the console (optionally send lines first)
set -euo pipefail
here=$(cd "$(dirname "$0")" && pwd); root=$(dirname "$here")
PORT=${NSN_PORT:-}
[ -n "$PORT" ] || PORT=$(python3 -c "import sys; sys.path.insert(0, '$here'); import nsn_port; print(nsn_port.find_port())") || exit 1
[ -e "$PORT" ] || { echo "board.sh: $PORT missing" >&2; exit 1; }
PY=$(ls -d "$HOME"/.espressif/python_env/*/bin/python | head -1)
case "${1:-}" in
flash) PORT="$PORT" ROOT="$root" bash -c 'source "$HOME/.espressif/tools/activate_idf_v6.1.sh" >/dev/null 2>&1; cd "$ROOT/app" && idf.py --preview -p "$PORT" flash 2>&1 | grep -E "Hash of data|Done|rror"' || true ;;
log) shift; secs=${1:-8}; [ $# -gt 0 ] && shift; "$PY" - "$PORT" "$secs" "$@" <<'P'
import sys, time, serial
port, secs, lines = sys.argv[1], float(sys.argv[2]), sys.argv[3:]
s = serial.Serial(); s.port = port; s.baudrate = 115200; s.timeout = 0.2
s.dtr = False; s.rts = False; s.open()
if "NSN_NORESET" not in __import__("os").environ:      # reset the board so the log starts at boot and typed lines meet a fresh console
    for d, r in ((False, True), (False, False)): s.dtr = d; s.rts = r; time.sleep(0.15)
# the board was just reset: wait for it to boot before typing; a line "sleep N" waits N seconds
buf = b""
def pump(t):
    global buf
    end = time.time() + t
    while time.time() < end: buf += s.read(4096)
if lines and "NSN_NORESET" not in __import__("os").environ: pump(5)
for l in lines:
    if l.startswith("sleep "): pump(float(l[6:])); continue
    s.write((l + "\n").encode()); pump(0.6)
pump(secs)
sys.stdout.write(buf.decode(errors="replace"))
P
;;
*) echo "usage: board.sh flash | log [secs] [lines...]" >&2; exit 2 ;;
esac
