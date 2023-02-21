#include "PluginProcessor.h"
#include "PluginEditor.h"

SampleNavigatorAudioProcessor::SampleNavigatorAudioProcessor()
{
    parameters.add(*this);

    // Add the WAV audio format to the format manager
    formatManager.registerFormat(new juce::WavAudioFormat(), true);
}

void SampleNavigatorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    transport.prepareToPlay(samplesPerBlock, sampleRate);
}

void SampleNavigatorAudioProcessor::releaseResources()
{
    transport.releaseResources();
}

void SampleNavigatorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                   juce::MidiBuffer& midiMessages)

{
    const juce::ScopedTryLock myScopedTryLock (lock);
    if (!myScopedTryLock.isLocked())
    {
        buffer.clear();
        return;
    }

    if (readerSource.get() == nullptr)
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
    for (size_t i = 0; i < filePaths.size(); i++)
    {
        auto file = juce::File(filePaths[i]);
        auto* reader = formatManager.createReaderFor (file);
        if (reader != nullptr)
        {   
            auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
            transport.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
            newSource->setLooping(false);
            readerSource.reset(newSource.release());
            transport.setPosition(0.0);
            transport.stop();
            //transport.start();
        }
            
        
        break;

        // if (reader != nullptr)
        // {
        //     // Create a new PositionableAudioSource for each sample
        //     //sources.emplace_back(std::make_unique<juce::AudioFormatReaderSource>(reader.get(), true));
        //     //sources.back()->setLooping(true);
        //     //sources.back()->prepareToPlay(getSampleRate(), getBlockSize());
        // }
    }

    //std::cerr << "Reloaded " << sources.size() << " samples" << std::endl;
    //std::cerr << "Total length" << currentSource->getTotalLength() << std::endl;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SampleNavigatorAudioProcessor();
}
