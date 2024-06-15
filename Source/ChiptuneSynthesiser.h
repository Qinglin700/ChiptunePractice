/*
  ==============================================================================

    ChiptuneSynthesiser.h

  ==============================================================================
*/

#pragma once

#include "Bitcrusher.h"
#include "PulseWidthModulation.h"
#include "Arpeggiator.h"
#include "PitchBend.h"
#include "PolyBLEPOscillator.h"
#include "Vibrato.h"
#include "Noise.h"

/**
 * @class ChiptuneSynthSound
 * 
 * @brief Represents a synthesizer sound for a chiptune synthesizer.
 *
 * This class defines the behavior of sounds in the synthesizer,
 * indicating that every sound can be applied to any MIDI note and any channel.
 */
class ChiptuneSynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote      (int) override      { return true; }
    //--------------------------------------------------------------------------
    bool appliesToChannel   (int) override      { return true; }
};

//==============================================================================
/**
 * @class ChiptuneSynthVoice
 *
 * @brief Implements a synthesizer voice for generating chiptune-style sounds.
 *
 * This class extends juce::SynthesiserVoice and provides functionalities specific to chiptune audio synthesis.
 * It integrates various modulation and synthesis techniques, including arpeggiation, pitch bending, vibrato,
 * and pulse width modulation (PWM). Each instance of this class is capable of handling a single voice in
 * a synthesizer setup, responding to MIDI events such as note on/off.
 *
 * The construction of the ChiptuneSynthVoice relies on passing an AudioProcessorValueTreeState object,
 * which facilitates easy integration with a plugin's parameter system for real-time control and automation
 * of the synthesizer's parameters.
 *
 * Key features:
 * - Arpeggiator: Modulates the pitch of the note in a rhythmic pattern.
 * - Pitch Bend: Allows dynamic changing of the note pitch during playback.
 * - Vibrato: Adds a periodic modulation to the pitch for a vibrating effect.
 * - Pulse Width Modulation: Offers control over the timbre of the note by adjusting the pulse width.
 */
class ChiptuneSynthVoice : public juce::SynthesiserVoice
{
public:
    /// Constructs a ChiptuneSynthVoice with necessary bindings to audio processor parameters.
    ChiptuneSynthVoice(const juce::AudioProcessorValueTreeState& apvts) : pulseWidthModulation(apvts), arpeggiator(apvts), pitchBend(apvts), vibrato(apvts), apvts(apvts){}
    //--------------------------------------------------------------------------
    /**
     * @brief Begins playing a note with a given MIDI note number and velocity.
     *
     * This function is called when a MIDI noteOn message is received. It sets up the oscillators,
     * pulse width modulation, pitch bending, arpeggiator, vibrato, and envelope generator according to
     * the received note and velocity.
     *
     * @param midiNoteNumber The MIDI note number that corresponds to the pitch of the note.
     * @param velocity The velocity at which the note is being played, affecting its volume and potentially other parameters.
     * @param sound A pointer to the SynthesiserSound object (not used here but required by the override).
     * @param unused An unused parameter for the current pitch wheel position, included to match the virtual function signature in the base class.
     */
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        playing = true;
        
        // Convert the MIDI note number to a frequency in Hz.
        freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        
        // Determine the oscillator type from parameters and configure the corresponding oscillator.
        currentOscType = updateOscType();
        switch (currentOscType) 
        {
            case 0: // Square oscillator
                squareOsc.setSampleRate(getSampleRate());
                squareOsc.setFrequency(freq);
                break;
            case 1: // Tri oscillator
                triWave.setSampleRate(getSampleRate());
                triWave.setFrequency(freq);
                break;
            case 2: // Noise generator
                noise.setSampleRate(getSampleRate());
                noise.setFrequency(freq);
                break;
        }
        
        // Initialize and set the pulse width for the square oscillator based on the current setting.
        currentPwIndex = updatePulseWidth();
        switch (currentPwIndex)
        {
            case 0: // 12.5%
                pulseWidth = 0.125f;
                break;
            case 1: // 25%
                pulseWidth = 0.25f;
                break;
            case 2: // 50%
                pulseWidth = 0.5f;
                break;
        }
        squareOsc.setPulseWidth(pulseWidth);
        
        
        // Initialize pulse width modulation with the current sample rate and reset its state.
        pulseWidthModulation.setSampleRate(getSampleRate());
        pulseWidthModulation.setRate();
        pulseWidthModulation.resetSustainCounter();
        
        // Initialize and start the pitch bend processor.
        pitchBend.setSampleRate(getSampleRate());
        pitchBend.startPitchBend(midiNoteNumber);
        
