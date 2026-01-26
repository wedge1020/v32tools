LIBS =
CFLAGS = -Wall --std=gnu18 -funsigned-char -Wno-unused-value
INC = 
CC = gcc
SRC = $(wildcard *.c)
BIN = $(SRC:.c=)
all: gcc

debug: CFLAGS += -DDEBUG -g
debug: DEBUG = debug
debug: gcc

gcc:
	@make -f Makefile.gcc

cscript:
	@make -f Makefile.cscript

install:
	@mkdir -p /home/$(USER)/bin
	@cp -av ../bin/* /home/$(USER)/bin/

sysinstall:	
	@cp -av ../bin/* /usr/local/bin/

clean:
	@rm -vf .*.sw[op] *.save* *~ ../bin/* core errors
