#include "FFTProcessor.h"

FFTProcessor::FFTProcessor() :
    fft(fftOrder),
    window(fftSize + 1, juce::dsp::WindowingFunction<float>::WindowingMethod::hann, false)
{
    // Note that the window is of length `fftSize + 1` because JUCE's windows
    // are symmetrical, which is wrong for overlap-add processing. To make the
    // window periodic, set size to 1025 but only use the first 1024 samples.
    for (int i = 0; i < 20; i++) {
        avg_roundtable[i] = 0.0f;
    }
}

void FFTProcessor::reset()
{
    count = 0;
    pos = 0;

    // Zero out the circular buffers.
    std::fill(inputFifo.begin(), inputFifo.end(), 0.0f);
    std::fill(outputFifo.begin(), outputFifo.end(), 0.0f);
}

void FFTProcessor::processBlock(float* data, int numSamples, bool bypassed)
{
    for (int i = 0; i < numSamples; ++i) {
        data[i] = processSample(data[i], bypassed);
    }
}

float FFTProcessor::processSample(float sample, bool bypassed)
{
    // Push the new sample value into the input FIFO.
    inputFifo[pos] = sample;

    // Read the output value from the output FIFO. Since it takes fftSize
    // timesteps before actual samples are read from this FIFO instead of
    // the initial zeros, the sound output is delayed by fftSize samples,
    // which we will report as our latency.
    float outputSample = outputFifo[pos];

    // Once we've read the sample, set this position in the FIFO back to
    // zero so we can add the IFFT results to it later.
    outputFifo[pos] = 0.0f;

    // Advance the FIFO index and wrap around if necessary.
    pos += 1;
    if (pos == fftSize) {
        pos = 0;
    }

    // Process the FFT frame once we've collected hopSize samples.
    count += 1;
    if (count == hopSize) {
        count = 0;
        processFrame(bypassed);
    }

    return outputSample;
}

void FFTProcessor::processFrame(bool bypassed)
{
    const float* inputPtr = inputFifo.data();
    float* fftPtr = fftData.data();

    // Copy the input FIFO into the FFT working space in two parts.
    std::memcpy(fftPtr, inputPtr + pos, (fftSize - pos) * sizeof(float));
    if (pos > 0) {
        std::memcpy(fftPtr + fftSize - pos, inputPtr, pos * sizeof(float));
    }

    // Apply the window to avoid spectral leakage.
    window.multiplyWithWindowingTable(fftPtr, fftSize);

    if (!bypassed) {
        // Perform the forward FFT.
        fft.performRealOnlyForwardTransform(fftPtr, true);

        // Do stuff with the FFT data.
        processSpectrum(fftPtr, numBins);

        // Perform the inverse FFT.
        fft.performRealOnlyInverseTransform(fftPtr);
    }

    // Apply the window again for resynthesis.
    window.multiplyWithWindowingTable(fftPtr, fftSize);

    // Scale down the output samples because of the overlapping windows.
    for (int i = 0; i < fftSize; ++i) {
        fftPtr[i] *= windowCorrection;
    }

    // Add the IFFT results to the output FIFO.
    for (int i = 0; i < pos; ++i) {
        outputFifo[i] += fftData[i + fftSize - pos];
    }
    for (int i = 0; i < fftSize - pos; ++i) {
        outputFifo[i + pos] += fftData[i];
    }
}

struct Bin
{
    std::complex<float> c;
    int i;
};

void FFTProcessor::processSpectrum(float* data, int numBins)
{
    // The spectrum data is floats organized as [re, im, re, im, ...]
    // but it's easier to deal with this as std::complex values.
    auto* cdata = reinterpret_cast<std::complex<float>*>(data);

    auto *vdata = new Bin[numBins];
    for (int i = 0; i < numBins; i++) {
        auto &bin = vdata[i];
        bin.c = cdata[i];
        bin.i = i;
    }

    // sort by peaks
    std::sort(vdata, vdata + numBins, [&](const Bin &a, const Bin &b)
    {
        return std::abs(a.c) > std::abs(b.c);
    });

    const int no_peaks = 20;
    
    auto get_freq = [&](int i) {
        constexpr float hz_mult = 48000.0f / fftSize;
        if (i == 0 || i == numBins - 1) return std::max(i * hz_mult, 1.0f);

        auto divisor = (std::abs(cdata[i - 1]) - 2.0f * std::abs(cdata[i]) + std::abs(cdata[i + 1]));
        if (std::abs(divisor) <= 0.0001f) return std::max(i * hz_mult, 1.0f);
        auto delta = 0.5f * (std::abs(cdata[i - 1]) - std::abs(cdata[i + 1])) / divisor;
        return std::max((i + delta) * hz_mult, 1.0f);
    };

    float z_re = 0.0f;
    float z_im = 0.0f;
    float mag_sum = 0.0f;

    const float inv_std_hz = 1.0f / 440.0f;
    const float two_pi = 2.0f * 3.141592653589793f;

    for (int i = 0; i < no_peaks; i++) {
        auto &peak_bin = vdata[i];
        auto peak_mag = std::abs(peak_bin.c);
        auto peak_hz = get_freq(peak_bin.i);
        auto peak_cents = 1200.0f * std::log2f(peak_hz * inv_std_hz);
        auto peak_angle = two_pi * 0.01f * peak_cents;

        // printf("%f : %f : %f : %f\n", peak_mag, peak_hz, peak_cents, peak_angle);

        z_re += peak_mag * std::cosf(peak_angle);
        z_im += peak_mag * std::sinf(peak_angle);
        mag_sum += peak_mag;
    }

    // printf("%f : %f : %f\n", z_re, z_im, mag_sum);
    if (std::abs(mag_sum) >= 0.0001f) {
        z_re /= mag_sum;
        z_im /= mag_sum;
    }

    auto phase = std::arg(std::complex<float>(z_re, z_im));
    auto delta_cents = (100.0f / two_pi) * phase;
    auto estimated_tuning = std::powf(2.0f, delta_cents * 0.000833333333333f) * 440.0f;

    avg_roundtable[avg_idx] = delta_cents;
    avg_idx = (avg_idx + 1) % avg_size;

    float avg = 0.0f;
    for (int i = 0; i < avg_size; i++) {
        avg += avg_roundtable[i];
    }
    avg /= avg_size;

    //printf("%f : %f\n", estimated_tuning, avg_sum);
    DBG(estimated_tuning << " : " << avg);

    semitone_buffer.push_back(avg);
    if (semitone_buffer.size() >= buf_max_size) semitone_buffer.pop_front();

    

    delete[] vdata;

    // for (int i = 0; i < numBins; ++i) {
    //     // Usually we want to work with the magnitude and phase rather
    //     // than the real and imaginary parts directly.
    //     float mag = std::abs(cdata[i]);
    //     float ph = std::arg(cdata[i]);

    //     // This is where you'd do your spectral processing...

    //     // Silly example where we change the phase of each frequency bin
    //     // somewhat randomly. Uncomment the following line to enable.
    //     //phase *= float(i);

    //     // Convert magnitude and phase back into a complex number.
    //     cdata[i] = std::polar(mag, ph);
    // }
}
