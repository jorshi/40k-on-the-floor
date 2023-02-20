#pragma once

#include "Parameters.h"

class SampleNavigatorAudioProcessor : public PluginHelpers::ProcessorBase
{
public:
    SampleNavigatorAudioProcessor();

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    std::vector<juce::String>& getFilePaths() { return filePaths; };
    std::vector<float>& getX() { return x; };
    std::vector<float>& getY() { return y; }

private:

    Parameters parameters;

    std::vector<juce::String> filePaths;
    std::vector<float> x;
    std::vector<float> y;
};
