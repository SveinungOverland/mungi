#
# Mungi kernel C library build rules
#

KLIBC_BASE = kernel/lib/klibc
KLIBC = klibc

KLIBC_TARGET := $(KLIBC_BASE)/lib$(KLIBC).a
KLDLIBS := -l$(KLIBC)
KLDDIRS += -L$(KLIBC_BASE)

KLIBC_SRC := $(wildcard $(KLIBC_BASE)/*.c) \
	     $(wildcard $(KLIBC_BASE)/asm/$(KTARGET)/*.[cS])

SRC += $(KLIBC_SRC)
KLIBC_OBJ := $(patsubst %.S,%.o,$(filter %.S,$(KLIBC_SRC))) \
	     $(patsubst %.c,%.o,$(filter %.c,$(KLIBC_SRC)))

$(KLIBC_TARGET): $(KLIBC_TARGET)($(KLIBC_OBJ))
SUBTARGETS += $(KLIBC_TARGET)
