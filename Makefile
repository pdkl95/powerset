#################
###  OPTIONS  ###
#################

## First, the compiler
#CC       = cc
CC       = gcc

# compiler options
CFLAGS =
# preprocesor options
CPPFLAGS = 
# linker options
LDFLAGS = 

## Removing Files
RM   = rm -v
RM_F = $(RM) -f


#######################
###  COMPATABILITY  ###
#######################

## optionally use all compiler warnings?
CFLAGS += -Wall

## use extra non-standard warnings?
ifeq ($(CC), gcc)
  CFLAGS += -Wextra
endif

# comment this line to fix "stdbool.h" errors
CPPFLAGS += -DHAVE_STDBOOL

# comment this line to fix "_Bool" errors
CPPFLAGS += -DHAVE__BOOL


#####################
###  BUILD RULES  ###
#####################

TARGETS = powerset

all: build
build: $(TARGETS)

powerset: powerset.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o powerset powerset.c

clean:
	$(RM) $(TARGETS)

.PHONY: all build clean
