#pragma once

#include <shared_plugin_helpers/shared_plugin_helpers.h>

struct Parameters
{
    void add(juce::AudioProcessor& processor) const
    {
        processor.addParameter(sampleX);
        processor.addParameter(sampleY);
    }

    //Raw pointers. They will be owned by either the processor or the APVTS (if you use it)
    juce::AudioParameterFloat* sampleX =
        new juce::AudioParameterFloat({"sample_x", 1}, "X", 0.f, 1.f, 0.5f);
    
    juce::AudioParameterFloat* sampleY =
        new juce::AudioParameterFloat({"sample_y", 1}, "Y", 0.f, 1.f, 0.5f);
};