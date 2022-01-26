all:
	gcc -Wall -std=c11 main.c -o main -lm

clean:
	rm -f *.o
	rm -f main

test:
	make all
	./main 10
	./main 100
	make clean

