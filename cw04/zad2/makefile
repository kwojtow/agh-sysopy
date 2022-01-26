all:
	gcc -Wall -std=c11 -D_GNU_SOURCE=199309L main.c -o main -lm

clean:
	rm -f *.o
	rm -f main

test:
	make all
	./main
	make clean

