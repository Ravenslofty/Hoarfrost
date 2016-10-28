CC=gcc
CFLAGS=-std=c99 -c -Wall -Wno-format -Wno-char-subscripts -pipe
OPTFLAGS=-march=native -mtune=native -O3 -flto -DNDEBUG
DBGFLAGS=-g -O0
LDFLAGS=-flto
SOURCES=attacked.c eval.c fen.c magic.c main.c makemove.c movegen.c movesort.c perft.c search.c see.c tt.c zobrist.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=dorpsgek

ifeq ($(DEBUG), 1)
	CFLAGS += $(DBGFLAGS)
	EXECUTABLE = dorpsgek-debug
else
	CFLAGS += $(OPTFLAGS)
endif

.PHONY: all clean winclean test

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm -rf $(EXECUTABLE) $(OBJECTS)

winclean:
	del $(EXECUTABLE) $(OBJECTS)

test: $(EXECUTABLE)
	cat ./perft-$(TEST).epd | ./dorpsgek

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
