# vim: set noexpandtab:

CC=g++
CFLAGS=
CFLAGSPOST=-lGL -lglut

MDEPS=main.o

all: main

main.o: main.cpp vec2.h programstate.h
	$(CC) $(CFLAGS) -c main.cpp

main: $(MDEPS)
	$(CC) $(CFLAGS) $(MDEPS) -o main $(CFLAGSPOST)

.PHONY: run
.PHONY: clean

run: main
	@./main

clean:
	rm -f *.o main
