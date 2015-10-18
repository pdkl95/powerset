#################
###  OPTIONS  ###
#################

######################
## First, the compiler

# (choose one or setup your onw manually)

# generic UNIX compiler with no assumptions
#CC     = cc
#CFLAGS =

# GNU's C Compiler
CC     = gcc
CFLAGS = -Wall -Wextra

#################
## Removing Files
RM   = rm -v
RM_F = $(RM) -f

#####################
###  END OPTIONS  ###
#####################

TARGETS = powerset

all: build
build: $(TARGETS)

powerset: powerset.c
	$(CC) $(CFLAGS) -o powerset powerset.c

clean:
	$(RM) $(TARGETS)

.PHONY: all build clean
