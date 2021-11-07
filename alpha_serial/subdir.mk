#
# Mungi Driver subdir.mk
# 
# Edited by ceg to add alpha serial driver
ifeq (${MUNGI_ARCH},alpha)
SER_BASE = alpha_serial
SER_SRC = $(wildcard $(SER_BASE)/*.c) $(wildcard $(SER_BASE)/*.S)
SER_OBJ += $(patsubst %.S,%.o,$(filter %.S,$(SER_SRC))) \
	   $(patsubst %.c,%.o,$(filter %.c,$(SER_SRC)))

ALPHASERIAL += $(SER_OBJ)
SRC += $(SER_SRC)

endif
