#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <numbers>

/**
  STFT analysis and resynthesis of audio data.

  Each channel should have its own FFTProcessor.
 */
class FFTProcessor
{
public:
    FFTProcessor();

    int getLatencyInSamples() const { return fftSize; }

    void reset();
    float processSample(float sample, bool bypassed);
    void processBlock(float* data, int numSamples, bool bypassed);

    void changeOrder(int order);
    inline int getOrder() { return fftOrder; }

    std::deque<float> semitone_buffer;
    unsigned int buf_max_size = 20;
    int avg_size = 1;

    float sampleRate = 48000.0f;
    int no_peaks = 6;

private:
    void processFrame(bool bypassed);
    void processSpectrum(float* data, int numBins);

    // The FFT has 2^order points and fftSize/2 + 1 bins.
    int fftOrder = 13;
    int fftSize = 1 << fftOrder;      // 1024 samples
    int numBins = fftSize / 2 + 1;    // 513 bins
    int overlap = 4;                  // 75% overlap
    int hopSize = fftSize / overlap;  // 256 samples

    float avg_roundtable[20];
    int avg_idx = 0;

    // Gain correction for using Hann window with 75% overlap.
    static constexpr float windowCorrection = 2.0f / 3.0f;

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;

    // Counts up until the next hop.
    int count = 0;

    // Write position in input FIFO and read position in output FIFO.
    int pos = 0;

    // Circular buffers for incoming and outgoing audio data.
    std::vector<float> inputFifo;
    std::vector<float> outputFifo;

    // The FFT working space. Contains interleaved complex numbers.
    std::vector<float> fftData;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FFTProcessor)
};
