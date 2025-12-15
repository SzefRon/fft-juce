/*
  ==============================================================================

    LineGraph.h
    Created: 14 Dec 2025 9:04:52pm
    Author:  Miłosz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <deque>

//==============================================================================
/*
*/
class LineGraph  : public juce::Component
{
public:
    LineGraph(std::deque<float> &semitone_buffer, unsigned int &max_size);
    ~LineGraph() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LineGraph)

    unsigned int &max_size;
    std::deque<float>& semitone_buffer;
};
