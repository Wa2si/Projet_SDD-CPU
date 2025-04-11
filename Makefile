CC = gcc
CFLAGS = -Wall -g

all: test_memory test_parser test_cpu

# Compilation du test de memory
test_memory: memory.o hash.o main_test_memory.o
	$(CC) $(CFLAGS) memory.o hash.o main_test_memory.o -o test_memory

# Compilation du test de parser
test_parser: parser.o memory.o hash.o main_test_parser.o
	$(CC) $(CFLAGS) parser.o memory.o hash.o main_test_parser.o -o test_parser

# Compilation du test de cpu
test_cpu: main_test_cpu.o cpu.o memory.o hash.o parser.o
	$(CC) -o test_cpu main_test_cpu.o cpu.o memory.o hash.o parser.o

hash.o: hash.c hash.h
	$(CC) $(CFLAGS) -c hash.c

memory.o: memory.c memory.h hash.h
	$(CC) $(CFLAGS) -c memory.c

parser.o: parser.c parser.h hash.h
	$(CC) $(CFLAGS) -c parser.c

main_test_memory.o: main_test_memory.c memory.h hash.h
	$(CC) $(CFLAGS) -c main_test_memory.c

main_test_parser.o: main_test_parser.c parser.h hash.h
	$(CC) $(CFLAGS) -c main_test_parser.c

main_test_cpu.o: main_test_cpu.c cpu.h memory.h hash.h parser.h
	$(CC) $(CFLAGS) -c main_test_cpu.c

clean:
	rm -f *.o test_memory test_parser

.PHONY: all clean
