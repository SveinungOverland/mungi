#
# Mungi kernel makefile
#
POSIX_BASE = lib/libc/posix
POSIX_SRC = $(wildcard $(POSIX_BASE)/*.c)

POSIX_OBJ = $(patsubst %.c,%.o,$(filter %.c,$(POSIX_SRC))) 


SUBTARGETS2 += $(POSIX_OBJ)

SPECINCLUDES = $(LIBCINCLUDES)
SRC += $(POSIX_SRC)


