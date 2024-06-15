/*
  ==============================================================================

    PolyBLEPOscillator.h
    Created: 27 Apr 2024 6:11:46pm
    Author:  70

  ==============================================================================
*/

#pragma once
#include <cmath>
#include <JuceHeader.h>

/**
 * @class Phasor
 *
 * @brief Implements the foundational components of an oscillator used in audio synthesis.
 *
 * This class manages the phase of an oscillator, enabling the generation of various waveforms by deriving classes.
 * It offers functionality to set and retrieve the oscillator's frequency and sample rate, and calculates the phase increment
 * necessary for generating continuous waveforms. The class also includes a PolyBLEP function to minimize aliasing,
 * improving the quality of the output waveforms especially at higher frequencies.
 */
class Phasor 
{
public:
    virtual ~Phasor() {} // Virtual destructor
    
    float getPhase()
    {
        updatePhase(); // Update the phase before returning it
        return phase;
    }
    
    float process()
    {
        updatePhase();
        return output(phase);
    }
    
    virtual float output(float p)
    {
        return p;
    }
    
    void setSampleRate(float SR)
    {
        sampleRate = SR;
    }

    void setFrequency(float Freq)
    {
        frequency = Freq;
        phaseDelta = frequency / sampleRate;
    }
    
    float getPhaseDelta()
    {
        return phaseDelta;
    }
    
    /// PolyBLEP function to reduce aliasing in waveform generation
    float poly_blep(float t)
    {
        float dt = phaseDelta;
        if (t < dt) 
        {
            // Close to zero
            t /= dt;
            return t+t - t*t - 1.0;
        } else if (t > 1.0f - dt) {
            // Close to 1
            t = (t - 1.0) / dt;
            return t*t + t+t + 1.0;
        }
        
        // No correction needed elsewhere
        return 0.0;
    }

    
private:
    float frequency = 0.0f;       // Frequency of the oscillator
    float sampleRate = 44100.0f;  // samples per sec
    float phase = 0.0f;           // Current phase of the oscillator
    float phaseDelta = 0.0f;      // Change in phase per sample
    
    /// Updates the phase, wrapping around at 1.0
    void updatePhase()
    {
        phase += phaseDelta;
        if (phase > 1.0f)
        {
            phase -= 1.0f;
        }
    }

};
// ==================================

/// Sine wave oscillator derived from Phasor
class SinOsc: public Phasor
{
    float output(float p)override
    {
        return std::sin(p * 2.0 * M_PI);
    }
};

/// Square wave oscillator with PolyBLEP function derived from Phasor
class SquareOsc: public Phasor
{
public:

    float output(float p) override
    {
        float outVal = (p < pulseWidth) ? 1.0f : -1.0f;
        outVal += poly_blep(p);
        outVal -= poly_blep(fmod(p + (1.0 - pulseWidth), 1.0f));
        return outVal; 
    }
    
    void setPulseWidth(float pw)
    {
        pulseWidth = pw;
    }
    
private:
    float pulseWidth = 0.5f;
};

/// Triangle wave oscillator derived from Phasor
/// The TriWave in NES has an asymmetrical rise and fall.
/// The falling segment of the waveform is slightly curved and the rise is linear.
class TriOsc: public Phasor
{
    float output(float p) override 
    {
        if (p < 0.5)
        {
            // Linear rise: 0 to 1 in half the waveform
            p = p * 4.0f - 1.0f;  // Scale and shift to go from -1 to 1
        }
        else
        {
            // Curved fall: use a quadratic curve for slight curve effect
            float t = (p - 0.5f) * 2.0f; // Scale phase to go from 0 to 1
            p = 1.0f - 2.0f * t * t; // Quadratic curve
        }
        p /= 2; // // Scale from -1 to 1 to -0.5~0.5
        return p;
    }
};

///  Sawtooth wave oscillator derived from Phasor
class SawOsc: public Phasor
{
public:
    float output(float p)override
    {
        return p - 0.5f;
    }
};

