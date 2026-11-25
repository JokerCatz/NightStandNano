// SPDX-License-Identifier: LGPL-2.1-or-later
/* miniz 3.0.2 tinfl, compiled here with its symbols renamed so it never clashes with the ROM copy. */
#define MINIZ_NO_STDIO
#define MINIZ_NO_TIME
#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_ZLIB_APIS
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#define MINIZ_NO_DEFLATE_APIS
#define tinfl_decompress pc_tinfl_decompress
#define tinfl_decompress_mem_to_heap pc_tinfl_decompress_mem_to_heap
#define tinfl_decompress_mem_to_mem pc_tinfl_decompress_mem_to_mem
#define tinfl_decompress_mem_to_callback pc_tinfl_decompress_mem_to_callback
#define tinfl_decompressor_alloc pc_tinfl_decompressor_alloc
#define tinfl_decompressor_free pc_tinfl_decompressor_free
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "miniz_tinfl.c"
#pragma GCC diagnostic pop

#include "pc_inflate.h"

long pc_inflate_raw(const void *in, size_t in_len, void *out, size_t out_len)
{
    size_t n = tinfl_decompress_mem_to_mem(out, out_len, in, in_len, 0);
    return n == TINFL_DECOMPRESS_MEM_TO_MEM_FAILED ? -1 : (long)n;
}

size_t pc_inflate_state_size(void) { return sizeof(tinfl_decompressor); }

long pc_inflate_raw_state(const void *in, size_t in_len, void *out, size_t out_len, void *state)
{
    tinfl_decompressor *d = state;
    tinfl_init(d);
    size_t in_n = in_len, out_n = out_len;
    tinfl_status st = tinfl_decompress(d, (const mz_uint8 *)in, &in_n, (mz_uint8 *)out, (mz_uint8 *)out, &out_n,
                                       TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
    return st == TINFL_STATUS_DONE ? (long)out_n : -1;
}
