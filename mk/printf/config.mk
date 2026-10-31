#------------------------------------------ -*- tab-width: 8 -*-
PRINTF_DIR		:= printf

PRINTF_OBJS		= $(PRINTF_DIR)/printf.o

PRINTF_MODFLAGS		=	\
				-DPRINTF_DISABLE_SUPPORT_FLOAT \
				-DPRINTF_DISABLE_SUPPORT_EXPONENTIAL \
				-DPRINTF_DISABLE_SUPPORT_LONG_LONG \
				-DPRINTF_DISABLE_SUPPORT_PTRDIFF_T

$(call add-module,PRINTF)
