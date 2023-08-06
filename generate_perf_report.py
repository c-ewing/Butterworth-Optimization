import subprocess

# This script will generate a performance report for the Butterworth filter.
# It will compile the program with different optimization flags and benchmark it using hyperfine.

# TARGET_DIRECTORY:
#   Directory where the benchmarked program is located.
TARGET = ""
EXECUTABLE_NAME = "butterworth"

# Compiler Configuration
COMPILER = "gcc"
COMPILER_FLAGS = ["-Wall", "-Werror", "-march=native",
                  "-std=c99", "-pedantic"]
COMPILER_DEBUG_SYMBOLS = ["-g"]
OPTIMIZATION_FLAGS = ["O0", "O1", "O2", "O3", "Os", "Ofast"]

# Hyperfine Configuration
HYPERFINE = "hyperfine"
HYPERFINE_RUNS = "15"
HYPERFINE_FLAGS = ["--warmup", "3", "--runs",
                   HYPERFINE_RUNS, "-L", "flag", "O0,O1,O2,O3,Os,Ofast", "--export-markdown", f"{TARGET}{EXECUTABLE_NAME}_perf.md"]
HYPERFINE_COMMAND = [
    f"./{TARGET}{EXECUTABLE_NAME}_{{flag}} ts_sine.dat {TARGET}removeme_{{flag}}.dat"]

# Callgrind Configuration:
CALLGRIND = "valgrind"
CALLGRIND_FLAGS = ["-q", "--tool=callgrind",
                   "--dump-instr=yes"]

CALLGRIND_ANNOTATE = "callgrind_annotate"
CALLGRIND_ANNOTATE_FLAGS = ["--auto=yes",
                            "--show-percs=yes"]


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
    subprocess.run([COMPILER, *COMPILER_FLAGS,
                   f"-{flag}", "-o", f"{TARGET}{EXECUTABLE_NAME}_{flag}", f"{TARGET}{EXECUTABLE_NAME}.c"])

# Run the benchmark using hyperfine
print(f"Benchmarking target: {TARGET}")
subprocess.run([HYPERFINE, *HYPERFINE_FLAGS, *HYPERFINE_COMMAND])

# Clean up the executables and signal data
print(f"Cleaning benchmarking step for {TARGET}\n")
for flag in OPTIMIZATION_FLAGS:
    subprocess.run(["rm", f"{TARGET}removeme_{flag}.dat"])
    subprocess.run(["rm", f"{TARGET}{EXECUTABLE_NAME}_{flag}"])

# Finally use Callgrind to find instructions executed and the largest contributor to execution time.
# This will generate a callgrind.out file that can be viewed using kcachegrind.
print(f"Running Callgrind on target: {TARGET}")
for flag in OPTIMIZATION_FLAGS:
    # Compile the program with the current optimization flag and debug symbols using subprocess
    # print(f"Compiling DEBUG {TARGET} with {flag}")
    subprocess.run([COMPILER, *COMPILER_FLAGS, *COMPILER_DEBUG_SYMBOLS,
                   f"-{flag}", "-o", f"{TARGET}{EXECUTABLE_NAME}_{flag}", f"{TARGET}{EXECUTABLE_NAME}.c"])

    # Next we need to run the program with callgrind and generate the callgrind.out file
    print(f"Callgrind on: {TARGET} with {flag}")
    subprocess.run([CALLGRIND, *CALLGRIND_FLAGS, f"--callgrind-out-file={TARGET}{EXECUTABLE_NAME}_{flag}.out",
                   f"./{TARGET}{EXECUTABLE_NAME}_{flag}", "ts_sine.dat", f"{TARGET}removeme_{flag}.dat"], stdout=subprocess.DEVNULL)

    # Finally we need to run callgrind_annotate to generate a report
    # print(f"Callgrind Annotate {TARGET} for {flag}")
    # Open a file to redirect the output of callgrind_annotate into. Same as using ">" in bash.
    with open(f"{TARGET}{EXECUTABLE_NAME}_{flag}.txt", "w") as f:
        subprocess.run([CALLGRIND_ANNOTATE, *CALLGRIND_ANNOTATE_FLAGS,
                       f"{TARGET}{EXECUTABLE_NAME}_{flag}.out"], stdout=f)

    # Clean up the executables
    print(f"Cleaning callgrind step for {TARGET} with flag {flag}\n")
    subprocess.run(["rm", f"{TARGET}removeme_{flag}.dat",
                   f"{TARGET}{EXECUTABLE_NAME}_{flag}"])

# Clean up the signal data
print(f"Cleaning signal data for target: {TARGET}\n")
subprocess.run(["rm", f"ts_impulse.dat", f"ts_sine.dat"])
