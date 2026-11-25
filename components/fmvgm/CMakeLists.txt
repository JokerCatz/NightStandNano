# SPDX-License-Identifier: LGPL-2.1-or-later
# fmvgm: FM-chip music (YM2151, YM2203) from VGM / S98 on ymfm (BSD-3-Clause). Depends on player_core only for its headers (pc_backend.h, pc_io.h, pc_vgmhdr.h) and pc_vgmhdr.c.
# The platform glue (memory placement, a worker thread, the clock) is src/fmvgm_port_idf.c; everything else is plain C / C++ and builds on a PC (tools/hosttest/run_fm.sh).
set(FM_YMFM ymfm/ymfm_opm.cpp ymfm/ymfm_opn.cpp ymfm/ymfm_adpcm.cpp ymfm/ymfm_ssg.cpp)
set(FM_SRCS src/fmvgm_core.cpp src/fmvgm_src.cpp src/fmvgm_resamp.c src/fmvgm_port_idf.c)
idf_component_register(SRCS ${FM_SRCS} ${FM_YMFM} INCLUDE_DIRS "include" PRIV_INCLUDE_DIRS "ymfm" "src" REQUIRES player_core PRIV_REQUIRES esp_timer esp_psram heap freertos)
# -O2 whatever the project default is: ymfm measured much slower at -Os (the study: +71 % on x86; the project builds with -O2 too, this keeps it so if that changes).
# -w: GCC 15 warns inside ymfm_fm.ipp (OPL path, not compiled in use); -ffp-contract=off: the resampler's table must be the same bytes on every platform.
target_compile_options(${COMPONENT_LIB} PRIVATE -O2 -w -DNDEBUG -ffp-contract=off)
set_source_files_properties(ymfm/ymfm_opm.cpp ymfm/ymfm_opn.cpp ymfm/ymfm_adpcm.cpp ymfm/ymfm_ssg.cpp src/fmvgm_core.cpp PROPERTIES COMPILE_OPTIONS "-O2")
