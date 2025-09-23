#//////////////////////////////////////////////
#// filename: Makefile	                   	 //
#// IFJ_prekladac	varianta - vv-BVS   	 //
#// Authors:						  		 //
#//  * Jaroslav Mervart (xmervaj00) / 5r6t 	 //
#//  * Veronika Kubová (xkubovv00) / Veradko //
#//  * Jozef Matus (xmatusj00) / karfisk 	 //
#//  * Jan Hájek (xhajekj00) / Wekk 		 //
#//////////////////////////////////////////////
# Makefile

OBJDIR = build
CC = gcc
CFLAGS = -g -std=c11 -pedantic -Wall -Wextra -Werror -Iinclude -DDEBUG

TEST_TARGET = test 
TEST_SRC = testing/test.c
TEST_OBJ = $(OBJDIR)/test.o
LEX_OBJ = $(OBJDIR)/lex.o
COMMON_OBJ = $(OBJDIR)/common.o

.PHONY: all clean

$(OBJDIR)/$(TEST_TARGET): $(TEST_OBJ) $(LEX_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) $(TEST_OBJ) $(LEX_OBJ) $(COMMON_OBJ) -o $(OBJDIR)/$(TEST_TARGET)

all: $(OBJDIR)/$(TEST_TARGET)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/common.o: src/common.c include/common.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c src/common.c -o $(OBJDIR)/common.o

$(OBJDIR)/lex.o: src/lex.c src/common.c include/lex.h include/common.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c src/lex.c -o $(OBJDIR)/lex.o

$(OBJDIR)/test.o: testing/test.c include/lex.h include/common.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c testing/test.c -o $(OBJDIR)/test.o

clean:
	-rm -f $(OBJDIR)/*.o $(TEST_TARGET)

simple_test: 
	echo "Ifj Ifj . identif  identif" | ./build/test 0 /dev/stdin # Run lexer test with input from stdin !!! DOESN'T ACCEPT NEWLINES !!!

test:
	./build/test 0 build/test0 2> build/test_out && diff --brief build/test_out build/test_correct