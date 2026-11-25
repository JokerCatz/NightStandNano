# libmpcdec (Musepack SV7 / SV8 decoder), BSD-3-Clause, from musepack_src_r475 (see SOURCE.md). Float build (MPC_FIXED_POINT off), no stdio reader
# (our backend feeds it from a pc_reader_t). Patches: patches/ (Huffman tables into one PSRAM block, CRC without a static table).
set(MPC_SRCS huffman.c mpc_bits_reader.c mpc_decoder.c mpc_demux.c requant.c streaminfo.c synth_filter.c)
list(TRANSFORM MPC_SRCS PREPEND ${CMAKE_CURRENT_LIST_DIR}/libmpcdec/)
idf_component_register(SRCS ${MPC_SRCS} "common/crc32.c" "port/mpc_psram.c" INCLUDE_DIRS "include" PRIV_INCLUDE_DIRS "libmpcdec" "port" PRIV_REQUIRES heap
                       LDFRAGMENTS "linker.lf")
target_compile_options(${COMPONENT_LIB} PRIVATE -w -O2 -fcommon -msmall-data-limit=0 -include ${CMAKE_CURRENT_LIST_DIR}/port/mpc_psram.h)
