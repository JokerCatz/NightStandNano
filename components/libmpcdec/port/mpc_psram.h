/* Force-included into every libmpcdec unit (CMakeLists.txt): its heap goes to PSRAM (the decoder and demuxer are tens of KB; the seek
 * table and the small blocks would otherwise land in internal RAM). Host builds (tools/bench/chip_bench, host tests) use plain malloc. */
#pragma once
#include <stdlib.h>
#include <string.h>
#ifdef ESP_PLATFORM
void *mpc_psram_malloc(size_t n);
void *mpc_psram_calloc(size_t n, size_t size);
void *mpc_psram_realloc(void *p, size_t n);
#define malloc(n) mpc_psram_malloc(n)
#define calloc(n, size) mpc_psram_calloc(n, size)
#define realloc(p, n) mpc_psram_realloc(p, n)
#endif
