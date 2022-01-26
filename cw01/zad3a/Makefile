REPORT_HEADER=""

libstatic: clean
	gcc -Wall -c main.c
	gcc -Wall -c mylibrary.c
	ar crs libmylibrary.a main.o mylibrary.o
	gcc -Wall main.c -o main -L. -lmylibrary
	rm -f *.o
#libshared: clean
#	gcc -Wall -c main.c
#	gcc -Wall -c mylibrary.c
#	gcc -Wall -shared -fPIC -o libmylibrary.so main.o mylibrary.o
#	gcc main.c -Wall -o main -L. -lmylibrary -Wl,-rpath=`pwd`
#	rm -f *.o
	
libshared:
	gcc -Wall -fPIC -shared mylibrary.c -o libmylibrary.so

	
clean:
	rm -f *.a
	rm -f *.o
	rm -f *.out
	rm -f *test*
	rm -f main
	rm -f *.so

run: clean libstatic
	gcc -Wall -std=c11 -g -c main.c -o main.o
	gcc -Wall -std=c11 -g -static main.o -L. libmylibrary.a -o main
	./main 2 create_table 2 merge_files small.txt:big.txt big.txt:small.txt
	make clean

test_static: libstatic
	echo "\n\n----- STATIC LIBRARY -----" > ./raport3a.txt
	gcc -Wall -std=c11 -g -c main.c -o main.o
	gcc -Wall -std=c11 -g -static main.o -L. libmylibrary.a -o main
	make test
	
test_shared: libshared
	echo "\n\n----- SHARED LIBRARY -----" >> ./raport3a.txt
	gcc -Wall main.c -L. -Wl,-rpath=. -lmylibrary -o main
	make test	
	
test_dynamic:
	echo "\n\n----- DYNAMIC LIBRARY -----" >> ./raport3a.txt
	make libshared
	gcc -Wall -std=c11 -g main_dynamic.c -o main -ldl
	make test
	
test:
	./main test small.txt:small.txt 50 | tee -a ./raport3a.txt
	./main test small.txt:small.txt 100 | tee -a ./raport3a.txt
	./main test small.txt:small.txt 1000 | tee -a ./raport3a.txt
	./main test medium.txt:medium.txt 50 | tee -a ./raport3a.txt
	./main test medium.txt:medium.txt 100 | tee -a ./raport3a.txt
	./main test big.txt:big.txt 1 | tee -a ./raport3a.txt
	./main test big.txt:big.txt 2 | tee -a ./raport3a.txt
	./main test big.txt:big.txt 5 | tee -a ./raport3a.txt
	make clean
	
all_tests:
	make test_static
	make test_shared
	make test_dynamic
	

