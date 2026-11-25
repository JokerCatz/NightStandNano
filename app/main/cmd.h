// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
/* One command interpreter for every control channel (UART console, BLE control characteristic). Text in, text out. */
typedef void (*nsn_out_t)(void *ctx, const char *text);
void nsn_cmd_exec(const char *line, nsn_out_t out, void *ctx);
/* same, for the wired console only: also accepts commands that write files (putx) */
void nsn_cmd_exec_local(const char *line, nsn_out_t out, void *ctx);
