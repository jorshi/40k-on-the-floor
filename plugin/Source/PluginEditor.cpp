#include "PluginProcessor.h"
#include "PluginEditor.h"

SampleNavigatorAudioProcessorEditor::SampleNavigatorAudioProcessorEditor(
    SampleNavigatorAudioProcessor& p)
    : AudioProcessorEditor(&p)
{
    addAndMakeVisible(editor);
    setSize(400, 300);

    openFileButton.setButtonText("Open File");
    openFileButton.onClick = [this] { openFile(); };
    addAndMakeVisible(openFileButton);
}

void SampleNavigatorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void SampleNavigatorAudioProcessorEditor::resized()
{
    editor.setBounds(getLocalBounds());
    openFileButton.setBounds(getLocalBounds().reduced(10));
}

void SampleNavigatorAudioProcessorEditor::openFile()
{
    juce::FileChooser chooser("Select a JSON file to load...",
                              juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                              "*.json");

    if (chooser.browseForFileToOpen())
    {
        auto file = chooser.getResult();
        juce::var json = juce::JSON::parse(file);

        if (json.isObject())
        {
            std::vector<juce::String> filePaths;
            std::cerr << "Loading JSON file: " << file.getFullPathName() << std::endl;
        }
    }
}
