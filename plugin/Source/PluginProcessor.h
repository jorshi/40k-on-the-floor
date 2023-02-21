#pragma once

#include "Parameters.h"

class SampleNavigatorAudioProcessor : public PluginHelpers::ProcessorBase, public juce::AudioProcessorParameter::Listener
{
public:
    SampleNavigatorAudioProcessor();

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    juce::AudioProcessorEditor* createEditor() override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    void reloadSamples();

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {};

    std::vector<juce::String>& getFilePaths() { return filePaths; }
    std::vector<float>& getX() { return x; }
    std::vector<float>& getY() { return y; }

private:

    Parameters parameters;

    int counter;
    int currentSample = 0;

    //std::vector<std::unique_ptr<juce::AudioFormatReaderSource>> sources;

    std::vector<juce::String> filePaths;
    std::vector<float> x;
    std::vector<float> y;

    std::vector<std::unique_ptr<juce::AudioFormatReaderSource>> readers;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transport;

    juce::CriticalSection lock;
};
