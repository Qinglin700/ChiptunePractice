/*
  ==============================================================================

    Vibrato.h
    Created: 28 Apr 2024 1:52:51pm
    Author:  70

  ==============================================================================
*/

#pragma once
#include "PluginProcessor.h"
#include "PolyBLEPOscillator.h"

/**
 * @class Vibrato
 *
 * @brief Applies a vibrato effect to audio signals using a low-frequency oscillator (LFO).
 *
 * This class modulates the pitch of audio signals to create a vibrato effect, characterized by
 * a periodic variation in frequency. The vibrato parameters such as frequency and amount can be
 * dynamically adjusted based on plugin parameters. The class also supports a sustain period where
 * the vibrato effect is temporarily disabled to maintain a stable pitch.
 */
class Vibrato
{
public:
    /// Constructor that initializes the APVTS reference for accessing plugin parameters.
    Vibrato(const juce::AudioProcessorValueTreeState& apvts)
       : apvts(apvts)
    {
    }
    
    /// Sets the sample rate and updates the LFO with the new rate.
    void setSampleRate(float newSampleRate)
    {
        sampleRate = newSampleRate;
        vibratoLFO.setSampleRate(sampleRate);
    }
    
    /// Updates the frequency of the vibrato effect from plugin parameters.
    void setFrequency()
    {
        VibratoFreq = updateSpeed();
        vibratoLFO.setFrequency(VibratoFreq); // Frequency range: 0.5~5Hz
    }

    /// Resets the sustain counter to zero.
    void resetSustainCounter()
    {
        sustainCounter = 0;
    }
    
    /** Processes the vibrato effect, adjusting parameters and applying the effect as per the LFO output.
        @return Returns the calculated vibrato effect value, which modulates pitch. This value should be added to the raw sample value to                          achieve the vibrato effect.
    */
    float process()
    {
        updateSustainParameters();
        
        if (sustainCounter < sustainSamples)
        {
            ++sustainCounter; // Increment the sustain counter
            return 0.0f; // Return no vibrato effect during the sustain period.
        }

        // Update vibrato settings every time after the sustain period
        VibratoFreq = updateSpeed() * 5 + 3; // Scale to 3~8Hz
        VibratoAmount = updateAmount() / 20000; // Scale the amount for subtle modulation
        vibratoLFO.setFrequency(VibratoFreq);
        
        auto vibratoEffect = vibratoLFO.process() * VibratoAmount;
        return vibratoEffect;
    }
    
private:
    SinOsc vibratoLFO; // LFO used for vibrato effect
    float VibratoFreq = 5.0f;     // Default Vibrato frequency
    float VibratoAmount = 0.005f; // Default Vibrato amount
    float sampleRate = 44100.0f; // Default sample rate
    int sustainSamples = 0;  // Number of samples to sustain a pulse width index
    int sustainCounter = 0;  // Counter to track how long to sustain the current pulse width index
    

    const juce::AudioProcessorValueTreeState& apvts; // Reference to plugin parameters
    
    /// Retrieves the current sustain duration from plugin parameters.
    float updateSustain()
    {
        return apvts.getRawParameterValue("vibSustain")->load();
    }
    
    /// Retrieves the current vibrato speed from plugin parameters.
    float updateSpeed()
    {
        return apvts.getRawParameterValue("vibSpeed")->load();
    }
    
    /// Retrieves the current vibrato amount from plugin parameters.
    float updateAmount()
    {
        return apvts.getRawParameterValue("vibAmount")->load();
    }
    
    /// Updates the number of samples over which the vibrato settings should be sustained.
    void updateSustainParameters()
    {
        float newSustain = updateSustain();
        if (newSustain * sampleRate != sustainSamples) // Check if sustainSamples needs an update
        {
            sustainSamples = static_cast<int>(newSustain * sampleRate);
            resetSustainCounter();
        }
    }
};
