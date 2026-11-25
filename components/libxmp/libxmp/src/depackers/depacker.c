/* Extended Module Player
 * Copyright (C) 1996-2025 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <errno.h>

#include "../common.h"
#include "depacker.h"
#include "../hio.h"
#include "xfnmatch.h"

/* JukeDial patch 0001: only the in-memory Amiga-era depackers are built (no zip/lha/gzip/bzip2/xz/compress, no external
 * helper programs, no temp files). */

static const struct depacker *const depacker_list[] = {
	&libxmp_depacker_pp,
	&libxmp_depacker_sqsh,
	&libxmp_depacker_arc,
	&libxmp_depacker_arcfs,
	&libxmp_depacker_mmcmp,
	&libxmp_depacker_ice1,
	&libxmp_depacker_ice2,
	&libxmp_depacker_lzx,
	&libxmp_depacker_s404,
	NULL
};

static int decrunch_internal(HIO_HANDLE *h, const struct depacker *depacker)
{
	void *out;
	long outlen;

	D_(D_WARN "Depacking file... ");

	/* Depack file */
	D_(D_INFO "Internal depacker");
	if (depacker->depack(h, &out, &outlen) < 0) {
		D_(D_CRIT "failed");
		return -1;
	}

	D_(D_INFO "done");

	if (hio_reopen_mem(out, outlen, 1, h) < 0) {
		free(out);
		return -1;
	}
	return 0;
}

int libxmp_decrunch(HIO_HANDLE *h, const char *filename, char **temp)
{
	unsigned char b[1024];
	int headersize;
	int i;
	const struct depacker *depacker = NULL;

	(void)filename;
	*temp = NULL;

	headersize = hio_read(b, 1, 1024, h);
	if (headersize < 64) {	/* minimum valid file size */
		return 0;
	}

	/* Check built-in depackers */
	for (i = 0; depacker_list[i] != NULL; i++) {
		const struct depacker *d = depacker_list[i];
		if ((d->test && d->test(b)) || (d->test_hio && d->test_hio(h))) {
			depacker = d;
			D_(D_INFO "Use depacker %d", i);
			break;
		}
	}

	if (hio_seek(h, 0, SEEK_SET) < 0) {
		return -1;
	}

	if (depacker && depacker->depack) {
		return decrunch_internal(h, depacker);
	}
	D_(D_INFO "Not packed");
	return 0;
}

/*
 * Check whether the given string matches one of the blacklisted glob
 * patterns. Used to filter file names stored in archive files.
 */
int libxmp_exclude_match(const char *name)
{
	int i;

	static const char *const exclude[] = {
		"README", "readme", "ReadMe",
		"ReadMe!", "readMe!", "!ReadMe!",
		"*.DIZ", "*.diz",
		"*.NFO", "*.nfo",
		"*.DOC", "*.Doc", "*.doc",
		"*.INFO", "*.info", "*.Info",
		"*.TXT", "*.txt",
		"*.EXE", "*.exe",
		"*.COM", "*.com",
		"*.README", "*.readme", "*.Readme", "*.ReadMe",
		"*.HTM", "*.htm",
		"*.HTML", "*.html",
		/* Found in Spark archives. */
		"\\?From", "From\\?", "InfoText",
		NULL
	};

	for (i = 0; exclude[i] != NULL; i++) {
		if (fnmatch(exclude[i], name, 0) == 0) {
			return 1;
		}
	}

	return 0;
}
