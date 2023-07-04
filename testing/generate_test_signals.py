import argparse
import matplotlib.pyplot as plt
import numpy as np


def write_signal_file(file_path, signal):
    signal.astype(np.uint16).tofile(file_path, sep='\n')


# Constants:
FILE_PREFIX = 'test_signal_'
FILE_SUFFIX = '.dat'
SAMPLING_FREQUENCY = 22 * 1_000  # 10 kHz
NUM_SAMPLES = SAMPLING_FREQUENCY * 1  # 1 second
MIN_INTENSITY = 0  # 0 intensity
MAX_INTENSITY = 65535  # 1 intensity
RAMP_AMOUNT = 0.125  # 12.5% of the total number of samples
IMPLUSE_POSITION = 0.5  # Halfway through the samples


def generate_dc_test_signal():

    # Create the DC test signal:

    # DC test signal consists of a constant value of max_frequency for all samples after a ramp up period.
    # The ramp up period consists of floor(0.125 * num_samples) of the total number of samples and starts at min_frequency.
    # There is at least one sample at min_frequency.
    num_ramp_up_samples = int(np.floor(RAMP_AMOUNT * NUM_SAMPLES)) + 1

    # Create the numpy array of samples.
    dc_samples = np.zeros(NUM_SAMPLES, dtype=np.uint16)

    # Ramp up the DC signal.
    dc_samples[:num_ramp_up_samples] = np.linspace(
        MIN_INTENSITY, MAX_INTENSITY, num_ramp_up_samples, dtype=np.uint16)

    # Set the DC signal to maximum intensity.
    dc_samples[num_ramp_up_samples:] = MAX_INTENSITY

    # Write the DC test signal to a file.
    write_signal_file(FILE_PREFIX + 'dc' + FILE_SUFFIX, dc_samples)


def generate_nyquist_test_signal():
    # Create the Nyquist test signal:

    # Nyquist test signal consists of alternating max and min intensity values.
    # Create the numpy array of samples.
    nyquist_samples = np.zeros(NUM_SAMPLES, dtype=np.uint16)

    # Set the Nyquist signal to alternating max and min intensity values.
    nyquist_samples[::2] = MAX_INTENSITY
    nyquist_samples[1::2] = MIN_INTENSITY

    # Write the Nyquist test signal to a file.
    write_signal_file(FILE_PREFIX + 'nyquist' + FILE_SUFFIX, nyquist_samples)


def generate_half_nyquist_test_signal():
    # Create the Half Nyquist test signal:

    # Create the numpy array of samples.
    half_nyquist_samples = np.zeros(NUM_SAMPLES, dtype=np.uint16)

    # Set the Nyquist signal to alternating max and min intensity values.
    half_nyquist_samples[::2] = (MAX_INTENSITY-MIN_INTENSITY) / 2
    half_nyquist_samples[1::4] = MAX_INTENSITY
    half_nyquist_samples[3::4] = MIN_INTENSITY

    # Write the Half Nyquist test signal to a file.
    write_signal_file(FILE_PREFIX + 'half_nyquist' +
                      FILE_SUFFIX, half_nyquist_samples)


def generate_quarter_nyquist_test_signal():
    # Create the Quarter Nyquist test signal:

    # Create the numpy array of samples.
    quarter_nyquist_samples = np.zeros(NUM_SAMPLES, dtype=np.uint16)

    # Set the Nyquist signal to alternating max and min intensity values.
    quarter_nyquist_samples[::4] = (MAX_INTENSITY-MIN_INTENSITY) / 2
    quarter_nyquist_samples[1::8] = (MAX_INTENSITY-MIN_INTENSITY) * 3 / 4
    quarter_nyquist_samples[2::8] = MAX_INTENSITY
    quarter_nyquist_samples[3::8] = (MAX_INTENSITY-MIN_INTENSITY) * 3 / 4
    quarter_nyquist_samples[5::8] = (MAX_INTENSITY-MIN_INTENSITY) / 4
    quarter_nyquist_samples[6::8] = MIN_INTENSITY
    quarter_nyquist_samples[7::8] = (MAX_INTENSITY-MIN_INTENSITY) / 4

    # Write the Quarter Nyquist test signal to a file.
    write_signal_file(FILE_PREFIX + 'quarter_nyquist' +
                      FILE_SUFFIX, quarter_nyquist_samples)


def generate_impulse_test_signal():
    # Create the Impulse test signal:

    # Ensure the impulse position is within the bounds of the samples.
    position = np.clip(
        int(IMPLUSE_POSITION * NUM_SAMPLES), 0, NUM_SAMPLES-1)

    # Impulse test signal consists of a single sample at max intensity occurring at the midpoint of the samples.
    impulse_samples = np.zeros(NUM_SAMPLES, dtype=np.uint16)
    impulse_samples[::1] = MIN_INTENSITY
    impulse_samples[position] = MAX_INTENSITY

    # Write the Impulse test signal to a file.
    write_signal_file(FILE_PREFIX + 'impulse' + FILE_SUFFIX, impulse_samples)


def main():
    global SAMPLING_FREQUENCY
    global NUM_SAMPLES
    global MIN_INTENSITY
    global MAX_INTENSITY
    global RAMP_AMOUNT
    global IMPLUSE_POSITION

    parser = argparse.ArgumentParser(
        description="Generate samples with optional arguments.")
    parser.add_argument("--sample-rate", type=int,
                        default=10000, help="Sample rate (default: 10_000)")
    parser.add_argument("--num-samples", type=int, default=10000,
                        help="Number of samples (default: 10000)")
    parser.add_argument("--min-intensity", type=int, default=0,
                        help="Minimum intensity (default: 0)")
    parser.add_argument("--max-intensity", type=int,
                        default=65535, help="Maximum intensity (default: 65535)")
    parser.add_argument("--dc-ramp", type=float,
                        default=0.125, help="Percentage of the DC samples to ramp [0,1] (default: 0.125)")
    parser.add_argument("--impluse-position", type=float,
                        default=0.5, help="Location of the impulse signal in the samples [0,1] (default: 0.5)")

    args = parser.parse_args()

    # Perform validation or default value assignment if needed.
    if args.sample_rate <= 0:
        print("Error: Sample rate must be greater than 0.")
        return

    SAMPLING_FREQUENCY = args.sample_rate

    if args.num_samples <= 0:
        print("Error: Number of samples must be greater than 0.")
        return

    NUM_SAMPLES = args.num_samples

    if args.min_intensity > args.max_intensity:
        print("Error: Minimum intensity cannot be greater than maximum intensity.")
        return

    MIN_INTENSITY = args.min_intensity
    MAX_INTENSITY = args.max_intensity
    RAMP_AMOUNT = args.dc_ramp
    IMPLUSE_POSITION = args.impluse_position

    generate_dc_test_signal()
    generate_nyquist_test_signal()
    generate_half_nyquist_test_signal()
    generate_quarter_nyquist_test_signal()
    generate_impulse_test_signal()
    print(
        f'Generated test signals successfully:')
    print(f'Sample Rate: {SAMPLING_FREQUENCY} Hz, Number of Samples: {NUM_SAMPLES}, Minimum Intensity: {MIN_INTENSITY}, Maximum Intensity: {MAX_INTENSITY}, Ramp Amount: {RAMP_AMOUNT}, Impulse Position: {IMPLUSE_POSITION}')


if __name__ == "__main__":
    main()
