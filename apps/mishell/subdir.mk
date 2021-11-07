# Build rules for the mungi app

MISHELL_NAME = mishell
MISHELL_BASE = apps/mishell
MISHELL_SRC = $(wildcard $(MISHELL_BASE)/*.c)
MISHELL_OBJ = $(patsubst %.c,%.o,$(wildcard $(MISHELL_BASE)/*.c)) 


SUBTARGETS := $(SUBTARGETS) $(MISHELL_OBJ)
USERLAND_VARS := $(USERLAND_VARS) MISHELL
SRC += $(MISHELL_SRC)
