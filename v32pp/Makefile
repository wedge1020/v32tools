LIBS =
CFLAGS = -Wall --std=gnu18 -funsigned-char -Wno-unused-value -Wno-char-subscripts -Wno-pointer-sign
INC = 
CC = gcc
SRC = $(wildcard *.c)
BIN = $(SRC:.c=)
PROJ = v32pp
all: $(SRC) $(BIN)

debug: CFLAGS += -DDEBUG -g
debug: DEBUG = debug
debug: $(SRC) $(BIN)

.c:
	@mkdir -p ../bin
	$(CC)  $(INC) -o ../bin/$@ $< $(CFLAGS) $(LIBS)

install:
	@mkdir -p /home/$(USER)/bin
	@cp -av $(PROJ) /home/$(USER)/bin/

clean:
	@rm -vf .*.sw[op] *.save* *~ ../bin/* core errors
