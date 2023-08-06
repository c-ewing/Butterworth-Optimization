# Compiler and compilation flags
CC := gcc
# -g enable debug information
CFLAGS := -Wall -Werror -march=native -std=c99 -pedantic -O0
PROFILEFLAGS := -g 

# Source file and executable name
SOURCE := butterworth.c
EXECUTABLE := butterworth

# Default target to build the executable
all: $(EXECUTABLE)

# Compile the source file into an executable, with the given flags and libraries
$(EXECUTABLE): $(SOURCE)
	$(CC) $(CFLAGS) $< -o $@ -lm

# Target for testing the executable
test: $(EXECUTABLE)
	./$(EXECUTABLE) testing/ts_impulse.dat removeme.dat && python3 testing/analyze_frequency_response.py testing/ts_impulse.dat removeme.dat --output testing/ts_impulse


# Target for debugging the executable
debug: $(SOURCE)
	$(CC) $(CFLAGS) $(PROFILEFLAGS) $< -o $(EXECUTABLE)_debug -lm

# Callgrind the executable
callgrind: debug
	valgrind --tool=callgrind --dump-instr=yes ./$(EXECUTABLE)_debug testing/ts_impulse.dat removeme.dat
	callgrind_annotate --auto=yes --show-percs=yes callgrind.out.* > performance_report.txt

# Target to clean up generated files
clean:
	rm -f $(EXECUTABLE) $(EXECUTABLE)_debug removeme.dat cachegrind.out.* callgrind.out.* performance_report.txt

.PHONY: all debug callgrind clean test
