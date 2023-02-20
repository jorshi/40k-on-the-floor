#include "PluginProcessor.h"
#include "PluginEditor.h"

SampleNavigatorAudioProcessorEditor::SampleNavigatorAudioProcessorEditor(
    SampleNavigatorAudioProcessor& p)
    : AudioProcessorEditor(&p)
{
    addAndMakeVisible(editor);
    setSize(400, 300);
}

void SampleNavigatorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void SampleNavigatorAudioProcessorEditor::resized()
{
    editor.setBounds(getLocalBounds());
}
