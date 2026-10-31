#------------------------------------------ -*- tab-width: 8 -*-
CCLS_DEPS =
CCLS_FLAGS =	clang \
		--target=$(TARGET) \
		$(filter -mcpu=%,$(ARCHFLAGS)) \
		-isystem$(shell $(CC) --print-sysroot)/include \
		$(filter -I%,$(CPPFLAGS)) \
		$(filter -D%,$(CPPFLAGS))
