# Commands
CC=/usr/local/Cellar/llvm/11.0.0_1/bin/clang
RM=rm -rf
MKDIR=mkdir -p

# Directories
SRC=src
BUILD=out
TEST=test

# Files
EXECUTABLE=main
SOURCE_FILES=$(SRC)/main.c $(SRC)/i8080.c

# Flags
CC_FLAGS=-std=c11

# Test Binaries
CPUDIAG=cpudiag

all: clean $(EXECUTABLE)
	@./$(BUILD)/$(EXECUTABLE)

$(EXECUTABLE): $(BUILD)
	@$(CC) $(SOURCE_FILES) -o $(BUILD)/$(EXECUTABLE) $(CC_FLAGS)

$(BUILD):
	@$(MKDIR) $(BUILD)

clean:
	@$(RM) $(BUILD)

# Test Targets
$(CPUDIAG): clean $(EXECUTABLE)
	@./$(BUILD)/$(EXECUTABLE) $(TEST)/$(CPUDIAG).bin
