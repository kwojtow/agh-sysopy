libstatic: 
	gcc -Wall -std=c11 -g -c mylibrary.c -o mylibrary.o
	ar rcs mylibrary.a mylibrary.o

clean:
	rm -f *.a
	rm -f *.o
	rm -f *.out
	rm -f *test*
	rm -f main

test: clean libstatic
	gcc -Wall -std=c11 -g -c main.c -o main.o
	gcc -Wall -std=c11 -g -static main.o -L. mylibrary.a -o main
	./main test small.txt:small.txt 50 | tee ./raport.txt
	./main test small.txt:small.txt 100 | tee -a ./raport.txt
	./main test small.txt:small.txt 1000 | tee -a ./raport.txt
	./main test medium.txt:medium.txt 50 | tee -a ./raport.txt
	./main test medium.txt:medium.txt 100 | tee -a ./raport.txt
	./main test big.txt:big.txt 1 | tee -a ./raport.txt
	./main test big.txt:big.txt 2 | tee -a ./raport.txt
	./main test big.txt:big.txt 5 | tee -a ./raport.txt
	make clean
