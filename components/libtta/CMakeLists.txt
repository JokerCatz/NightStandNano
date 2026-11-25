# libtta 2.3 (True Audio decoder), LGPL, from libtta-c-2.3 (see SOURCE.md). Decoder only (TTA_NO_ENCODER), portable C filter.
# Patches: patches/ (global state to PSRAM, a 24-bit store without a misaligned access, a one-decoder claim).
idf_component_register(SRCS "libtta/libtta.c" INCLUDE_DIRS "libtta" PRIV_INCLUDE_DIRS "port" PRIV_REQUIRES heap LDFRAGMENTS "linker.lf")
target_compile_definitions(${COMPONENT_LIB} PRIVATE TTA_NO_ENCODER)
target_compile_options(${COMPONENT_LIB} PRIVATE -w -O2 -fwrapv -msmall-data-limit=0)   # the adaptive filter relies on wrap-around of signed sums
