/*
  ==============================================================================

    Noise.h
    Created: 28 Apr 2024 8:34:44pm
    Author:  70

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

/**
 * @class Noise
 *
 * @brief Generates pseudo-random noise using a pre-computed wavetable approach.
 *
 * This class provides a noise generator that simulates 4-bit noise by using a wavetable filled with
 * random values. The wavetable implementation allows for precise control over playback frequency
 * and phase, making it suitable for audio synthesis applications where noise is a desired component.
 * Parameters such as sample rate and frequency can be dynamically adjusted.
 */
class Noise
{
public:
    /// Constructor that initializes and fills the wavetable with pseudo-random 4-bit scaled values.
    Noise()
    {
        waveTable.resize(wtSize);
        for (int i = 0; i < wtSize; ++i)
        {
            waveTable[i] = static_cast<float>((std::rand() % 16) - 8) / 8.0f; // Scale to [-1,1)
        }
    }

    /// Sets the sample rate for noise generation, influencing the frequency calculation.
    void setSampleRate(float newSampleRate)
    {
        sampleRate = newSampleRate;
    }

    /// Sets the playback frequency of the noise and updates the phase increment accordingly.
    void setFrequency(float freq)
    {
        frequency = freq;
        increment = frequency * static_cast<double>(wtSize) / sampleRate;
    }

    /// Generates the next sample of noise from the wavetable and handles phase wrapping.
    float process()
    {
        // Calculate the current sample and update phase
        int index = static_cast<int>(phase);
        float output = waveTable[index];

        // Increment and wrap phase
        phase += increment;
        if (phase >= wtSize)
            phase -= wtSize;

        return output;  // Return the current sample of noise.
    }

private:
    std::vector<float> waveTable; // Wavetable storing the noise samples.
    
    const int wtSize = 3000;
    double frequency = 440.0; // Default frequency (A4)
    double phase = 0.0;
    double increment;
    float sampleRate = 44100.0f; // Default sample rate
};
    
