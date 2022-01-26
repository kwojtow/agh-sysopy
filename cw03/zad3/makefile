all:
	gcc -Wall -std=c11 -D_DEFAULT_SOURCE main.c -o main -lm

clean:
	rm -f *.o
	rm -f main

test:
	make all
	./main test test.txt 1
	./main "test" test.txt 2
	./main "test" test.txt 3
	./main "test" test.txt 4
	make clean

