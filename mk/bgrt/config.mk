#------------------------------------------ -*- tab-width: 8 -*-
BGRT_DIR	:= bgrt
BGRT_ARCH	:= cm4f

BGRT_DIRS	= arch/$(BGRT_ARCH)/gcc kernel kernel/default libs/native

BGRT_SOURCES	= $(wildcard $(patsubst %,%/*.c,$(addprefix $(BGRT_DIR)/,$(BGRT_DIRS))))

BGRT_OBJS	= $(subst .c,.o,$(BGRT_SOURCES))

BGRT_CPPFLAGS	= $(addprefix -I$(BGRT_DIR)/,$(BGRT_DIRS))
BGRT_MODFLAGS	= $(OPENCM3_CPPFLAGS) $(BGRT_CPPFLAGS)

$(call add-module,BGRT)
