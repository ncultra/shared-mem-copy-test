.PHONY: all
all: consumer producer
consumer: consumer.c prod-cons.h
	gcc -lrt -std=c11 -g -o consumer consumer.c

producer: producer.c prod-cons.h
	gcc -lrt -std=c11 -g -o producer producer.c

