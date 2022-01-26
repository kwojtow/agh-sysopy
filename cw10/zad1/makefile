all:
	@gcc -Wall -std=c11 -D_DEFAULT_SOURCE server.c -o server -lpthread
	@gcc -Wall -std=c11 -D_DEFAULT_SOURCE client.c -o client -lpthread

clean:
	@rm -f *.o
	@rm -f server
	@rm -f client

test_server:
	@make all
	@./server 8080 socket.soc

test_net_client:
	@make all
	@./client netman net 127.0.0.1
	
test_local_client:
	@make all
	@./client locman local socket.soc
