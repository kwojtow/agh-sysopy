all:
	gcc -Wall -std=c11 sys.c -o sys
	gcc -Wall -std=c11 lib.c -o lib

clean:
	rm -f *.o
	rm -f sys
	rm -f lib

test:
	make all
	echo "\n\n----- SYSTEM FUNCTIONS -----" > ./pomiar_zad_2.txt
	./sys 4 data1.txt 2>>pomiar_zad_2.txt
	echo "\n\n----- LIBRARY FUNCTIONS -----" >> ./pomiar_zad_2.txt
	./lib 4 data1.txt 2>>pomiar_zad_2.txt
	
	make clean

