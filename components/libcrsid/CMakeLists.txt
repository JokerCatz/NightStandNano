# cRSID 1.58 (Hermit; "WTF" licence: do what you want, credit the author), patched per-instance by the owner (see SOURCE.md).
# One translation unit: ccs_player.c includes the whole library (cRSID is a unity build). The machine (70 KB) is one calloc: PSRAM.
# Oversampling ratio 2 instead of cRSID's 5: the host bench (SOURCE.md) puts the default at 40-50 % of one core on the board before the
# 20-point pipeline base; ratio 2 is ~25-30 %. Compiled -O2 like libhvl (the cycle-stepped 6502 + SID loop is the hot path).
idf_component_register(SRCS "ccs_player.c" INCLUDE_DIRS ".")
target_compile_definitions(${COMPONENT_LIB} PRIVATE CRSID_OVERSAMPLING_RATIO=2)
target_compile_options(${COMPONENT_LIB} PRIVATE -w -O2 -fwrapv)   # the dither LFSR shifts a signed int past its top bit (GCC wraps)
