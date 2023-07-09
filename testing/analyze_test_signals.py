import matplotlib.pyplot as plt
from scipy.fftpack import fft
import scipy.signal as signal
import argparse
import numpy as np


# CONSTANTS:
SAMPLING_FREQUENCY = 22 * 1_000  # 10 kHz
CUTOFF_FREQUENCY = 2000  # 2 kHz
MIN_INTENSITY = 0  # 0 intensity
MAX_INTENSITY = 65535  # 1 intensity


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
    parser.add_argument("--sample-rate", type=int,
                        default=22000, help="Sample rate (default: 22_000)")
    parser.add_argument("--min-intensity", type=int, default=0,
                        help="Minimum intensity (default: 0)")
    parser.add_argument("--max-intensity", type=int,
                        default=65535, help="Maximum intensity (default: 65535)")

    args = parser.parse_args()

    # Frequency Analysis of Impulse Signal after Low-Pass Filtering:

    # Read in impulse signal
    unfiltered_samples = np.fromfile(
        args.input_samples, dtype=np.uint16, sep='\n')
    filtered_samples = np.fromfile(
        args.filtered_samples, dtype=np.uint16, sep='\n')

    if (unfiltered_samples.shape != filtered_samples.shape):
        print(
            f'Error, input samples mismatched with filtered samples: Expected {unfiltered_samples.shape} but found {filtered_samples.shape}')
        return -1

    # Calculate the FFT of the impulse signal
    unfiltered_fft = fft(unfiltered_samples)
    filtered_fft = fft(filtered_samples)

    # Calculate frequency values to plot
    frequencies_impulse = np.linspace(0, unfiltered_samples.size *
                                      SAMPLING_FREQUENCY / unfiltered_samples.size / 2, unfiltered_samples.size // 2)

    frequencies_filtered_impulse = np.linspace(0, filtered_samples.size *
                                               SAMPLING_FREQUENCY / filtered_samples.size / 2, filtered_samples.size // 2)

    # Calculate the amplitude values to plot
    amplitudes_impulse = np.abs(unfiltered_fft)[
        :unfiltered_samples.size // 2]
    amplitudes_filtered_impulse = np.abs(filtered_fft)[
        :filtered_samples.size // 2]

    # Plot the frequency spectrum of the impulse signal
    plt.plot(frequencies_impulse, amplitudes_impulse,
             label='Impulse Signal', color='blue', alpha=0.7)
    plt.plot(frequencies_filtered_impulse, amplitudes_filtered_impulse,
             label='Filtered Impulse Signal', color='red', alpha=0.7)
    plt.axvline(CUTOFF_FREQUENCY, color='black',
                linestyle='--', label='Cutoff Frequency', alpha=0.5)
    plt.title('Impulse Signal (Frequency Domain)')
    plt.xscale('log')
    plt.xlabel('Frequency (Hz)')
    plt.ylabel('Amplitude')
    plt.grid(True)
    plt.legend()

    # Calculate the Decibel change of the impulse signal at the cutoff frequency
    # First find the frequency closest to the cutoff frequency
    closest_frequency_index = np.abs(
        frequencies_filtered_impulse - CUTOFF_FREQUENCY).argmin()

    decibel_change = 20 * \
        np.log10(amplitudes_filtered_impulse[closest_frequency_index] /
                 amplitudes_impulse[closest_frequency_index])

    plt.annotate(f'{decibel_change:0.2f}db @ {CUTOFF_FREQUENCY}hz', xy=(CUTOFF_FREQUENCY, amplitudes_filtered_impulse[closest_frequency_index]), xytext=(CUTOFF_FREQUENCY + 100, amplitudes_filtered_impulse[closest_frequency_index] + 0.1), arrowprops=dict(
        arrowstyle='->', connectionstyle='arc3'), fontsize=10)
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()
