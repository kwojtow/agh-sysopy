all:
	@gcc -Wall -std=c11 -D_DEFAULT_SOURCE main.c -o main -lm -lpthread

clean:
	@rm -f *.o
	@rm -f main
	@rm -f out.ascii.pgm

test:
	@make all
	@echo "\nBLOCK - 1 THREAD" | tee Times.txt
	@./main 1 block fractal_tree.ascii.pgm out.ascii.pgm | tee -a Times.txt
	@echo "\nBLOCK - 2 THREADS" | tee -a Times.txt
	@./main 2 block fractal_tree.ascii.pgm out.ascii.pgm | tee -a Times.txt
	@echo "\nBLOCK - 4 THREADS" | tee -a Times.txt
	@./main 4 block fractal_tree.ascii.pgm out.ascii.pgm | tee -a Times.txt
	@echo "\nBLOCK - 8 THREADS" | tee -a Times.txt
	@./main 8 block fractal_tree.ascii.pgm out.ascii.pgm | tee -a Times.txt
	@echo "\nBLOCK - 16 THREADS" | tee -a Times.txt
	@./main 16 block fractal_tree.ascii.pgm out.ascii.pgm | tee -a Times.txt
	@echo "\nBLOCK - 64 THREADS" | tee -a Times.txt
	@./main 64 block fractal_tree.ascii.pgm out.ascii.pgm | tee -a Times.txt
	@echo "\nNUMBERS - 1 THREAD" | tee -a Times.txt
	@./main 1 numbers fractal_tree.ascii.pgm out.ascii.pgm | tee -a Times.txt
	@echo "\nNUMBERS - 2 THREADS" | tee -a Times.txt
	@./main 2 numbers fractal_tree.ascii.pgm out.ascii.pgm | tee -a Times.txt
	@echo "\nNUMBERS - 4 THREADS" | tee -a Times.txt
	@./main 4 numbers fractal_tree.ascii.pgm out.ascii.pgm | tee -a Times.txt
	@echo "\nNUMBERS - 8 THREADS" | tee -a Times.txt
	@./main 8 numbers fractal_tree.ascii.pgm out.ascii.pgm | tee -a Times.txt
	@echo "\nNUMBERS - 16 THREADS" | tee -a Times.txt
	@./main 16 numbers fractal_tree.ascii.pgm out.ascii.pgm | tee -a Times.txt
	@echo "\nNUMBERS - 64 THREADS" | tee -a Times.txt
	@./main 64 numbers fractal_tree.ascii.pgm out.ascii.pgm | tee -a Times.txt
