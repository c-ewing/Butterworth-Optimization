/*
REFERENCES:
[1]     https://tttapa.github.io/Pages/Mathematics/Systems-and-Control-Theory/Digital-filters/Discretization/Discretization-of-a-fourth-order-Butterworth-filter.html
[2]     https://en.wikipedia.org/wiki/Butterworth_filter
[3,4]   https://www.youtube.com/watch?v=HJ-C4Incgpw AND https://github.com/curiores/ArduinoTutorials/blob/main/BasicFilters/Design/LowPass/ButterworthFilter.ipynb
*/

/*
FLOATING POINT FORMAT:
- 1 sign bit, 16 integer bits, 15 fractional bits
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Define Fixed point pi:
// 1 sign bits, 2 integer bits, 29 fractional bits.
// NOTE: probably way more precision than needed
#define F_PI 1686629713 // 0b0_11_00100100001111110110101010001: 0 for positive, 3 integer, 0.14159265358979323846 fractional

// Constants for Butterworth filter
#define ORDER 2
#define SAMPLING_RATE 22 * 1000   // Hertz [0, 65535]
#define CUTOFF_FREQUENCY 2 * 1000 // Hertz [0, 65535], must be less than half the sampling rate

// ## FIXED POINT FUNCTIONS ##
typedef struct FixedPoint
{
    uint32_t value;
} FixedPoint;

// Get FixedPoint sign:
uint32_t fixedPointGetSign(FixedPoint fp)
{
    return fp.value >> 31;
}

// Set FixedPoint sign:
FixedPoint fixedPointSetSign(FixedPoint fp, uint32_t sign)
{
    fp.value = (fp.value & 0x7FFFFFFF) | (sign << 31);
    return fp;
}

// Get FixedPoint integer:
uint32_t fixedPointGetInteger(FixedPoint fp)
{
    return (fp.value >> 15) & 0x0000FFFF;
}

// Set FixedPoint integer:
FixedPoint fixedPointSetInteger(FixedPoint fp, uint32_t integer)
{
    fp.value = (fp.value & 0x80007FFF) | (integer << 15);
    return fp;
}

// Get FixedPoint fractional:
uint32_t fixedPointGetFractional(FixedPoint fp)
{
    return fp.value & 0x00007FFF;
}

// Set FixedPoint fractional:
FixedPoint fixedPointSetFractional(FixedPoint fp, uint32_t fractional)
{
    fp.value = (fp.value & 0xFFFF8000) | fractional;
    return fp;
}

// Print FixedPoint value:
void fixedPointPrint(FixedPoint fp)
{
    printf("%d:%d:%d (SIF)\n", fixedPointGetSign(fp), fixedPointGetInteger(fp), fixedPointGetFractional(fp));
}

// uint16_t to FixedPoint:
FixedPoint uint16ToFixedPoint(uint16_t value)
{
    FixedPoint fp;
    fp = fixedPointSetSign(fp, 0);
    fp = fixedPointSetInteger(fp, value);
    fp = fixedPointSetFractional(fp, 0);
    return fp;
}

// FixedPoint to output uint16_t:
// This rounds the value to the nearest integer and is on the range [-32767, 32767]
// 1 sign, 15 integer
uint16_t fixedPointToUint16(FixedPoint fp)
{
    // Get the sign
    uint32_t sign = fixedPointGetSign(fp);

    // Get the integer
    uint32_t integer = fixedPointGetInteger(fp);

    // Get the fractional
    uint32_t fractional = fixedPointGetFractional(fp);

    // Round the fractional
    if (fractional >= 0x00004000)
    {
        integer += 1;
    }

    // TODO: scale the output down to 15 bits, right shift one and then round again
    if (integer & 0x00000001)
    {
        integer = integer >> 1;
        integer += 1;
    }
    else
    {
        integer = integer >> 1;
    }

    // Check for overflow
    if (integer > 32767)
    {
        integer = 32767;
    }

    // Set the sign
    if (sign)
    {
        integer = integer | 0x00008000;
    }

    return integer;
}

// Structure to hold Butterworth filter coefficients
// uint32_t is used as it is the largest single register value that can be stored on armV6l
typedef struct FilterCoefficients
{
    // Numerator coefficients
    uint32_t b0, b1, b2;
    // Denominator coefficients
    uint32_t a0, a1, a2;

    // Previous input
    uint32_t x1, x2;

    // Previous output
    uint32_t y1, y2;
} ButterworthFilter;

// Function to initialize Butterworth filter
// TODO: Convert to fixed point
void butterworthFilterInit(ButterworthFilter *filter)
{
    double lambda = 1.0 / tan(M_PI * CUTOFF_FREQUENCY / SAMPLING_RATE);
    // printf("lambda: %d\n", lambda);

    // Calculate the coefficients
    // All of these are adjusted so that a0 is normalized to 1.0, a0 is calculated first and then used to scale all the other coefficients.
    // These coefficients are usually divided by a0 but thats slow so instead 1/a0 is calculated and multiplied.
    // TODO: Optimization: pow(lambda, 2) is reused, might be good to try pulling that out into a variable
    // TODO: Optimization: sqrt(2.0) * lambda is reused, might be good to try pulling that out into a variable
    double inv_a0 = 1.0 / (pow(lambda, 2) + sqrt(2.0) * lambda + 1.0);

    // Calculate the coefficients:
    filter->a0 = 1.0; // Due to the normalizing around a0, this is always 1.0
    filter->a1 = (-2.0 * pow(lambda, 2) + 2.0) * inv_a0;
    filter->a2 = (pow(lambda, 2) - sqrt(2.0) * lambda + 1.0) * inv_a0;

    // These are fixed for a second order Butterworth filter using the bilinear transform and adjusted using the value of a0.
    // TODO: Optimization: b0 == b1
    filter->b0 = 1.0 * inv_a0;
    filter->b1 = 2.0 * inv_a0;
    filter->b2 = 1.0 * inv_a0;

    // Print the coefficients
    printf("b0: %d, b1: %d, b2: %d\n", filter->b0, filter->b1, filter->b2);
    printf("a0: %d, a1: %d, a2: %d\n", filter->a0, filter->a1, filter->a2);

    // Initialize the previous input and output values to zero
    filter->x1 = 0.0;
    filter->x2 = 0.0;
    filter->y1 = 0.0;
    filter->y2 = 0.0;
}

// Function to apply Butterworth filter
// TODO: Convert to fixed point
double butterworthFilterApply(ButterworthFilter *f, double input)
{
    // Calculate the output
    double output = (f->b0 * input + f->b1 * f->x1 + f->b2 * f->x2) - (f->a1 * f->y1 + f->a2 * f->y2);

    // Update the previous input and output values
    f->x2 = f->x1;
    f->x1 = input;

    f->y2 = f->y1;
    f->y1 = output;

    return output;
}

int main(int argc, char *argv[])
{
    printf("Starting Butterworth filter\n");
    printf("F_PI: %x\n", F_PI);
    if (argc < 3)
    {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    FILE *inputFile = fopen(argv[1], "r");
    FILE *outputFile = fopen(argv[2], "w");

    if (inputFile == NULL || outputFile == NULL)
    {
        printf("Failed to open input or output file\n");
        return 1;
    }

    // Count the number of lines in the input file
    // TODO: Optimization: dont scan the file, instead apply as reading the file
    size_t numSamples = 0;
    int c;
    while ((c = fgetc(inputFile)) != EOF)
    {
        if (c == '\n')
        {
            ++numSamples;
        }
    }
    fseek(inputFile, 0, SEEK_SET);

    // Allocate memory for input and output buffers
    // TODO: Change these to fixed point when the filter is working
    FixedPoint *inputBuffer = (FixedPoint *)malloc(numSamples * sizeof(FixedPoint));
    FixedPoint *outputBuffer = (FixedPoint *)malloc(numSamples * sizeof(FixedPoint));

    // Read input samples from file
    uint16_t sample;
    for (size_t i = 0; i < numSamples; ++i)
    {
        if (fscanf(inputFile, "%hu", &sample) != 1)
        {
            printf("Error reading input sample at line %zu\n", i + 1);
            return 1;
        }
        else
        {
            inputBuffer[i] = uint16ToFixedPoint(sample);
        }
    }

    // TODO: NEXT CONVERSION
    exit(0);

    // Initialize the filter
    // TODO: Convert to fixed point
    // ButterworthFilter ButterworthFilter;
    // butterworthFilterInit(&ButterworthFilter);

    // Apply Butterworth filter
    // for (size_t i = 0; i < numSamples; ++i)
    // {
    //     outputBuffer[i] = butterworthFilterApply(&ButterworthFilter, inputBuffer[i]);
    // }

    // Write output samples to file
    for (size_t i = 0; i < numSamples; ++i)
    {
        fprintf(outputFile, "%hu\n", fixedPointToUint16(outputBuffer[i]));
    }

    // Cleanup
    fclose(inputFile);
    fclose(outputFile);
    free(inputBuffer);
    free(outputBuffer);

    return 0;
}