all:
	@gcc -Wall -std=c11 -D_DEFAULT_SOURCE client.c -o client -lrt
	@gcc -Wall -std=c11 -D_DEFAULT_SOURCE server.c -o server -lrt

clean:
	@rm -f *.o
	@rm -f client
	@rm -f server
