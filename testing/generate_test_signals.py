import argparse
import numpy as np


def write_signal_file(file_path, signal):
    signal.astype(np.uint16).tofile(file_path, sep='\n')


# Constants:
FILE_PREFIX = 'ts_'
FILE_SUFFIX = '.dat'
SAMPLING_FREQUENCY = 22 * 1_000  # 10 kHz
NUM_SAMPLES = SAMPLING_FREQUENCY * 1  # 1 second
MIN_INTENSITY = 0  # 0 intensity
MAX_INTENSITY = 65535  # 1 intensity

IMPLUSE_POSITION = 0.5  # Halfway through the samples

CARRIER_FREQUENCY = 500  # 500 Hz
CARRIER_AMPLITUDE = 0.9  # 90% of the max intensity
NOISE_FREQUENCY = 3_000  # 3 kHz
NOISE_AMPLITUDE = 0.1  # 10% of the max intensity


def generate_impulse_test_signal():
    # Create the Impulse test signal:
    global SAMPLING_FREQUENCY
    global NUM_SAMPLES
    global MIN_INTENSITY
    global MAX_INTENSITY

    global IMPLUSE_POSITION

    global CARRIER_FREQUENCY
    global CARRIER_AMPLITUDE
    global NOISE_FREQUENCY
    global NOISE_AMPLITUDE

    # Ensure the impulse position is within the bounds of the samples.
    position = np.clip(
        int(IMPLUSE_POSITION * NUM_SAMPLES), 0, NUM_SAMPLES-1)

    # Impulse test signal consists of a single sample at max intensity occurring at the midpoint of the samples.
    impulse_samples = np.zeros(NUM_SAMPLES, dtype=np.uint16)
    impulse_samples[::1] = MIN_INTENSITY
    impulse_samples[position] = MAX_INTENSITY

    # Write the Impulse test signal to a file.
    write_signal_file(FILE_PREFIX + 'impulse' + FILE_SUFFIX, impulse_samples)


def generate_sine_test_signal():
    # Create the Sine test signal:
    global SAMPLING_FREQUENCY
    global NUM_SAMPLES
    global MIN_INTENSITY
    global MAX_INTENSITY

    global IMPLUSE_POSITION

    global CARRIER_FREQUENCY
    global CARRIER_AMPLITUDE
    global NOISE_FREQUENCY
    global NOISE_AMPLITUDE

    # Sine test signal consists of a carrier signal with noise added.
    time_points = np.linspace(0, 1 / CARRIER_FREQUENCY *
                              2, NUM_SAMPLES, endpoint=False)
    carrier_samples = CARRIER_AMPLITUDE * MAX_INTENSITY/2 * \
        np.sin(2 * np.pi * CARRIER_FREQUENCY * time_points)
    noise_samples = NOISE_AMPLITUDE * MAX_INTENSITY/2 * \
        np.sin(2 * np.pi * NOISE_FREQUENCY * time_points)

    result_samples = carrier_samples + noise_samples
    # Shift the signal up by half the max intensity to get the signal in the range [0, MAX_INTENSITY]
    result_samples = np.add(result_samples, MAX_INTENSITY/2)

    # Scale the signal to the range [MIN_INTENSITY, MAX_INTENSITY]
    result_samples = np.divide(result_samples, MAX_INTENSITY) * \
        (MAX_INTENSITY - MIN_INTENSITY) + MIN_INTENSITY

    # Write the Sine test signal to a file.
    write_signal_file(FILE_PREFIX + 'sine' + FILE_SUFFIX,
                      result_samples.astype(np.uint16))


def main():
    global SAMPLING_FREQUENCY
    global NUM_SAMPLES
    global MIN_INTENSITY
    global MAX_INTENSITY

    global IMPLUSE_POSITION

    global CARRIER_FREQUENCY
    global CARRIER_AMPLITUDE
    global NOISE_FREQUENCY
    global NOISE_AMPLITUDE

    parser = argparse.ArgumentParser(
        description="Generate samples with optional arguments.")
    parser.add_argument("--sample-rate", type=int,
                        default=22_000, help="Sample rate (Default: 22_000)")
    parser.add_argument("--num-samples", type=int, default=10_000,
                        help="Number of samples (Default: 10_000)")
    parser.add_argument("--min-intensity", type=int, default=0,
                        help="Minimum intensity (Default: 0)")
    parser.add_argument("--max-intensity", type=int,
                        default=65_535, help="Maximum intensity (Default: 65_535)")
    parser.add_argument("--impluse-position", type=float,
                        default=0.5, help="Location of the impulse signal in the samples [0,1] (Default: 0.5)")
    parser.add_argument("--carrier-frequency", type=int,
                        default=500, help="Sine Carrier frequency (Default: 500)")
    parser.add_argument("--carrier-amplitude", type=float, default=0.9,
                        help="Sine Carrier amplitude [0,1] (Default: 0.9)")
    parser.add_argument("--noise-frequency", type=int,
                        default=3_000, help="Sine Noise frequency (Default: 3_000)")
    parser.add_argument("--noise-amplitude", type=float, default=0.1,
                        help="Sine Noise amplitude [0,1] (Default: 0.1)")
    parser.add_argument("-q", "--quiet", action="store_true",
                        default=False, help="Suppress output (Default: False)")

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
    IMPLUSE_POSITION = args.impluse_position

    # Generate the test signals.
    generate_impulse_test_signal()
    generate_sine_test_signal()

    if not args.quiet:
        print(f'Generated test signals successfully:')
        print(
            f'\tSample Rate: {SAMPLING_FREQUENCY} Hz, Number of Samples: {NUM_SAMPLES}, Minimum Intensity: {MIN_INTENSITY}, Maximum Intensity: {MAX_INTENSITY}')
        print(f'\tImpulse Position: {IMPLUSE_POSITION}')
        print(
            f'\tCarrier Frequency: {CARRIER_FREQUENCY} Hz, Carrier Amplitude: {CARRIER_AMPLITUDE}, Noise Frequency: {NOISE_FREQUENCY} Hz, Noise Amplitude: {NOISE_AMPLITUDE}')


if __name__ == "__main__":
    main()
