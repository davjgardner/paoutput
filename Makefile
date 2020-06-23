CC=gcc

LDLIBS=-lpulse

all: test

test:
	gcc -o test -g -lpulse test.cpp

clean:
	rm -f *.o test

.PHONY: all clean
