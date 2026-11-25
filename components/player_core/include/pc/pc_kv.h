// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* key=value settings files (/config/NAME.txt). One pair per line, # comments, blank lines ignored. */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct { char key[32]; char val[256]; } pc_kv_t;   /* val: the instrument's settings line reaches ~220 */

int  pc_kv_parse(const char *text, pc_kv_t *out, int max);    /* number of pairs; a repeated key overrides */
int  pc_kv_load(const char *path, pc_kv_t *out, int max);     /* missing file -> 0 */
int  pc_kv_save(const char *path, const pc_kv_t *kv, int n);  /* writes <path>.tmp then renames; 0 on success */
const char *pc_kv_get(const pc_kv_t *kv, int n, const char *key);
int  pc_kv_get_int(const pc_kv_t *kv, int n, const char *key, int def);
int  pc_kv_set(pc_kv_t *kv, int *n, int max, const char *key, const char *val);  /* -1 when full */
int  pc_kv_del(pc_kv_t *kv, int *n, const char *key);   /* 1 = removed (later pairs move up), 0 = not there */

#ifdef __cplusplus
}
#endif
