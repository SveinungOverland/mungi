# Build rules for the mungi app

INIT_NAME = init
INIT_BASE = apps/init
INIT_SRC = $(wildcard $(INIT_BASE)/*.c) $(wildcard $(INIT_BASE)/serial_tty/*.c)
INIT_OBJ = $(patsubst %.c,%.o,$(INIT_SRC))


SUBTARGETS := $(SUBTARGETS) $(INIT_OBJ)
USERLAND_VARS := $(USERLAND_VARS) INIT
SRC += $(INIT_SRC)
