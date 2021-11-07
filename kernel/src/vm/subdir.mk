VM_BASE = kernel/src/vm
#MPR_BASE = kernel/src/mpr

VM_SRC = $(wildcard $(VM_BASE)/*.c $(VM_BASE)/asm/${KTARGET}/*.S) $(wildcard $(MPR_BASE)/*.c) $(wildcard $(VM_BASE)/srdisk/*.c)

# Additional files needed for the mpr
VM_SRC += lib/libc/string/memcmp.c lib/libc/string/memmove.c

RPAGERSRC += $(VM_SRC) $(VMDBGSRC)

RPOBJ := $(RPAGERSRC:.c=.o)
RPOBJ := $(RPOBJ:.S=.o)
RPOBJ += kernel/src/asm/${KTARGET}/crt0.o

SUBTARGETS += $(RPOBJ)
SRC += $(RPAGERSRC)
