#pragma once
#include <JuceHeader.h>

class OmegaSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

// Voix polyphonique : oscillateur PolyBLEP, ADSR, filtre passe-bas resonant.
class OmegaVoice : public juce::SynthesiserVoice
{
public:
    struct Params
    {
        int wave = 1;
        float cutoff = 12000.0f, resonance = 0.7f;
        float attack = 0.01f, decay = 0.2f, sustain = 0.7f, release = 0.35f;
    };

    bool canPlaySound(juce::SynthesiserSound* s) override { return dynamic_cast<OmegaSound*>(s) != nullptr; }
    void setParams(const Params& p);
    void prepare(double sampleRate, int blockSize);

    void startNote(int midiNote, float velocity, juce::SynthesiserSound*, int pitchWheel) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newValue) override;
    void controllerMoved(int, int) override {}
    void renderNextBlock(juce::AudioBuffer<float>&, int startSample, int numSamples) override;

private:
    float renderOsc(double ph, double inc);
    void updateIncrement();

    Params params;
    juce::ADSR adsr;
    juce::dsp::StateVariableTPTFilter<float> filter;
    double phase = 0.0, baseFrequency = 440.0, increment = 0.0;
    float velocityGain = 0.0f, pitchBendSemis = 0.0f, triState = 0.0f;
};

class OmegaInfinityAudioProcessor : public juce::AudioProcessor
{
public:
    OmegaInfinityAudioProcessor();
    ~OmegaInfinityAudioProcessor() override = default;

    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    static constexpr int numVoices = 8;
    juce::Synthesiser synth;
    juce::dsp::Reverb reverb;
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OmegaInfinityAudioProcessor)
};
