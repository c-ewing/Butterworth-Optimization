
import matplotlib.pyplot as plt
import scipy.signal as signal
import numpy as np
import argparse

# CONSTANTS:
FILE_PREFIX = 'ts_'
FILE_SUFFIX = '.dat'
SAMPLING_FREQUENCY = 22 * 1_000  # 22 kHz
NUM_SAMPLES = SAMPLING_FREQUENCY  # SAMPLING_FREGUENCY samples
CUTOFF_FREQUENCY = 2000  # 2 kHz


def butterworth_filter(signal_data, cutoff_freq=2000, sampling_rate=22000, order=2):
    b, a = signal.butter(order, cutoff_freq, btype='low',
                         output='ba', fs=sampling_rate)
    # print(b[0], b[1], b[2], a[0], a[1], a[2])
    filtered_signal = signal.lfilter(b, a, signal_data)
    return filtered_signal


def main():
    global SAMPLING_FREQUENCY
    global CUTOFF_FREQUENCY

    parser = argparse.ArgumentParser(
        description="Analyze filter with optional arguments.")
    parser.add_argument("input_samples", type=str,
                        help="Path to unmodified sample data")
    parser.add_argument("filtered_samples", type=str,
                        help="Path to filtered sample data")
    parser.add_argument("--frequency-cutoff", type=int,
                        default=2000, help="Frequency for -3db (default: 2_000)")
    parser.add_argument("--sample-rate", type=int,
                        default=22000, help="Sample rate (default: 22_000)")

    parser.add_argument("--output", type=str, default="",
                        help="Output file path(default: none)")

    args = parser.parse_args()

    # Load the signals from files.
    # Test sine wave signal:
    sine_wave = np.fromfile(
        args.input_samples, dtype=np.uint16, sep='\n')

    # Filtered sine wave signal:

    filtered_sine_wave = np.fromfile(
        args.filtered_samples, dtype=np.uint16, sep='\n')

    # Adjust for the rezeroing of the signal:
    filtered_sine_wave = (filtered_sine_wave.astype(np.int32) - 32768) * 2

    # Reference filtering:
    scipy_filtered_sine = butterworth_filter(
        sine_wave, cutoff_freq=CUTOFF_FREQUENCY, sampling_rate=SAMPLING_FREQUENCY, order=2)

    # Plot the signals:

    # Plot the sine wave:
    plt.plot(sine_wave, label='Input Sine wave', color='green')
    plt.plot(filtered_sine_wave, label='Filtered sine', color='red')
    plt.plot(scipy_filtered_sine, label='Scipy filtered sine', color='blue')

    plt.xlim(0, SAMPLING_FREQUENCY * 0.005)

    plt.title('Filtering applied to a noisy sine wave')
    plt.xlabel('Sample Number')
    plt.ylabel('Intensity')
    plt.legend()

    if (args.output == ""):
        plt.show()
    else:
        plt.savefig(f'{args.output}', bbox_inches='tight')


if __name__ == "__main__":
    main()
