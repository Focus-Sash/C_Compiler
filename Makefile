CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS: .c=.o)

mycompiler:
	gcc -o compiler $(OBJS) $(CFLAGS)

$(OBJS): header.h

test: compiler
		./test.sh

clean:
		rm -f compiler *.o *~ tmp*

.PHONY: test clean