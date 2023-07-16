#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3
#endif

// Set SAMPLE_RATE and CUTOFF_FREQ to the desired values to 22kHz and 2kHz respectively
#define SAMPLE_RATE 22000
#define CUTOFF_FREQ 2000

// Set the fixed-point shift for the filter coefficients
#define FIXED_POINT_SHIFT 12
#define FIXED_POINT_MULTIPLIER ((uint16_t)1 << FIXED_POINT_SHIFT)

typedef struct {
    uint16_t a1;
    uint16_t a2;
    uint16_t b0;
    uint16_t b1;
    uint16_t b2;
    uint16_t x1;
    uint16_t x2;
    uint16_t y1;
    uint16_t y2;
} ButterworthFilter;

// Initialize the filter coefficients
void initFilter(ButterworthFilter* filter) {
    uint32_t omega = (2U * M_PI * CUTOFF_FREQ * FIXED_POINT_MULTIPLIER) / SAMPLE_RATE;
    uint32_t c = FIXED_POINT_MULTIPLIER / tan(omega);
    uint32_t c2 = (c * c) / FIXED_POINT_MULTIPLIER;

    filter->a1 = (uint16_t)((2U * (FIXED_POINT_MULTIPLIER - c2)) / (FIXED_POINT_MULTIPLIER + c * (uint32_t)(sqrt(2.0) * FIXED_POINT_MULTIPLIER) + c2));
    filter->a2 = (uint16_t)(-(FIXED_POINT_MULTIPLIER - c * (uint32_t)(sqrt(2.0) * FIXED_POINT_MULTIPLIER) + c2) / (FIXED_POINT_MULTIPLIER + c * (uint32_t)(sqrt(2.0) * FIXED_POINT_MULTIPLIER) + c2));
    filter->b0 = filter->b2 = (uint16_t)FIXED_POINT_MULTIPLIER;
    filter->b1 = (uint16_t)(2U * FIXED_POINT_MULTIPLIER);
    filter->x1 = filter->x2 = filter->y1 = filter->y2 = 0U;
}

// Apply the filter to a sample
uint16_t applyFilter(ButterworthFilter* filter, uint16_t input) {
    uint32_t scaledInput = (uint32_t)input << FIXED_POINT_SHIFT; // Scale up input

    uint32_t output = (filter->b0 * scaledInput + filter->b1 * filter->x1 + filter->b2 * filter->x2
                       - filter->a1 * filter->y1 - filter->a2 * filter->y2) / FIXED_POINT_MULTIPLIER;

    filter->x2 = filter->x1;
    filter->x1 = (uint16_t)(scaledInput >> FIXED_POINT_SHIFT); // Scale down input
    filter->y2 = filter->y1;
    filter->y1 = (uint16_t)output;

    return (uint16_t)(output >> FIXED_POINT_SHIFT); // Scale down output
}


int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s input_file output_file\n", argv[0]);
        return 1;
    }

    const char* inputFilepath = argv[1];
    const char* outputFilepath = argv[2];

    FILE* inputFile = fopen(inputFilepath, "r");
    if (inputFile == NULL) {
        printf("Failed to open input file: %s\n", inputFilepath);
        return 1;
    }

    FILE* outputFile = fopen(outputFilepath, "w");
    if (outputFile == NULL) {
        printf("Failed to open output file: %s\n", outputFilepath);
        fclose(inputFile);
        return 1;
    }

    ButterworthFilter filter;
    initFilter(&filter);

    uint16_t inputSample, outputSample;

    // Filtering loop
    while (fscanf(inputFile, "%hu", &inputSample) != EOF) {
        outputSample = applyFilter(&filter, inputSample);
        fprintf(outputFile, "%hu\n", outputSample);
    }

    fclose(inputFile);
    fclose(outputFile);

    printf("Filtering complete.\n");
    return 0;
}
