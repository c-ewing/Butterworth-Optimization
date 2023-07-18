/*
REFERENCES:
[1] https://tttapa.github.io/Pages/Mathematics/Systems-and-Control-Theory/Digital-filters/Discretization/Discretization-of-a-fourth-order-Butterworth-filter.html
[2] https://en.wikipedia.org/wiki/Butterworth_filter
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Constants for Butterworth filter
#define ORDER 2
#define SAMPLING_RATE 22 * 1000   // Hertz
#define CUTOFF_FREQUENCY 2 * 1000 // Hertz

typedef struct FilterCoefficients
{
    // Numerator coefficients
    double b0, b1, b2;
    // Denominator coefficients
    double a0, a1, a2;

    // Previous input
    double x1, x2;

    // Previous output
    double y1, y2;
} ButterworthFilter;

// Function to initialize Butterworth filter
void butterworthFilterInit(ButterworthFilter *filter)
{
    /*
    These are the steps outlined in the reference [1] to calculate the coefficients.
    It includes warping of the frequencies to the discrete time domain.
    ```
        // Calculate the cutoff frequency, CUTOFF_FREQUENCY is in Hz
        double wc = 2 * M_PI * CUTOFF_FREQUENCY;
        // Warp the cutoff frequency for the discrete time domain
        double wc_d = wc / SAMPLING_RATE;
        // Calculate the analog cuttoff frequency
        double wc_a = 2 * tan(wc_d / 2) * SAMPLING_RATE;
        // Print the cutoff frequencies:
        printf("wc: %0.8f, wc_d: %0.8f, wc_a: %0.8f\n", wc, wc_d, wc_a);
        // Calculate lambda (as defined in reference [1])
        double lambda = 2 * SAMPLING_RATE / wc_a;
        printf("lambda: %0.8f\n", lambda);
    ```
    */

    // This is all of the code in the comment above, but optimized to remove the intermediate variables
    double lambda = 1.0 / tan(M_PI * CUTOFF_FREQUENCY / SAMPLING_RATE);
    printf("lambda: %0.8f\n", lambda);

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
    printf("b0: %0.8f, b1: %0.8f, b2: %0.8f\n", filter->b0, filter->b1, filter->b2);
    printf("a0: %0.8f, a1: %0.8f, a2: %0.8f\n", filter->a0, filter->a1, filter->a2);

    // Initialize the previous input and output values to zero
    filter->x1 = 0.0;
    filter->x2 = 0.0;
    filter->y1 = 0.0;
    filter->y2 = 0.0;
}

// Function to apply Butterworth filter
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

// Transform the output of the Butterworth filter from a double to a uint16_t
uint16_t butterworthOutputAdjustment(double input)
{
    if (input > 1.0)
    {
        return 65535;
    }
    else if (input < -1.0)
    {
        return 0;
    }
    else
    {
        return (uint16_t)((input + 1.0) * 32767.5);
    }
}

int main(int argc, char *argv[])
{
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
    double *inputBuffer = (double *)malloc(numSamples * sizeof(double));
    double *outputBuffer = (double *)malloc(numSamples * sizeof(double));

    // Read input samples from file
    uint16_t sample;
    for (size_t i = 0; i < numSamples; ++i)
    {
        if (fscanf(inputFile, "%hu", &sample) != 1)
        {
            printf("Error reading input sample at line %zu\n", i + 1);
            return 1;
        } // TODO: Temporary, remove this when the filter is converted to fixed point
        else
        {
            inputBuffer[i] = sample / 65535.0;
        }
    }

    // Initialize the filter
    ButterworthFilter ButterworthFilter;
    butterworthFilterInit(&ButterworthFilter);

    // Apply Butterworth filter
    for (size_t i = 0; i < numSamples; ++i)
    {
        outputBuffer[i] = butterworthFilterApply(&ButterworthFilter, inputBuffer[i]);
        // View all of the output samples
        // if (outputBuffer[i] != 0.0)
        // {
        //     printf("outputBuffer[%zu]: %0.8f uint16_t value: %d\n", i, outputBuffer[i], butterworthOutputAdjustment(outputBuffer[i]));
        // }
    }

    // Write output samples to file
    for (size_t i = 0; i < numSamples; ++i)
    {
        fprintf(outputFile, "%hu\n", butterworthOutputAdjustment(outputBuffer[i]));
    }

    // Cleanup
    fclose(inputFile);
    fclose(outputFile);
    free(inputBuffer);
    free(outputBuffer);

    return 0;
}