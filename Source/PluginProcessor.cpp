/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DrumSnapperAudioProcessor::DrumSnapperAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), parameters(*this, nullptr, "PARAMETER", createParameterLayout())

                
#endif
{
    
}

DrumSnapperAudioProcessor::~DrumSnapperAudioProcessor()
{
}

AudioProcessorValueTreeState::ParameterLayout DrumSnapperAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr <RangedAudioParameter>> params;

    auto snapParam = std::make_unique <AudioParameterFloat>(SNAP_ID, SNAP_NAME, -5.0f, 5.0f, 0.0f);
    auto focusParam = std::make_unique <AudioParameterFloat>(FOCUS_ID, FOCUS_NAME, 1.0f, 5.0f, 0.1f);
    auto releaseParam = std::make_unique <AudioParameterFloat>(RELEASE_ID, RELEASE_NAME, 5.0f, 350.0f, 150.0f);
    auto outputParam = std::make_unique <AudioParameterFloat>(OUTPUT_ID, OUTPUT_NAME, -24.0f, 6.0f, 0.0f);
    auto gainParam = std::make_unique <AudioParameterFloat>(GAIN_ID, GAIN_NAME, 1.0f, 10.f, 1.0f);
    auto saturationParam = std::make_unique <AudioParameterFloat>(SATURATION_ID, SATURATION_NAME, 0.0f, 100.f, 0.0f);
    auto clipParam = std::make_unique <AudioParameterBool>(CLIP_ID, CLIP_NAME, true);

    params.push_back(std::move(snapParam));
    params.push_back(std::move(focusParam));
    params.push_back(std::move(releaseParam));
    params.push_back(std::move(outputParam));
    params.push_back(std::move(gainParam));
    params.push_back(std::move(saturationParam));
    params.push_back(std::move(clipParam));
    

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String DrumSnapperAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DrumSnapperAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DrumSnapperAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DrumSnapperAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DrumSnapperAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DrumSnapperAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DrumSnapperAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DrumSnapperAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DrumSnapperAudioProcessor::getProgramName (int index)
{
    return {};
}

void DrumSnapperAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void DrumSnapperAudioProcessor::setParams()
{
    snap = (*parameters.getRawParameterValue(SNAP_ID));
    focus = (*parameters.getRawParameterValue(FOCUS_ID));
    release = (*parameters.getRawParameterValue(RELEASE_ID));
    output = (*parameters.getRawParameterValue(OUTPUT_ID));
    gain = (*parameters.getRawParameterValue(GAIN_ID));
    saturation = (*parameters.getRawParameterValue(SATURATION_ID));
    clip = (*parameters.getRawParameterValue(CLIP_ID));


    if (snap >= 0)
    {
        mEnvelopeSlow.setAttack(20.f);
        mEnvelopeSlow.setRelease(release);

        mEnvelopeFast.setAttack(0.f);
        mEnvelopeFast.setRelease(release);
    }
    else 
    {
        mEnvelopeSlow.setAttack(0.f);
        mEnvelopeSlow.setRelease(release);

        mEnvelopeFast.setAttack(0.f);
        mEnvelopeFast.setRelease((release) / 5);
    }
    

    
}

//==============================================================================
void DrumSnapperAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    lastSampleRate = static_cast <int>(sampleRate);

    mEnvelopeFast.prepareForPlayback(lastSampleRate);

    mEnvelopeSlow.prepareForPlayback(lastSampleRate);

    setParams();


    dsp::ProcessSpec spec;
    spec.sampleRate = lastSampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();

    hiPassFilter.reset();
    hiPassFilter.prepare(spec);

    hiPassFilter.state->type = dsp::StateVariableFilter::Parameters<float>::Type::highPass;
    hiPassFilter.state->setCutOffFrequency(lastSampleRate, 1300 / 10, 0.1f);
    
    mOversample.reset(new juce::dsp::Oversampling<float>(getTotalNumOutputChannels(), 3, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true));
    mOversample->initProcessing(static_cast<size_t>(samplesPerBlock));

}

void DrumSnapperAudioProcessor::updateFilter()
{
   
}

void DrumSnapperAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DrumSnapperAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

void DrumSnapperAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    auto bufferNumSamples = buffer.getNumSamples();


    mOversample->setUsingIntegerLatency(true);

    if (clip == true)
    {
        setLatencySamples(static_cast <int>(mOversample->getLatencyInSamples()));
    }
    else
    {
        setLatencySamples(0);
    }

    
    mDry.setSize(totalNumInputChannels, bufferNumSamples, false, false, false);
    mWet.setSize(totalNumInputChannels, bufferNumSamples, false, false, false);
    
    mRectified.makeCopyOf(buffer);

    setParams();

    //Rectify copy of buffer for envelope
    {
        float* w = mRectified.getWritePointer(0);
        if (totalNumInputChannels > 1)
        {
            for (int i = 0; i < bufferNumSamples; i++)
            {
                w[i] += buffer.getSample(1, i);
                w[i] /= 2.f;
            }
        }

        for (int i = 0; i < bufferNumSamples; i++)
        {
            w[i] = fabsf(w[i]);
        }
        
    }

    mRectified.setSize(1, bufferNumSamples, true, true, true);
    //create 2 envelopes
    mRectified2.makeCopyOf(mRectified);
    {
        float* wRect = mRectified.getWritePointer(0);
        float* wRect2 = mRectified2.getWritePointer(0);
    
        for (int i = 0; i < bufferNumSamples; i++)
        {
            //Envelope 1
            float sRect = mRectified.getSample(0, i);
            mEnvelopeFast.processAudioSample(sRect);
            wRect[i] = sRect;

            //Envelope 2
            float sRect2 = mRectified2.getSample(0, i);
            mEnvelopeSlow.processAudioSample(sRect2);
            wRect2[i] = sRect2;
        }


        //apply focus when snap is positive
        if (snap >=0)
        {
            mRectified2.applyGain(focus);
        }
        
        //avoid envelope to go below zero when inceasing snap and above 1 when decreasing
        for (int i = 0; i < bufferNumSamples; i++)
        {
            wRect[i] -= mRectified2.getSample(0, i); 
            if (mRectified.getSample(0, i) < 0 && snap >= 0)
            {
                wRect[i] = 0;
            }
            else if (mRectified.getSample(0, i) > 0 && snap < 0)
            {
                wRect[i] = 1;
            }
        }

        //apply gain from envelope
        mRectified.applyGain(snap);

    }
    //process with envelope and store in attack or sustain buffer
    for (int ch = 0; ch < totalNumInputChannels; ch++)
    {
        float* wAtt = mWet.getWritePointer(ch);
        float* wSus = mDry.getWritePointer(ch);
        for (int i = 0; i < bufferNumSamples; i++)
        {
            float sClean = buffer.getSample(ch, i);
            
            float sRect = mRectified.getSample(0, i);

            wAtt[i] = ((sClean * powf(2.f, sRect)) - sClean) * 2;
            wSus[i] = sClean;

        }
    }

    for (int ch = 0; ch < totalNumInputChannels; ch++)
    {
        float* w = buffer.getWritePointer(ch);
        for (int i = 0; i < bufferNumSamples; i++)
        {
            w[i] = mWet.getSample(ch, i) + mDry.getSample(ch, i);
            
        }
    }

    if (saturation > 0)
    {
        //make copies for highfreq
        mHighFreq.makeCopyOf(buffer);
        mHighFreqCopy.makeCopyOf(buffer);

        //high pass
        dsp::AudioBlock<float> mHighs(mHighFreq);
        hiPassFilter.process(dsp::ProcessContextReplacing<float>(mHighs));

        //get difference from highpass and total to get a low pass filtered signal
        for (int ch = 0; ch < totalNumInputChannels; ch++)
        {
            float* w = mHighFreqCopy.getWritePointer(ch);
            for (int i = 0; i < bufferNumSamples; i++)
            {
                w[i] = (mHighFreq.getSample(ch, i) * -1) + mHighFreqCopy.getSample(ch, i);
            }
        }

        //saturate high freq
        for (int ch = 0; ch < totalNumInputChannels; ch++)
        {
            float* w = mHighFreq.getWritePointer(ch);
            float* wB = buffer.getWritePointer(ch);
            for (int i = 0; i < bufferNumSamples; i++)
            {
                auto sat = (saturation / 100);
                w[i] = (tapeClipper((mHighFreq.getSample(ch, i) * gain)) / (gain * 0.5f)) + mHighFreqCopy.getSample(ch, i);

                wB[i] = (buffer.getSample(ch, i) * (1 - sat)) + (mHighFreq.getSample(ch, i) * sat);

            }
        }

    }

    buffer.applyGain(Decibels::decibelsToGain(output)); //Output gain

    //clipper
    if (clip == true)
    {

        //prepare for oversampling
        dsp::AudioBlock<float> processBlock(buffer);

        dsp::AudioBlock<float> highSampleRateBlock = mOversample->processSamplesUp(processBlock);

        for (int ch = 0; ch < totalNumOutputChannels; ch++)
        {
            auto* w = highSampleRateBlock.getChannelPointer(ch);
            for (int i = 0; i < highSampleRateBlock.getNumSamples(); i++)
            {
                w[i] = tapeClipper(highSampleRateBlock.getSample(ch, i));
            }
        }

        mOversample->processSamplesDown(processBlock);

        //prevent overs
        for (int ch = 0; ch < totalNumOutputChannels; ch++)
        {
            auto* w = buffer.getWritePointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); i++)
            {
                w[i] = jlimit<float>(-0.999f, 0.999f, buffer.getSample(ch, i));
            }
        }

    }


}

float DrumSnapperAudioProcessor::tapeClipper(float sample)
{
    float x = sample;

    float s = jlimit<float>(-0.95f, 0.95f, tanhf(powf(x, 5) + x) * 0.95f);
    
    return s;
}

//==============================================================================
bool DrumSnapperAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DrumSnapperAudioProcessor::createEditor()
{
    return new DrumSnapperAudioProcessorEditor (*this);
}

//==============================================================================
void DrumSnapperAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    MemoryOutputStream stream(destData, false);
    parameters.state.writeToStream(stream);
}

void DrumSnapperAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    ValueTree tree = ValueTree::readFromData(data, size_t(sizeInBytes));
    if (tree.isValid())
        parameters.state = tree;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DrumSnapperAudioProcessor();
}
