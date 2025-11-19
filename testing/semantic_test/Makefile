CC = gcc
CFLAGS = -g -std=c11 -pedantic -Wall -Wextra -I../../include

TARGET = semantic_test

$(TARGET): functionTable.o semantic.o symtab.o test.o
	$(CC) $(CFLAGS) functionTable.o semantic.o symtab.o test.o -o $(TARGET)

functionTable.o: ../../src/functionTable.c ../../include/functionTable.h
	$(CC) $(CFLAGS) -c ../../src/functionTable.c -o functionTable.o

semantic.o: ../../src/semantic.c ../../include/semantic.h
	$(CC) $(CFLAGS) -c ../../src/semantic.c -o semantic.o

symtab.o: ../../src/symtable.c ../../include/symtable.h
	$(CC) $(CFLAGS) -c ../../src/symtable.c -o symtab.o

test.o: semantic_test.c
	$(CC) $(CFLAGS) -c semantic_test.c -o test.o

run:
	make && ./$(TARGET)

clean:
	-rm -f *.o $(TARGET)
