#!/bin/sh
# builds the fmvgm host runner; YMFM_DIR=<dir> builds against another copy of ymfm (the bit-exactness check against an unpatched one); OUT=<dir> and the name suffix below keep the builds apart
# builds the fmvgm host runner (plain g++, ymfm at -O2) into $OUT (default /tmp/nsn_fm): $OUT/test_fmvgm (and test_fmvgm_asan with ASan + UBSan)
set -e
here=$(cd "$(dirname "$0")" && pwd); root=$(dirname "$(dirname "$here")")
C=$root/components/fmvgm; PC=$root/components/player_core
OUT=${OUT:-/tmp/nsn_fm}; mkdir -p "$OUT"
YD=${YMFM_DIR:-$C/ymfm}
INC="-I$C/include -I$YD -I$C/src -I$PC/include -I$root/app/main"
YD=${YMFM_DIR:-$C/ymfm}
Y="$YD/ymfm_opm.cpp $YD/ymfm_opn.cpp $YD/ymfm_adpcm.cpp $YD/ymfm_ssg.cpp"
S="$C/src/fmvgm_core.cpp $C/src/fmvgm_src.cpp"
CS="$C/src/fmvgm_resamp.c $PC/src/pc_vgmhdr.c $PC/src/pc_io.c $root/app/main/vgmgate.c"
build() { # name, extra flags
  g++ -std=c++14 -O2 -DNDEBUG -ffp-contract=off $2 $INC -c $Y $S -w 2>&1 | head -40
  gcc -std=gnu99 -O2 -ffp-contract=off $2 $INC -c $CS -w
  g++ -std=c++14 -O2 -ffp-contract=off $2 $INC -c "$here/test_fmvgm.cpp" -w
  g++ $2 -o "$OUT/$1" *.o -lz -lpthread
  rm -f *.o
}
cd "$OUT"
build test_fmvgm${SUFFIX:-} ""
[ "$1" = "asan" ] && build test_fmvgm_asan${SUFFIX:-} "-fsanitize=address,undefined -g -fno-omit-frame-pointer"
echo built $OUT
