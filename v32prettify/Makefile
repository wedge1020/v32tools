LIBS =
CFLAGS = 
INC = 
CC = gcc
SRC = $(wildcard *.c)
BIN = $(SRC:.c=)
PROJ = v32prettify
all: $(SRC) $(BIN)

debug: CFLAGS += -DDEBUG -g
debug: DEBUG = debug
debug: $(SRC) $(BIN)

install:
	@mkdir -p /home/$(USER)/bin
	@cp -av $(PROJ) /home/$(USER)/bin/

clean:
	@rm -vf .*.sw[op] *.save* *~ core errors
