#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    // Correction PolyBLEP pour limiter l'aliasing des fronts saw/square.
    inline float polyBlep(double t, double dt)
    {
        if (t < dt)       { t /= dt; return (float) (t + t - t * t - 1.0); }
        if (t > 1.0 - dt) { t = (t - 1.0) / dt; return (float) (t * t + t + t + 1.0); }
        return 0.0f;
    }

    struct Preset { const char* name; int wave; float cutoff, res, a, d, s, r, reverb; };
    const Preset presets[] = {
        { "Init Saw",     1, 12000.0f, 0.7f, 0.01f, 0.20f, 0.70f, 0.35f, 0.15f },
        { "Warm Pad",     1,  2200.0f, 0.9f, 0.80f, 0.50f, 0.80f, 1.80f, 0.40f },
        { "Pluck",        2,  5000.0f, 1.2f, 0.00f, 0.25f, 0.00f, 0.20f, 0.20f },
        { "Soft Sine",    0, 18000.0f, 0.7f, 0.02f, 0.10f, 0.90f, 0.50f, 0.25f },
        { "Hollow Bass",  2,   900.0f, 1.5f, 0.00f, 0.30f, 0.50f, 0.15f, 0.05f },
        { "Ethereal Lead",1,  7000.0f, 1.0f, 0.15f, 0.30f, 0.70f, 1.20f, 0.50f },
    };
}

// ---------------------------------------------------------------- Voix

void OmegaVoice::prepare(double sampleRate, int blockSize)
{
    setCurrentPlaybackSampleRate(sampleRate);
    adsr.setSampleRate(sampleRate);
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) blockSize, 1 };
    filter.prepare(spec);
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}

void OmegaVoice::setParams(const Params& p)
{
    params = p;
    adsr.setParameters({ p.attack, p.decay, p.sustain, p.release });
    filter.setCutoffFrequency(juce::jlimit(20.0f, 20000.0f, p.cutoff));
    filter.setResonance(juce::jlimit(0.5f, 5.0f, p.resonance));
}

void OmegaVoice::updateIncrement()
{
    const double f = baseFrequency * std::pow(2.0, pitchBendSemis / 12.0);
    increment = f / getSampleRate();
}

void OmegaVoice::startNote(int midiNote, float velocity, juce::SynthesiserSound*, int pitchWheel)
{
    baseFrequency = juce::MidiMessage::getMidiNoteInHertz(midiNote);
    pitchBendSemis = ((pitchWheel - 8192) / 8192.0f) * 2.0f;
    velocityGain = velocity * 0.3f;
    phase = 0.0;
    triState = 0.0f;
    filter.reset();
    updateIncrement();
    adsr.noteOn();
}

void OmegaVoice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
        adsr.noteOff();
    else
    {
        adsr.reset();
        clearCurrentNote();
    }
}

void OmegaVoice::pitchWheelMoved(int newValue)
{
    pitchBendSemis = ((newValue - 8192) / 8192.0f) * 2.0f;
    updateIncrement();
}

float OmegaVoice::renderOsc(double ph, double inc)
{
    switch (params.wave)
    {
        case 0:  return (float) std::sin(ph * juce::MathConstants<double>::twoPi);
        case 1:  return (float) (2.0 * ph - 1.0) - polyBlep(ph, inc);
        default:
        {
            float s = ph < 0.5 ? 1.0f : -1.0f;
            s += polyBlep(ph, inc);
            s -= polyBlep(std::fmod(ph + 0.5, 1.0), inc);
            return s;
        }
    }
}

void OmegaVoice::renderNextBlock(juce::AudioBuffer<float>& out, int start, int count)
{
    if (!adsr.isActive())
        return;

    for (int n = 0; n < count; ++n)
    {
        const float env = adsr.getNextSample();
        float s = renderOsc(phase, increment);
        phase += increment;
        if (phase >= 1.0) phase -= 1.0;

        s = filter.processSample(0, s);
        const float y = s * env * velocityGain;
        for (int ch = 0; ch < out.getNumChannels(); ++ch)
            out.addSample(ch, start + n, y);

        if (!adsr.isActive())
        {
            clearCurrentNote();
            break;
        }
    }
}

// ----------------------------------------------------------- Processeur

OmegaInfinityAudioProcessor::OmegaInfinityAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    for (int i = 0; i < numVoices; ++i)
        synth.addVoice(new OmegaVoice());
    synth.addSound(new OmegaSound());
}

