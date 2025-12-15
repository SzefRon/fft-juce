/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MyAudioProcessorEditor::MyAudioProcessorEditor (FFTExampleAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    lineGraph(p.fft.semitone_buffer, p.fft.buf_max_size),
    avg_slider(juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft),
    avg_label("avg values", "avg values"),
    peaks_slider(juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft),
    peaks_label("num of peaks", "num of peaks"),
    order_slider(juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft),
    order_label("fft order", "fft order")
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (450, 400);
    setResizable(true, true);

    addAndMakeVisible(lineGraph);

    startTimerHz(30);

    avg_slider.setRange(juce::Range<double>(1.0, 20.0), 1.0);
    avg_slider.onValueChange = [&]() {
        p.fft.avg_size = avg_slider.getValue();
        };
    avg_slider.setValue(p.fft.avg_size);
    addAndMakeVisible(avg_slider);
    
    avg_label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(avg_label);



    peaks_slider.setRange(juce::Range<double>(1.0, 20.0), 1.0);
    peaks_slider.onValueChange = [&]() {
        p.fft.no_peaks = peaks_slider.getValue();
        };
    peaks_slider.setValue(p.fft.no_peaks);
    addAndMakeVisible(peaks_slider);

    peaks_label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(peaks_label);



    
    order_slider.setRange(juce::Range<double>(8.0, 14.0), 1.0);
    order_slider.onValueChange = [&]() {
        p.fft.changeOrder(order_slider.getValue());
        };
    order_slider.setValue(p.fft.getOrder());
    addAndMakeVisible(order_slider);

    order_label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(order_label);
}

MyAudioProcessorEditor::~MyAudioProcessorEditor()
{
}

//==============================================================================
void MyAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MyAudioProcessorEditor::resized()
{
    auto windowBounds = getLocalBounds();
    auto lineGraphBounds = windowBounds;
    auto bottomBounds = lineGraphBounds.removeFromBottom(60);
    lineGraph.setBounds(lineGraphBounds);

    auto sliderBounds = bottomBounds.removeFromLeft(windowBounds.getWidth() / 3);
    auto labelBounds = sliderBounds.removeFromLeft(windowBounds.getWidth() / 8);
    avg_slider.setBounds(sliderBounds.reduced(10));
    avg_label.setBounds(labelBounds);

    sliderBounds = bottomBounds.removeFromLeft(windowBounds.getWidth() / 3);
    labelBounds = sliderBounds.removeFromLeft(windowBounds.getWidth() / 8);
    peaks_slider.setBounds(sliderBounds.reduced(10));
    peaks_label.setBounds(labelBounds);

    sliderBounds = bottomBounds;
    labelBounds = sliderBounds.removeFromLeft(windowBounds.getWidth() / 8);
    order_slider.setBounds(sliderBounds.reduced(10));
    order_label.setBounds(labelBounds);

}

void MyAudioProcessorEditor::timerCallback()
{
    lineGraph.repaint();
}
