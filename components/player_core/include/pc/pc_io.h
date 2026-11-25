// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include <stdint.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

/* The largest file a reader takes: 2 GiB - 1 (owner ruling 2026-11-13). The target's off_t is 32-bit signed, so no offset of
 * 2^31 or more may reach lseek (and exFAT would sign-extend one into a clamp at the end of the file); pc_reader_file refuses a bigger file. */
#define PC_FILE_MAX 2147483647u

/* Random-access byte source: a file on the TF card or a buffer in RAM. */
typedef struct pc_reader {
    int (*read)(void *u, uint32_t off, void *buf, uint32_t n);   /* bytes read (short only at EOF), <0 on error */
    uint32_t size;
    void *u;
    struct { const uint8_t *p; } mem;   /* storage for pc_reader_mem */
    char ext[8];                        /* the file name's extension with its dot, lower case ("" = unknown): pc_reader_set_ext; only the AdPlug backend reads it */
} pc_reader_t;

/* Sets size and read; 0 on success, -1 if f is NULL, unreadable, or larger than PC_FILE_MAX.
 * The reader is stateless: u is the FILE itself and every read is lseek(fileno(f)) + read on the descriptor (never stdio, so there is
 * no buffer to disagree with, and u is equally valid in every by-value copy of the reader). After this call nothing else may
 * fread/fseek/ftell that FILE; fclose is fine. */
int  pc_reader_file(pc_reader_t *r, FILE *f);
void pc_reader_set_ext(pc_reader_t *r, const char *name);   /* remembers the extension of a file name or path (at most 7 characters with the dot; longer = none) */
void pc_reader_mem(pc_reader_t *r, const void *data, uint32_t len);   /* r must outlive its use; data is not copied */

#ifdef __cplusplus
}
#endif
