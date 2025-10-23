CC = gcc
CFLAGS = -g -std=c11 -pedantic -Wall -Wextra -I../../include

TARGET = symtable_test

$(TARGET): symtab.o test.o
	$(CC) $(CFLAGS) symtab.o test.o -o $(TARGET)

symtab.o: ../../src/symtable.c ../../include/symtable.h
	$(CC) $(CFLAGS) -c ../../src/symtable.c -o symtab.o

test.o: symtab_test.c ../../include/symtable.h
	$(CC) $(CFLAGS) -c symtab_test.c -o test.o

run:
	make && ./$(TARGET) keywords.txt keywords2.txt

clean:
	-rm -f *.o $(TARGET) out.txt
