/*  -*- mode: c; tab-width: 8 -*-
 */

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <bugurt.h>
#include <native.h>

#define systicks bgrt_kernel.timer.val

#include "debug.h"

/*
 *
 */
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
