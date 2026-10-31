#------------------------------------------ -*- tab-width: 8 -*-
OPENCM3DIR		:= libopencm3

ifeq ($(DEVICE),)
$(warning no DEVICE specified for linker script generator)
endif

LDSCRIPT		:= generated.$(DEVICE).ld

DEVICES_DATA 		:= $(OPENCM3DIR)/ld/devices.data
GENLINK			:= $(OPENCM3DIR)/scripts/genlink.py

define genlink
$(shell $(GENLINK) $(DEVICES_DATA) $(DEVICE) $(1))
endef

genlink_family		:= $(call genlink,FAMILY)
genlink_fpu		:= $(call genlink,FPU)

ARCHFLAGS		= -mthumb -mcpu=$(call genlink,CPU)
OPENCM3_CPPFLAGS	= $(call genlink,CPPFLAGS)
OPENCM3_CPPFLAGS	+= -I$(OPENCM3DIR)/include
CPPFLAGS		+= $(OPENCM3_CPPFLAGS)

ifeq ($(genlink_fpu),soft)
ARCHFLAGS		+= -msoft-float
else ifeq ($(genlink_fpu),hard-fpv4-sp-d16)
ARCHFLAGS		+= -mfloat-abi=hard -mfpu=fpv4-sp-d16
else ifeq ($(genlink_fpu),hard-fpv5-d16)
ARCHFLAGS		+= -mfloat-abi=hard -mfpu=fpv5-d16
else ifeq ($(genlink_fpu),hard-fpv5-sp-d16)
ARCHFLAGS		+= -mfloat-abi=hard -mfpu=fpv5-sp-d16
else
$(warning No match for the FPU flags)
endif

OPENCM3LIBNAME		= opencm3_$(genlink_family)
OPENCM3LIB		= $(OPENCM3DIR)/lib/lib$(OPENCM3LIBNAME).a

LDLIBS			+= -l$(OPENCM3LIBNAME)
LIBDEPS			+= $(OPENCM3LIB)
LDFLAGS			+= -L$(OPENCM3DIR)/lib
