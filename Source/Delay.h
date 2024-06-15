/*
  ==============================================================================

    Delay.h
    Created: 8 Mar 2024 11:24:52am
    Author:  70

  ==============================================================================
*/

#pragma once

/**
 * @class Delay
 *
 * @brief Implements an audio delay effect with feedback and variable delay time.
 *
 * This class manages an audio delay line, allowing dynamic control over the delay time, feedback level,
 * and the wet/dry mix. It utilizes a circular buffer to handle the delay with real-time adjustments to
 * parameters possible. Integration with audio plugin interfaces is facilitated through the use of an
 * `AudioProcessorValueTreeState` for easy parameter management and automation.
 */
class Delay
{
public:
    /// set the maximum size of the delay line(in samples)
    void setSize(int _newSize)
    {
        size = _newSize;
        buffer.resize(size, 0.0f); // Resize the buffer to the new size and initialize with zeros.
    }
    
    /// Sets the feedback amount for the delay line. Range: 0.0 (no feedback) to just below 1.0 (high feedback).
    void setFeedback(float _feedback)
    {
        feedback = juce::jlimit(0.0f, 0.99f, _feedback); // Ensure feedback is within the valid range.
    }
    
    /// Sets the delay time in samples, adjusting the read position accordingly.
    void setDelayTime(float _delayTimeinSamples)
    {
        delayTime = _delayTimeinSamples;
        readPos = writePos - delayTime; // Calculate the new read position.
        
        if (readPos < 0) // Handle wrap-around if the new position is negative.
            readPos += size;
    }
    
    /// Sets the dry/wet mix ratio. Range: 0.0 (all dry) to 1.0 (all wet).
    void setDryWetMix(float _mix)
    {
        dryWetMix = juce::jlimit(0.0f, 1.0f, _mix); // Ensure the mix is within the valid range.
    }
    
    /// Processes a single sample, applying delay with feedback and returns the processed sample.
    float process(float inVal)
    {
        if (delayTime > 0)
        {
            float outVal = linearInterpolation(); // Read the interpolated value from the delay buffer.
            buffer[writePos] = inVal + outVal * feedback; // Write input plus feedback to buffer.
            
            // increment and wrap
            writePos++;
            if(writePos >=size)
                writePos -= size;
            
            readPos++;
            if(readPos >=size)
                readPos -= size;
            
            return inVal * (1.0f - dryWetMix) + outVal * dryWetMix;
        }
        
        // If delay time is zero, the output should ideally be the input.
        return inVal;
    }
    
private:
    std::vector<float> buffer; // Buffer to store delay samples.
    float readPos = 1;         // Current read position in the buffer.
    float writePos = 0;        // Current write position in the buffer.
    float feedback = 0.5;      // Feedback factor.
    float delayTime;           // Current delay time in samples.
    int size;                  // Maximum size of the delay line in samples.
    float dryWetMix = 0.2;     // Dry/wet mix ratio. Default 20% wet.
        
    /// Performs linear interpolation between the closest sample points.
    float linearInterpolation()
    {
        int indexA = floor(readPos);  // Integer part of read position.
        int indexB = indexA + 1; // Next index, wrap around if necessary.
        indexB %=size;
        
        // Sample value
        float ValA = buffer[indexA];
        float ValB = buffer[indexB];
        
        // Fractional part of read position.
        float frac = readPos - indexA;
        
        return (1-frac)* ValA + frac * ValB;
    }
};
