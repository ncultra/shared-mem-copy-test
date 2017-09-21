.PHONY: all
all: consumer producer
consumer: consumer.c prod-cons.h
	gcc -lrt -std=c99 -Wall -g -o consumer consumer.c

producer: producer.c prod-cons.h
	gcc -lrt -std=c99 -Wall -g -o producer producer.c

.PHONY: clean
clean:
	rm -v *.o producer consumer 

.PHONY: lint
lint:
	find . -name "*.c"  -exec cppcheck --force {} \;
	find . -name "*.h"  -exec cppcheck --force {} \;

.PHONY: pretty
pretty:
	find . -name "*.c"  -exec indent -gnu {} \;
	find . -name "*.h"  -exec indent -gnu {} \;

.PHONY: scan-build
scan-build:
	scan-build make all
