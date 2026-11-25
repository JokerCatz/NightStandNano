/* Force-included into every libxmp unit (CMakeLists.txt): its heap goes to PSRAM. The system headers come
 * first, so the macros only rename calls, not the declarations. free() needs nothing: it takes any heap. */
#pragma once
#include <stdlib.h>
#include <string.h>
void *xmp_psram_malloc(size_t n);
void *xmp_psram_calloc(size_t n, size_t size);
void *xmp_psram_realloc(void *p, size_t n);
#define malloc(n) xmp_psram_malloc(n)
#define calloc(n, size) xmp_psram_calloc(n, size)
#define realloc(p, n) xmp_psram_realloc(p, n)
