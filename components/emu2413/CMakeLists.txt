# emu2413 (YM2413 / VRC7 FM emulator, MIT), the owner's fork, branch nodouble: see SOURCE.md.
# EMU2413_CONST_TABLES: the sine / tll / rks / default-patch tables are const data in flash (emu2413_tables.h), not RAM filled at start-up.
idf_component_register(SRCS emu2413.c INCLUDE_DIRS .)
target_compile_definitions(${COMPONENT_LIB} PUBLIC EMU2413_CONST_TABLES)
target_compile_options(${COMPONENT_LIB} PRIVATE -w)
