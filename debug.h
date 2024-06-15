/*
 */

#ifdef DEBUG

#include <inttypes.h>
#include "printf/printf.h"
#include "trace.h"

#define trace(port, val)				\
	do {						\
		trace_send_blocking32(port, systicks);  \
		trace_send_blocking32(port, val);       \
	} while(0)

#define debugf(fmt, args...)					\
	do {							\
		printf("[%08lx] " fmt, systicks, ##args);	\
	} while (0)

#else

#define trace(port, val)
#define debugf(fmt, args...)

#endif
