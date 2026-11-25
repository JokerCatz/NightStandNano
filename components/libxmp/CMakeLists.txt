# libxmp 4.7.2 (MIT), the full library: every module loader, the ProWizard Amiga packer loaders and the in-memory Amiga depackers.
# Left out (patches/ and SOURCE.md say why): the Paula mixer, temp files, zip/lha/gzip/bzip2/xz/compress, external helper programs.
# port/xmp_psram.h is force-included so every libxmp malloc/calloc/realloc goes to PSRAM: module patterns
# are hundreds of allocations under 1 KB, which CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL would put in internal RAM.
# xmp_sources.cmake is the one list of source files.
include(${CMAKE_CURRENT_LIST_DIR}/xmp_sources.cmake)
idf_component_register(SRCS ${XMP_SRCS} "port/xmp_psram.c" INCLUDE_DIRS "libxmp/include" PRIV_REQUIRES heap)
target_compile_definitions(${COMPONENT_LIB} PUBLIC LIBXMP_STATIC PRIVATE ${XMP_DEFS})
target_compile_options(${COMPONENT_LIB} PRIVATE -w -include ${CMAKE_CURRENT_LIST_DIR}/port/xmp_psram.h)
