# Pre-compiler and Compiler flags
CXX_FLAGS := -fno-stack-protector -ffreestanding -fno-stack-check  -fno-rtti -fno-exceptions -nostartfiles -D__x86_64__ -I inc -fPIC
PRE_FLAGS := -MMD -MP

# Project directory structure
BIN := bin
SRC := src
LIB := lib
INC := include
MAINFILE := $(SRC)/kernel.cpp

# Build directories and output
TARGET := $(BIN)/kernel.elf
BUILD := build

# Library search directories and flags
EXT_LIB :=
LDFLAGS := -nostdlib -z max-page-size=0x1000 -Ttext=0x01000000 -T kernel.ld -z noexecstack
LDPATHS := $(addprefix -L,$(LIB) $(EXT_LIB))

# Include directories
INC_DIRS := $(INC) $(shell find $(SRC) -type d) 
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# Construct build output and dependency filenames
SRCS := $(shell find $(SRC) -name *.cpp)
ASM_SRCS := $(shell find $(SRC) -name *.asm)
OBJS := $(subst $(SRC)/,$(BUILD)/,$(addsuffix .o,$(basename $(SRCS))))
SRCS += $(shell find $(SRC) -name *.asm)

OBJS += $(subst $(SRC)/,$(BUILD)/,$(addsuffix _asm.o,$(basename $(ASM_SRCS))))
DEPS := $(OBJS:.o=.d)

build: clean all

# Main task
all: $(TARGET)

# Task producing target from built files
$(TARGET): $(OBJS)
	echo Linking
	mkdir -p $(dir $@)
	ld $(LDFLAGS) --no-relax -o $@ $(OBJS)

# Compile all cpp files
$(BUILD)/%.o: $(SRC)/%.cpp
	echo Compiling $^
	mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) $(INC_FLAGS) -c -o $@ $< $(LDPATHS) $(LDFLAGS)

$(BUILD)/%_asm.o: $(SRC)/%.asm
	echo Compiling $^
	mkdir -p $(dir $@)
	nasm $^ -f elf64 -o $@

$(BUILD)/cpu/smp_asm.o: $(SRC)/cpu/smp.asm
	echo Compiling $^
	mkdir -p $(dir $@)
	nasm $^ -f elf64 -o $@

# Interrupt files have to have different flags
$(BUILD)/int/isr.o: $(SRC)/int/isr.cpp
	echo Compiling interrupt file $^
	mkdir -p $(dir $@)
	$(CXX) -mno-red-zone -mgeneral-regs-only -ffreestanding -fPIC $(INC_FLAGS) -c -o $@ $< $(LDPATHS) $(LDFLAGS)

# Clean task
.PHONY: clean
clean:
	rm -rf build