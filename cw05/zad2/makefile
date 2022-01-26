all:
	@gcc -Wall -std=c11 -D_DEFAULT_SOURCE main.c -o main -lm

clean:
	@rm -f *.o
	@rm -f main

test:
	@make all
	@echo "----- SORTED BY SENDER -----" > raport.txt
	@./main sender >> raport.txt
	@echo "\n\n----- SORTED BY DATE -----" >> raport.txt
	@./main date >> raport.txt
	@echo "\n\n----- SENDING EMAIL -----" >> raport.txt
	@./main krzysztof test treść >> raport.txt
	@echo "\n\n----- SORTED BY SENDER -----" >> raport.txt
	@./main sender >> raport.txt
	@echo "\n\n----- SORTED BY DATE -----" >> raport.txt
	@./main date >> raport.txt
	@make clean

