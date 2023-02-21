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
        // Parse JSON file, which contains a list of files to load
        auto file = chooser.getResult();
        juce::var json = juce::JSON::parse(file);

        if (json.isObject())
        {
            SampleNavigatorAudioProcessor& p = dynamic_cast<SampleNavigatorAudioProcessor&>(processor);
            auto& filePaths = p.getFilePaths();
            auto& x = p.getX();
            auto& y = p.getY();

            // Clear existing file paths
            filePaths.clear();
            x.clear();
            y.clear();

            std::cerr << "Loading JSON file: " << file.getFullPathName() << std::endl;

            juce::var files = json["files"];
            if (files.isArray())
            {
                for (auto& item : *files.getArray())
                {
                    juce::String filePath = item["file"];
                    filePaths.push_back(filePath);
                    x.push_back(item["x"]);
                    y.push_back(item["y"]);
                }
            }
            std:: cerr << "Loading " << filePaths.size() << " files" << std::endl;
            p.reloadSamples();
        }
    }
}
