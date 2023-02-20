#pragma once

#include "PluginProcessor.h"

class SampleNavigatorAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit SampleNavigatorAudioProcessorEditor(SampleNavigatorAudioProcessor&);

private:
    juce::TextButton openFileButton;

    void paint(juce::Graphics&) override;
    void resized() override;

    void openFile();

    SampleNavigatorAudioProcessor& processor;

    juce::GenericAudioProcessorEditor editor {processor};
};
