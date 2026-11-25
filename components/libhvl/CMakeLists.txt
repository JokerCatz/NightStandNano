# HivelyTracker replayer (AHX / HVL), BSD-3, see SOURCE.md. Patches: patches/ (RISC-V char, tables on demand, mute mask, hardening).
idf_component_register(SRCS "hvl/hvl_replay.c" "hvl/hvl_tables.c" INCLUDE_DIRS "hvl" LDFRAGMENTS "linker.lf")
target_compile_options(${COMPONENT_LIB} PRIVATE -w -O2 -fwrapv -msmall-data-limit=0)   # the filter code and the mixer shift negative values and rely on wrap-around (GCC defines both)
