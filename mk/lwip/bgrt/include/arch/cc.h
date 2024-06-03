#ifndef LWIP_ARCH_CC_H
#define LWIP_ARCH_CC_H

#ifndef LWIP_DEBUG

#define LWIP_NOASSERT	1

#else

unsigned long sys_now(void);
int printf_(const char* format, ...);

#define LWIP_PLATFORM_printf(fmt, args...)				\
	do {								\
		printf_("[%08lx] " fmt, sys_now(), ##args);		\
	} while (0)

#define LWIP_PLATFORM_DIAG(x)			\
	do {					\
		LWIP_PLATFORM_printf x;		\
	} while(0)

#define LWIP_PLATFORM_ASSERT(x)						\
	do {								\
		__asm__ __volatile__("cpsid i" ::: "memory");		\
		LWIP_PLATFORM_printf("Assertion \"%s\" failed"		\
				     " at line %d in %s\n",		\
				     x, __LINE__, __FILE__);		\
		__asm__ __volatile__("bkpt #0");			\
	} while(0)

#endif // LWIP_DEBUG

#define LWIP_PROVIDE_ERRNO	1

#endif // LWIP_ARCH_CC_H
