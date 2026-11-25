#!/bin/sh
# builds the fmvgm host runner; YMFM_DIR=<dir> builds against another copy of ymfm (the bit-exactness check against an unpatched one); OUT=<dir> and the name suffix below keep the builds apart
# builds the fmvgm host runner (plain g++, ymfm at -O2) into $OUT (default /tmp/nsn_fm): $OUT/test_fmvgm (and test_fmvgm_asan with ASan + UBSan)
set -e
here=$(cd "$(dirname "$0")" && pwd); root=$(dirname "$(dirname "$here")")
C=$root/components/fmvgm; PC=$root/components/player_core
OUT=${OUT:-/tmp/nsn_fm}; mkdir -p "$OUT"
YD=${YMFM_DIR:-$C/ymfm}
INC="-I$C/include -I$YD -I$C/src -I$C/pcm -I$PC/include -I$root/app/main"
YD=${YMFM_DIR:-$C/ymfm}
Y="$YD/ymfm_opm.cpp $YD/ymfm_opn.cpp $YD/ymfm_adpcm.cpp $YD/ymfm_ssg.cpp $YD/ymfm_misc.cpp"
S="$C/src/fmvgm_core.cpp $C/src/fmvgm_chips.cpp $C/src/fmvgm_src.cpp"
CS="$C/pcm/fmvgm_pcm_sn76496.c $C/pcm/fmvgm_pcm_okim6295.c $C/src/fmvgm_resamp.c $PC/src/pc_vgmhdr.c $PC/src/pc_io.c $root/app/main/vgmgate.c"
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
[ "$1" = "tsan" ] && build test_fmvgm_tsan${SUFFIX:-} "-fsanitize=thread -g -fno-omit-frame-pointer"
REFY=${REF_YMFM_DIR:-$YD}
g++ -std=c++14 -O2 -DNDEBUG -I"$REFY" -I$C/pcm -I$C/include -c "$REFY/ymfm_opm.cpp" "$REFY/ymfm_opn.cpp" "$REFY/ymfm_adpcm.cpp" "$REFY/ymfm_ssg.cpp" "$REFY/ymfm_misc.cpp" -w
gcc -std=gnu99 -O2 -I$C/pcm -I$C/include -c $C/pcm/fmvgm_pcm_sn76496.c $C/pcm/fmvgm_pcm_okim6295.c -w
g++ -std=c++14 -O2 -I"$REFY" -I$C/pcm -I$C/include -c "$here/refrun.cpp" -w
g++ -o "$OUT/refrun" *.o -lz
rm -f *.o
echo built $OUT
