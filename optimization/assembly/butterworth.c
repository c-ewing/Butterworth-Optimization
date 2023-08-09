/*
REFERENCES:
[1]     https://tttapa.github.io/Pages/Mathematics/Systems-and-Control-Theory/Digital-filters/Discretization/Discretization-of-a-fourth-order-Butterworth-filter.html
[2]     https://en.wikipedia.org/wiki/Butterworth_filter
[3,4]   https://www.youtube.com/watch?v=HJ-C4Incgpw AND https://github.com/curiores/ArduinoTutorials/blob/main/BasicFilters/Design/LowPass/ButterworthFilter.ipynb
[5]     https://sourceforge.net/projects/fixedptc/
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "fixedpoint.h"

// Constants for Butterworth filter
#define ORDER 2
#define SAMPLING_RATE 22 * 1000   // Hertz [0, 65535]
#define CUTOFF_FREQUENCY 2 * 1000 // Hertz [0, 65535], must be less than half the sampling rate

// Structure to hold Butterworth filter coefficients
typedef struct FilterCoefficients
{
    // Numerator coefficients
    fixedpoint_t b0, b1, b2;
    // Denominator coefficients
    fixedpoint_t a0, a1, a2;

    // Previous input
    fixedpoint_t x1, x2;

    // Previous output
    fixedpoint_t y1, y2;
} ButterworthFilter;

// Function to initialize Butterworth filter
void butterworthFilterInit(ButterworthFilter *filter)
{
    // TODO: Switch to a lookup table rather than a calculated value for the specific case
    // double lambda = 1.0 / tan(M_PI * CUTOFF_FREQUENCY / SAMPLING_RATE);
    fixedpoint_t lambda = fixedpoint_from_real(3.40568723888925); // TODO: Hardcoded the value for now

#ifdef DEBUG
    printf("lambda:\t%s\n", fixedpoint_str(lambda));
#endif

    // Calculate the coefficients
    // All of these are adjusted so that a0 is normalized to 1.0, a0 is calculated first and then used to scale all the other coefficients.
    // These coefficients are usually divided by a0 but thats slow so instead 1/a0 is calculated and multiplied.
    // double inv_a0 = 1.0 / (pow(lambda, 2) + sqrt(2.0) * lambda + 1.0);
    fixedpoint_t lambda_squared = fixedpoint_mul(lambda, lambda);
    fixedpoint_t sqrt2_lambda = fixedpoint_mul(FIXEDPOINT_SQRT2, lambda);
    fixedpoint_t a0 = lambda_squared + sqrt2_lambda + FIXEDPOINT_ONE;
    fixedpoint_t inv_a0 = fixedpoint_div(FIXEDPOINT_ONE, a0);

#ifdef DEBUG
    printf("inv_a0:\t%s\n", fixedpoint_str(inv_a0));
#endif

    // Calculate the coefficients:
    filter->a0 = FIXEDPOINT_ONE; // Due to the normalizing around a0, this is always 1.0
    // a1 = (-2.0 * pow(lambda, 2) + 2.0) * inv_a0;
    filter->a1 = fixedpoint_mul((fixedpoint_mul(fixedpoint_from_int(-2), lambda_squared) + FIXEDPOINT_TWO), inv_a0);
    // a2 = (pow(lambda, 2) - sqrt(2.0) * lambda + 1.0) * inv_a0;
    filter->a2 = fixedpoint_mul((lambda_squared - sqrt2_lambda + FIXEDPOINT_ONE), inv_a0);

    // These are fixed for a second order Butterworth filter using the bilinear transform and adjusted using the value of a0.
    filter->b0 = inv_a0;
    // b1 = 2.0 * inv_a0
    filter->b1 = fixedpoint_mul(FIXEDPOINT_TWO, inv_a0);
    filter->b2 = inv_a0;

// Print the coefficients
#ifdef DEBUG
    printf("b0:\t%s\n", fixedpoint_str(filter->b0));
    printf("b1:\t%s\n", fixedpoint_str(filter->b1));
    printf("b2:\t%s\n", fixedpoint_str(filter->b2));

    printf("a0:\t%s\n", fixedpoint_str(filter->a0));
    printf("a1:\t%s\n", fixedpoint_str(filter->a1));
    printf("a2:\t%s\n", fixedpoint_str(filter->a2));
#endif

    // Initialize the previous input and output values to zero
    filter->x1 = FIXEDPOINT_ZERO;
    filter->x2 = FIXEDPOINT_ZERO;
    filter->y1 = FIXEDPOINT_ZERO;
    filter->y2 = FIXEDPOINT_ZERO;
}

uint16_t fixedpoint_to_uint16(fixedpoint_t input)
{
    // Adjust the range from [0,65535] of the input to [-32727, 32727] of the output
    fixedpoint_t adjusted = fixedpoint_div(input, FIXEDPOINT_TWO);
    fixedpoint_t scaled = adjusted + fixedpoint_from_int(32767);

#ifdef DEBUG
    printf("fp val:\t%s\n", fixedpoint_str(input));
    printf("adj:\t%s\n", fixedpoint_str(adjusted));
    printf("uint16:\t%s\n", fixedpoint_str(scaled));
#endif
    return fixedpoint_to_int(scaled);
}

int main(int argc, char *argv[])
{
    printf("Applying Butterworth Filter\n");
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
            numSamples++;
        }
    }
    numSamples++; // Add one for the last line
    // Reset the file pointer to the beginning of the file
    fseek(inputFile, 0, SEEK_SET);

    // Allocate memory for input and output buffers
    fixedpoint_t *inputBuffer = (fixedpoint_t *)malloc(numSamples * sizeof(fixedpoint_t));
    fixedpoint_t *outputBuffer = (fixedpoint_t *)malloc(numSamples * sizeof(fixedpoint_t));

    // Read input samples from file
    uint16_t sample;
    for (size_t i = 0; i < numSamples; i++)
    {
        if (fscanf(inputFile, "%hu", &sample) != 1)
        {
            printf("Error reading input sample at line %zu\n", i + 1);
            return 1;
        }
        else
        {
            inputBuffer[i] = fixedpoint_from_int(sample);
        }
    }

    // Initialize the filter
    ButterworthFilter f;
    butterworthFilterInit(&f);

    // Apply Butterworth filter
    for (size_t i = 0; i < numSamples; i++)
    {
        fixedpoint_t input_value = inputBuffer[i];
        fixedpoint_t output;

        asm volatile(
            // Assembly code
            "ldr r4, [%[input_ptr]]        \n\t" // Load inputBuffer[i] into r4
            "ldr r5, [%[filter], #0]      \n\t"  // Load f.b0 into r5
            "smull r6, r7, r4, r5         \n\t"  // Multiply r4 and r5; result in r6:r7
            "mov r8, r7, lsr #15          \n\t"  // r8 = (f.b0 * inputBuffer[i]) >> 15

            "ldr r5, [%[filter], #4]      \n\t"
            "ldr r4, [%[filter], #8]      \n\t"
            "smull r6, r7, r4, r5         \n\t"
            "add r8, r8, r7, lsr #15      \n\t" // Add (f.b1 * f.x1) >> 15 to r8

            "ldr r5, [%[filter], #12]     \n\t"
            "ldr r4, [%[filter], #16]     \n\t"
            "smull r6, r7, r4, r5         \n\t"
            "add r8, r8, r7, lsr #15      \n\t" // Add (f.b2 * f.x2) >> 15 to r8

            "ldr r5, [%[filter], #20]     \n\t"
            "ldr r4, [%[filter], #24]     \n\t"
            "smull r6, r7, r4, r5         \n\t"
            "sub r8, r8, r7, lsr #15      \n\t" // Subtract (f.a1 * f.y1) >> 15 from r8

            "ldr r5, [%[filter], #28]     \n\t"
            "ldr r4, [%[filter], #32]     \n\t"
            "smull r6, r7, r4, r5         \n\t"
            "sub r8, r8, r7, lsr #15      \n\t" // Subtract (f.a2 * f.y2) >> 15 from r8

            // Update the previous input and output values
            "ldr r4, [%[filter], #8]      \n\t"
            "str r4, [%[filter], #16]     \n\t" // Store r4 into f.x2

            "str %[input_value], [%[filter], #8] \n\t" // Store input_value into f.x1

            "ldr r4, [%[filter], #24]     \n\t"
            "str r4, [%[filter], #32]     \n\t" // Store r4 into f.y2

            "str r8, [%[filter], #24]     \n\t" // Store r8 (output) into f.y1
            "str r8, [%[output_ptr]]      \n\t" // Store r8 into outputBuffer[i]

            // Outputs
            : [output] "=r"(output)
            // Inputs
            : [filter] "r"(&f), [input_ptr] "r"(&inputBuffer[i]), [output_ptr] "r"(&outputBuffer[i]), [input_value] "r"(input_value)
            // Clobbered registers
            : "r4", "r5", "r6", "r7", "r8");
    }

    // Write output samples to file
    for (size_t i = 0; i < numSamples; ++i)
    {
        fprintf(outputFile, "%hu\n", fixedpoint_to_uint16(outputBuffer[i]));
    }

    printf("Finished Applying Butterworth Filter\n");

    // Cleanup
    fclose(inputFile);
    fclose(outputFile);
    free(inputBuffer);
    free(outputBuffer);

    return 0;
}