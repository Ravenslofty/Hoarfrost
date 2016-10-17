CC=gcc
CFLAGS=-c -Wall -Wno-format -Wno-char-subscripts -pipe -O3 -flto
LDFLAGS=-flto
SOURCES=attacked.c fen.c magic.c main.c makemove.c movegen.c movesort.c perft.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=dorpsgek

.PHONY: all clean

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm -rf $(EXECUTABLE) $(OBJECTS)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
