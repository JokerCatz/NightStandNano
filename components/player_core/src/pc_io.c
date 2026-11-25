// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_io.h"
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* File size from fstat as an unsigned 32-bit value. On the target off_t is 32-bit signed and a size of 2 GiB or more arrives as a
 * negative number: it is simply "too big". Above PC_FILE_MAX: refused (-1). */
static int size_from_stat(long long st_size, uint32_t *out)
{
    if (st_size < 0 || st_size > (long long)PC_FILE_MAX) return -1;
    *out = (uint32_t)st_size;
    return 0;
}

static int file_read(void *u, uint32_t off, void *buf, uint32_t n)
{
    int fd = fileno((FILE *)u);
    if (n == 0 || off > PC_FILE_MAX) return 0;       /* no file this reader opened reaches there; off stays below 2^31 for lseek */
    if (lseek(fd, (off_t)off, SEEK_SET) != (off_t)off) return -1;   /* past the end FatFs clamps, the read then returns 0 */
    if (n > 0x7FFFFFFFu) n = 0x7FFFFFFFu;
    uint32_t got = 0;
    while (got < n) {
        ssize_t k = read(fd, (uint8_t *)buf + got, n - got);
        if (k < 0) return -1;                        /* an error is an error, never a short count */
        if (k == 0) break;                           /* end of file */
        got += (uint32_t)k;
    }
    return (int)got;
}

int pc_reader_file(pc_reader_t *r, FILE *f)
{
    struct stat st;
    uint32_t size;
    if (!f || fstat(fileno(f), &st) || size_from_stat(st.st_size, &size)) return -1;
    memset(r, 0, sizeof *r);
    r->read = file_read;
    r->size = size;
    r->u = f;
    return 0;
}

static int mem_read(void *u, uint32_t off, void *buf, uint32_t n)
{
    pc_reader_t *r = u;
    if (off >= r->size) return 0;
    if (n > r->size - off) n = r->size - off;
    memcpy(buf, r->mem.p + off, n);
    return (int)n;
}

void pc_reader_mem(pc_reader_t *r, const void *data, uint32_t len)
{
    r->read = mem_read;
    r->size = len;
    r->u = r;
    r->mem.p = data;
    r->ext[0] = 0;
}

void pc_reader_set_ext(pc_reader_t *r, const char *name)
{
    r->ext[0] = 0;
    const char *dot = name ? strrchr(name, '.') : NULL, *slash = name ? strrchr(name, '/') : NULL;
    if (!dot || (slash && slash > dot) || strlen(dot) > 7) return;
    for (int i = 0; dot[i]; i++) r->ext[i] = (char)((dot[i] >= 'A' && dot[i] <= 'Z') ? dot[i] + 32 : dot[i]), r->ext[i + 1] = 0;
}
