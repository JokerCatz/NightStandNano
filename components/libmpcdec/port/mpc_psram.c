/* PSRAM only, no fallback to internal RAM: a file that does not fit fails to open instead of eating the RAM the radio and Bluetooth need. */
#include "esp_heap_caps.h"
#include "mpc_psram.h"
#undef malloc
#undef calloc
#undef realloc
void *mpc_psram_malloc(size_t n) { return heap_caps_malloc(n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); }
void *mpc_psram_calloc(size_t n, size_t size) { return heap_caps_calloc(n, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); }
void *mpc_psram_realloc(void *p, size_t n) { return heap_caps_realloc(p, n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); }