juce::AudioProcessorValueTreeState::ParameterLayout
OmegaInfinityAudioProcessor::createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    p.push_back(std::make_unique<AudioParameterFloat>("gain", "Output", 0.0f, 1.0f, 0.5f));
    p.push_back(std::make_unique<AudioParameterChoice>("wave", "Waveform", StringArray{ "Sine", "Saw", "Square" }, 1));
    p.push_back(std::make_unique<AudioParameterFloat>("cutoff", "Filter Cutoff",
        NormalisableRange<float>(80.0f, 18000.0f, 1.0f, 0.35f), 12000.0f));
    p.push_back(std::make_unique<AudioParameterFloat>("res", "Filter Resonance",
        NormalisableRange<float>(0.5f, 5.0f, 0.01f), 0.7f));
    p.push_back(std::make_unique<AudioParameterFloat>("attack", "Attack",
        NormalisableRange<float>(0.001f, 3.0f, 0.001f, 0.4f), 0.01f));
    p.push_back(std::make_unique<AudioParameterFloat>("decay", "Decay",
        NormalisableRange<float>(0.001f, 3.0f, 0.001f, 0.4f), 0.2f));
    p.push_back(std::make_unique<AudioParameterFloat>("sustain", "Sustain", 0.0f, 1.0f, 0.7f));
    p.push_back(std::make_unique<AudioParameterFloat>("release", "Release",
        NormalisableRange<float>(0.005f, 5.0f, 0.001f, 0.4f), 0.35f));
    p.push_back(std::make_unique<AudioParameterFloat>("reverb", "Reverb Mix", 0.0f, 1.0f, 0.15f));
    return { p.begin(), p.end() };
}

bool OmegaInfinityAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void OmegaInfinityAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<OmegaVoice*>(synth.getVoice(i)))
            v->prepare(sampleRate, samplesPerBlock);

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock,
                                  (juce::uint32) getTotalNumOutputChannels() };
    reverb.prepare(spec);
}

void OmegaInfinityAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    OmegaVoice::Params vp;
    vp.wave      = (int) apvts.getRawParameterValue("wave")->load();
    vp.cutoff    = apvts.getRawParameterValue("cutoff")->load();
    vp.resonance = apvts.getRawParameterValue("res")->load();
    vp.attack    = apvts.getRawParameterValue("attack")->load();
    vp.decay     = apvts.getRawParameterValue("decay")->load();
    vp.sustain   = apvts.getRawParameterValue("sustain")->load();
    vp.release   = apvts.getRawParameterValue("release")->load();
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<OmegaVoice*>(synth.getVoice(i)))
            v->setParams(vp);

    synth.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());

    const float mix = apvts.getRawParameterValue("reverb")->load();
    juce::dsp::Reverb::Parameters rp;
    rp.roomSize = 0.5f; rp.damping = 0.5f; rp.width = 1.0f;
    rp.wetLevel = mix * 0.5f; rp.dryLevel = 1.0f - mix * 0.5f;
    reverb.setParameters(rp);
    juce::dsp::AudioBlock<float> block(buffer);
    reverb.process(juce::dsp::ProcessContextReplacing<float>(block));

    buffer.applyGain(juce::jlimit(0.0f, 1.0f, apvts.getRawParameterValue("gain")->load()));
}

int OmegaInfinityAudioProcessor::getNumPrograms() { return (int) std::size(presets); }
const juce::String OmegaInfinityAudioProcessor::getProgramName(int i)
{
    return juce::isPositiveAndBelow(i, getNumPrograms()) ? presets[i].name : juce::String();
}

void OmegaInfinityAudioProcessor::setCurrentProgram(int i)
{
    if (!juce::isPositiveAndBelow(i, getNumPrograms())) return;
    currentProgram = i;
    const auto& pr = presets[i];
    auto set = [this](const char* id, float v)
    {
        if (auto* prm = apvts.getParameter(id))
            prm->setValueNotifyingHost(prm->convertTo0to1(v));
    };
    set("wave", (float) pr.wave); set("cutoff", pr.cutoff); set("res", pr.res);
    set("attack", pr.a); set("decay", pr.d); set("sustain", pr.s);
    set("release", pr.r); set("reverb", pr.reverb);
}

void OmegaInfinityAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void OmegaInfinityAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* OmegaInfinityAudioProcessor::createEditor()
{
    return new OmegaInfinityAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OmegaInfinityAudioProcessor();
}
