#------------------------------------------ -*- tab-width: 8 -*-
bgrt:	$(BGRT_LIB)

$(BGRT_LIB):	$(BGRT_OBJS)
		@printf "  LD      $@\n"
		$(Q)$(CC)-ar rcs $(BGRT_LIB) $?

-include	$(BGRT_OBJS:.o=.d)
