# v32as: the C implementation of a Vircon32 assembler (flex + bison)
#
#   make            build bin/v32as
#   make debug      build with -g and no optimization
#   make test       run the regression tests (tests/run_tests.sh)
#   make install    copy the tool into ~/bin
#
# Requires: a C compiler, flex and bison. GNU readline is used for the interactive
# mode when its development files are found (make READLINE=0 to opt out).

CC       ?= gcc
FLEX     ?= flex
BISON    ?= bison
CFLAGS   ?= -O2
CFLAGS   += -Wall -Wextra -std=gnu11 -Wno-unused-parameter
LIBS     =
PROJ     = v32as
OBJ      = parser.tab.o lexer.o asm.o tables.o main.o

READLINE ?= auto
ifeq ($(READLINE),auto)
  HAVE_RL := $(shell echo '\#include <readline/readline.h>' | $(CC) -E - >/dev/null 2>&1 && echo 1)
else
  HAVE_RL := $(filter 1,$(READLINE))
endif
ifeq ($(HAVE_RL),1)
  CFLAGS += -DHAVE_READLINE
  LIBS   += -lreadline
endif

all: bin/$(PROJ)

debug: CFLAGS += -g -O0 -DDEBUG
debug: all

bin/$(PROJ): $(OBJ)
	@mkdir -p bin
	$(CC) -o $@ $(OBJ) $(LIBS)

parser.tab.c parser.tab.h: parser.y
	$(BISON) -Wall -d -o parser.tab.c parser.y

lexer.c: lexer.l parser.tab.h
	$(FLEX) -o lexer.c lexer.l

parser.tab.o: parser.tab.c v32as.h
	$(CC) $(CFLAGS) -Wno-unused-but-set-variable -c -o $@ $<

lexer.o: lexer.c parser.tab.h v32as.h
	$(CC) $(CFLAGS) -Wno-sign-compare -Wno-unused-function -c -o $@ $<

asm.o: asm.c parser.tab.h v32as.h
	$(CC) $(CFLAGS) -c -o $@ $<

tables.o: tables.c v32as.h
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c v32as.h
	$(CC) $(CFLAGS) -c -o $@ $<

test: all
	@sh tests/run_tests.sh bin/$(PROJ)

install: all
	@mkdir -p /home/$(USER)/bin
	@cp -av bin/$(PROJ) /home/$(USER)/bin/

clean:
	@rm -vf *.o parser.tab.c parser.tab.h lexer.c .*.sw[op] *~ core bin/$(PROJ) ../v32as-project.zip

archive: clean
	@zip -r ../v32as-project.zip *

.PHONY: all debug test install clean
