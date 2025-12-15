/*
  ==============================================================================

    LineGraph.cpp
    Created: 14 Dec 2025 9:04:52pm
    Author:  Miłosz

  ==============================================================================
*/

#include <JuceHeader.h>
#include "LineGraph.h"

//==============================================================================
LineGraph::LineGraph(std::deque<float> &semitone_buffer, unsigned int &max_size)
    : max_size(max_size), semitone_buffer(semitone_buffer)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
}

LineGraph::~LineGraph()
{
}

void LineGraph::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
    auto bounds = getLocalBounds();

    g.fillAll (juce::Colours::darkgrey);   // clear the background

    auto step = bounds.getWidth() * (1.0f / (max_size - 2));

    auto map_val = [](float a, float b, float c, float d, float value) {
        return c + (value - a) * (d - c) / (b - a);
        };

    juce::Path p1;
    p1.startNewSubPath(0.0f, bounds.getHeight() * 0.5f);
    p1.lineTo(bounds.getWidth(), bounds.getHeight() * 0.5f);

    g.setColour(juce::Colours::dimgrey);
    g.strokePath(p1, juce::PathStrokeType(1.0f));

    juce::Path p;
    bool first = true;
    int i = 0;
    for (auto& semitone : semitone_buffer) {
        float mapped_y = bounds.getHeight() - map_val(-50.0f, 50.0f, 0.0f, bounds.getHeight(), semitone);
        float mapped_x = step * i;
        if (first) {
            p.startNewSubPath(mapped_x, mapped_y);
            first = false;
        }
        else {
            p.lineTo(mapped_x, mapped_y);
        }

        i++;
    }

    float cur_semitone = 0.0f;
    if (semitone_buffer.size() != 0) cur_semitone = semitone_buffer.back();

    g.setColour(juce::Colours::white);
    g.strokePath(p, juce::PathStrokeType(2.0f));

    g.setColour (juce::Colours::grey);
    g.drawRect (bounds, 2);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.0f));
    g.drawText (std::to_string(cur_semitone), getLocalBounds(),
                juce::Justification::centredBottom, true);   // draw some placeholder text
}

void LineGraph::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
