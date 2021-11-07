# Build rules for the mungi app

TEST_NAME = test
TEST_BASE = apps/test
TEST_SRC = $(wildcard $(TEST_BASE)/*.c)
TEST_OBJ = $(patsubst %.c,%.o,$(wildcard $(TEST_BASE)/*.c))

SUBTARGETS := $(SUBTARGETS) $(TEST_OBJ)
USERLAND_VARS := $(USERLAND_VARS) TEST

SRC += $(TEST_SRC)

