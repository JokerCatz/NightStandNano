// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once
#include "EmuStructs.h"
/* logging is off: the cores' emu_logf calls compile to nothing */
#define emu_logf(logger, level, ...) ((void)0)
#define dev_logger_set(l, a, f, p) ((void)0)
