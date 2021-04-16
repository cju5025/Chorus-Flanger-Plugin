#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Chorus_FlangerAudioProcessor::Chorus_FlangerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    addParameter(mDryWetParameter = new AudioParameterFloat("drywet", "Dry / Wet", 0.f, 1.f, 0.5f));
     
    addParameter(mFeedbackParameter = new AudioParameterFloat("feedback", "Feedback", 0.f, 1.f, 0.5f));
    
    addParameter(mDepthParameter = new AudioParameterFloat("depth", "Depth", 0.0, 1.f, 0.5f));
    
    addParameter(mRateParameter = new AudioParameterFloat("rate", "Rate", 0.1f, 20.f, 10.f));
    
    addParameter(mPhaseOffsetParameter = new AudioParameterFloat("phaseoffset", "Phase Offset", 0.f, 1.f, 0.f));
    
    addParameter(mTypeParameter = new AudioParameterInt("type", "Type", 0, 1, 0));
     
     
     
     mCircularBufferLeft = nullptr;
     mCircularBufferRight = nullptr;
     mCircularBufferWriteHead = 0;
     mCircularBufferLength = 0;
     
     mFeedbackLeft = 0;
     mFeedbackRight = 0;
    
     mLFOPhase = 0;
}

Chorus_FlangerAudioProcessor::~Chorus_FlangerAudioProcessor()
{
    if (mCircularBufferLeft != nullptr)
    {
        delete [] mCircularBufferLeft;
        mCircularBufferLeft = nullptr;
    }
    
    if (mCircularBufferRight != nullptr)
    {
        delete [] mCircularBufferRight;
        mCircularBufferRight = nullptr;
        
    }
}

//==============================================================================
const juce::String Chorus_FlangerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Chorus_FlangerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Chorus_FlangerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Chorus_FlangerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Chorus_FlangerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Chorus_FlangerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Chorus_FlangerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Chorus_FlangerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Chorus_FlangerAudioProcessor::getProgramName (int index)
{
    return {};
}

void Chorus_FlangerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Chorus_FlangerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
        mLFOPhase = 0;
    
        mCircularBufferLength = sampleRate * MAX_DELAY_TIME;
        
        if (mCircularBufferLeft == nullptr)
        {
            mCircularBufferLeft = new float[mCircularBufferLength];
        }
        
        zeromem(mCircularBufferLeft, mCircularBufferLength * sizeof(float));
        
        if (mCircularBufferRight == nullptr)
        {
            mCircularBufferRight = new float[mCircularBufferLength];
        }
        
        zeromem(mCircularBufferRight, mCircularBufferLength * sizeof(float));
        
        mCircularBufferWriteHead = 0;
        
}

