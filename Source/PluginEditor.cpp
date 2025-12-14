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
    avg_slider(juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    setResizable(true, true);

    addAndMakeVisible(lineGraph);

    startTimerHz(30);

    avg_slider.setRange(juce::Range<double>(1.0, 20.0), 1.0);
    avg_slider.onValueChange = [&]() {
        p.fft.avg_size = avg_slider.getValue();
        };
    addAndMakeVisible(avg_slider);
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
    auto bottomBounds = lineGraphBounds.removeFromBottom(40);
    lineGraph.setBounds(lineGraphBounds);

    auto sliderBounds = bottomBounds.removeFromLeft(windowBounds.getWidth() / 2);
    avg_slider.setBounds(sliderBounds);

}

void MyAudioProcessorEditor::timerCallback()
{
    lineGraph.repaint();
}
