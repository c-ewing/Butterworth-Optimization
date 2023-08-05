# Setup:

## Virtual Machine:

The version of Valgrind available in the Fedora Repositories is too old to be used for this project. To install the latest version of Valgrind from source run the following commands:
```bash
dnf install -y wget tar lbzip2 # Install wget and tar (and the bzip2 library), used to get and unpack the valgrind source

wget https://sourceware.org/pub/valgrind/valgrind-3.21.0.tar.bz2 # Download the valgrind source, 3.21.0 is the latest version at the time of writing
tar -xvf valgrind-3.21.0.tar.bz2 # Unpack the source

cd valgrind-3.21.0 # Change directory into the unpacked source
./configure # Configure the source for the host machine
make # Build the source
make install # Install the built binaries
```

## Host Machine:

**Analysis of the test results cannot be done on the VM as the available NumPy, MatPlotLib, and SciPy versions are too old. To run the analysis scripts you will need to install more recent versions of these libraries.**

If you do not have pip installed, install it using:
```bash
python3 -m ensurepip --upgrade
```

Install the required dependencies using pip:
```bash
pip3 install -r requirements.txt
```

Generated assembly was viewed using [KCacheGrind](https://github.com/KDE/kcachegrind), which is part of the [KDE](https://kde.org/) project. Install KCacheGrind to gain additional insight from the callgrind reports. KCacheGrind installation varies by system, installing the latest from source is recommended.


# Building:
To build the project run:
```bash
make
```
This will build the project and generate the executable `butterworth`. The executable takes two arguments, the input file and the output file. The input file is the signal to be filtered and the output file is the filtered signal.

This project is built with the following flags:
- `-Wall` Enable all warnings
- `-Werror` Treat warnings as errors
- `-march=native` Enable all CPU extensions available on the host machine
- `-msoft-float` Disable floating point hardware support, the code should not contain any floating point operations, this is a safety measure.
- `std=c99 -pedantic` Use the C99 standard and enforce strict standards compliance
- `-O0` No optimization

# Testing:
To test the Butterworth filter implementation first generate test files using either `generate_signals.ipynb` or `generate_test_signals.py`. Both generate the same test files, `generate_test_signals.py` is intended for automated testing as all options are configurable from the command line.

Command line arguments:
- `--sample-rate [int]` The sample rate of the sample files (Default: 22kHz)
- `--num-samples [int]` The number of samples to generate (Default: 10_000)
- `--minimum-intensity [int]` The minimum intensity value possible in the sample files (Default: 0)
- `--maximum-intensity [int]` The maximum intensity value possible in the sample files (Default: 65535)
- `--impulse-position [float]` Location of the impulse signal in the samples [0,1] (Default: 0.5)
- `--carrier-frequency [int]` The frequency of the carrier signal (Default: 500Hz)
- `--carrier-amplitude [float]` The amplitude of the carrier signal [0,1] (Default: 0.9)
- `--noise-frequency [int]` The frequency of the noise signal (Default: 3kHz)
- `--noise-amplitude [float]` The amplitude of the noise signal [0,1] (Default: 0.1)
- `-q` Quiet mode, do not print any information to the console

## Generated Signals
The scripts both generate two test signals: An Impulse signal and a sine wave with noise that will be filtered out. 

### Impulse:
The most important signal for this project is the Impulse signal as it is used to test the frequency response of the filter.

### Sine Wave:
Used to show visually that the filter is successfully suppressing noise past the cutoff frequency. 

# Analysis:
Frequency response analysis is completed either in `analyze_frequency_response.[py|ipynb]` both provide the same functionality. The `.py` file is intended for automated testing. Both generate a graph comparing the FFT of the input signal, filtered signal, and a reference filter implementation provided by SciPy. Plotted on the chart is also the gain at the cutoff frequency. This allows verification of hitting -3 decibels at the cutoff frequency.

## `analyze_frequency_response.py`
Takes five command line arguments:
- Input sample file (signal before filter processing)
- Output sample file (signal after the filtering is applied)
- `--frequency-cutoff [int]` The cutoff frequency (Default: 2kHz)
- `--sample-rate [int]` The sample rate of the sample files (Default: 22kHz)
- `--minimum-intensity [int]` The minimum intensity value possible in the sample files (Default: 0)
- `--maximum-intensity [int]` The maximum intensity value possible in the sample files (Default: 65535)
- `--output [string]` The output file name, if not provided the output will be displayed in a window

After the program has completed a window will open with a frequency response chart. The chart has the magnitude (intensity/amplitude) of the signal on the vertical axis and the frequency of the signal on the horizontal axis.

# Performance analysis:
Performance analysis was performed using the [callgrind](https://valgrind.org/docs/manual/cl-manual.html) tool within [valgrind](https://valgrind.org/). 

Performance is tested against `testing/test_signal_impulse.dat` which can be generated using `generate_signals.py`. For all results in this projects `10_000` samples are used. A performance report is generated by running the command: `make callgrind`, which automatically builds the binary with debug symbols, generates the performance report, and interprets the report into `performance_report.txt`.

The tool [KCacheGrind](https://github.com/KDE/kcachegrind) was used to retrieve the number of instructions generated by the compiler. Using the `--dump-instr=yes` option for callgrind, the source code is annotated with the each assembly instructions generated for each line of code. This allows us to determine where the each instruction comes from for further optimization.

To view the report within KCacheGrind, open the generated `callgrind.out.*` file after running `make callgrind`

# Optimizations Attempted:
The main source of optimization is likely to occur in the `fixedpoint.h` library that we wrote. Applying the filter requires a large number of fixed point operations, so optimizing this library will have the largest impact on performance. 
GCC Optimization Flags
Firstly no code changes were made and the compiler was used to optimize the code. The following flags were used:
- `-O0` No optimization, used as a baseline
- `-O1` Basic optimization
- `-O2` More advanced optimization
- `-O3` All optimizations, at the cost of binary size
- `-Os` Optimize for binary size, included to see if reducing the number of instructions would improve performance
- `-Ofast` All optimizations, including unsafe optimizations that break standards compliance

## 1. Operator Strength Reduction
Care was taken while writing the filter to avoid division operations. In this hand optimization pass further attempts were made to replace multiplication and division with less expensive operations. This optimization was tested with all optimization flags noted above.

Line 70 of butterworth.c was changed from:
```c
    filter->b1 = fixedpoint_mul(FIXEDPOINT_TWO, inv_a0);
```
to:
```c
    filter->b1 = inv_a0 << 1;
```
This removes a multiplication by 2 operation and replaces it with a bit shift.

Line 111 of butterworth.c was changed from:
```c
    fixedpoint_t adjusted = fixedpoint_div(input, FIXEDPOINT_TWO);
```
to:
```c
    fixedpoint_t adjusted = input >> 1;
```
This removes a division by 2 operation and replaces it with a bit shift. This optimization is valid because the underlying type of fixedpoint_t is a signed integer. Arithmetic right shift is used for signed integers, which preserves the sign bit.

## 2. Inline Functions
The `fixedpoint.h` library is used extensively throughout the code, so inlining the functions should improve performance. This was done by adding the `inline` keyword to the function declarations in `fixedpoint.h`. The `inline` keyword was also added to `butterworthFilterApply()` and `butterworthFilterApply()` This optimization was tested with all optimization flags noted above.

## 3. Macro Functions
Instead of using functions, macros can be used to move work to compile time. This was done by replacing the function declarations in `fixedpoint.h` with macros. This optimization was tested with all optimization flags noted above.

## 4. Loop Counter size optimization:
The loop counter in `butterworth.c` is used to index into the sample array. The loop counter is an `size_t` which is 64 bits on the target architecture. However we assume the sample array is only 32 bits, so the loop counter can be reduced to a `uint32_t` which is 32 bits and can fit in one register. This optimization was tested with all optimization flags noted above.

## 5. Loop Unrolling
Loop unrolling is a technique that reduces the overhead of looping by reducing the number of iterations. This was done by unrolling the loops in `butterworth.c` into groupings of 2, 3, and 4 samples per loop. This optimization was tested with all optimization flags noted above.

Memory Aliasing? Localize variables? Unaligned memory access(unlikely)? Register spillage? 


## Techniques not attempted or noted:
- Boolean Expression Simplification: The code was written to be as simple as possible and does not contain complex boolean expressions or branching. Therefore this optimization was not explored any further.
- Constant Folding, Sub Expression Elimination: Not explored as the compiler should be able to do this automatically (-O1+).
- Dead Code Elimination: Not explored as the compiler will complain if there is dead code (-Wall -Werror).
- Loop Fission / Fusion: Not explored as the hot loop is extraordinarily simple and only contains one function call.