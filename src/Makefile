CC=g++
OPTS=-g -std=c++11 

all: main.o cache.o
	$(CC) $(OPTS) -lm -o cache main.o cache.o

main.o: main.c cache.h
	$(CC) $(OPTS) -c main.c

cache.o: cache.h cache.c
	$(CC) $(OPTS) -c cache.c

clean:
	rm -f *.o cache;
