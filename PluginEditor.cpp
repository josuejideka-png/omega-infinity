#include "PluginProcessor.h"
#include "PluginEditor.h"

void OmegaInfinityAudioProcessorEditor::addKnob(Knob& k, const juce::String& id, const juce::String& text)
{
    k.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    k.slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffe8c76a));
    addAndMakeVisible(k.slider);
    k.label.setText(text, juce::dontSendNotification);
    k.label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(k.label);
    k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, id, k.slider);
}

OmegaInfinityAudioProcessorEditor::OmegaInfinityAudioProcessorEditor(OmegaInfinityAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(640, 420);

    title.setText("OMEGA INFINITY", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(28.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, juce::Colour(0xffe8c76a));
    addAndMakeVisible(title);

    wave.addItem("Sine", 1); wave.addItem("Saw", 2); wave.addItem("Square", 3);
    addAndMakeVisible(wave);
    waveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.apvts, "wave", wave);
    waveLabel.setText("OSCILLATOR", juce::dontSendNotification);
    addAndMakeVisible(waveLabel);

    for (int i = 0; i < p.getNumPrograms(); ++i)
        presetBox.addItem(p.getProgramName(i), i + 1);
    presetBox.setSelectedId(p.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] { processor.setCurrentProgram(presetBox.getSelectedId() - 1); };
    addAndMakeVisible(presetBox);
    presetLabel.setText("PRESET", juce::dontSendNotification);
    addAndMakeVisible(presetLabel);

    addKnob(gain, "gain", "OUTPUT");
    addKnob(cutoff, "cutoff", "CUTOFF");
    addKnob(res, "res", "RESO");
    addKnob(attack, "attack", "ATTACK");
    addKnob(decay, "decay", "DECAY");
    addKnob(sustain, "sustain", "SUSTAIN");
    addKnob(release, "release", "RELEASE");
    addKnob(reverb, "reverb", "REVERB");
}

void OmegaInfinityAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff10131b));
    g.setColour(juce::Colour(0xff245d9c));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(8), 14.0f, 2.0f);
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.drawFittedText("V1.1  |  8-VOICE SYNTH  |  ADSR  |  FILTER  |  REVERB",
                     20, 390, 600, 22, juce::Justification::centred, 1);
}

void OmegaInfinityAudioProcessorEditor::resized()
{
    title.setBounds(24, 16, 360, 44);
    presetLabel.setBounds(400, 14, 100, 20);
    presetBox.setBounds(400, 34, 210, 28);
    waveLabel.setBounds(24, 76, 150, 20);
    wave.setBounds(24, 96, 150, 28);

    auto place = [](Knob& k, int x, int y)
    {
        k.label.setBounds(x, y, 110, 20);
        k.slider.setBounds(x, y + 20, 110, 110);
    };
    place(cutoff, 40, 140);  place(res, 170, 140);   place(reverb, 300, 140); place(gain, 430, 140);
    place(attack, 40, 260);  place(decay, 170, 260); place(sustain, 300, 260); place(release, 430, 260);
}
