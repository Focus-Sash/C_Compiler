CFLAGS=-std=c17 -g -static

mycompiler:
	gcc -g -static -o compiler MyCompiler.c

test: compiler
		./test.sh

clean:
		rm -f compiler *.o *~ tmp*

.PHONY: test clean