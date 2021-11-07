# Build rules for the mungi app

NAME_BASE = lib/naming
NAME_SRC = $(wildcard $(NAME_BASE)/*.c) $(wildcard $(NAME_BASE)/lib/*.c) $(wildcard $(NAME_BASE)/paxtools/*.c) $(wildcard $(NAME_BASE)/tree_cmpt/*.c)
NAME_OBJ = $(patsubst %.c,%.o,$(filter %.c,$(NAME_SRC)))
SUBTARGETS2 := $(SUBTARGETS2) $(NAME_OBJ) 

SRC += $(NAME_SRC)