/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AudioDevEffect.h"
#include "EnvelopeShaper.h"
#include "EnvelopeShaper2.h"


#define SNAP_ID "snap/body"
#define SNAP_NAME "Snap/Body"

#define FOCUS_ID "focus"
#define FOCUS_NAME "Focus"

#define RELEASE_ID "release (ms)"
#define RELEASE_NAME "Release (ms)"

#define SATURATION_ID "hf saturation mix (%)"
#define SATURATION_NAME "HF Saturation Mix (%)"

#define OUTPUT_ID "output"
#define OUTPUT_NAME "Output"

#define CLIP_ID "clip"
#define CLIP_NAME "Clip"

#define GAIN_ID "hf gain"
#define GAIN_NAME "HF Gain"


#define UISIZE_ID "ui size"
#define UISIZE_NAME "UI Size"

//==============================================================================
/**
*/



class DrumSnapperAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    DrumSnapperAudioProcessor();
    ~DrumSnapperAudioProcessor() override;

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
    
    float tapeClipper(float sample);

    AudioProcessorValueTreeState parameters;
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    float snap, release, saturation, focus, output, gain;
    bool clip;
    float meterSample;

private:

    int atkDownRamp{ 100 };
    int susUpRamp{ 0 };
    float lastSampleRate{44100};
    int atkCount{ 0 };

    void setParams();
    void updateFilter();

    EnvelopeShaper mEnvelopeFast;
    EnvelopeShaper2 mEnvelopeSlow;

    AudioSampleBuffer mWet, mDry, mRectified, mRectified2, mHighFreq, mHighFreqCopy;

    std::unique_ptr<juce::dsp::Oversampling<float>> mOversample;
   

    dsp::ProcessorDuplicator<dsp::StateVariableFilter::Filter<float>, dsp::StateVariableFilter::Parameters<float>> hiPassFilter;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumSnapperAudioProcessor)
};
