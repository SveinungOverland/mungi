#
# Mungi kernel makefile
#
SYSCALLS_BASE = syscalls
SYSCALLS_SRC = $(wildcard $(SYSCALLS_BASE)/*.c)

SUBCALLS_OBJ = $(patsubst %.c,%.o,$(filter %.c,$(SYSCALLS_SRC))) 

SUBTARGETS2 += $(SUBCALLS_OBJ)

SPECINCLUDES += $(LIBCINCLUDES)

SRC += $(SYSCALLS_SRC)