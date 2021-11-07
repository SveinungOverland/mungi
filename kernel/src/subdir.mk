#
# Mungi kernel makefile
#
K_BASE = kernel/src
KS_SRC = $(wildcard $(K_BASE)/*.c $(K_BASE)/asm/${KTARGET}/*.S) 
#			$(wildcard $(K_BASE)/vm/*.c)
KS_SRC += $(K_BASE)/vm/ptable.c $(K_BASE)/vm/asm/$(KTARGET)/mutex.S

KOBJ += $(patsubst %.S,%.o,$(filter %.S,$(KS_SRC))) \
	$(patsubst %.c,%.o,$(filter %.c,$(KS_SRC))) 
	


SRC += $(KS_SRC)
OBJ += $(KOBJ)

SUBTARGETS += $(KOBJ)
