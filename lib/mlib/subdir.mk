# Build rules for the mungi app

MLIB_BASE = lib/mlib
MLIB_SRC = $(wildcard $(MLIB_BASE)/*.c)
MLIB_OBJ = $(patsubst %.c,%.o,$(wildcard $(MLIB_BASE)/*.c))

SUBTARGETS2 := $(SUBTARGETS2) $(MLIB_OBJ)
SRC += $(MLIB_SRC)