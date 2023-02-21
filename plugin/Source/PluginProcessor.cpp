#include "PluginProcessor.h"
#include "PluginEditor.h"

SampleNavigatorAudioProcessor::SampleNavigatorAudioProcessor() : numChannels(48)
{
    parameters.add(*this);

    parameters.sampleX->addListener(this);
    parameters.sampleY->addListener(this);

    // Add the WAV audio format to the format manager
    formatManager.registerFormat(new juce::WavAudioFormat(), true);
    for (int i = 0; i < numChannels; i++)
    {
        transportChannels.push_back(std::make_unique<juce::AudioTransportSource>());
        mixer.addInputSource(transportChannels[i].get(), false);
    }
}

void SampleNavigatorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    transport.prepareToPlay(samplesPerBlock, sampleRate);
    counter = 0;
    currentSample = -1;
    currentChannel = 0;

    for (int i = 0; i < numChannels; i++)
        transportChannels[i]->prepareToPlay(samplesPerBlock, sampleRate);

    mixer.prepareToPlay(samplesPerBlock, sampleRate);
}

void SampleNavigatorAudioProcessor::releaseResources()
{
    transport.releaseResources();
}

void SampleNavigatorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                   juce::MidiBuffer& midiMessages)

{
    counter += buffer.getNumSamples();
    if (counter > getSampleRate() / 2)
    {
        counter = 0;
        midiMessages.addEvent(juce::MidiMessage::noteOn(1, 60, 1.0f), 0);
    }

    const juce::ScopedTryLock myScopedTryLock (lock);
    if (!myScopedTryLock.isLocked())
    {
        buffer.clear();
        return;
    }

    if (readers.size() == 0 || currentSample == -1)
    {
        buffer.clear();
        return;
    }

    for (const auto metadata: midiMessages)
    {
        const auto message = metadata.getMessage();
        if (message.isNoteOn())
        {   
            // Restart the current sample
            transportChannels[currentChannel]->setPosition(0);
            transportChannels[currentChannel]->start();
        }
    }
    
    juce::AudioSourceChannelInfo info(buffer);
    mixer.getNextAudioBlock(info);

    juce::ignoreUnused(midiMessages);
}

juce::AudioProcessorEditor* SampleNavigatorAudioProcessor::createEditor()
{
    return new SampleNavigatorAudioProcessorEditor(*this);
}

void SampleNavigatorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    //Serializes your parameters, and any other potential data into an XML:

    juce::ValueTree params("Params");

    for (auto& param: getParameters())
    {
        juce::ValueTree paramTree(PluginHelpers::getParamID(param));
        paramTree.setProperty("Value", param->getValue(), nullptr);
        params.appendChild(paramTree, nullptr);
    }

    juce::ValueTree pluginPreset("MyPlugin");
    pluginPreset.appendChild(params, nullptr);
    //This a good place to add any non-parameters to your preset

    copyXmlToBinary(*pluginPreset.createXml(), destData);
}

void SampleNavigatorAudioProcessor::setStateInformation(const void* data,
                                                          int sizeInBytes)
{
    //Loads your parameters, and any other potential data from an XML:

    auto xml = getXmlFromBinary(data, sizeInBytes);

    if (xml != nullptr)
    {
        auto preset = juce::ValueTree::fromXml(*xml);
        auto params = preset.getChildWithName("Params");

        for (auto& param: getParameters())
        {
            auto paramTree = params.getChildWithName(PluginHelpers::getParamID(param));

            if (paramTree.isValid())
                param->setValueNotifyingHost(paramTree["Value"]);
        }

        //Load your non-parameter data now
    }
}

void SampleNavigatorAudioProcessor::reloadSamples()
{   
    const juce::ScopedLock myScopedLock (lock);

    readers.clear();
    currentSample = -1;
    annoyIndex.reset(new AnnoyType(2));

    for (size_t i = 0; i < filePaths.size(); i++)
    {
        auto file = juce::File(filePaths[i]);
        auto* reader = wavFormat.createMemoryMappedReader(file);
        if (reader != nullptr)
        {   
            reader->mapEntireFile();
            reader->touchSample(0);
            auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
            readers.push_back(std::move(newSource));

            // Add to annoy index
            std::vector<float> v = {x[i], y[i]};
            annoyIndex->add_item(i, v.data());
        }
    }

    // Build nearest neighbour index
    annoyIndex->build(10);
    std::cerr << "Created readers for " << readers.size() << " samples" << std::endl;
}

void SampleNavigatorAudioProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
    if (parameterIndex == parameters.sampleX->getParameterIndex() || parameterIndex == parameters.sampleY->getParameterIndex())
    {
        // Get the updated and current X and Y values
        if (parameterIndex == parameters.sampleX->getParameterIndex()) {
            float x = newValue;
            float y = parameters.sampleY->get();
        }
        else {
            float x = parameters.sampleX->get();
            float y = newValue;
        }

        if (readers.size() > 0)
        {   
            std::vector<float> v = {x, y};
            std::vector<float> distances;
            std::vector<int> indices;
            annoyIndex->get_nns_by_vector(v.data(), 1, 5, &indices, &distances);
            int index = indices[0];

            if (index < readers.size())
            {   
                if (currentSample == index)
                    return;

                currentSample = index;
                currentChannel = (currentChannel + 1) % numChannels;
                if (transportChannels[currentChannel]->isPlaying()) {   
                    return;
                }
                transportChannels[currentChannel]->stop();
                transportChannels[currentChannel]->setSource(readers[index].get(), 0, nullptr, readers[index]->getAudioFormatReader()->sampleRate);
                transportChannels[currentChannel]->setPosition(0.0);
            }
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SampleNavigatorAudioProcessor();
}
