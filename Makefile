.PHONY: all
all: consumer producer
consumer: consumer.c prod-cons.h
	gcc -lrt -std=c99 -Wall -g -o consumer consumer.c

producer: producer.c prod-cons.h
	gcc -lrt -std=c99 -Wall -g -o producer producer.c

.PHONY: clean
clean:
	rm -v *.o producer consumer 
