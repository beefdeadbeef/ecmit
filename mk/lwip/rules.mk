#------------------------------------------ -*- tab-width: 8 -*-
lwip:		$(LWIPLIB)

$(LWIPLIB):	$(LWIPOBJS)
		@printf "  LD      $@\n"
		$(Q)$(CC)-ar rcs $(LWIPLIB) $?

-include	$(LWIPOBJS:.o=.d)
