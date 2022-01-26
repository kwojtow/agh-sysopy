all:
	gcc -Wall -std=c11 sys.c -o sys -lm
	gcc -Wall -std=c11 lib.c -o lib -lm

clean:
	rm -f *.o
	rm -f sys
	rm -f lib
	rm -f a.txt
	rm -f b.txt
	rm -f c.txt

test:
	make all
	echo "\n\n----- SYSTEM FUNCTIONS -----" > ./pomiar_zad_3.txt
	./sys 2>>pomiar_zad_3.txt
	echo "\n\n----- LIBRARY FUNCTIONS -----" >> ./pomiar_zad_3.txt
	./lib 2>>pomiar_zad_3.txt
	
	#make clean

