#------------------------------------------ -*- tab-width: 8 -*-
OPENCM3_DIR	= libopencm3
DEVICE		= at32f403acgu
BINARY		= fw
OBJS		=

CFLAGS		+= -pipe -g -Os -flto
CFLAGS		+= -Wall -Wextra -Wshadow
CPPFLAGS	+= -MMD

include		$(OPENCM3_DIR)/mk/genlink-config.mk
include		$(OPENCM3_DIR)/mk/gcc-config.mk
include		mk/debug/config.mk
include		mk/lwip/config.mk
include		mk/bgrt/config.mk

LDFLAGS		+= --static -nostartfiles -Wl,--gc-sections -Wl,--no-warn-rwx-segments
LDLIBS		+= -Wl,--start-group -lc -lgcc -Wl,--end-group

ifneq ($(V),1)
Q := @
MAKEFLAGS += --no-print-directory
endif
#---------------------------------------------------------------
all:		lib $(BINARY).elf $(BINARY).bin $(BINARY)

$(BINARY):      $(BINARY).elf
		$(Q)install -p $< $(BINARY)

lib:
		$(Q)$(MAKE) -C $(OPENCM3_DIR) lib TARGETS=at32/f40x CFLAGS=-flto AR=$(CC)-ar

include		$(OPENCM3_DIR)/mk/genlink-rules.mk
include		$(OPENCM3_DIR)/mk/gcc-rules.mk
include		mk/debug/rules.mk
include		mk/lwip/rules.mk
include		mk/bgrt/rules.mk

-include	$(OBJS:.o=.d)

.PHONY:		all lib
