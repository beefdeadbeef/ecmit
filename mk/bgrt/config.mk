#------------------------------------------ -*- tab-width: 8 -*-
BGRT_DIR	= bgrt
BGRT_ARCH	= cm4f
BGRT_OBJDIR	= mk/bgrt

BGRT_KERNEL_SOURCES	=		\
	$(BGRT_DIR)/kernel/crit_sec.c	\
	$(BGRT_DIR)/kernel/index.c	\
	$(BGRT_DIR)/kernel/item.c	\
	$(BGRT_DIR)/kernel/kernel.c	\
	$(BGRT_DIR)/kernel/pcounter.c	\
	$(BGRT_DIR)/kernel/pitem.c	\
	$(BGRT_DIR)/kernel/proc.c	\
	$(BGRT_DIR)/kernel/sched.c	\
	$(BGRT_DIR)/kernel/sync.c	\
	$(BGRT_DIR)/kernel/syscall.c	\
	$(BGRT_DIR)/kernel/timer.c	\
	$(BGRT_DIR)/kernel/vint.c	\
	$(BGRT_DIR)/kernel/xlist.c

BGRT_KERNEL_SOURCES	+=		\
	$(BGRT_DIR)/arch/$(BGRT_ARCH)/gcc/bugurt_port.c

BGRT_KERNEL_INCLUDES	=		\
	$(BGRT_OBJDIR)  		\
	$(BGRT_DIR)/kernel		\
	$(BGRT_DIR)/kernel/default	\
	$(BGRT_DIR)/arch/$(BGRT_ARCH)/gcc

BGRT_NATIVE_API_SOURCES =		\
	$(BGRT_DIR)/libs/native/cond.c	\
	$(BGRT_DIR)/libs/native/ipc.c	\
	$(BGRT_DIR)/libs/native/mutex.c \
	$(BGRT_DIR)/libs/native/sem.c	\
	$(BGRT_DIR)/libs/native/queue.c

BGRT_NATIVE_API_INCLUDES =		\
	$(BGRT_DIR)/libs/native

BGRT_TEST_SOURCES	=		\
	$(BGRT_DIR)/tests/arch/$(BGRT_ARCH)/gcc/test_func.c

ifneq	($(TEST),)
BGRT_TEST_SOURCES	+= $(BGRT_DIR)/tests/main/$(TEST)/main.c
endif

BGRT_TEST_INCLUDES	=		\
	$(BGRT_DIR)/tests/arch/$(BGRT_ARCH)/gcc .

CPPFLAGS	+=	$(addprefix -I,$(BGRT_KERNEL_INCLUDES))	\
			$(addprefix -I,$(BGRT_NATIVE_API_INCLUDES))

ifneq		($(TEST),)
CPPFLAGS	+=	$(addprefix -I,$(BGRT_TEST_INCLUDES))
OBJS		=	$(subst .c,.o,$(BGRT_TEST_SOURCES))
endif

BGRT_OBJS	=	$(subst .c,.o,$(BGRT_KERNEL_SOURCES))		\
			$(subst .c,.o,$(BGRT_NATIVE_API_SOURCES))	\
			$(subst .c,.o,$(BGRT_EXTRA_SOURCES))

BGRT_LIBNAME	= bgrt
BGRT_LIB	= $(BGRT_OBJDIR)/lib$(BGRT_LIBNAME).a

LDLIBS		+= -l$(BGRT_LIBNAME)
LIBDEPS		+= $(BGRT_LIB)
LDFLAGS		+= -L$(BGRT_OBJDIR)
