#if defined(SEMIHOSTING)
extern int _write(int, char *, int);
extern void initialise_monitor_handles(void);
static void __attribute__((constructor)) init_semi(void)
{
	initialise_monitor_handles();
}
void _putchar(char c)
{
	char cc = c;
	_write(1, &cc, 1);
}
#elif defined(RTT)
extern unsigned SEGGER_RTT_Write(unsigned, const void *, unsigned);
void _putchar(char c)
{
	char cc = c;
	SEGGER_RTT_Write(0, &cc, 1);
}
#else
#include "trace.h"
void _putchar(char c)
{
	if (c == '\n') trace_send8(0, '\r');
	trace_send_blocking8(0, c);
}
#endif
