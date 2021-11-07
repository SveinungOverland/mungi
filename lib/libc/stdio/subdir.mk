#
# Mungi kernel makefile
#
STDIO_BASE = lib/libc/stdio
STDIO_SRC = $(wildcard $(STDIO_BASE)/*.c)

STDIO_OBJ = $(patsubst %.c,%.o,$(filter %.c,$(STDIO_SRC))) 


SUBTARGETS2 += $(STDIO_OBJ)

SPECINCLUDES = $(LIBCINCLUDES)
SRC += $(STDIO_SRC)


