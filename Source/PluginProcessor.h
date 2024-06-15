/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ChiptuneSynthesiser.h"
#include "Bitcrusher.h"
#include "Delay.h"
#include <vector>

//==============================================================================
/**
*/
class AP_assessment3AudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    AP_assessment3AudioProcessor();
    ~AP_assessment3AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    
    //--------------------------------------------------------------------------
    // ADSR envelope parameters
    std::atomic<float>* attackParam;
    std::atomic<float>* decayParam;
    std::atomic<float>* sustainParam;
    std::atomic<float>* releaseParam;
    
    //--------------------------------------------------------------------------
    // Oscillator type selection parameter
    std::atomic<float>* oscTypeParam;
    
    //--------------------------------------------------------------------------
    // Parameters related to pulse width modulation
    std::atomic<float>* pulseWidthParam;
    std::atomic<float>* pwmSwitchParam;
    std::atomic<float>* pwmSustainParam;
    std::atomic<float>* pwmModeParam;
    std::atomic<float>* pwmRateParam;
    
    //--------------------------------------------------------------------------
    // Tri wave distortion enable parameter
    std::atomic<float>* triDistortionParam;
    
    //--------------------------------------------------------------------------
    // Noise distortion enable parameter
    std::atomic<float>* noiseDistortionParam;
    
    //--------------------------------------------------------------------------
    // Arpeggiator related parameters
    std::atomic<float>* arpSwitchParam;
    std::atomic<float>* arpPatternParam;
    std::atomic<float>* arpOctaveParam;
    std::atomic<float>* arpSpeedParam;
    
    //--------------------------------------------------------------------------
    // Pitch Bend related parameters
    std::atomic<float>* pbSwitchParam;
    std::atomic<float>* pbInitPitchParam;
    std::atomic<float>* pbTimeParam;
    
    //--------------------------------------------------------------------------
    // Vibrato related parameters
    std::atomic<float>* vibSwitchParam;
    std::atomic<float>* vibSpeedParam;
    std::atomic<float>* vibAmountParam;
    std::atomic<float>* vibSustainParam;
    
    //--------------------------------------------------------------------------
    // Bitcrusher parameters
    std::atomic<float>* rateReductionParam;
    std::atomic<float>* bitDepthParam;
    
    //--------------------------------------------------------------------------
    // Delay parameters
    std::atomic<float>* delayTimeParam;
    std::atomic<float>* feedbackParam;
    std::atomic<float>* dryWetMixParam;
    
    //==============================================================================
    // Audio processor value tree state to manage and automate plugin parameters.
    juce::AudioProcessorValueTreeState apvts;
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        
        // Osc type
        layout.add (std::make_unique <juce::AudioParameterChoice>(juce::ParameterID("oscType", 1), "Osc Type", juce::StringArray({ "Pulse", "Triangle", "Noise"}),0));
        
        // Pulse Width Modulation
        layout.add (std::make_unique <juce::AudioParameterChoice>(juce::ParameterID("pulseWidth", 1), "Pulse Width", juce::StringArray({ "12.5%", "25%", "50%"}),0));
        layout.add (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"pwmSwitch", 1},
            "PW Mod: On/Off", false));
        layout.add (std::make_unique <juce::AudioParameterFloat> (juce::ParameterID("pwmSustain", 1), "PW Mod: Sustain", 0.0, 1.0, 0.0));
        layout.add (std::make_unique <juce::AudioParameterChoice>(juce::ParameterID("pwmMode", 1), "PW Mod: Mode", juce::StringArray({ "12.5%to25%", "12.5%to50%", "25%to50%", "25%to12.5%", "50%to25%", "50%to12.5%"}),0));
        layout.add (std::make_unique <juce::AudioParameterFloat> (juce::ParameterID("pwmRate", 1), "PW Mod: Rate", 0.0, 1.0, 0.5));
        
        // triDistortion
        layout.add (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"triDistortion", 1},
            "Tri Distortion",true));
        
        // noiseDistortion
        layout.add (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"noiseDistortion", 1},
            "Noisy Noise",true));
        
        // Pitch Bend
        layout.add (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"pbSwitch", 1},
            "Bend: On/Off", false));
        layout.add (std::make_unique <juce::AudioParameterInt> (juce::ParameterID("pbInitPitch", 1), "Bend: Init.Pitch", -24, 24, 0));
        layout.add (std::make_unique <juce::AudioParameterFloat> (juce::ParameterID("pbTime", 1), "Bend: Time", 0.01, 3.0, 0.0));
        
        // Vibrato
        layout.add (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"vibSwitch", 1},
            "Vibrato: On/Off", false));
        layout.add (std::make_unique <juce::AudioParameterFloat> (juce::ParameterID("vibSpeed", 1), "Vibrato: Speed", 0.0, 1.0, 0.1));
        layout.add (std::make_unique <juce::AudioParameterFloat> (juce::ParameterID("vibAmount", 1), "Vibrato: Amount", 0.0, 1.0, 0.1));
        layout.add (std::make_unique <juce::AudioParameterFloat> (juce::ParameterID("vibSustain", 1), "Vibrato: Sustain", 0.0, 1.0, 0.0));
        
        // Arpeggiator
        layout.add (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"arpSwitch", 1},
            "Arp: On/Off", false));
        layout.add (std::make_unique <juce::AudioParameterChoice>(juce::ParameterID("arpPattern", 1), "Arp:Pattern", juce::StringArray({ "Minor3rd", "Major3rd", "Fourth", "Fifth", "Minor triad", "Major triad", "Major 7", "Major 9", "Random"}),0));
        layout.add (std::make_unique <juce::AudioParameterChoice>(juce::ParameterID("arpOctave", 1), "Arp:LoopMode", juce::StringArray({ "1 Repeat", "1 Octave", "2 Octaves"}),0));
        layout.add (std::make_unique <juce::AudioParameterFloat> (juce::ParameterID("arpSpeed", 1), "Arp:Speed", 0.0, 1.0, 0.5));
        
        // ADSR
        layout.add (std::make_unique <juce::AudioParameterFloat> (juce::ParameterID("attack", 1), "Envelope: Attack", 0.01, 5.0, 0.01));
        layout.add (std::make_unique <juce::AudioParameterFloat> (juce::ParameterID("decay", 1), "Envelope: Decay", 0.0, 5.0, 0.0));
        layout.add (std::make_unique <juce::AudioParameterFloat> (juce::ParameterID("sustain", 1), "Envelope: Sustain", 0.0, 1.0, 1.0));
        layout.add (std::make_unique <juce::AudioParameterFloat> (juce::ParameterID("release", 1), "Envelope: Release", 0.01, 5.0, 0.01));
        
        // Bitcrusher
        layout.add (std::make_unique <juce::AudioParameterInt> (juce::ParameterID("rateReduction", 1), "Bitcrusher: Rate Reduction", 1, 10, 1));
        layout.add (std::make_unique <juce::AudioParameterInt> (juce::ParameterID("bitDepth", 1), "Bitcrusher: Bit Depth", 1, 24, 24));
        
        // Delay
        layout.add (std::make_unique <juce::AudioParameterFloat> (juce::ParameterID("delayTime", 1), "Delay: Delay Time", 0.0, 1.0, 0.0));
        layout.add (std::make_unique <juce::AudioParameterFloat> (juce::ParameterID("feedback", 1), "Delay: Feedback", 0.0, 0.99, 0.0));
        layout.add (std::make_unique <juce::AudioParameterFloat> (juce::ParameterID("dryWetMix", 1), "Delay: Dry/Wet Mix", 0.0, 1.0, 0.2));
        
        return layout;
    }
    //==============================================================================

    juce::SmoothedValue<float> smoothVal; // Smoothed value to manage parameter transitions smoothly.
    std::vector<Delay> delays;
    std::vector<Bitcrusher> bitcrushers;
    
    juce::Synthesiser synth; // Synthesizer instance to manage multiple synthesis voices.
    int voiceCount = 10; // Number of voices the synthesizer can use.
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AP_assessment3AudioProcessor)
};
