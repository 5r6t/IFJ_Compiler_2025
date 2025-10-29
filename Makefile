#//////////////////////////////////////////////
#// filename: Makefile	                   	 //
#// IFJ_prekladac	varianta - vv-BVS   	 //
#// Authors:						  		 //
#//  * Jaroslav Mervart (xmervaj00) / 5r6t 	 //
#//  * Veronika Kubová (xkubovv00) / Veradko //
#//  * Jozef Matus (xmatusj00) / karfisk 	 //
#//  * Jan Hájek (xhajekj00) / Wekk 		 //
#//////////////////////////////////////////////
# ---- settings ----
CC       := gcc
CFLAGS   := -g -std=c11 -pedantic -Wall -Wextra -Werror -Iinclude -DDEBUG
OBJDIR   := build
SRCDIR   := src
INCDIR   := include
TESTDIR  := testing
TARGET   := $(OBJDIR)/test

# ---- sources ----
SRC      := $(wildcard $(SRCDIR)/*.c)
OBJ      := $(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TEST_SRC := $(wildcard $(TESTDIR)/*.c)
TEST_OBJ := $(TEST_SRC:$(TESTDIR)/%.c=$(OBJDIR)/%.o)

# ---- rules ----
.PHONY: all clean test

all: $(TARGET)

$(OBJDIR):
	mkdir -p $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(TESTDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ) $(TEST_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

test: $(TARGET)
	./$(TARGET) 0 $(OBJDIR)/test.txt

clean:
	$(RM) -r $(OBJDIR)/*.o 