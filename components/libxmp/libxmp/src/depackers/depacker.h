#ifndef LIBXMP_DEPACKER_H
#define LIBXMP_DEPACKER_H

#include "../common.h"
#include "../hio.h"

LIBXMP_BEGIN_DECLS

extern const struct depacker libxmp_depacker_pp;
extern const struct depacker libxmp_depacker_sqsh;
extern const struct depacker libxmp_depacker_arc;
extern const struct depacker libxmp_depacker_arcfs;
extern const struct depacker libxmp_depacker_mmcmp;
extern const struct depacker libxmp_depacker_ice1;
extern const struct depacker libxmp_depacker_ice2;
extern const struct depacker libxmp_depacker_lzx;
extern const struct depacker libxmp_depacker_s404;

struct depacker {
	int (*test)(unsigned char *);
	int (*test_hio)(HIO_HANDLE *);
	int (*depack)(HIO_HANDLE *, void **, long *);
};

int	libxmp_decrunch		(HIO_HANDLE *h, const char *filename, char **temp);
int	libxmp_exclude_match	(const char *);

LIBXMP_END_DECLS

#endif /* LIBXMP_DEPACKER_H */
