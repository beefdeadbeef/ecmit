#------------------------------------------ -*- tab-width: 8 -*-
DEVICE		= at32f437zmt7
BINARY		= fw

OBJS		= main.o ecm.o

CFLAGS		= -pipe -g -Os -ffunction-sections -fdata-sections -flto=auto
CFLAGS		+= -Wall -Wextra -Wshadow
CPPFLAGS	= -MMD

LDFLAGS		= -static -nostartfiles -Wl,--gc-sections -Wl,--no-warn-rwx-segments
LDLIBS		= -Wl,--start-group -lc -lgcc -Wl,--end-group

ifneq ($(V),1)
Q := @
endif

define		add-module
$(eval $$(addprefix %/,$$($(1)_OBJS)):	CPPFLAGS = $$($(1)_MODFLAGS))
$(eval CPPFLAGS	+= $$($(1)_CPPFLAGS))
$(eval OBJS	+= $$($(1)_OBJS))
endef

include		mk/gcc/config.mk
include		mk/genlink/config.mk
include		mk/debug/config.mk
include		mk/printf/config.mk
include		mk/bgrt/config.mk
include		mk/lwip/config.mk
include		mk/objs/config.mk
include		mk/ccls/config.mk

#---------------------------------------------------------------
all:		genlink $(BINARY).elf $(BINARY).bin

include		mk/gcc/rules.mk
include		mk/genlink/rules.mk
include		mk/objs/rules.mk
include		mk/ccls/rules.mk

-include	$(OBJS:.o=.d)

.PHONY:		all genlink
