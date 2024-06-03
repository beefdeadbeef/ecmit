#------------------------------------------ -*- tab-width: 8 -*-
ifeq		($(DEBUG),1)

include		mk/rtt/config.mk

VPATH		+= $(OPENCM3_DIR)/tests/shared
VPATH		+= printf
CPPFLAGS	+= -DDEBUG -DBGRT_DEBUG -DLWIP_DEBUG
CPPFLAGS	+= -I$(OPENCM3_DIR)/tests/shared
OBJS		+= trace.o printf.o putchar.o

printf.o:	CPPFLAGS += \
			-DPRINTF_DISABLE_SUPPORT_FLOAT \
			-DPRINTF_DISABLE_SUPPORT_EXPONENTIAL \
			-DPRINTF_DISABLE_SUPPORT_LONG_LONG \
			-DPRINTF_DISABLE_SUPPORT_PTRDIFF_T

ifeq		($(SEMIHOSTING),1)
CPPFLAGS	+= -DSEMIHOSTING
LDFLAGS		+= --specs=rdimon.specs
LDLIBS		+= -lrdimon
endif

ifeq		($(RTT),1)
CPPFLAGS	+= -DRTT
LDLIBS		+= -lrtt
LIBDEPS		+= $(RTTLIB)
LDFLAGS		+= -L$(RTTOBJDIR) #-Wl,-section-start=.rtt=0x20000000
endif

endif
