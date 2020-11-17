#pragma once

// interface class all Audio Dev effects inherit from
class AudioDevEffect
{
public:
    // callback for initializing with samplerate
    virtual void prepareForPlayback(float samplerate) = 0;
    
    // callback for audio processing
    virtual void processAudioSample(float & sample) = 0;
    

};