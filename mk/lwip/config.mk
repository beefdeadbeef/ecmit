#------------------------------------------ -*- tab-width: 8 -*-
LWIPDIR		:= lwip/src
LWIPPORTDIR	:= mk/lwip

LWIPPORTFILES	= $(LWIPPORTDIR)/bgrt/sys_arch.c

include		$(LWIPDIR)/Filelists.mk

LWIPFILES	= $(LWIPNOAPPSFILES) $(LWIPPORTFILES)

LWIP_OBJS	= $(LWIPFILES:.c=.o)

LWIP_CPPFLAGS	= -I$(LWIPPORTDIR)/bgrt/include -I$(LWIPDIR)/include
LWIP_MODFLAGS	= $(LWIP_CPPFLAGS) $(OPENCM3_CPPFLAGS) $(BGRT_CPPFLAGS) $(PRINTF_CPPFLAGS)

$(call add-module,LWIP)
