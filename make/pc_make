#!/usr/bin/env make

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

LIBGBA := $(DEVKITPRO)/libgba

# Pre-compiler and Compiler flags
CXX_FLAGS := -Wall -O2 -ggdb
PRE_FLAGS := -MMD -MP

# Project directory structure
BIN := bin
SRC := source
LIB := lib
INC := include
DATA := data
MUSIC := audio
MAINFILE := $(SRC)/main.cpp

# Build directories and output
TARGET := $(BIN)/main
BUILD := build

# Library search directories and flags
EXT_LIB :=
LDFLAGS := -lTilengine -lSDL2 -lSDL2_mixer
LDPATHS := $(addprefix -L,$(LIB) $(EXT_LIB))

# Include directories
INC_DIRS := $(INC) $(shell find $(SRC) -type d) $(BUILD) $(DATA) $(LIBGBA)/include
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# Construct build output and dependency filenames
SRCS := $(shell find $(SRC) -name *.cpp)
BINFILES := $(shell find $(DATA) -name '*.bin')

ifneq ($(strip $(MUSIC)),)
	export AUDIOFILES := $(foreach dir,$(notdir $(wildcard $(MUSIC)/*.*)),$(CURDIR)/$(MUSIC)/$(dir))
	BINFILES += $(BUILD)/soundbank.bin
endif

OBJS_BIN := $(subst $(DATA)/,$(BUILD)/,$(addsuffix .o,$(BINFILES)))

OBJS_SOURCES := $(subst $(SRC)/,$(BUILD)/,$(addsuffix .o,$(basename $(SRCS))))

HFILES := $(subst $(DATA)/,$(BUILD)/,$(addsuffix .h,$(subst .,_,$(BINFILES))))

OBJS := $(OBJS_SOURCES) $(OBJS_BIN)

DEPS := $(OBJS:.o=.d)

# # Run task
# run: build
# 	@echo "Executing..."
# 	./$(TARGET) $(ARGS)

# Build task
build: all

# Main task


all: $(TARGET)

# Task producing target from built files
$(TARGET): $(OBJS)
	@echo "Building..."
	@mkdir -p $(dir $@)
	@$(CXX) $(OBJS) -o $@ $(LDPATHS) $(LDFLAGS)

$(BUILD)/soundbank.bin $(BUILD)/soundbank.h : $(AUDIOFILES)
#---------------------------------------------------------------------------------
	@mmutil $^ -o$(BUILD)/soundbank.bin -h$(BUILD)/soundbank.h

# Compile all cpp files
$(BUILD)/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	@echo $(notdir $<)
	@$(CXX) $(CXX_FLAGS) $(PRE_FLAGS) $(INC_FLAGS) -c -o $@ $< $(LDPATHS) $(LDFLAGS)

$(BUILD)/%.bin.o $(BUILD)/%_bin.h : $(DATA)/%.bin
	@mkdir -p $(dir $@)
	@tools/bin2s -a 4 -H `(echo $(BUILD)/$(<F) | tr . _)`.h $< | $(AS) -o $(BUILD)/$(<F).o

$(BUILD)/%.bin.o $(BUILD)/%_bin.h : $(BUILD)/%.bin
	@mkdir -p $(dir $@)
	@tools/bin2s -a 4 -H `(echo $(BUILD)/$(<F) | tr . _)`.h $< | $(AS) -o $(BUILD)/$(<F).o

$(OBJS_SOURCES) : $(HFILES)
# @echo "$<"

# Clean task
.PHONY: clean
clean:
	@echo "Cleaning..."
	rm -rf build

help:
	@echo "bin: $(HFILES)"

# Include all dependencies
-include $(DEPS)

