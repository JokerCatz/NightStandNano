# AdPlug 2.4 (LGPL-2.1-or-later) + libbinio 1.5 (LGPL-2.1-or-later) for DOS AdLib music; emulator: fmopl 0.37a from AdPlug (YM3812, LGPL relicensed).
# Patched copies (patches/, SOURCE.md): no database / file-system provider / std::list player table, no Westwood ADL (GPL block in its header),
# no JBM (GPL), no aPlib (no licence: A2M v9-11 refused). The C API is adplug_c.h; its C++ `new` goes to PSRAM (adplug_c.cpp).
file(GLOB AP_CPP "${CMAKE_CURRENT_LIST_DIR}/adplug/*.cpp")
file(GLOB AP_C "${CMAKE_CURRENT_LIST_DIR}/adplug/*.c")
idf_component_register(SRCS ${AP_CPP} ${AP_C} "binio/binio.cpp" "binio/binstr.cpp" "adplug_c.cpp" "players_table.cpp"
                       INCLUDE_DIRS "." PRIV_INCLUDE_DIRS "adplug" "binio" LDFRAGMENTS "linker.lf")
# -msmall-data-limit=0: fmopl's and unlzh's zero-initialised statics (33 + 46 KB) go to PSRAM through linker.lf
target_compile_options(${COMPONENT_LIB} PRIVATE -w -Os -fwrapv -msmall-data-limit=0 -Dstricmp=strcasecmp -DNDEBUG)   # NDEBUG: the players assert on damaged input (the asserts that guarded an index are patched into clamps)
target_compile_options(${COMPONENT_LIB} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti>)
set_source_files_properties("adplug/fmopl.c" PROPERTIES COMPILE_OPTIONS "-O2")   # the one hot file

include(${CMAKE_CURRENT_LIST_DIR}/adplug_scope.cmake)
adplug_scope_new(${COMPONENT_LIB})   # no global operator new replacement: only AdPlug's allocations are tracked (adplug_c.cpp)
