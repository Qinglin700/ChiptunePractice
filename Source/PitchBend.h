/*
  ==============================================================================

    PitchBend.h
    Created: 27 Apr 2024 12:48:18pm
    Author:  70

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>

/**
 * @class PitchBend
 *
 * @brief Handles pitch bending effects using MIDI note frequencies.
 *
 * This class manipulates pitch based on MIDI inputs, changing frequencies over a specified time interval,
 * controlled via parameters stored in a `AudioProcessorValueTreeState`.
 */
class PitchBend
{
public:
    /// Constructor that initializes the APVTS reference for accessing plugin parameters.
    PitchBend(const juce::AudioProcessorValueTreeState& apvts)
    : apvts(apvts){}
    
    /// Sets the sample rate and recalculates the number of samples over which to apply the pitch bend.
    void setSampleRate(double _sampleRate)
    {
        sampleRate = _sampleRate;
        calculateBendSamples();
    }

    /// Starts the pitch bending process from the initial note derived from settings, gradually bending to the input MIDI note.
    void startPitchBend(int _inputNote)
    {
        inputNote = _inputNote; // Store the input MIDI note
        inputFreq = juce::MidiMessage::getMidiNoteInHertz(inputNote); // Convert the MIDI note to frequency
        
        initNote = updateInitPitch(); // Get the initial pitch offset from the parameters
        currentFreq = juce::MidiMessage::getMidiNoteInHertz(inputNote + initNote); // Calculate the initial frequency and store in currentFreq
        
        bendDelta = (inputFreq - currentFreq) / bendSamples; // Compute the frequency change per sample for the pitch bend
    }
    
    /// Processes one sample of the pitch bend effect, gradually changing frequency towards the target.
    float process()
    {
        // Adjust frequency only if it has not reached the target frequency
        if ((bendDelta > 0 && currentFreq < inputFreq) || (bendDelta < 0 && currentFreq > inputFreq))
        {
            currentFreq += bendDelta; // Adjust frequency towards the target
        }
        else
        {
            currentFreq = inputFreq; // Finalize at target frequency to prevent overshooting
        }
        return currentFreq;
    }
    
    
private:
    int inputNote = 0;           // MIDI note number of the input note.
    int initNote = 0;            // Initial MIDI note number to start bending from.
    float currentFreq = 0.0f;    // Current frequency during the pitch bend process.
    float inputFreq = 0.0f;      // Target frequency based on the input MIDI note.
    int bendSamples = 0;         // Total number of samples over which to spread the pitch bend.
    float bendDelta = 0;         // Amount of frequency change per sample.
    double sampleRate = 44100.0; // Default sample rate, should be set to match the host environment.
    
    
    const juce::AudioProcessorValueTreeState& apvts; // Reference to all controllable parameters.
    
    /// Retrieves the initial pitch bend setting from the parameters.
    int updateInitPitch()
    {
        return apvts.getRawParameterValue("pbInitPitch")->load();
    }
    
    /// Retrieves the time over which the pitch bend should occur.
    float updateTime()
    {
        return apvts.getRawParameterValue("pbTime")->load();
    }
    
    /// Calculates the number of samples over the specified bend time.
    void calculateBendSamples()
    {
        bendSamples = static_cast<int>(updateTime() * sampleRate);
    }
};
