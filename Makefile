# Compiler and compilation flags
CC := gcc
CFLAGS := -Wall -Werror -march=native -std=c99

# Source file and executable name
SOURCE := butterworth.c
EXECUTABLE := butterworth

# Default target to build the executable
all: $(EXECUTABLE)

# Compile the source file into an executable, with the given flags and libraries
$(EXECUTABLE): $(SOURCE)
	$(CC) $(CFLAGS) $< -o $@  -lm

# Target to clean up generated files
clean:
	rm -f $(EXECUTABLE)
