/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SoundTouch/SoundTouch.h"

//==============================================================================
/**
*/
class SemitoneCalculatorAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SemitoneCalculatorAudioProcessor();
    ~SemitoneCalculatorAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    std::atomic<int> transposeAmount { 0 };
    std::atomic<bool> isPitchingActive { false };
    std::atomic<int> fromNoteId { 1 };
    std::atomic<int> toNoteId { 3 };
    std::atomic<int> fromModeId { 1 }; // 1 = Major, 2 = Minor
    std::atomic<int> toModeId { 1 };   // 1 = Major, 2 = Minor
    std::atomic<bool> useLiquidGlassTheme { false };

    bool getUseLiquidGlassTheme() const { return useLiquidGlassTheme.load(); }
    void setUseLiquidGlassTheme(bool b) { useLiquidGlassTheme.store(b); }

private:
    std::vector<float> interleavedBuffer;
    soundtouch::SoundTouch soundTouchEngine;
    juce::AudioBuffer<float> fifoBuffer;
    bool lastPitchingActive = false;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SemitoneCalculatorAudioProcessor)
};