        // Initialize and start the arpeggiator.
        arpeggiator.setSampleRate(getSampleRate());
        arpeggiator.startArpeggio(midiNoteNumber);
        
        // Initialize vibrato, set its frequency, and reset its state.
        vibrato.setSampleRate(getSampleRate());
        vibrato.setFrequency();
        vibrato.resetSustainCounter();
        
        // Initialize the envelope generator, reset its state, and start the note.
        env.setSampleRate(getSampleRate());
        env.reset();
        env.noteOn();
        
        // Update ADSR parameters from the plugin's parameters.
        updateAdsrFromParameters();
    }
    
    //--------------------------------------------------------------------------
    /**
     * @brief Called when a MIDI noteOff message is received.
     *
     * This method handles the actions required when a note should stop playing. It triggers the
     * release phase of the envelope, allowing for a natural decay of the sound if `allowTailOff` is true.
     *
     * @param unused An unused parameter for the velocity, included to match the virtual function signature in the base class.
     * @param allowTailOff A unused boolean that determines whether the note should be allowed to decay naturally (true) or stop immediately (false).
     */
    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        env.noteOff();
    }
    
    //--------------------------------------------------------------------------
    /**
     * @brief Renders the next block of samples for playback.
     *
     * This method processes and generates audio samples for the duration specified by numSamples. It calculates
     * each sample based on the current state of the synthesizer voice, including modulations from the arpeggiator,
     * pitch bend, and vibrato effects. It applies waveform generation and processes effects like pulse width modulation
     * and distortion, before applying the envelope to the final output. Each channel of the output buffer is filled
     * with the generated audio samples.
     *
     * @param outputBuffer pointer to output
     * @param startSample position of first sample in buffer
     * @param numSamples number of smaples in output buffer
     */
    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
        if (playing) // check to see if this voice should be playing
        {
            // iterate through the necessary number of samples (from startSample up to startSample + numSamples)
            for (int sampleIndex = startSample; sampleIndex < (startSample+numSamples); ++sampleIndex)
            {
                float outputSample = 0.0f; // Initialize the output sample to zero for accumulation

                // Handle arpeggiator
                bool arpEnabled = updateArpSwitch();
                if (arpEnabled)
                    freq = arpeggiator.getNextFrequency();
                
                // Handle pitch bend
                bool pbEnabled = updatePbSwitch();
                if (pbEnabled)
                    freq = pitchBend.process();
                
                // Handle vibrato
                bool vibEnabled = updateVibSwitch();
                float vibratoEffect = vibrato.process();
                if (vibEnabled)
                    freq = freq * (1.0f + vibratoEffect);
                
                // Process oscillator types
                switch (currentOscType)
                {
                    case 0: // Square oscillator
                    {
                        squareOsc.setFrequency(freq);
                        bool pwmEnabled = updatePwmSwitch();
                        if (pwmEnabled)
                        {
                            pulseWidth = pulseWidthModulation.process();
                            squareOsc.setPulseWidth(pulseWidth);
                        }
                        outputSample = squareOsc.process() / 2; // reduce the volume, output range +-0.5
                        break;
                    }
                        
                    case 1: // Triangle oscillator with optional distortion
                    {
                        triWave.setFrequency(freq);
                        bool triDistortionEnabled = updateTriDistortion();
                        if (triDistortionEnabled)
                        {
                            float rawSample = triWave.process();
                            bitcrusher.setSampleRateReduction(2);
                            bitcrusher.setBitDepth(4);
                            outputSample = bitcrusher.process(rawSample) * 1.2; // Adjust volume
                        }
                        else
                        {
                            outputSample = triWave.process() * 1.2; // Adjust volume
                        }
                        break;
                    }
                        
                    case 2: // Noise oscillator with optional distortion
                    {
                        noise.setFrequency(freq);
                        bool noiseDistortionEnabled = updateNoiseDistortion();
                        //std::cout << "The value is: " << noiseDistortionEnabled << std::endl;
                        if (noiseDistortionEnabled)
                        {
                            outputSample = noise.process() * 0.5; // reduce the volume
                        }
                        else
                        {
                            outputSample = random.nextFloat() - 0.5; // Generate simple random noise, (-0.5 ~ 0.5)
                        }
                        break;
                    }
                }
                
                // Get the next sample from the envelope generator
                float envValue = env.getNextSample();
                
                // for each channel, write the currentSample float to the output
                for (int chan = 0; chan<outputBuffer.getNumChannels(); ++chan)
                {
                    // The output sample is scaled by 0.5 so that it is not too loud by default
                    outputBuffer.addSample (chan, sampleIndex, outputSample * 0.5 * envValue);
                }
                
                // Handle note-off and clean up if the envelope has completed its release phase
                if( ! env.isActive() )
                {
                    clearCurrentNote();
                    playing = false;
                }
            }
        }
    }
    //--------------------------------------------------------------------------
    void pitchWheelMoved(int) override {}
    //--------------------------------------------------------------------------
    void controllerMoved(int, int) override {}
    //--------------------------------------------------------------------------
    /**
     Can this voice play a sound.

     @param sound a juce::SynthesiserSound* base class pointer
     @return sound cast as a pointer to an instance of ChiptuneSynthSound
     */
    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<ChiptuneSynthSound*> (sound) != nullptr;
    }

    
