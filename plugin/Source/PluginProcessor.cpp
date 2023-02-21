#include "PluginProcessor.h"
#include "PluginEditor.h"

SampleNavigatorAudioProcessor::SampleNavigatorAudioProcessor()
{
    parameters.add(*this);

    parameters.sample->addListener(this);

    // Add the WAV audio format to the format manager
    formatManager.registerFormat(new juce::WavAudioFormat(), true);
}

void SampleNavigatorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    transport.prepareToPlay(samplesPerBlock, sampleRate);
    counter = 0;
    currentSample = -1;
}

void SampleNavigatorAudioProcessor::releaseResources()
{
    transport.releaseResources();
}

void SampleNavigatorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                   juce::MidiBuffer& midiMessages)

{
    counter += buffer.getNumSamples();
    if (counter > getSampleRate() / 4)
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
            //std::cerr << "Note on: " << message.getNoteNumber() << std::endl;
            transport.setPosition(0.0);
            transport.start();
        }
    }
    
    juce::AudioSourceChannelInfo info(buffer);
    transport.getNextAudioBlock(info);

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

    //currentSource.reset();
    readers.clear();
    currentSample = -1;
    for (size_t i = 0; i < filePaths.size(); i++)
    {
        auto file = juce::File(filePaths[i]);
        auto* reader = formatManager.createReaderFor (file);
        if (reader != nullptr)
        {   
            auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
            //transport.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
            readers.push_back(std::move(newSource));
            //newSource->setLooping(false);
            //readerSource.reset(newSource.release());
            //transport.setPosition(0.0);
            //transport.stop();
            //transport.start();
        }
    }

    //std::cerr << "Reloaded " << sources.size() << " samples" << std::endl;
    //std::cerr << "Total length" << currentSource->getTotalLength() << std::endl;
    std::cerr << "Created readers for " << readers.size() << " samples" << std::endl;
}

void SampleNavigatorAudioProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
    if (parameterIndex == parameters.sample->getParameterIndex())
    {
        if (readers.size() > 0)
        {
            auto mappedIndex = juce::jmap<float>(newValue, 0.0, 1.0, 0.0, static_cast<float>(readers.size() - 1));
            int index = static_cast<int>(mappedIndex);
            if (index < readers.size())
            {
                currentSample = index;
                transport.setSource(readers[index].get(), 0, nullptr, readers[index]->getAudioFormatReader()->sampleRate);
                std::cerr << "Set source to " << index << std::endl;
            }
        }
    }
    //std::cerr << "Parameter " << parameterIndex << " changed to " << newValue << std::endl;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SampleNavigatorAudioProcessor();
}
