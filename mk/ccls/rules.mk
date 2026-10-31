#------------------------------------------ -*- tab-width: 8 -*-
.ccls:		$(CCLS_DEPS)
		$(foreach O,$(CCLS_FLAGS),$(file >>$@,$O))
