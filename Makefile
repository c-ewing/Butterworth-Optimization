# Compiler and compilation flags
CC := gcc
# -g enable debug information
CFLAGS := -Wall -Werror -march=native -std=c99 -O0
PROFILEFLAGS := -g 

# Source file and executable name
SOURCE := butterworth.c
EXECUTABLE := butterworth

# Default target to build the executable
all: $(EXECUTABLE)

# Compile the source file into an executable, with the given flags and libraries
$(EXECUTABLE): $(SOURCE)
	$(CC) $(CFLAGS) $< -o $@ -lm

# Target for debugging the executable
debug: $(SOURCE)
	$(CC) $(CFLAGS) $(PROFILEFLAGS) $< -o $(EXECUTABLE)_debug -lm

# Callgrind the executable
callgrind: debug
	valgrind --tool=callgrind --dump-instr=yes ./$(EXECUTABLE)_debug test_files/test_signal_impulse.dat removeme.dat
	callgrind_annotate --auto=yes callgrind.out.* > performance_report.txt

# Target to clean up generated files
clean:
	rm -f $(EXECUTABLE) $(EXECUTABLE)_debug removeme.dat cachegrind.out.* callgrind.out.* performance_report.txt

.PHONY: all debug callgrind clean
