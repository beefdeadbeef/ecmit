#------------------------------------------ -*- tab-width: 8 -*-
RTTDIR		= rtt
RTTOBJDIR	= mk/rtt

RTTOBJS		= \
		$(RTTDIR)/RTT/SEGGER_RTT.o \
		$(RTTDIR)/RTT/SEGGER_RTT_ASM_ARMv7M.o

$(RTTOBJS):	ASFLAGS	= -mcpu=cortex-m4 -mthumb
$(RTTOBJS):	CPPFLAGS += -I$(RTTDIR)/RTT -DRTT_USE_ASM=1

RTTLIBNAME	= rtt
RTTLIB		= $(RTTOBJDIR)/lib$(RTTLIBNAME).a
