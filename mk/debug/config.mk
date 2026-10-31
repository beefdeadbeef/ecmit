#------------------------------------------ -*- tab-width: 8 -*-

ifeq		($(DEBUG),)

CPPFLAGS	+= -DNDEBUG

else

DEBUG_OBJS	= debug.o
DEBUG_CPPFLAGS	= -DDEBUG
DEBUG_MODFLAGS	= $(DEBUG_CPPFLAGS)

ifeq		($(DEBUG),RTT)

DEBUG_MODFLAGS	+= -DRTT

RTTDIR		:= rtt/RTT
RTT_OBJS	= $(RTTDIR)/SEGGER_RTT.o
RTT_MODFLAGS	= -I$(RTTDIR) -I$(RTTDIR)/../Config \
			-DSEGGER_RTT_MAX_NUM_UP_BUFFERS=1 \
			-DSEGGER_RTT_MAX_NUM_DOWN_BUFFERS=1 \
			-DSEGGER_RTT_SECTION=\".data.rtt\"
$(call add-module,RTT)

else

DEBUG_OBJS	+= $(OPENCM3DIR)/tests/shared/trace.o
DEBUG_MODFLAGS	+= -I$(OPENCM3DIR)/include

endif

$(call add-module,DEBUG)

endif
