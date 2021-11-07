#
# Mungi kernel makefile
#
STRING_BASE = lib/libc/string
STRING_SRC = $(wildcard $(STRING_BASE)/*.c)

STRING_OBJ = $(patsubst %.c,%.o,$(filter %.c,$(STRING_SRC))) 


SUBTARGETS2 += $(STRING_OBJ)

SPECINCLUDES = $(LIBCINCLUDES)

SRC += $(STRING_SRC)