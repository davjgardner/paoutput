CC=gcc

LDLIBS=-lpulse

CFLAGS=-g

all: test paoutput

test:
	gcc -o test -g -lpulse test.c

paoutput:

clean:
	rm -f *.o test paoutput

.PHONY: all clean
