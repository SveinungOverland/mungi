#
# Rules to build the code for Mungi's upcall mechanism
#

UPCALL_BASE = kernel/lib/upcall

UPCALL_SRC = $(wildcard $(UPCALL_BASE)/*.c $(UPCALL_BASE)/asm/$(KTARGET)/*.S)
SRC += $(UPCALL_SRC)
UPCALL_OBJ = $(patsubst %.S,%.o,$(filter %.S,$(UPCALL_SRC))) \
	     $(patsubst %.c,%.o,$(filter %.c,$(UPCALL_SRC))) \
             lib/mlib/clock.o lib/libc/stdlib/ctype_.o

ifeq (${MUNGI_ARCH}, alpha)
UPCALL_OBJ += kernel/src/asm/alpha/clock.o
endif

OBJ += $(UPCALL_OBJ)

SUBTARGETS += $(UPCALL_OBJ)

