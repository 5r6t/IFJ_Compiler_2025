#//////////////////////////////////////////////
#// filename: Makefile (for codegen_test)    //
#// IFJ_prekladac	varianta - vv-BVS   	 //
#// Authors:						  		 //
#//  * Jaroslav Mervart (xmervaj00) / 5r6t 	 //
#//  * Veronika Kubová (xkubovv00) / Veradko //
#//  * Jozef Matus (xmatusj00) / karfisk 	 //
#//  * Jan Hájek (xhajekj00) / Wekk 		 //
#//////////////////////////////////////////////

# ---- paths ----
BUILD_DIR      := ../../build
SRC_DIR        := ../../src
INCLUDE_DIR    := ../../include
TEST_SRC_DIR   := .
OUT_DIR        := $(BUILD_DIR)/test_out

# ---- compiler settings ----
CC             := gcc
CFLAGS         := -std=c11 -Wall -Wextra -D_GNU_SOURCE -g -DDEBUG
INCLUDES       := -I$(INCLUDE_DIR) -I$(TEST_SRC_DIR)

# ---- files ----
TARGET         := $(BUILD_DIR)/test_codegen
SRC            := $(TEST_SRC_DIR)/codegen_test.c \
                   $(SRC_DIR)/codegen.c \
                   $(SRC_DIR)/common.c \

# ---- default ----
all: run

# ---- build target ----
$(TARGET): $(SRC)
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OUT_DIR)
	@echo "Building codegen test binary..."
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC) -o $(TARGET)
	@echo "Build complete -> $(TARGET)"

# ---- run target ----
run: $(TARGET)
	@bash $(TEST_SRC_DIR)/lex_test.sh | tee $(OUT_DIR)/lex_summary.txt

# ---- cleanup ----
clean:
	@echo "Cleaning build artifacts..."
	@rm -f $(TARGET)
	@rm -rf $(OUT_DIR)

.PHONY: all run clean
