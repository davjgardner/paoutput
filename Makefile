CC=gcc

LDLIBS=-lpulse

CFLAGS=-g

all: paoutput

paoutput:

clean:
	rm -f *.o paoutput

.PHONY: all clean
