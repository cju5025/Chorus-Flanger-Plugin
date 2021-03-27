#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Chorus_FlangerAudioProcessorEditor::Chorus_FlangerAudioProcessorEditor (Chorus_FlangerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (300, 175);
    
    auto& params = processor.getParameters();
    
    //drywet slider
    AudioParameterFloat* dryWetParameter = (AudioParameterFloat*)params.getUnchecked(0);
    
    mDryWetSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    mDryWetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 15);
    mDryWetSlider.setRange(dryWetParameter->range.start, dryWetParameter->range.end);
    mDryWetSlider.setValue(*dryWetParameter);
    addAndMakeVisible(mDryWetSlider);
    
    mDryWetSlider.onValueChange = [this, dryWetParameter] { *dryWetParameter = mDryWetSlider.getValue(); };
    
    mDryWetSlider.onDragStart = [dryWetParameter] { dryWetParameter->beginChangeGesture(); };
    
    mDryWetSlider.onDragEnd = [dryWetParameter] { dryWetParameter->endChangeGesture(); };
    
    //feedback slider
    AudioParameterFloat* feedbackParameter = (AudioParameterFloat*)params.getUnchecked(1);
    
    mFeedbackSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    mFeedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 15);
    mFeedbackSlider.setRange(feedbackParameter->range.start, feedbackParameter->range.end);
    
    mFeedbackSlider.setValue(*feedbackParameter);
    addAndMakeVisible(mFeedbackSlider);
    
    mFeedbackSlider.onValueChange = [this, feedbackParameter] { *feedbackParameter = mFeedbackSlider.getValue(); };
    
    mFeedbackSlider.onDragStart = [feedbackParameter] { feedbackParameter->beginChangeGesture(); };
    
    mFeedbackSlider.onDragEnd = [feedbackParameter] { feedbackParameter->endChangeGesture(); };
}

Chorus_FlangerAudioProcessorEditor::~Chorus_FlangerAudioProcessorEditor()
{
}

//==============================================================================
void Chorus_FlangerAudioProcessorEditor::paint (juce::Graphics& g)
{
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
        g.setColour (juce::Colours::white);
        g.setFont (15.0f);
        
        g.drawText("Dry / Wet", ((getWidth() / 6.3) * 1) - (50), (getHeight() / 2) - 15, 100, 100, Justification::centred, false);
        g.drawText("Feedback", ((getWidth() / 4) * 2) - (50), (getHeight() / 2) - 15, 100, 100, Justification::centred, false);
        g.drawText("Time", ((getWidth() / 3.6) * 3) - (50), (getHeight() / 2) - 15, 100, 100, Justification::centred, false);

}

void Chorus_FlangerAudioProcessorEditor::resized()
{
    mDryWetSlider.setBounds(0, 0, 100, 100);
    mFeedbackSlider.setBounds(100, 0, 100, 100);
}
