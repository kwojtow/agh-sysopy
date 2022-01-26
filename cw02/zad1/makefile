all:
	gcc -Wall -std=c11 sys.c -o sys
	gcc -Wall -std=c11 lib.c -o lib

clean:
	rm -f *.o
	rm -f sys
	rm -f lib

test:
	make all
	echo "\n\n----- SYSTEM FUNCTIONS -----" > ./pomiar_zad_1.txt
	./sys data1.txt data2.txt 2>>pomiar_zad_1.txt
	echo "\n\n----- LIBRARY FUNCTIONS -----" >> ./pomiar_zad_1.txt
	./lib data1.txt data2.txt 2>>pomiar_zad_1.txt
	
	make clean

