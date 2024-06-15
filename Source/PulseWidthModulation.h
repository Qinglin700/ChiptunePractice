/*
  ==============================================================================

    PulseWidthModulation.h
    Created: 24 Apr 2024 10:35:03pm
    Author:  70

  ==============================================================================
*/

#pragma once
#include "PluginProcessor.h"
#include "PolyBLEPOscillator.h"

/**
 * @class PulseWidthModulation
 *
 * @brief Implements a pulse width modulation module using a phasor-based oscillator.
 *
 * This class provides dynamic control over pulse width modulation (PWM) by adjusting parameters like
 * pulse width, modulation rate, and sustain time, facilitated by a PolyBLEP oscillator. It uses
 * parameters from an `AudioProcessorValueTreeState` to allow seamless integration with audio plugin interfaces.
 */
class PulseWidthModulation : public Phasor
{
public:
    /// Constructor that initializes the APVTS reference.
    PulseWidthModulation(const juce::AudioProcessorValueTreeState& apvts)
    : apvts(apvts) 
    {
    }
    
    /// Sets the sample rate for the internal oscillator.
    void setSampleRate(float newSampleRate)
    {
        sampleRate = newSampleRate;
        arpOsc.setSampleRate(sampleRate);
        smoothPulseWidth.reset(sampleRate, 0.01f); // Set the sample rate and smoothing time
    }

    /// Updates the frequency of the internal oscillator based on the modulation rate.
    void setRate()
    {
        arpOsc.setFrequency(updateRate() * 10.0); // Updates frequency to range 0~10Hz
    }
    
    /// Resets the counter used for sustaining the current pulse width.
    void resetSustainCounter()
    {
        sustainCounter = 0;  // Initialize the counter to 0
    }
    
    /// Processes one sample of pulse width modulation and returns the current pulse width.
    float process()
    {
        setRate(); // Update rate each sample for DAW automation compatibility
        updateSustainParameters(); // Handle changes in sustain and PWM mode parameters
        
        if (sustainCounter < sustainSamples)
        {
            ++sustainCounter; // Increment the sustain counter
            switch (currentPwMode) // Selects the pulse width index based on the current mode
            {
                case 0: // Intended for modes 0 and 1
                case 1:
                    pwIndex = 0; // Corresponds to 12.5% pulse width
                    break;
                case 2: // Intended for modes 2 and 3
                case 3:
                    pwIndex = 1; // Corresponds to 25% pulse width
                    break;
                case 4: // Intended for modes 4 and 5
                case 5:
                    pwIndex = 2; // Corresponds to 50% pulse width
                    break;
            }
        } 
        else
        {
            calculateIndex(); // Adjusts pulse width index based on oscillator output
        }
        smoothPulseWidth.setTargetValue(pulseWidths[pwIndex]); // Set the target value for smoothing
        
        return smoothPulseWidth.getNextValue();  // Return the smoothed pulse width
    }
    


private:
    std::vector<float>pulseWidths {0.125, 0.25, 0.5}; // List of possible pulse widths
    Phasor arpOsc; // Oscillator used for pulse width modulation
    float sampleRate = 44100.0f; // Default sample rate
    
    int currentPwMode = 0; // Current mode of PWM
    int pwIndex = 0; // Index of the current pulse width
    int sustainSamples = 0;  // Samples to sustain a particular pulse width
    int sustainCounter = 0;  // Counts samples for sustain duration
    
    juce::SmoothedValue<float> smoothPulseWidth; // Smoothed value for pulse width
    
    
    const juce::AudioProcessorValueTreeState& apvts; // Reference to plugin parameters
    
    /// Updates the sustain time based on parameter value
    float updateSustain()
    {
        return apvts.getRawParameterValue("pwmSustain")->load();
    }
    
    /// Updates the current mode of PWM based on parameter value
    float updateMode()
    {
        return apvts.getRawParameterValue("pwmMode")->load();
    }
    
    /// Updates the modulation rate based on parameter value
    float updateRate()
    {
        return apvts.getRawParameterValue("pwmRate")->load();
    }
    
    /// Updates sustain and mode parameters from the APVTS, and reset sustain counter
    void updateSustainParameters()
    {
        float newSustain = updateSustain();
        if (newSustain * sampleRate != sustainSamples) // Check if sustainSamples needs an update
        {
            sustainSamples = static_cast<int>(newSustain * sampleRate);
            resetSustainCounter();
        }
        
        int newMode = updateMode();
        if (newMode != currentPwMode)
        {
            currentPwMode = newMode; // Check if mode has changed
            resetSustainCounter(); // Reset the counter when mode changes
        }
    }
    
    /// Calculates the new index for pulse width based on the oscillator output
    void calculateIndex()
    {
        float oscOutput = arpOsc.process(); // range 0~1
        currentPwMode = updateMode();

        switch (currentPwMode)
        {
            
            case 0: // 12.5%to25%
                pwIndex = static_cast<int>(oscOutput * 1.99); // range 0~1.99, should output 0, 1
                break;
            
            case 1: // 12.5%to50%
                pwIndex = static_cast<int>(oscOutput * 2.99); // range 0~2.99, should output 0, 1, 2
                break;
                
            case 2: // 25%to50%
                pwIndex = static_cast<int>(oscOutput * 1.99) + 1; // range 1~2.99, should output 1, 2
                break;
            
            case 3: // 25%to12.5%
                pwIndex = static_cast<int>((1.0f - oscOutput) * 1.99); //range 1.99~0, should output 1 0
                break;

            case 4: // 50%to25%
                pwIndex = static_cast<int>((1.0f - oscOutput) * 1.99) + 1; //range 2.99~1, should output  2 1
                break;
                
            case 5: // 50%to12.5%
                pwIndex = static_cast<int>((1.0f - oscOutput) * 2.99); //range 2.99~0, should output 2 1 0
                break;
        }
    }
};
