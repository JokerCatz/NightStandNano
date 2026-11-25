# AOSP external/libldac (Apache-2.0, see LICENSE / NOTICE - LDAC in a product needs Sony's certification).
# Only the two Android build units; the others are #included by them.
idf_component_register(SRCS "src/ldaclib.c" "src/ldacBT.c" INCLUDE_DIRS "inc" PRIV_INCLUDE_DIRS "src")
target_compile_options(${COMPONENT_LIB} PRIVATE -O2 -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-implicit-fallthrough -Wno-type-limits -Wno-error)
# fixed point by default (the player, measured faster on the S31); -DLDAC_FLOAT=1 builds the floating-point variant
if(NOT LDAC_FLOAT)
  target_compile_definitions(${COMPONENT_LIB} PUBLIC _32BIT_FIXED_POINT)   # PUBLIC: the bench prints which one it is
endif()
