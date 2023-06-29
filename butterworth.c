#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265359

typedef struct {
  int b0, b1, b2;  // Numerator coefficients
  int a1, a2;      // Denominator coefficients
  int x1, x2;      // Input delay elements
  int y1, y2;      // Output delay elements
} ButterworthFilter;

void initButterworth(ButterworthFilter *filter, double cutoffFreq, double sampleRate) {
  double omegaC = tan(PI * cutoffFreq / sampleRate);
  double omegaC2 = omegaC * omegaC;
  int scale_factor = 1 << 8;  // Scaling factor for fixed-point representation

  filter->b0 = round(scale_factor / (1.0 + 2.0 * omegaC + omegaC2));
  filter->b1 = round(2.0 * filter->b0);
  filter->b2 = round(filter->b0);
  filter->a1 = round(2.0 * filter->b0 * (1.0 - omegaC2));
  filter->a2 = round(filter->b0 * (1.0 - 2.0 * omegaC + omegaC2));
  filter->x1 = 0;
  filter->x2 = 0;
  filter->y1 = 0;
  filter->y2 = 0;
}

int applyButterworth(ButterworthFilter *filter, int input) {
  int output = filter->b0 * input + filter->b1 * filter->x1 + filter->b2 * filter->x2
             - filter->a1 * filter->y1 - filter->a2 * filter->y2;

  filter->x2 = filter->x1;
  filter->x1 = input;
  filter->y2 = filter->y1;
  filter->y1 = output >> 8;  // Apply scaling factor by right shifting

  return filter->y1;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Usage: %s <input_file> <output_file>\n", argv[0]);
    return 1;
  }

  FILE *input_file, *output_file;
  int input, filtered_output;
  ButterworthFilter filter;

  // Open input and output files
  input_file = fopen(argv[1], "r");
  output_file = fopen(argv[2], "w");

  if (input_file == NULL || output_file == NULL) {
    printf("Failed to open files.\n");
    return 1;
  }

  // Initialize Butterworth filter
  double cutoffFreq = 1000.0;  // Example cutoff frequency
  double sampleRate = 5000.0;  // Example sample rate
  initButterworth(&filter, cutoffFreq, sampleRate);

  // Process input samples and apply the filter
  while (fscanf(input_file, "%d", &input) != EOF) {
    filtered_output = applyButterworth(&filter, input);
    fprintf(output_file, "%d\n", filtered_output);
  }

  // Close files
  fclose(input_file);
  fclose(output_file);

  return 0;
}
