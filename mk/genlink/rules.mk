#------------------------------------------ -*- tab-width: 8 -*-
genlink:	| $(OPENCM3LIB)

$(OPENCM3LIB):
	$(Q)$(MAKE) -C $(OPENCM3DIR) lib TARGETS=stm32/f4 CFLAGS=-flto AR=$(CC)-ar

$(LDSCRIPT):	$(OPENCM3DIR)/ld/linker.ld.S $(DEVICES_DATA)
	@printf "  GENLNK  $(DEVICE)\n"
	$(Q)$(CPP) $(ARCHFLAGS) $(call genlink,DEFS) -P -E $< -o $@
