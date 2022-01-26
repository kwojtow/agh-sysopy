all:
	@gcc -Wall -std=c11 -D_DEFAULT_SOURCE main.c -o main -lpthread -lrt
	@gcc -Wall -std=c11 -D_DEFAULT_SOURCE cook.c -o cook -lpthread -lrt
	@gcc -Wall -std=c11 -D_DEFAULT_SOURCE supplier.c -o supplier -lpthread -lrt

clean:
	@rm -f *.o
	@rm -f main
	@rm -f cook
	@rm -f supplier

test1:
	@make all
	@./main 2 3
	@make clean
	
test2:
	@make all
	@./main 7 10
	@make clean

