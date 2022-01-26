all:
	@gcc -Wall -std=c11 -D_DEFAULT_SOURCE client.c -o client -lm
	@gcc -Wall -std=c11 -D_DEFAULT_SOURCE server.c -o server -lm

clean:
	@rm -f *.o
	@rm -f client
	@rm -f server

