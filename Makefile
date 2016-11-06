CXX?=g++
CXXFLAGS=-std=c++11 -Wall -Wno-format -Wno-char-subscripts -pipe
OPTFLAGS=-march=native -mtune=native -O3 -flto -DNDEBUG
DBGFLAGS=-g -O0
LDFLAGS=
SOURCES=attacked.cpp board.cpp eval.cpp fen.cpp magic.cpp main.cpp makemove.cpp movegen.cpp movesort.cpp perft.cpp search.cpp see.cpp tt.cpp tune.cpp zobrist.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=dorpsgek

ifeq ($(DEBUG), 1)
	CXXFLAGS += $(DBGFLAGS)
	EXECUTABLE = dorpsgek-debug
else
	CXXFLAGS += $(OPTFLAGS)
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
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ -lm

.c.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@
