#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


class Chorus_FlangerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Chorus_FlangerAudioProcessorEditor (Chorus_FlangerAudioProcessor&);
    ~Chorus_FlangerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    Slider mDryWetSlider;
    Slider mFeedbackSlider;

    Chorus_FlangerAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Chorus_FlangerAudioProcessorEditor)
};
