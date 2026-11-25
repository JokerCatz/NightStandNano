#!/bin/sh
# builds and runs the VGM inspector host test; generates the synthetic test VGMs into $1 (default /tmp/nsn_vgm_cases)
set -e
here=$(cd "$(dirname "$0")" && pwd); root=$(dirname "$(dirname "$here")")
gcc -std=c99 -Wall -Wextra -O1 -fsanitize=address,undefined -I"$root/components/player_core/include" -o "${TMPDIR:-/tmp}/test_pc_vgmhdr" "$here/test_pc_vgmhdr.c" "$root/components/player_core/src/pc_vgmhdr.c"
"${TMPDIR:-/tmp}/test_pc_vgmhdr"
python3 -I "$here/gen_vgm_cases.py" "${1:-/tmp/nsn_vgm_cases}"
