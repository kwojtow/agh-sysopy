all:
	@gcc -Wall -std=c11 -D_DEFAULT_SOURCE main.c -o main -lm

clean:
	@rm -f *.o
	@rm -f main

test:
	@make all
	@./main data.txt
	@make clean

