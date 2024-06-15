/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AP_assessment3AudioProcessor::AP_assessment3AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
apvts(*this, nullptr, "ParamTree", createParameterLayout())
{
    // ADSR
    attackParam = apvts.getRawParameterValue("attack");
    decayParam = apvts.getRawParameterValue("decay");
    sustainParam = apvts.getRawParameterValue("sustain");
    releaseParam = apvts.getRawParameterValue("release");
    
    // pulse width
    pulseWidthParam = apvts.getRawParameterValue("pulseWidth");
    pwmSustainParam = apvts.getRawParameterValue("pwmSustain");
    pwmSwitchParam = apvts.getRawParameterValue("pwmSwitch");
    pwmModeParam = apvts.getRawParameterValue("pwmMode");
    pwmRateParam = apvts.getRawParameterValue("pwmRate");
    
    // Osc & Distortion
    oscTypeParam = apvts.getRawParameterValue("oscType");
    triDistortionParam = apvts.getRawParameterValue("triDistortion");
    noiseDistortionParam = apvts.getRawParameterValue("noiseDistortion");
    
    // Pitch Bend
    pbSwitchParam = apvts.getRawParameterValue("pbSwitch");
    pbInitPitchParam = apvts.getRawParameterValue("pbInitPitch");
    pbTimeParam = apvts.getRawParameterValue("pbTime");
    
    // Vibrato
    vibSwitchParam = apvts.getRawParameterValue("vibSwitch");
    vibSpeedParam = apvts.getRawParameterValue("vibSpeed");
    vibAmountParam = apvts.getRawParameterValue("vibAmount");
    vibSustainParam = apvts.getRawParameterValue("vibSustain");
    
    // Arpeggiator
    arpSwitchParam = apvts.getRawParameterValue("arpSwitch");
    arpPatternParam = apvts.getRawParameterValue("arpPattern");
    arpOctaveParam = apvts.getRawParameterValue("arpOctave");
    arpSpeedParam = apvts.getRawParameterValue("arpSpeed");
    
    
    // bitcrusher
    rateReductionParam = apvts.getRawParameterValue("rateReduction");
    bitDepthParam = apvts.getRawParameterValue("bitDepth");
    
    // delay
    delayTimeParam = apvts.getRawParameterValue("delayTime");
    feedbackParam = apvts.getRawParameterValue("feedback");
    dryWetMixParam = apvts.getRawParameterValue("dryWetMix");
    
    // init synth
    synth.addSound( new ChiptuneSynthSound() );
    
    for (int i = 0; i< voiceCount; i++)
    {
        synth.addVoice( new ChiptuneSynthVoice(apvts) );
    }
}

AP_assessment3AudioProcessor::~AP_assessment3AudioProcessor()
{
}

//==============================================================================

const juce::String AP_assessment3AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AP_assessment3AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AP_assessment3AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AP_assessment3AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AP_assessment3AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AP_assessment3AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AP_assessment3AudioProcessor::getCurrentProgram()
{
    return 0;
}

void AP_assessment3AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AP_assessment3AudioProcessor::getProgramName (int index)
{
    return {};
}

void AP_assessment3AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AP_assessment3AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // init synth&smoother
    synth.setCurrentPlaybackSampleRate(sampleRate);
    smoothVal.reset(sampleRate, 2.0); // set sample rate and ramp time
    smoothVal.setCurrentAndTargetValue(1.0); // initialise
    
    // init delay
    int delayCount = 2;
    delays.clear();
    for (int i = 0; i < delayCount; i++)
    {
        delays.push_back(Delay());
        delays[i].setSize(sampleRate * 3);
        delays[i].setDelayTime(sampleRate * 0.5);
        delays[i].setFeedback(0.1);  // Example feedback value
        delays[i].setDryWetMix(0.2);
    }
    
    // init bitcrusher
    int bitcrusherCount = 2;
    bitcrushers.clear();
    for (int j = 0; j < bitcrusherCount; j++)
    {
        bitcrushers.push_back(Bitcrusher());
        bitcrushers[j].setSampleRateReduction(1);
        bitcrushers[j].setBitDepth(24);
    }
}

void AP_assessment3AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AP_assessment3AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void AP_assessment3AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Get the number of input and output channels for the audio buffer
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // clear buffer
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Retrieve pointers to the audio buffer's left and right channels
    int numSamples = buffer.getNumSamples();
    float* samplesLeft = buffer.getWritePointer(0);
    float* samplesRight = buffer.getWritePointer(1);
    
    // Render the next block of MIDI data through the synthesizer into our audio buffer
    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());
    
    // update bitcrusher
    for (int k = 0; k < 2; ++k)
    {
        bitcrushers[k].setSampleRateReduction(*rateReductionParam );
        bitcrushers[k].setBitDepth(*bitDepthParam );
    }
    
    // update delay
    for (int j = 0; j < 2; ++j)
    {
        delays[j].setDelayTime(getSampleRate() * (*delayTimeParam));
        delays[j].setFeedback(*feedbackParam);
        delays[j].setDryWetMix(*dryWetMixParam);
    }

    // Process each sample for DSP effects (bitcrushing followed by delay)
    for(int i = 0; i< numSamples; ++i)
    {
        float processedLeft = bitcrushers[0].process(samplesLeft[i]);
        float processedRight = bitcrushers[1].process(samplesRight[i]);
        
        samplesLeft[i] = delays[0].process(processedLeft);
        samplesRight[i] = delays[1].process(processedRight);
        
//        samplesLeft[i] = delays[0].process(samplesLeft[i]);
//        samplesRight[i] = delays[1].process(samplesRight[i]);
    }
}

//==============================================================================
bool AP_assessment3AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AP_assessment3AudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void AP_assessment3AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AP_assessment3AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AP_assessment3AudioProcessor();
}
