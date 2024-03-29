CC=g++
CFLAGS=-O3 -funroll-loops -c
LDFLAGS=-O2 -lm
SOURCES=Bot.cc MyBot.cc State.cc scn/scnLayer.cc scn/scnNode.cc scn/scnANN.cc pathfind/idastar.cpp pathfind/tiling.cpp pathfind/statistics.cpp pathfind/astar.cpp pathfind/searchutils.cpp pathfind/util.cpp pathfind/environment.cpp pathfind/search.cpp pathfind/error.cpp pathfind/version.cpp
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=MyBot

#Uncomment the following to enable debugging
#CFLAGS+=-g -DDEBUG

all: $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cc.o: *.h
	$(CC) $(CFLAGS) $< -o $@

clean: 
	-rm -f ${EXECUTABLE} ${OBJECTS} *.d
	-rm -f debug.txt

.PHONY: all clean

