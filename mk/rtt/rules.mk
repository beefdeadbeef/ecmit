#------------------------------------------ -*- tab-width: 8 -*-
rtt:		$(RTTLIB)

$(RTTLIB):	$(RTTOBJS)
		@printf "  LD      $@\n"
		$(Q)$(CC)-ar rcs $(RTTLIB) $?
