#------------------------------------------ -*- tab-width: 8 -*-
LWIPDIR		= lwip/src
LWIPOBJDIR	= mk/lwip

LWIPPORTFILES	= $(LWIPOBJDIR)/bgrt/sys_arch.c

include		$(LWIPDIR)/Filelists.mk

LWIPFILES	= $(LWIPNOAPPSFILES) $(LWIPPORTFILES)
LWIPOBJS	= $(LWIPFILES:.c=.o)

CPPFLAGS	+= -I$(LWIPOBJDIR)/bgrt/include -I$(LWIPDIR)/include

LWIPLIBNAME	= lwipcommon
LWIPLIB		= $(LWIPOBJDIR)/lib$(LWIPLIBNAME).a

LDLIBS		+= -l$(LWIPLIBNAME)
LIBDEPS		+= $(LWIPLIB)
LDFLAGS		+= -L$(LWIPOBJDIR)
