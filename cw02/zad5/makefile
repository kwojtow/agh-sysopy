all:
	gcc -Wall -std=c11 sys.c -o sys
	gcc -Wall -std=c11 lib.c -o lib

clean:
	rm -f *.o
	rm -f sys
	rm -f lib
	rm -f a.txt

test:
	make all
	echo "\n\n----- SYSTEM FUNCTIONS -----" > ./pomiar_zad_5.txt
	./sys data1.txt a.txt 2>>pomiar_zad_5.txt
	echo "\n\n----- LIBRARY FUNCTIONS -----" >> ./pomiar_zad_5.txt
	./lib data1.txt a.txt 2>>pomiar_zad_5.txt
	
	#make clean