void Chorus_FlangerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Chorus_FlangerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void Chorus_FlangerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
        juce::ScopedNoDenormals noDenormals;
        auto totalNumInputChannels  = getTotalNumInputChannels();
        auto totalNumOutputChannels = getTotalNumOutputChannels();


        for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
            buffer.clear (i, 0, buffer.getNumSamples());
            
        float* leftChannel = buffer.getWritePointer(0);
        float* rightChannel = buffer.getWritePointer(1);

        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            float lfoOutLeft = sin(2*M_PI * mLFOPhase);
            
            float lfoPhaseRight = mLFOPhase + *mPhaseOffsetParameter;
            
            if (lfoPhaseRight > 1)
            {
                lfoPhaseRight -= 1;
            }
            
            float lfoOutRight = sin(2*M_PI * lfoPhaseRight);
            
            lfoOutLeft *= *mDepthParameter;
            lfoOutRight *= *mDepthParameter;
            

            float lfoOutMappedLeft = jmap(lfoOutLeft, -1.f, 1.f, 0.005f, 0.03f);
            float lfoOutMappedRight = jmap(lfoOutRight, -1.f, 1.f, 0.005f, 0.03f);

            if (*mTypeParameter == 1)
            {
                lfoOutMappedLeft = jmap(lfoOutLeft, -1.f, 1.f, 0.001f, 0.005f);
                lfoOutMappedRight = jmap(lfoOutRight, -1.f, 1.f, 0.001f, 0.005f);
            }
            
            float delayTimeSamplesLeft = getSampleRate() * lfoOutMappedLeft;
            float delayTimeSamplesRight = getSampleRate() * lfoOutMappedRight;
            
            mLFOPhase += *mRateParameter / getSampleRate();
            
            if ( mLFOPhase > 1 )
            {
                mLFOPhase -= 1;
            }
            
            mCircularBufferLeft[mCircularBufferWriteHead] = leftChannel[i] + mFeedbackLeft;
            mCircularBufferRight[mCircularBufferWriteHead] = rightChannel[i] + mFeedbackRight;
            
            float delayReadHeadLeft = mCircularBufferWriteHead - delayTimeSamplesLeft;
            
            if (delayReadHeadLeft < 0)
            {
                delayReadHeadLeft += mCircularBufferLength;
            }
            
            float delayReadHeadRight = mCircularBufferWriteHead - delayTimeSamplesRight;
            
            if (delayReadHeadRight < 0)
            {
                delayReadHeadRight += mCircularBufferLength;
            }
            
            int readHeadLeft_x = (int)delayReadHeadLeft;
            int readHeadLeft_x1 = readHeadLeft_x + 1;
            float readHeadFloatLeft = delayReadHeadLeft - readHeadLeft_x;
            
            if ( readHeadLeft_x1 >= mCircularBufferLength )
            {
                readHeadLeft_x1 -= mCircularBufferLength;
            }
            
            int readHeadRight_x = (int)delayReadHeadRight;
            int readHeadRight_x1 = readHeadRight_x + 1;
            float readHeadFloatRight = delayReadHeadRight - readHeadRight_x;
            
            if ( readHeadRight_x1 >= mCircularBufferLength )
            {
                readHeadRight_x1 -= mCircularBufferLength;
            }
            
            
            float delaySampleLeft = linear_interpolation(mCircularBufferLeft[readHeadLeft_x], mCircularBufferLeft[readHeadLeft_x1], readHeadFloatLeft);
            float delaySampleRight = linear_interpolation(mCircularBufferRight[readHeadRight_x], mCircularBufferRight[readHeadRight_x1], readHeadFloatRight);
            
            mFeedbackLeft = delaySampleLeft * *mFeedbackParameter;
            mFeedbackRight = delaySampleRight * *mFeedbackParameter;
            
            mCircularBufferWriteHead++;
            
            buffer.setSample(0, i, buffer.getSample(0, i) * (1 - *mDryWetParameter) + delaySampleLeft * *mDryWetParameter);
            buffer.setSample(1, i, buffer.getSample(1, i) * (1 - *mDryWetParameter) + delaySampleRight * *mDryWetParameter);
            
            
            if ( mCircularBufferWriteHead >= mCircularBufferLength )
            {
                mCircularBufferWriteHead = 0;
            }
        }
}

//==============================================================================
bool Chorus_FlangerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Chorus_FlangerAudioProcessor::createEditor()
{
    return new Chorus_FlangerAudioProcessorEditor (*this);
}

//==============================================================================
void Chorus_FlangerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
        std::unique_ptr<XmlElement> xml(new XmlElement("ChorusFlanger"));

        xml->setAttribute("DryWet", *mDryWetParameter);
        xml->setAttribute("Feedback", *mFeedbackParameter);
        xml->setAttribute("Depth", *mDepthParameter);
        xml->setAttribute("Rate", *mRateParameter);
        xml->setAttribute("PhaseOffset", *mPhaseOffsetParameter);
        xml->setAttribute("Type", *mTypeParameter);

        copyXmlToBinary(*xml, destData);
}

void Chorus_FlangerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml.get() != nullptr && xml->hasTagName("ChorusFlanger"))
    {
        *mDryWetParameter = xml->getDoubleAttribute("DryWet");
        *mFeedbackParameter = xml->getDoubleAttribute("Feedback");
        *mDepthParameter = xml->getDoubleAttribute("Depth");
        *mRateParameter = xml->getDoubleAttribute("Rate");
        *mPhaseOffsetParameter = xml->getDoubleAttribute("PhaseOffset");
        *mTypeParameter = xml->getIntAttribute("Type");
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Chorus_FlangerAudioProcessor();
}

float Chorus_FlangerAudioProcessor::linear_interpolation(float sample_x, float sample_x1, float inPhase)
{
    return (1 - inPhase) * sample_x + inPhase * sample_x1;
}
