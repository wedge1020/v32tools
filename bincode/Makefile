LIBS =
CFLAGS = -Wall --std=gnu18 -funsigned-char -Wno-unused-value -Wno-char-subscripts -Wno-pointer-sign
INC = 
CC = gcc
SRC = $(wildcard *.c)
BIN = $(SRC:.c=)
all: $(SRC) $(BIN)

debug: CFLAGS += -DDEBUG -g
debug: DEBUG = debug
debug: $(SRC) $(BIN)

.c:
	$(CC)  $(INC) -o $@ $< $(CFLAGS) $(LIBS)

install:
	@mkdir -p /home/$(USER)/bin
	@cp -av $(BIN) /home/$(USER)/bin/

clean:
	@rm -vf .*.sw[op] *.save* *~ $(BIN) core errors
