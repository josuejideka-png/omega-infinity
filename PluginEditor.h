#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class OmegaInfinityAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit OmegaInfinityAudioProcessorEditor(OmegaInfinityAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    void addKnob(Knob& k, const juce::String& paramId, const juce::String& text);

    OmegaInfinityAudioProcessor& processor;
    juce::Label title, waveLabel, presetLabel;
    juce::ComboBox wave, presetBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveAttachment;
    Knob gain, cutoff, res, attack, decay, sustain, release, reverb;
};
