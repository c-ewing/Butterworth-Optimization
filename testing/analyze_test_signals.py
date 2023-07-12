import matplotlib.pyplot as plt
from scipy.fftpack import fft
import scipy.signal as signal
import numpy as np
import argparse


# CONSTANTS:
SAMPLING_FREQUENCY = 22 * 1_000  # 22 kHz
NUM_SAMPLES = SAMPLING_FREQUENCY  # 22k samples
MIN_INTENSITY = 0  # 0 intensity
MAX_INTENSITY = 65535  # 1 intensity
CUTOFF_FREQUENCY = 2000  # 2 kHz


def butterworth_filter(signal_data, cutoff_freq=2000, sampling_rate=10000, order=2):
    b, a = signal.butter(order, cutoff_freq, btype='low',
                         output='ba', fs=sampling_rate)
    # print(b[0], b[1], b[2], a[0], a[1], a[2])
    filtered_signal = signal.lfilter(b, a, signal_data)
    return filtered_signal


def main():
    global SAMPLING_FREQUENCY
    global CUTOFF_FREQUENCY
    global MIN_INTENSITY
    global MAX_INTENSITY

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
    parser.add_argument("--min-intensity", type=int, default=0,
                        help="Minimum intensity (default: 0)")
    parser.add_argument("--max-intensity", type=int,
                        default=65535, help="Maximum intensity (default: 65_535)")

    args = parser.parse_args()

    # Assign constants
    SAMPLING_FREQUENCY = args.sample_rate
    CUTOFF_FREQUENCY = args.frequency_cutoff
    MIN_INTENSITY = args.min_intensity
    MAX_INTENSITY = args.max_intensity

    # Read in impulse signal
    unfiltered_samples = np.fromfile(
        args.input_samples, dtype=np.uint16, sep='\n')
    filtered_samples = np.fromfile(
        args.filtered_samples, dtype=np.uint16, sep='\n')

    if (unfiltered_samples.shape != filtered_samples.shape):
        print(
            f'Error, input samples mismatched with filtered samples: Expected {unfiltered_samples.shape} but found {filtered_samples.shape}')
        return -1

    # Calculate the SciPy butterworth filter of the input signal
    scipy_filtered_samples = butterworth_filter(
        unfiltered_samples, cutoff_freq=CUTOFF_FREQUENCY, sampling_rate=SAMPLING_FREQUENCY, order=2)

    # Calculate the FFT of the impulse signal
    unfiltered_fft = fft(unfiltered_samples)
    filtered_fft = fft(filtered_samples)
    scipy_fft = fft(scipy_filtered_samples)

    # Calculate frequency values to plot
    frequencies = np.linspace(0, unfiltered_samples.size *
                              SAMPLING_FREQUENCY / unfiltered_samples.size / 2, unfiltered_samples.size // 2)

    # Calculate the amplitude values to plot
    input_amplitudes = np.divide(np.abs(unfiltered_fft)[
        :unfiltered_samples.size // 2], MAX_INTENSITY)  # Normalize
    filtered_amplitudes = np.divide(np.abs(filtered_fft)[
        :filtered_samples.size // 2], MAX_INTENSITY)  # Normalize
    scipy_amplitudes = np.divide(np.abs(scipy_fft)[
        :scipy_filtered_samples.size // 2], MAX_INTENSITY)  # Normalize

    # Calculate the Decibel change of the impulse signal at the cutoff frequency
    # First find the frequency closest to the cutoff frequency
    closest_frequency_index = np.abs(
        frequencies - CUTOFF_FREQUENCY).argmin()

    decibel_change_filt = 20 * \
        np.log10(filtered_amplitudes[closest_frequency_index] /
                 input_amplitudes[closest_frequency_index])

    decibel_change_scipy = 20 * \
        np.log10(scipy_amplitudes[closest_frequency_index] /
                 input_amplitudes[closest_frequency_index])

    # Plotting Code
    plt.plot(frequencies, input_amplitudes,
             label='Input Signal', color='green', alpha=0.7)
    plt.plot(frequencies, filtered_amplitudes,
             label='Filtered Signal', color='red', alpha=0.7)
    plt.plot(frequencies, scipy_amplitudes,
             label='SciPy Filter', color='blue', alpha=0.7)

    plt.axvline(CUTOFF_FREQUENCY, color='black',
                linestyle='--', label='Cutoff Frequency', alpha=0.5)

    plt.annotate(f'{decibel_change_filt:0.2f}db @ {CUTOFF_FREQUENCY}hz', xy=(CUTOFF_FREQUENCY, filtered_amplitudes[closest_frequency_index]), xytext=(CUTOFF_FREQUENCY + 500, filtered_amplitudes[closest_frequency_index] + 0.1), arrowprops=dict(
        arrowstyle='->', connectionstyle='arc3', color='red'), fontsize=10)

    plt.annotate(f'{decibel_change_scipy:0.2f}db @ {CUTOFF_FREQUENCY}hz', xy=(CUTOFF_FREQUENCY, scipy_amplitudes[closest_frequency_index]), xytext=(CUTOFF_FREQUENCY + 500, scipy_amplitudes[closest_frequency_index] - 0.1), arrowprops=dict(
        arrowstyle='->', connectionstyle='arc3', color='blue'), fontsize=10)

    plt.title('Filtered Signal (Frequency Domain)')
    # plt.xscale('log')
    plt.xlabel('Frequency (Hz)')
    plt.ylabel('Amplitude')
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()
