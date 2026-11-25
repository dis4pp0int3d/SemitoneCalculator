/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SemitoneCalculatorAudioProcessor::SemitoneCalculatorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
                    #endif
{
}

SemitoneCalculatorAudioProcessor::~SemitoneCalculatorAudioProcessor()
{
}

//==============================================================================
const juce::String SemitoneCalculatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SemitoneCalculatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SemitoneCalculatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SemitoneCalculatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SemitoneCalculatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SemitoneCalculatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SemitoneCalculatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SemitoneCalculatorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SemitoneCalculatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void SemitoneCalculatorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SemitoneCalculatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    const uint sr = (uint)juce::jmax(1.0, sampleRate);
    const uint numCh = (uint)juce::jmax(1, getTotalNumInputChannels());

    soundTouchEngine.setSampleRate(sr);
    soundTouchEngine.setChannels(numCh);

    // Smaller sequences -> lower latency (quality can be lower).
    soundTouchEngine.setSetting(SETTING_SEQUENCE_MS, 40);
    soundTouchEngine.setSetting(SETTING_SEEKWINDOW_MS, 15);
    soundTouchEngine.setSetting(SETTING_OVERLAP_MS, 8);

    soundTouchEngine.clear();
    lastPitchingActive = false;
}

void SemitoneCalculatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SemitoneCalculatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SemitoneCalculatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    const bool pitchingNow = isPitchingActive.load();
    if (!pitchingNow)
    {
        // If we were pitching before, drop any queued processed samples.
        if (lastPitchingActive)
            soundTouchEngine.clear();

        lastPitchingActive = false;
        return;
    }

    if (!lastPitchingActive)
        soundTouchEngine.clear();

    lastPitchingActive = true;

    float st = static_cast<float>(transposeAmount.load());
    if (std::abs(st) < 0.01f)
    {
        soundTouchEngine.clear();
        return; // Почти ноль
    }

    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();

    interleavedBuffer.resize(numSamples * numChannels);

    for (int i = 0; i < numSamples; ++i) {
        for (int ch = 0; ch < numChannels; ++ch) {
            interleavedBuffer[i * numChannels + ch] = buffer.getSample(ch, i);
        }
    }

    float pitchKey = std::pow(2.0f, st / 12.0f);
    soundTouchEngine.setPitch(pitchKey);

    soundTouchEngine.putSamples(interleavedBuffer.data(), (uint)numSamples);

    uint samplesReceived = soundTouchEngine.receiveSamples(interleavedBuffer.data(), (uint)numSamples);

    for (int i = 0; i < (int)samplesReceived; ++i) {
        for (int ch = 0; ch < numChannels; ++ch) {
            buffer.setSample(ch, i, interleavedBuffer[i * numChannels + ch]);
        }
    }

    // If SoundTouch hasn't produced enough output yet, keep the remaining
    // samples dry (original) so the effect doesn't look like "nothing works".
}

//==============================================================================
bool SemitoneCalculatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SemitoneCalculatorAudioProcessor::createEditor()
{
    return new SemitoneCalculatorAudioProcessorEditor (*this);
}

//==============================================================================
void SemitoneCalculatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ValueTree tree("SemitoneCalculatorState");
    tree.setProperty("fromNoteId", fromNoteId.load(), nullptr);
    tree.setProperty("toNoteId", toNoteId.load(), nullptr);
    tree.setProperty("fromModeId", fromModeId.load(), nullptr);
    tree.setProperty("toModeId", toModeId.load(), nullptr);
    tree.setProperty("transposeAmount", transposeAmount.load(), nullptr);
    tree.setProperty("isPitchingActive", (int)isPitchingActive.load(), nullptr);
    tree.setProperty("useLiquidGlassTheme", (int)useLiquidGlassTheme.load(), nullptr);

    if (auto xml = tree.createXml())
        copyXmlToBinary(*xml, destData);
}

void SemitoneCalculatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        auto tree = juce::ValueTree::fromXml(*xml);

        fromNoteId.store((int)tree.getProperty("fromNoteId", 1));
        toNoteId.store((int)tree.getProperty("toNoteId", 3));
        fromModeId.store((int)tree.getProperty("fromModeId", 1));
        toModeId.store((int)tree.getProperty("toModeId", 1));

        isPitchingActive.store((int)tree.getProperty("isPitchingActive", 0) != 0);
        useLiquidGlassTheme.store((int)tree.getProperty("useLiquidGlassTheme", 0) != 0);

        // Recompute transpose amount from stored From/To to keep consistency.
        int fromIdx = fromNoteId.load() - 1;
        int toIdx = toNoteId.load() - 1;

        if (fromModeId.load() == 2) fromIdx = (fromIdx + 3) % 12;
        if (toModeId.load() == 2) toIdx = (toIdx + 3) % 12;

        int diff = toIdx - fromIdx;
        if (diff > 6) diff -= 12;
        if (diff < -6) diff += 12;
        transposeAmount.store(diff);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SemitoneCalculatorAudioProcessor();
}