private:
    bool playing = false; // State variable to indicate whether the synth voice is currently playing.
    Bitcrusher bitcrusher;
    PulseWidthModulation pulseWidthModulation;
    Arpeggiator arpeggiator;
    PitchBend pitchBend;
    Vibrato vibrato;
    SquareOsc squareOsc;
    TriOsc triWave;
    Noise noise;
    juce::Random random; // Utility for generating random numbers, used in noise synthesis.
    juce::ADSR env; // Envelope generator for controlling the amplitude envelope of the sound.
    
    float pulseWidth = 0.5f; // Current setting for the pulse width of the square wave oscillator.
    float freq = 440.0f;     // Current frequency of the note being played.
    int currentOscType = 0;  // 0 for pulse, 1 for tri, 2 for noise.
    int currentPwIndex = 0;  // 0 for 12.5%, 1 for 25%, 2 for 50%.
    
    //--------------------------------------------------------------------------
    
    const juce::AudioProcessorValueTreeState& apvts; // Reference to plugin parameters
    
    /// Updates the ADSR envelope parameters from the plugin's parameter state.
    void updateAdsrFromParameters()
    {
        auto attackParam = apvts.getRawParameterValue("attack")->load();
        auto decayParam = apvts.getRawParameterValue("decay")->load();
        auto sustainParam = apvts.getRawParameterValue("sustain")->load();
        auto releaseParam = apvts.getRawParameterValue("release")->load();
        
        juce::ADSR::Parameters envParams;
        envParams.attack = attackParam;
        envParams.decay = decayParam;
        envParams.sustain = sustainParam;
        envParams.release = releaseParam;
        
        env.setParameters(envParams);
    }
    
    /// Retrieves and returns the current oscillator type based on user settings.
    float updateOscType()
    {
        return apvts.getRawParameterValue("oscType")->load();
    }
    
    /// Retrieves and returns the current pulse width setting from the parameters.
    float updatePulseWidth()
    {
        return apvts.getRawParameterValue("pulseWidth")->load();
    }
    
    /// Retrieves the arpeggiator speed setting from the parameters.
    float updateArpSpeed()
    {
        return apvts.getRawParameterValue("arpSpeed")->load();
    }
    
    /// Determines whether triangle wave distortion should be enabled based on the user parameter.
    bool updateTriDistortion()
    {
        auto* triDistortionParam = apvts.getRawParameterValue("triDistortion");
        return triDistortionParam ? (triDistortionParam->load() > 0.5f) : false;
    }
    
    /// Determines whether noise distortion should be enabled based on the user parameter.
    bool updateNoiseDistortion()
    {
        auto* noiseDistortionParam = apvts.getRawParameterValue("noiseDistortion");
        return noiseDistortionParam ? (noiseDistortionParam->load() > 0.5f) : false;
    }
    
    /// Checks if PWM should be activated based on a user-controlled switch.
    bool updatePwmSwitch()
    {
        auto* pwmSwitchParam = apvts.getRawParameterValue("pwmSwitch");
        return pwmSwitchParam ? (pwmSwitchParam->load() > 0.5f) : false;
    }
    
    /// Checks if the arpeggiator should be activated based on a user-controlled switch.
    bool updateArpSwitch()
    {
        auto* arpSwitchParam = apvts.getRawParameterValue("arpSwitch");
        return arpSwitchParam ? (arpSwitchParam->load() > 0.5f) : false;
    }
    
    /// Checks if pitch bending should be activated based on a user-controlled switch.
    bool updatePbSwitch()
    {
        auto* pbSwitchParam = apvts.getRawParameterValue("pbSwitch");
        return pbSwitchParam ? (pbSwitchParam->load() > 0.5f) : false;
    }
    
    /// Checks if vibrato should be activated based on a user-controlled switch.
    bool updateVibSwitch()
    {
        auto* vibSwitchParam = apvts.getRawParameterValue("vibSwitch");
        return vibSwitchParam ? (vibSwitchParam->load() > 0.5f) : false;
    }
};
