/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FFTExampleAudioProcessor::FFTExampleAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

FFTExampleAudioProcessor::~FFTExampleAudioProcessor()
{
}

//==============================================================================
const juce::String FFTExampleAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FFTExampleAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FFTExampleAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FFTExampleAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FFTExampleAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FFTExampleAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FFTExampleAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FFTExampleAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FFTExampleAudioProcessor::getProgramName (int index)
{
    return {};
}

void FFTExampleAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

juce::AudioProcessorParameter* FFTExampleAudioProcessor::getBypassParameter() const
{
    return apvts.getParameter("Bypass");
}

//==============================================================================
void FFTExampleAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    setLatencySamples(fft.getLatencyInSamples());

    fft.reset();
}

void FFTExampleAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool FFTExampleAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void FFTExampleAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto numInputChannels = getTotalNumInputChannels();
    auto numOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();

    for (auto i = numInputChannels; i < numOutputChannels; ++i) {
        buffer.clear(i, 0, numSamples);
    }

    bool bypassed = apvts.getRawParameterValue("Bypass")->load();

    float* channelL = buffer.getWritePointer(0);

    // Processing on a sample-by-sample basis:
    for (int sample = 0; sample < numSamples; ++sample) {
        float sampleL = channelL[sample];

        sampleL = fft.processSample(sampleL, bypassed);

        channelL[sample] = sampleL;
    }

    /*
    // Processing the entire block at once:
    for (int channel = 0; channel < numInputChannels; ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        fft[channel].processBlock(channelData, numSamples, bypassed);
    }
    */
}

//==============================================================================
bool FFTExampleAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FFTExampleAudioProcessor::createEditor()
{
    return new MyAudioProcessorEditor(*this);
}

//==============================================================================
void FFTExampleAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    copyXmlToBinary(*apvts.copyState().createXml(), destData);
}

void FFTExampleAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml.get() != nullptr && xml->hasTagName(apvts.state.getType())) {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout FFTExampleAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("Bypass", 1),
        "Bypass",
        false));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FFTExampleAudioProcessor();
}
