#
# Mungi kernel C library build rules
#

MUNGILIB_BASE = lib/mungi
MUNGILIB = mungi

MUNGILIB_TARGET := $(MUNGILIB_BASE)/lib$(MUNGILIB).a
MLDLIBS := -l$(MUNGILIB)
MLDDIRS += -L$(MUNGILIB_BASE)

MUNGILIB_SRC := $(wildcard $(MUNGILIB_BASE)/*.c) \
	     $(wildcard $(MUNGILIB_BASE)/asm/$(KTARGET)/*.S)

SRC += $(MUNGILIB_SRC)
MUNGILIB_OBJ := $(patsubst %.S,%.o,$(filter %.S,$(MUNGILIB_SRC))) \
	     $(patsubst %.c,%.o,$(filter %.c,$(MUNGILIB_SRC)))

$(MUNGILIB_TARGET): $(MUNGILIB_TARGET)($(MUNGILIB_OBJ))
SUBTARGETS += $(MUNGILIB_TARGET)

