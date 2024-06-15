/*
  ==============================================================================

    Arpeggiator.h
    Created: 25 Apr 2024 11:15:01pm
    Author:  70

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>

/**
 * @class Arpeggiator
 *
 * @brief Implements an arpeggiator module for MIDI note manipulation in a musical context.
 *
 * This class generates arpeggiated patterns based on input MIDI notes, selectable patterns, octave ranges,
 * and speeds. It supports real-time control adjustments through an `AudioProcessorValueTreeState`, which
 * is integrated with the host's audio processing environment. The arpeggiator can dynamically adjust to
 * parameter changes like arpeggio pattern, number of octaves, and arpeggio speed, allowing for flexible
 * musical expression during audio production.
 */
class Arpeggiator
{
public:
    /// Constructor that initializes the APVTS reference for accessing plugin parameters.
    Arpeggiator(const juce::AudioProcessorValueTreeState& apvts)
       : apvts(apvts)
    {
        switchArpPattern(); // Initialize pattern on construction
        switchArpOctave(); // Initialize octave settings
    }

    /// Sets the sample rate of the audio processing, necessary for timing calculations
    void setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        updateSamplesPerNote();
    }

    /// Starts the arpeggio based on a given root MIDI note
    void startArpeggio(int _rootNote)
    {
        rootNote = _rootNote;
        currentNote = rootNote; // Start with the root note
        noteIncrement = 0; // Start at the initial octave
        noteIndex = 1; // since root note is played, arpegiator will start at the second note
        sampleCounter = 0;  // Reset sample counter
        switchArpPattern(); // Update the pattern
        switchArpOctave(); // Update the octave settings
    }

    /// Calculates and returns the next frequency to play based on the arpeggio pattern
    double getNextFrequency()
    {
        setSpeed();  // Update arpeggio speed (may vary due to DAW automation)
        
        if (sampleCounter >= samplesPerNote)
        {
            sampleCounter = 0; // Reset counter
            
            if (noteIndex <= pattern.size())
            {
                // Calculate the current MIDI note with octave adjustments
                int newNote = rootNote + pattern[noteIndex] + 12 * (noteIncrement);
                
                // Update currentNote to the new MIDI note value
                currentNote = newNote;

                incrementPattern(); // Move to the next note in the pattern
            }
        }
        sampleCounter++;
        
        return juce::MidiMessage::getMidiNoteInHertz(currentNote);
    }

private:
    std::vector<int> pattern;
    int noteIndex = 1;
    int noteIncrement = 0;       // Keeps track of octave shifts based on completed cycles
    int numOctaves = 0;
    int rootNote = 0;
    int currentNote = 0;
    double speed = 1.0;          // spped in Hz, how many notes per second
    double sampleRate = 44100.0; // Default sample rate, should be set from outside
    int samplesPerNote = 44100;  // How many samples between note changes
    int sampleCounter = 0;       // A counter to track samples between notes
    int currentArpPattern = 0;   // Index of the current arpeggio pattern
    int currentArpOctave = 0;    // Index of the current octave setting
    juce::Random randomEngine;   // Random number generator
    
    /// Updates the pattern index and handles octave wrapping
    void incrementPattern()
    {
        noteIndex++;
        if (noteIndex >= pattern.size()) 
        {
            if (numOctaves > 0)
            {
                noteIndex = 0;  // Only wrap around if we are using octaves
                noteIncrement++;  // Move to the next octave
                
                // Reset the note increment when the set number of octaves is reached
                if (noteIncrement >= numOctaves)
                {
                    noteIncrement = 0;
                }
            }
            else
            {
                noteIndex = pattern.size() - 1; // Hold the last note if not using octaves
            }
        }
    }
    /// Generate a random pattern using the random engine
    void generateRandomPattern()
    {
        int numberOfValues = 6;  // set the number of random values
        pattern.clear();  // Clear existing pattern
        pattern.push_back(0);  // Start with the root note

        for (int i = 0; i < numberOfValues; ++i) 
        {
            int randomValue = randomEngine.nextInt(juce::Range<int>(-7, 8)); // Generates a random number between -7 and 7
            pattern.push_back(randomValue);  // Add random interval
        }
    }
    
    /// Updates the speed of the arpeggio and recalculates the samplesPerNote
    void setSpeed()
    {
        speed = updateArpSpeed();
        updateSamplesPerNote();
    }
    
    /// Recalculates the number of samples per note based on the current speed
    void updateSamplesPerNote()
    {
        samplesPerNote = static_cast<int>(sampleRate * 0.5 * (1.01 - updateArpSpeed())); // Scale to proper range
    }
    
    //--------------------------------------------------------------------------
    
    const juce::AudioProcessorValueTreeState& apvts; // Reference to all controllable parameters.
    
    /// Retrieves and returns the selected arpeggio pattern
    float updateArpPattern()
    {
        return apvts.getRawParameterValue("arpPattern")->load();
    }
    
    /// Retrieves and returns the selected octave range
    float updateArpOctave()
    {
        return apvts.getRawParameterValue("arpOctave")->load();
    }
    
    /// Retrieves and returns the speed setting
    float updateArpSpeed()
    {
        return apvts.getRawParameterValue("arpSpeed")->load();
    }
    
    /// Selects the arpeggio pattern based on the selected pattern parameter
    void switchArpPattern()
    {
        currentArpPattern = static_cast<int>(updateArpPattern());
        
        if (currentArpPattern == 8) 
        {
            generateRandomPattern(); // set to random pattern
        }
        else
        {
            switch (currentArpPattern)
            {
                case 0: pattern = {0, 3}; break; // Minor Third
                case 1: pattern = {0, 4}; break; // Major Third
                case 2: pattern = {0, 5}; break; // Fourth
                case 3: pattern = {0, 7}; break; // Fifth
                case 4: pattern = {0, 3, 7}; break; // Minor triad
                case 5: pattern = {0, 4, 7}; break; // Major triad
                case 6: pattern = {0, 4, 7, 11}; break; // Major7
                case 7: pattern = {0, 4, 7, 11, 14}; break; // Major9
            }
        }
    }

    /// Selects the octave range for arpeggiation based on the selected octave parameter
    void switchArpOctave()
    {
        currentArpOctave = static_cast<int>(updateArpOctave());
        switch (currentArpOctave)
        {
            case 0: numOctaves = 0; break;
            case 1: numOctaves = 1; break;
            case 2: numOctaves = 2; break;
        }
    }

};
