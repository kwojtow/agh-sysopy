all:
	gcc -Wall -std=c11 -D_POSIX_C_SOURCE=199309L main.c -o main -lm
	gcc -Wall -std=c11 -D_POSIX_C_SOURCE=199309L child.c -o child -lm

clean:
	rm -f *.o
	rm -f main
	rm -f child

test:
	make all
	./main mask | tee raport2.txt
	./main pending | tee -a raport2.txt
	./main ignore | tee -a raport2.txt
	./main handler | tee -a raport2.txt
	make clean

