/* PSRAM only, no fallback to internal RAM: a module too big for PSRAM fails to load (the library skips it)
 * instead of eating the 512 KB the radio, Bluetooth and the display need. */
#include "esp_heap_caps.h"
#include "xmp_psram.h"

void *xmp_psram_malloc(size_t n) { return heap_caps_malloc(n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); }
void *xmp_psram_calloc(size_t n, size_t size) { return heap_caps_calloc(n, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); }
void *xmp_psram_realloc(void *p, size_t n) { return heap_caps_realloc(p, n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); }
