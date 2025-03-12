CC = gcc
CFLAGS = -Wall -g

all: test_memory

test_memory: Exo2.o Exo1.o main_test.o
	$(CC) $(CFLAGS) Exo2.o Exo1.o main_test.o -o test_memory

Exo2.o: Exo2.c memory.h hash.h
	$(CC) $(CFLAGS) -c Exo2.c

Exo1.o: Exo1.c hash.h
	$(CC) $(CFLAGS) -c Exo1.c

main_test.o: main_test.c memory.h hash.h
	$(CC) $(CFLAGS) -c main_test.c

clean:
	rm -f *.o test_memory

.PHONY: all clean