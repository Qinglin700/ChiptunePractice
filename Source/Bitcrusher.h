/*
  ==============================================================================

    Bitcrusher.h
    Created: 31 Mar 2024 11:14:56pm
    Author:  70

  ==============================================================================
*/

#pragma once

#include <vector>
#include <cmath>

/**
 * @class Bitcrusher
 *
 * @brief Simulates bit crushing by reducing sample rate and bit depth of audio signals.
 *
 * This class modifies audio fidelity by selectively skipping samples and quantizing sample values,
 * allowing for adjustable levels of distortion. The primary methods include setting the sample rate
 * reduction and bit depth.
 */
class Bitcrusher
{
public:
    
    /** Set the sample rate reduction factor.
        @param reductionFactor      Controls the interval at which samples are processed, where
                                 values greater than 1 reduce the audio fidelity by skipping samples.
    */
    void setSampleRateReduction(int reductionFactor)
    {
        sampleRateReduction = std::max(1, reductionFactor); // Ensure at least 1
    }

    /** Set the bit depth reduction.
        @param depth      between 1 to 24, where 24 means full resolution (no reduction)
    */
    void setBitDepth(int depth) 
    {
        bitDepth = std::clamp(depth, 1, 24); // Ensure bit depth is in valid range
        bitDepthScale = std::pow(2, bitDepth) - 1;
    }

    
    /// Process a single sample and return the bitcrushed value
    float process(float inVal) 
    {
        // Adjust input range to full scale [-1, 1] for processing
        inVal = 2 * inVal;  // Map from [-0.5, 0.5] to [-1, 1]

        // Apply sample rate reduction by only processing every N-th sample
        if (++currentSampleCount >= sampleRateReduction) 
        {
            currentSampleCount = 0;

            // Apply bit depth reduction
            float scaled = inVal * bitDepthScale; // Scale input to bit depth
            float quantized = std::round(scaled); // Quantize to nearest integer
            lastProcessedSample = quantized / bitDepthScale; // Scale back to original range
            
            // Adjust output back to original range [-0.5, 0.5]
            lastProcessedSample /= 2;
        }
        return lastProcessedSample;
    }

private:
    int sampleRateReduction = 1; // No reduction by default
    int bitDepth = 24; // Full resolution by default
    float bitDepthScale = std::pow(2, 24) - 1; // Scale for the current bit depth
    float lastProcessedSample = 0; // Last processed sample (for sample rate reduction)
    int currentSampleCount = 0; // Counter for sample rate reduction
};
