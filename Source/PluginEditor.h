/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#define green Colour::fromRGBA(7, 165, 18, 30);
#define blue Colour::fromRGBA(7, 112, 165, 30);
#define orange Colour(165, 71, 7);
#define grey Colours::darkgrey;
#define white Colours::whitesmoke;
//==============================================================================
/**
*/
class buttonLookAndFeel : public LookAndFeel_V4
{
public:
    buttonLookAndFeel() : LookAndFeel_V4()
    {

    }

    void buttonLookAndFeel::drawTickBox(Graphics& g, Component& component,
        float x, float y, float w, float h,
        const bool ticked,
        const bool isEnabled,
        const bool shouldDrawButtonAsHighlighted,
        const bool shouldDrawButtonAsDown) override
    {
        ignoreUnused(isEnabled, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

        Rectangle<float> tickBounds(x, y, w, h);

        g.setColour(component.findColour(ToggleButton::tickDisabledColourId));
        g.drawRect(tickBounds, 1.0f);

        if (ticked)
        {
            g.setColour(component.findColour(ToggleButton::tickColourId));
            //auto tick = getCrossShape(1.f);
            g.fillRect(tickBounds.reduced(2.f, 2.f));
            //g.fillPath(tick, tick.getTransformToScaleToFit(tickBounds.reduced(4, 4).toFloat(), false));
        }
    };

};

class rotaryLookAndFeel : public LookAndFeel_V4
{
public:

    rotaryLookAndFeel() : LookAndFeel_V4()
    {

    }

    virtual void drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override

    {

        auto outline = Colours::whitesmoke;
        auto attackColor = Colour(7, 165, 18);
        auto sustainColor = Colour(7, 112, 165);

        auto bounds = Rectangle<int>(x, y, width, height).toFloat().reduced(10);

        auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = jmin(8.0f, radius * 0.5f);
        auto arcRadius = radius - lineW * 0.5f;

        Path backgroundArc;
        backgroundArc.addCentredArc(bounds.getCentreX(),
            bounds.getCentreY(),
            arcRadius,
            arcRadius,
            0.0f,
            rotaryStartAngle,
            rotaryEndAngle,
            true);

        Path arcOutline;
        //outside arc
        arcOutline.addCentredArc(bounds.getCentreX(),
            bounds.getCentreY(),
            arcRadius + lineW,
            arcRadius + lineW,
            0.0f,
            rotaryStartAngle,
            rotaryEndAngle,
            true);

        //inside arc
        arcOutline.addCentredArc(bounds.getCentreX(),
            bounds.getCentreY(),
            arcRadius - lineW,
            arcRadius - lineW,
            0.0f,
            rotaryEndAngle,
            rotaryStartAngle,
            false);

        arcOutline.closeSubPath();
        
        
        g.setColour(outline);
        g.setOpacity(0.75);
        g.strokePath(arcOutline, PathStrokeType(1, PathStrokeType::curved, PathStrokeType::rounded));
        

        if (slider.isEnabled())
        {
            //fill with color for attack
            if (sliderPos > 0.5f)
            {
                Path valueArc;
                valueArc.addCentredArc(bounds.getCentreX(),
                    bounds.getCentreY(),
                    arcRadius,
                    arcRadius,
                    0.0f,
                    MathConstants<float>::twoPi,
                    toAngle,
                    true);

                g.setColour(attackColor);
                
                g.strokePath(valueArc, PathStrokeType(lineW * 2, PathStrokeType::curved, PathStrokeType::butt));
            }
            else if (sliderPos < 0.5f)
            {
                //fill with color for sustain
                Path valueArc;
                valueArc.addCentredArc(bounds.getCentreX(),
                    bounds.getCentreY(),
                    arcRadius,
                    arcRadius,
                    0.0f,
                    toAngle,
                    MathConstants<float>::twoPi,
                    true);

                g.setColour(sustainColor);

                g.strokePath(valueArc, PathStrokeType(lineW * 2, PathStrokeType::curved, PathStrokeType::butt));

            }

        }

        auto thumbWidth = lineW * 2.0f;
        Point<float> thumbPoint(bounds.getCentreX() + arcRadius * std::cos(toAngle - MathConstants<float>::halfPi),
            bounds.getCentreY() + arcRadius * std::sin(toAngle - MathConstants<float>::halfPi));

        g.setColour(Colours::whitesmoke);
        if (sliderPos == 0.5) g.fillRect(Rectangle<float>(1, thumbWidth).withCentre(thumbPoint));
    }
    

};

class DrumSnapperAudioProcessorEditor  : public juce::AudioProcessorEditor, public Timer, public Slider::Listener, public Button::Listener
{
public:
    DrumSnapperAudioProcessorEditor (DrumSnapperAudioProcessor&);
    ~DrumSnapperAudioProcessorEditor() override;

    //==============================================================================
    void timerCallback() override;

    void sliderValueChanged(Slider* sliderThatHasChanged) override;
    void buttonClicked(Button* buttonThatHasChanged) override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void paintOverChildren(Graphics& g) override;
    void keepAspectRatio();

    void newButtonStyle(Colour textColour, Colour tickColour, Colour disableColour);
    void newSliderStyle(Colour textColour, float trackTransp, Slider::TextEntryBoxPosition textBoxStyle, bool readonly, int width, int height);

    static const char* logoSmall_png;
    static const int logoSmall_pngSize;

    Image cachedImage_logo2020_png2_1;

    void DrumSnapperAudioProcessorEditor::paramTextStyle(Graphics& g, String name, float x, float y, float width, float fontHeight, bool drawBox, Justification justification, Colour textColour = Colour(245, 245, 245));

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DrumSnapperAudioProcessor& audioProcessor;

    Slider snapBodySlider;
    Slider focusSlider;
    Slider releaseSlider;
    Slider hfGainSlider;
    Slider hfSatMixSlider;
    Slider outputSlider;
    buttonLookAndFeel clipButtonLAF;
    ToggleButton clipButton;
    std::unique_ptr<HyperlinkButton> help;
    rotaryLookAndFeel rotaryLAF;
    Label snapBodyText;


    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>
        snapBodySliderValue;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>
        focusSliderValue;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>
        releaseSliderValue;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>
        hfGainSliderValue;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>
        hfSatMixSliderValue;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment>
        clipButtonValue;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>
        outputSliderValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumSnapperAudioProcessorEditor)
};
