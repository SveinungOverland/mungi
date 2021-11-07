#
# Mungi kernel makefile
#
STDLIB_BASE = lib/libc/stdlib
STDLIB_SRC = $(wildcard $(STDLIB_BASE)/*.c)

STDLIB_OBJ += $(patsubst %.c,%.o,$(filter %.c,$(STDLIB_SRC))) 


SUBTARGETS2 += $(STDLIB_OBJ)

SPECINCLUDES = $(LIBCINCLUDES)

SRC += $(STDLIB_SRC)

