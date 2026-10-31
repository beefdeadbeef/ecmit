/*  -*- mode: c; tab-width: 8 -*-
 */

#include "debug.h"

#ifdef DEBUG

#if defined(RTT)
extern unsigned SEGGER_RTT_Write(unsigned, const void *, unsigned);
void _putchar(char c)
{
	char cc = c;
	SEGGER_RTT_Write(0, &cc, 1);
}
#else
void _putchar(char c)
{
	if (c == '\n') trace_send8(0, '\r');
	trace_send_blocking8(0, c);
}
#endif

#endif
