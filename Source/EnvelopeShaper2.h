#pragma once

#include "AudioDevEffect.h"

#include <cmath>

class EnvelopeShaper2 : public AudioDevEffect
{
public:
    // callback for initializing with samplerate
    void prepareForPlayback(float samplerate) override
    {
        m_Samplerate = samplerate;

        update();
    }

    // callback for audio processing
    void processAudioSample(float& sample) override
    {
        //int nAttks = (m_Samplerate * (10 * 0.001));

        if (sample > m_Envelope)
        {
            m_Envelope += m_Attack * (sample - m_Envelope);
        }
        else if (sample <= m_Envelope)
        {
            m_Envelope += m_Release * (sample - m_Envelope);
        }
        sample = m_Envelope;
    }

    // setters for compression parameters
    void setAttack(float attack)
    {
        m_AttackInMilliseconds = attack;
        update();
    }

    void setRelease(float release)
    {
        m_ReleaseInMilliseconds = release;
        update();
    }

private:
    // envelope shaper private variables and functions
    float m_Envelope{ 0.0f };
    float m_Samplerate{ 44100 };
    float m_AttackInMilliseconds{ 0.0f };
    float m_ReleaseInMilliseconds{ 0.0f };
    float m_Attack{ 0.0f };
    float m_Release{ 0.0f };
    double holdcount{ 0.0 };

    // update attack and release scaling factors
    void update()
    {
        m_Attack = calculateExp(m_AttackInMilliseconds);
        m_Release = calculateExp(m_ReleaseInMilliseconds);
    }

    // calculate scaling factor from a value in milliseconds
    float calculate(float time)
    {
        if (time <= 0.f || m_Samplerate <= 0.f)
        {
            return 1.f;
        }
        return 1.f - exp(-1.f / (time * 0.001f * m_Samplerate));
        //return 1.f - pow(20.0, (-1.f / (time * 0.001f * m_Samplerate)));
        //return 1.f - log10l(-1.f / (time * 0.001f * m_Samplerate)); 
        
    }

    float calculateExp(float time)
    {
        if (time <= 0.f || m_Samplerate <= 0.f)
        {
            return 1.f;
        }
        return 1.f - exp(-1.f / (time * 0.001f * m_Samplerate));
        //return 1.f - pow(20.0, (-1.f / (time * 0.001f * m_Samplerate)));
    }
};