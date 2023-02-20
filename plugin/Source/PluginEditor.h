#pragma once

#include "PluginProcessor.h"

class SampleNavigatorAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit SampleNavigatorAudioProcessorEditor(SampleNavigatorAudioProcessor&);

private:
    void paint(juce::Graphics&) override;
    void resized() override;

    juce::GenericAudioProcessorEditor editor {processor};
};
