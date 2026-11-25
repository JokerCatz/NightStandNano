#ifndef LIBXMP_TEMPFILE_H
#define LIBXMP_TEMPFILE_H

/* JukeDial patch 0001: no temp files anywhere (ProWizard and the depackers work in memory), so there is nothing to unlink. */
static inline void unlink_temp_file(char *temp) { (void)temp; }

#endif
