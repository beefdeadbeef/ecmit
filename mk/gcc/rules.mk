#------------------------------------------ -*- tab-width: 8 -*-
%.bin:	%.elf
	@printf "  OBJCOPY $@\n"
	$(Q)$(OBJCOPY) -Obinary $< $@

%.elf:	$(OBJS) $(LDSCRIPT) $(LIBDEPS)
	@printf "  LD      $(*).elf\n"
	$(Q)$(LD) $(CFLAGS) $(ARCHFLAGS) $(OBJS) $(LDFLAGS) $(LDLIBS) -T$(LDSCRIPT) -o $@

$(O)/%.o:       %.c | $(O)
	@printf "  CC      $<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) $(ARCHFLAGS) $(CPPFLAGS) -o $@ -c $<
