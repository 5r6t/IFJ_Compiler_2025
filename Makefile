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
SRCDIR   := src
INCDIR   := include
TESTDIR  := testing
TARGET   := ifj25

# ---- sources ----
SRC      := $(wildcard $(SRCDIR)/*.c)
OBJ      := $(SRC:$(SRCDIR)/%.c=%.o)

# ---- rules ----
.PHONY: all clean test

all: $(TARGET)

%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@
	$(RM) $(OBJ)

clean:
	$(RM) $(OBJ) $(TARGET)
