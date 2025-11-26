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
CFLAGS   := -g -std=c11 -pedantic -Wall -Wextra -Werror -Iinclude -I. -I.. -DDEBUG
SRCDIR   := src
INCDIR   := include
TESTDIR  := testing
TARGET   := ifj25

CC       := gcc

# Detect whether repo or flat dir
SRCDIR   := $(if $(wildcard src),src,.)
INCDIR   := $(if $(wildcard include),include,.)

CFLAGS   := -g -std=c11 -pedantic -Wall -Wextra -Werror -I$(INCDIR) -DDEBUG

TARGET   := ifj25

# ---- sources ----
SRC      := $(wildcard $(SRCDIR)/*.c)

# Object files:  src/foo.c  → foo.o
OBJ      := $(patsubst $(SRCDIR)/%.c,%.o,$(SRC))

# ---- rules ----
.PHONY: all clean

all: $(TARGET)

# Generic rule for both repo/flat:
# - repo:     src/foo.c → foo.o
# - zip:      ./foo.c   → foo.o
%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)
	$(RM) $(OBJ)

clean:
	$(RM) $(OBJ) $(TARGET)

