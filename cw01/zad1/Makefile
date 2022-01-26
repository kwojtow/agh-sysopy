libstatic: 
	gcc -Wall -std=c11 -g -c mylibrary.c
	ar rcs libmylibrary.a mylibrary.o

libshared:
	gcc -fPIC -c mylibrary.c
