import subprocess

# TARGET_DIRECTORY:
#   Directory where the benchmarked program is located.
TARGET = ""
EXECUTABLE_NAME = "butterworth"

# Compiler Configuration
COMPILER = "gcc"
COMPILER_FLAGS = ["-Wall", "-Werror", "-march=native",
                  "-msoft-float", "-std=c99", "-pedantic"]
COMPILER_DEBUG_SYMBOLS = ["-g"]
OPTIMIZATION_FLAGS = ["O0", "O1", "O2", "O3", "Os", "Ofast"]

# Hyperfine Configuration
HYPERFINE = "hyperfine"
# "--show-output",
HYPERFINE_FLAGS = ["--warmup", "5", "--runs",
                   "25", "-L", "flag", "O0,O1,O2,O3,Os,Ofast", "--export-markdown"]
HYPERFINE_COMMAND = [
    f"./{TARGET}{EXECUTABLE_NAME}_{{flag}} ts_sine.dat {TARGET}removeme_{{flag}}.dat"]

# Callgrind Configuration:
# TODO: Add callgrind configuration

# Test signal configuration
SIGNAL_GEN = "python3"
SIGNAL_FLAGS = ["testing/generate_test_signals.py", "--sample-rate",
                "22000", "--num-samples", "44000"]

# First we need to generate the test signals.
print("Generating test signals")
subprocess.run([SIGNAL_GEN, *SIGNAL_FLAGS])

# Second we need to compile the program with different optimization flags and benchmark it using hyperfine.
# Hyperfine will generate a markdown file with the results.
for flag in OPTIMIZATION_FLAGS:
    # Compile the program with the current optimization flag using subprocess
    print(f"Compiling {TARGET} with {flag}")
    # print(COMPILER, *COMPILER_FLAGS,
    #   f"-{flag}", "-o", f"{TARGET}{EXECUTABLE_NAME}_{flag}", f"{TARGET}{EXECUTABLE_NAME}.c")
    subprocess.run([COMPILER, *COMPILER_FLAGS,
                   f"-{flag}", "-o", f"{TARGET}{EXECUTABLE_NAME}_{flag}", f"{TARGET}{EXECUTABLE_NAME}.c"])

# Run the benchmark using hyperfine
print(f"Benchmarking {TARGET}")
subprocess.run([HYPERFINE, *HYPERFINE_FLAGS,
                f"{TARGET}{EXECUTABLE_NAME}_perf.md", *HYPERFINE_COMMAND])

# Clean up the executables and signal data
print(f"Cleaning up {TARGET}")
subprocess.run(["rm", f"ts_impulse.dat", f"ts_sine.dat"])
for flag in OPTIMIZATION_FLAGS:
    subprocess.run(["rm", f"{TARGET}removeme_{flag}.dat"])
    subprocess.run(["rm", f"{TARGET}{EXECUTABLE_NAME}_{flag}"])

# Finally use Callgrind to find instructions executed and the largest contributor to execution time.
# TODO: Use Callgrind to find instructions executed and the largest contributor to execution time.
