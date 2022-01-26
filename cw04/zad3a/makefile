all:
	@gcc -Wall -std=c11 -D_GNU_SOURCE sender.c -o sender -lm
	@gcc -Wall -std=c11 -D_GNU_SOURCE catcher.c -o catcher -lm

clean:
	@rm -f *.o
	@rm -f sender
	@rm -f catcher

test:
	@make all
	@echo "----------KILL----------"
	@./catcher &
	@./sender `pidof -s catcher` 100 KILL
	@echo "----------SIGQUEUE----------"
	@./catcher &
	@./sender `pidof -s catcher` 100 SIGQUEUE
	@echo "----------SIGRT----------"
	@./catcher &
	@./sender `pidof -s catcher` 100 SIGRT
	@make clean

