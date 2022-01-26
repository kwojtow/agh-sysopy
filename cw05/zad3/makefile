CH_NO=8

all:
	gcc -Wall -std=c11 producer.c -o producer -lm
	gcc -Wall -std=c11 consumer.c -o consumer -lm

clean:
	rm -f *.o
	rm -f producer
	rm -f consumer
	rm fifo

test:
	make all
	mkfifo fifo
	./consumer fifo out.txt $(CH_NO) &
	./producer fifo 0 data0.txt $(CH_NO) &
	./producer fifo 1 data1.txt $(CH_NO) &
	./producer fifo 2 data2.txt $(CH_NO) &
	./producer fifo 3 data3.txt $(CH_NO)
	make clean

