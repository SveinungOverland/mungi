# Build rules for the mungi user-land stuff

# Auth has to be first (yes this is a problem)
USERLAND := init mishell

# check for testing
ifdef TESTING
USERLAND := $(USERLAND) test
endif

include $(patsubst %,apps/%/subdir.mk, $(USERLAND))
