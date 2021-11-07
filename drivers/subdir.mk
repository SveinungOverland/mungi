#
# Mungi Driver subdir.mk
# 
DRV_BASE = drivers
DRV_SRC = $(wildcard $(DRV_BASE)/*.c) $(wildcard $(DRV_BASE)/$(KTARGET)/*.c) \
	            $(wildcard $(DRV_BASE)/asm/$(KTARGET)/wbflush.S)
	            #$(wildcard $(DRV_BASE)/asm/$(KTARGET)/*.S)

DRV_OBJ += $(patsubst %.S,%.o,$(filter %.S,$(DRV_SRC))) \
	   $(patsubst %.c,%.o,$(filter %.c,$(DRV_SRC)))

KOBJ += $(DRV_OBJ)
OBJ += $(DRV_OBJ)
SRC += $(DRV_SRC)
