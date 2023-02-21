#pragma once

#include "Parameters.h"
#include "annoylib.h"
#include "kissrandom.h"

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
    int currentChannel = 0;
    int numChannels;

    //std::vector<std::unique_ptr<juce::AudioFormatReaderSource>> sources;

    std::vector<juce::String> filePaths;
    std::vector<float> x;
    std::vector<float> y;

    std::vector<std::unique_ptr<juce::AudioFormatReaderSource>> readers;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioFormatManager formatManager;
    juce::WavAudioFormat wavFormat;
    juce::AudioTransportSource transport;
    std::vector<std::unique_ptr<juce::AudioTransportSource>> transportChannels;

    // Audio Mixer
    juce::MixerAudioSource mixer;

    // ANNOY
    typedef Annoy::AnnoyIndex<int, float, Annoy::Euclidean, Annoy::Kiss32Random, Annoy::AnnoyIndexSingleThreadedBuildPolicy> AnnoyType;
    std::unique_ptr<AnnoyType> annoyIndex;
    //AnnoyIndex<int, float, Euclidean, Kiss64Random> annoyIndex;

    juce::CriticalSection lock;
};
