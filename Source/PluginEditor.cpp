/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <math.h>
using namespace juce;
//==============================================================================
DrumSnapperAudioProcessorEditor::DrumSnapperAudioProcessorEditor (DrumSnapperAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    setResizable(true, true);
    setResizeLimits(300, 200, 1800, 1200);
    getConstrainer()->setFixedAspectRatio(1.5);
    //set main size from Desktop screen size
    //juce::Rectangle<int> r = Desktop::getInstance().getDisplays().getMainDisplay().userArea;

    //auto screenHeight = r.getHeight() * 0.78125f;

    //setSize((4.f / 3.f) * screenHeight, screenHeight);

    uiSize = *audioProcessor.parameters.getRawParameterValue(UISIZE_ID);

    setSize(1.5 * uiSize, uiSize);

    customTypeface = Typeface::createSystemTypefaceFor(UbuntuFont::UbuntuRegular_ttf,
        UbuntuFont::UbuntuRegular_ttfSize);

    LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(customTypeface);


    help.reset(new HyperlinkButton(TRANS("?"), URL("http://www.lowwavestudios.com/#software_section"))); //Help link
    addAndMakeVisible(help.get());
    help->setTooltip(TRANS("Help"));
    help->setColour(HyperlinkButton::textColourId, Colours::antiquewhite);
    help->setFont(14.f, Font::plain);

    cachedImage_logo2020_png2_1 = ImageCache::getFromMemory(logoSmall_png, logoSmall_pngSize); // logo

    snapBodySliderValue = std::make_unique <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, SNAP_ID, snapBodySlider);
    focusSliderValue = std::make_unique <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, FOCUS_ID, focusSlider);
    releaseSliderValue = std::make_unique <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, RELEASE_ID, releaseSlider);
    hfGainSliderValue = std::make_unique <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, GAIN_ID, hfGainSlider);
    hfSatMixSliderValue = std::make_unique <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, SATURATION_ID, hfSatMixSlider);
    outputSliderValue = std::make_unique <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, OUTPUT_ID, outputSlider);

    clipButtonValue = std::make_unique <AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, CLIP_ID, clipButton);

    uiSizeSliderValue = std::make_unique <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, UISIZE_ID, uiSizeSlider);
    addAndMakeVisible(&uiSizeSlider);
    uiSizeSlider.setRange(300, 1800, 1);
    uiSizeSlider.addListener(this);

    LookAndFeel::setDefaultLookAndFeel(&rotaryLAF);
    addAndMakeVisible(&focusSlider);
    focusSlider.setRange(1.0f, 5.0f, 0.01f);
    focusSlider.setDoubleClickReturnValue(true, 1.0f);
    defaultSliderBehaviour(focusSlider);

    addAndMakeVisible(&snapBodyText);

    addAndMakeVisible(&snapBodySlider);
    snapBodySlider.setRange(-5.0f, 5.0f, 0.01f);
    snapBodySlider.setDoubleClickReturnValue(true, 0.0f);
    defaultSliderBehaviour(snapBodySlider);
    snapBodySlider.setSliderSnapsToMousePosition(false);
    

    addAndMakeVisible(&releaseSlider);
    releaseSlider.setRange(5, 350, 1);
    releaseSlider.setDoubleClickReturnValue(true, 150);
    defaultSliderBehaviour(releaseSlider);
    releaseSlider.setTextValueSuffix(" ms");


    addAndMakeVisible(&hfGainSlider);
    hfGainSlider.setRange(1.f, 10.f, 0.01f);
    hfGainSlider.setDoubleClickReturnValue(true, 1.f);
    defaultSliderBehaviour(hfGainSlider);
    hfGainSlider.setTextValueSuffix(" x");


    addAndMakeVisible(&hfSatMixSlider);
    hfSatMixSlider.setRange(0, 100, 1);
    hfSatMixSlider.setDoubleClickReturnValue(true, 0.0f);
    defaultSliderBehaviour(hfSatMixSlider);
    hfSatMixSlider.setTextValueSuffix("%");

    addAndMakeVisible(&outputSlider);
    outputSlider.setRange(-24.0f, 6.0f, 0.01f);
    outputSlider.setDoubleClickReturnValue(true, 0.0f);
    defaultSliderBehaviour(outputSlider);
    outputSlider.setTextValueSuffix(" dB");


    addAndMakeVisible(&clipButton);
    clipButton.addListener(this);
    clipButton.setLookAndFeel(&clipButtonLAF);

    

    startTimerHz(30);
}

DrumSnapperAudioProcessorEditor::~DrumSnapperAudioProcessorEditor()
{
}

void DrumSnapperAudioProcessorEditor::defaultSliderBehaviour(Slider& slider)
{
    slider.addListener(this);
    slider.setSliderSnapsToMousePosition(false);
    slider.setTextBoxIsEditable(true);
}

//==============================================================================
void DrumSnapperAudioProcessorEditor::paint (juce::Graphics& g)
{
    LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(customTypeface);
    LWSFont = g.getCurrentFont();
    //keepAspectRatio();

    getConstrainer()->setFixedAspectRatio(1.5);

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);

    g.setColour (juce::Colours::whitesmoke);
    g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);

    auto area = getLocalBounds();
    auto windowHeight = area.getHeight();
    auto windowWidth = area.getWidth();

    float square = area.getWidth() / 100;
    auto sliderHeight = 5 * square;
    auto sliderSpacing = 2 * square;
    auto sideMargin = 2 * square;


    auto sliderWidth = ((windowWidth - (2 * sideMargin)) / 3);

    {//draw header
        g.setColour(Colours::darkgrey);
        Rectangle<float> header;
        header.setBounds(0, 0, windowWidth, sliderHeight * 1.5);
        g.setOpacity(0.3f);
        g.drawRect(header);
        g.fillRect(header);

    }

    {//draw title

        int x = (area.getWidth() / 2) - (15 * square), y = square, width = 30 * square, height = 4 * square;

        /*g.setColour(Colours::black);
        jassert(title != nullptr);
        if (title != nullptr)
            title->drawWithin(g, Rectangle<int>(x, y, width, height).toFloat(),
                RectanglePlacement::centred, 1.000f);*/

        g.setColour(Colours::whitesmoke);
        g.setFont(LWSFont);
        g.setFont(height);
        g.drawText("Drum Snapper", x, y, width, height, Justification::centred);
    }

    {// help button
        help->setBounds(windowWidth - (square * 4.f), square * 2.f, 2 * square, 2 * square);

    }

    {//draw version number

        g.setColour(Colours::whitesmoke);

        g.setOpacity(0.5f);
        g.setFont(LWSFont.withStyle(0));
        g.setFont(2 * square);
        g.drawText(JucePlugin_VersionString, windowWidth - (square * 12), square * 2.f, square * 6, square * 2, Justification::centred);

    }

    {//draw logo
        int x = square, y = square, w = 15 * square, h = w * (372.f / 891.f);

        g.setOpacity(1.f);
        g.setColour(Colours::black);
        g.setOpacity(1.f);
        g.drawImageWithin(cachedImage_logo2020_png2_1,
            x, y, w, h,
            RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
            false);
    }

    {//draw param back rectangles
        auto rLeft = Rectangle<float>(hfGainSlider.getX() - (sideMargin * 0.5), hfGainSlider.getY() - (2 * sideMargin), hfGainSlider.getWidth() + sideMargin, (4 * sliderHeight) + sideMargin);
        g.setColour(Colour(7, 112, 165));
        g.setOpacity(0.25f);
        g.fillRoundedRectangle(rLeft, 5);

        auto rRight = Rectangle<float>(releaseSlider.getX() - (sideMargin * 0.5), releaseSlider.getY() - (2 * sideMargin), releaseSlider.getWidth() + sideMargin, (4 * sliderHeight) + sideMargin);
        g.setColour(Colour(7, 165, 18));
        g.setOpacity(0.25f);
        g.fillRoundedRectangle(rRight, 5);

        /*auto rTop = Rectangle<float>(snapBodySlider.getX() - (sideMargin * 0.5), snapBodySlider.getY() - (sideMargin * 0.5), focusSlider.getWidth() + sideMargin, (5 * sliderHeight) + sideMargin);
        g.setColour(Colour(165, 23, 7));
        g.setOpacity(0.25f);
        g.fillRoundedRectangle(rTop, 5);*/

        //fill focus background
        auto rFocus = Rectangle<float>(focusSlider.getX(), focusSlider.getY(), focusSlider.getWidth(), focusSlider.getHeight());
        g.setColour(Colour(165, 71, 7));
        g.setOpacity(0.25f);
        g.fillRect(rFocus);

    }

    paramTextStyle(g, "Snap", snapBodySlider.getX() + (snapBodySlider.getWidth() * 0.3), snapBodySlider.getY() + snapBodySlider.getHeight() - sliderHeight, snapBodySlider.getWidth(), 2 * square, false, Justification::horizontallyCentred, Colour::fromRGBA(171, 171, 171, 0.75));

    paramTextStyle(g, "Sustain", snapBodySlider.getX() - (snapBodySlider.getWidth() * 0.3), snapBodySlider.getY() + snapBodySlider.getHeight() - sliderHeight, snapBodySlider.getWidth(), 2 * square, false, Justification::horizontallyCentred, Colour::fromRGBA(171, 171, 171, 0.75));

    paramTextStyle(g, "HF Gain", hfGainSlider.getX(), hfGainSlider.getY() - (sliderHeight * 0.75), hfGainSlider.getWidth(), 2.5 * square, false, Justification::horizontallyCentred, Colour::fromRGBA(171, 171, 171, 0.75));
    paramTextStyle(g, "HF Saturation Mix", hfSatMixSlider.getX(), hfSatMixSlider.getY() - (sliderHeight * 0.75), hfSatMixSlider.getWidth(), 2.5 * square, false, Justification::horizontallyCentred, Colour::fromRGBA(171, 171, 171, 0.75));
    paramTextStyle(g, "Release", releaseSlider.getX(), releaseSlider.getY() - (sliderHeight * 0.75), releaseSlider.getWidth(), 2.5 * square, false, Justification::horizontallyCentred, Colour::fromRGBA(171, 171, 171, 0.75));
    paramTextStyle(g, "Output (Pre-Clipper)", outputSlider.getX(), outputSlider.getY() - (sliderHeight * 0.75), outputSlider.getWidth(), 2.5 * square, false, Justification::horizontallyCentred, Colour::fromRGBA(171, 171, 171, 0.75));
    paramTextStyle(g, "Oversampled", clipButton.getX() - (sliderWidth * 0.44), clipButton.getY() + (2*square), sliderWidth, 2 * square, false, Justification::centredBottom, Colour::fromRGBA(171, 171, 171, 0.75));
    paramTextStyle(g, "Clipper", clipButton.getX() - (sliderWidth * 0.44), clipButton.getY() + (4 * square), sliderWidth, 2 * square, false, Justification::centredBottom, Colour::fromRGBA(171, 171, 171, 0.75));
    paramTextStyle(g, "Focus", focusSlider.getX(), focusSlider.getY() + (sliderHeight * 0.75), focusSlider.getWidth(), 2.5 * square, false, Justification::horizontallyCentred, Colour::fromRGBA(171, 171, 171, 0.75));



}

void DrumSnapperAudioProcessorEditor::resized()
{
    LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(customTypeface);
    getConstrainer()->setFixedAspectRatio(1.5);

    auto area = getLocalBounds();
    auto windowHeight = area.getHeight();
    auto windowWidth = area.getWidth();

    uiSizeSlider.setValue(windowHeight);

    float square = area.getWidth() / 100;
    auto sliderHeight = 5 * square;
    auto sliderSpacing = 2 * square;
    auto sideMargin = 2 * square;

    auto sliderWidth = ((windowWidth - (2 * sideMargin)) / 3);

    newSliderStyle(Colours::whitesmoke, 1.f, Slider::TextBoxRight, true, area.getWidth() / 4, sliderHeight);
    newButtonStyle(Colours::whitesmoke, Colour(83, 163, 3), Colours::darkgrey);

    snapBodySlider.setBounds(sliderWidth + sideMargin, 2 * sliderHeight, sliderWidth, 4 * sliderHeight);
    focusSlider.setBounds(sliderWidth + sideMargin, 6 * sliderHeight, sliderWidth, sliderHeight);
    hfGainSlider.setBounds(sideMargin * 2, 8 * sliderHeight, sliderWidth, sliderHeight);
    hfSatMixSlider.setBounds(sideMargin * 2, 10 * sliderHeight, sliderWidth, sliderHeight);
    releaseSlider.setBounds(sliderWidth * 2, 8 * sliderHeight, sliderWidth, sliderHeight);
    outputSlider.setBounds(sliderWidth * 2, 10 * sliderHeight, sliderWidth, sliderHeight);
    clipButton.setBounds((windowWidth / 2) - (sliderHeight / 3), 9 * sliderHeight, sliderHeight, sliderHeight);

    {//set bounds for rotary text label
        auto w = 8 * square;
        auto h = 4 * square;
        auto sliderBounds = snapBodySlider.getBounds();

        snapBodyText.setSize(w, h);
        Point<int> textBounds = sliderBounds.getCentre();
        Point<int> addPoint(square, 0);
        textBounds += addPoint;
        snapBodyText.setCentrePosition(textBounds);
        
    }

   
}


void DrumSnapperAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
    LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(customTypeface);
    LWSFont = g.getCurrentFont();
    //keepAspectRatio();

    getConstrainer()->setFixedAspectRatio(1.5);

    snapBodyText.setText(snapBodySlider.getTextFromValue(snapBodySlider.getValue()), sendNotification);
}


void DrumSnapperAudioProcessorEditor::sliderValueChanged(Slider* sliderThatHasChanged)
{
    if (sliderThatHasChanged == &focusSlider)
    {
        audioProcessor.focus = focusSlider.getValue();
        
    }
    else if (sliderThatHasChanged == &snapBodySlider)
    {
        audioProcessor.snap = snapBodySlider.getValue();
        snapBodyText.setText(snapBodySlider.getTextFromValue(snapBodySlider.getValue()), dontSendNotification);
    }
    else if (sliderThatHasChanged == &hfGainSlider)
    {
        audioProcessor.gain = hfGainSlider.getValue();
    }
    else if (sliderThatHasChanged == &hfSatMixSlider)
    {
        audioProcessor.saturation = hfSatMixSlider.getValue();
    }
    else if (sliderThatHasChanged == &releaseSlider)
    {
        audioProcessor.release = releaseSlider.getValue();
    }
    else if (sliderThatHasChanged == &outputSlider)
    {
        audioProcessor.output = outputSlider.getValue();
    }
}


void DrumSnapperAudioProcessorEditor::buttonClicked(Button* buttonThatHasChanged)
{
    if (buttonThatHasChanged == &clipButton)
    {
        audioProcessor.clip = clipButton.getToggleState();
    }

}

void DrumSnapperAudioProcessorEditor::timerCallback()
{


}

void DrumSnapperAudioProcessorEditor::keepAspectRatio()
{
    setResizeLimits(200, 300, 1200, 1800);

    int w = getWidth();
    int h = getHeight();

    if (w > 300 && w < 1800)
    {
        setSize(w, w * 1.5f);
    }
    else if (h > 200 && h < 1200)
    {
        setSize(h * (3.0 / 2.0), h);
    }
}


void DrumSnapperAudioProcessorEditor::newButtonStyle(Colour textColour, Colour tickColour, Colour disableColour)
{

    clipButton.setColour(ToggleButton::textColourId, textColour);
    clipButton.setColour(ToggleButton::tickColourId, tickColour);
    clipButton.setColour(ToggleButton::tickDisabledColourId, disableColour);

}



void DrumSnapperAudioProcessorEditor::newSliderStyle(Colour textColour, float trackTransp, Slider::TextEntryBoxPosition textBoxStyle, bool readonly, int width, int height)
{
    snapBodySlider.setColour(Slider::trackColourId, Colour(7, 165, 18));
    snapBodySlider.setColour(Slider::textBoxTextColourId, textColour);
    snapBodySlider.setSliderStyle(Slider::Rotary);
    snapBodySlider.setAlpha(trackTransp);
    snapBodySlider.setTextBoxStyle(Slider::NoTextBox, false, snapBodySlider.getWidth() / 2, snapBodySlider.getHeight() / 5);

    hfGainSlider.setColour(Slider::trackColourId, Colour(7, 112, 165));
    hfGainSlider.setColour(Slider::textBoxTextColourId, textColour);
    hfGainSlider.setSliderStyle(Slider::LinearBar);
    hfGainSlider.setAlpha(trackTransp);

    hfSatMixSlider.setColour(Slider::trackColourId, Colour(7, 112, 165));
    hfSatMixSlider.setColour(Slider::textBoxTextColourId, textColour);
    hfSatMixSlider.setSliderStyle(Slider::LinearBar);
    hfSatMixSlider.setAlpha(trackTransp);

    releaseSlider.setColour(Slider::trackColourId, Colour(7, 165, 18));
    releaseSlider.setColour(Slider::textBoxTextColourId, textColour);
    releaseSlider.setSliderStyle(Slider::LinearBar);
    releaseSlider.setAlpha(trackTransp);

    focusSlider.setColour(Slider::trackColourId, Colour(165, 71, 7));
    focusSlider.setColour(Slider::textBoxTextColourId, textColour);
    focusSlider.setSliderStyle(Slider::LinearBar);
    focusSlider.setAlpha(trackTransp);

    outputSlider.setColour(Slider::trackColourId, Colour(7, 165, 18));
    outputSlider.setColour(Slider::textBoxTextColourId, textColour);
    outputSlider.setSliderStyle(Slider::LinearBar);
    outputSlider.setAlpha(trackTransp);
}

void DrumSnapperAudioProcessorEditor::paramTextStyle(Graphics& g, String name, float x, float y, float width, float fontHeight, bool drawBox, Justification justification, Colour textColour)
{
    LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(customTypeface);
    float floatHeight = releaseSlider.getHeight();

    if (drawBox == true)
    {
        //draw params boxes
        g.setColour(textColour);
        g.setOpacity(0.75f);
        g.drawRect(x, y, width, floatHeight, 0.5f);
    }

    //draw Text params
    auto Rect = juce::Rectangle<int>(width, floatHeight);
    Rect.setPosition(x, y);
    g.setColour(Colours::darkgrey);
    g.setOpacity(0.3f);

    if (drawBox == true)
    {
        g.fillRect(Rect);

    }

    g.setColour(textColour);
    g.setOpacity(0.75f);
    g.setFont(fontHeight);
    g.drawText(name, Rect, justification, true);


}

// JUCER_RESOURCE: logoSmall_png, 19731, "LOGO small.png"
static const unsigned char resource_NewComponent2_logoSmall_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,1,124,0,0,0,181,8,6,0,0,0,92,119,68,2,0,0,30,131,122,84,88,116,82,97,119,32,112,
114,111,102,105,108,101,32,116,121,112,101,32,101,120,105,102,0,0,120,218,237,155,89,118,21,61,150,133,223,53,138,26,130,250,102,56,106,215,170,25,228,240,235,219,138,107,48,96,131,201,202,124,251,49,
96,251,54,17,210,105,118,35,233,154,253,175,255,61,230,127,248,83,67,138,38,166,82,115,203,217,242,39,182,216,124,231,135,106,159,63,237,254,239,108,188,255,223,63,101,190,126,114,63,62,110,70,122,189,
201,243,80,224,123,120,189,161,63,223,93,231,241,244,253,13,111,247,112,227,199,199,77,125,61,227,235,235,66,175,39,222,46,24,116,103,207,15,235,253,32,121,220,63,143,187,248,186,80,219,207,15,185,213,
242,126,168,195,63,223,231,235,133,119,40,175,127,103,250,123,189,52,158,167,244,187,121,255,64,44,68,105,37,110,20,188,223,193,5,123,255,143,207,8,194,243,175,243,47,243,191,227,53,86,207,242,115,12,
201,220,135,242,107,36,4,228,135,233,189,125,183,246,125,128,126,8,242,219,79,230,231,232,151,246,113,240,125,127,189,34,252,20,203,252,138,17,63,124,248,132,75,31,7,255,134,248,221,141,195,183,17,249,
31,159,200,211,157,95,166,243,22,228,179,234,57,251,153,93,143,153,136,230,87,69,89,243,22,157,27,253,179,8,123,12,247,109,153,175,194,191,196,207,229,126,53,190,170,237,118,146,242,101,167,29,124,77,
215,156,39,43,199,184,232,150,235,238,184,125,191,79,55,25,98,244,219,23,190,123,63,125,184,143,213,80,124,243,51,60,121,226,203,29,95,66,11,43,84,114,57,253,54,33,240,176,255,54,22,119,239,219,238,253,
166,171,220,121,57,94,234,29,23,35,225,159,127,153,223,61,249,55,95,230,156,169,16,57,5,147,212,187,39,193,94,117,205,48,148,57,253,207,171,72,200,43,13,234,49,247,238,235,149,126,251,174,176,40,85,50,
152,110,152,43,19,236,118,60,151,24,201,125,175,173,112,243,28,120,93,226,251,211,66,206,148,245,186,0,33,226,222,137,193,184,64,6,108,118,33,185,236,108,241,190,56,71,28,43,9,234,140,220,135,232,7,25,
112,41,249,197,32,125,12,33,123,83,124,245,186,55,239,41,238,190,214,39,159,189,30,6,155,212,69,116,86,33,55,45,116,146,21,99,162,126,74,172,212,80,79,0,90,74,41,167,146,170,73,45,245,28,114,204,41,231,
92,178,64,174,151,80,98,73,37,151,82,106,105,165,215,80,99,77,53,215,82,107,109,181,55,223,2,24,152,90,110,165,213,214,90,239,222,116,110,212,185,86,231,245,157,71,134,31,97,196,145,70,30,101,212,209,
70,159,148,207,140,51,205,60,203,172,179,205,190,252,10,11,152,88,121,149,85,87,91,125,59,179,65,138,29,119,218,121,151,93,119,219,253,80,107,39,156,120,210,201,167,156,122,218,233,223,178,246,202,234,
47,95,127,145,53,247,202,154,191,153,210,235,202,183,172,241,168,41,229,237,18,78,112,146,148,51,50,230,163,35,227,69,25,16,130,41,103,182,186,24,189,50,167,156,217,230,105,138,228,25,100,82,110,204,114,
202,24,41,140,219,249,116,220,183,220,125,207,220,151,242,102,82,253,82,222,252,159,50,103,148,186,255,68,230,12,169,251,53,111,31,100,109,137,231,230,205,216,211,133,138,169,13,116,223,106,167,150,101,
142,61,35,250,227,124,169,105,149,100,87,116,121,219,185,153,101,60,107,205,49,64,193,12,40,101,63,122,59,188,177,166,8,120,165,115,218,46,229,62,82,206,57,164,255,244,21,11,112,166,135,154,35,73,103,
143,157,227,217,45,173,189,50,89,104,204,137,136,215,19,131,75,181,83,105,253,132,216,243,94,165,172,184,169,17,10,50,89,134,82,206,46,109,237,114,214,209,229,184,82,2,57,44,189,49,253,208,35,97,110,102,
187,159,103,249,151,250,24,139,224,158,213,102,24,78,83,43,204,245,240,50,94,88,1,202,111,227,102,116,207,200,203,219,155,53,112,56,181,91,238,160,223,231,9,99,158,51,158,231,153,218,202,125,159,92,123,
184,87,183,101,253,116,237,207,162,242,10,202,235,218,70,109,244,92,218,170,31,244,244,159,174,252,225,168,205,87,134,253,149,81,155,175,12,251,43,163,254,39,216,255,4,251,159,96,255,87,131,13,210,175,
227,66,152,6,36,206,194,200,4,237,213,0,45,180,232,227,226,169,113,90,61,99,52,91,184,4,162,164,212,94,102,190,23,64,25,130,195,23,41,247,131,154,29,168,205,41,117,180,142,205,203,135,81,252,158,113,229,
182,33,87,7,109,141,212,207,134,225,214,177,21,168,133,29,227,129,83,90,11,167,167,82,70,154,238,129,251,102,126,69,251,193,79,216,18,151,67,56,11,117,187,97,181,218,249,59,66,171,207,79,50,83,63,125,
55,159,61,241,235,247,232,103,183,80,6,17,218,131,193,174,112,198,36,0,248,175,208,171,129,84,224,194,13,39,238,18,243,247,31,173,221,221,183,241,253,229,238,167,119,254,244,70,243,229,119,14,168,109,
135,91,12,179,194,158,183,4,250,41,247,145,69,139,192,79,165,244,114,208,10,39,192,125,249,212,135,221,188,107,11,86,219,99,61,191,71,223,103,190,85,3,75,239,154,83,99,12,240,95,187,233,48,159,228,195,
161,71,130,149,170,155,3,89,130,68,154,168,42,95,19,111,202,27,182,181,88,154,144,102,56,47,142,54,63,147,180,61,245,182,2,151,166,52,161,224,49,61,213,195,236,86,66,29,81,161,240,246,140,125,103,212,
9,138,230,236,30,91,119,133,172,41,84,206,35,55,136,136,71,135,108,66,147,203,226,210,103,239,182,66,222,123,162,87,144,102,237,12,148,79,202,51,225,17,240,237,220,145,162,133,232,199,56,230,236,69,220,
210,225,122,210,54,46,174,58,40,69,180,3,5,213,219,44,57,49,241,132,22,58,41,47,100,83,162,206,70,218,4,118,216,212,184,31,21,193,240,204,65,92,166,73,240,130,207,195,73,87,160,249,42,181,72,56,9,118,
184,193,197,91,29,2,221,60,106,233,213,194,62,206,124,163,138,18,35,178,221,16,244,48,29,33,231,91,36,191,181,229,228,122,92,131,28,57,231,119,34,26,59,160,65,233,200,211,34,62,122,167,209,136,102,166,
183,79,170,2,132,25,65,200,204,20,237,66,251,17,87,203,96,72,190,187,163,24,142,27,208,234,146,169,141,135,170,171,68,54,164,237,168,149,76,53,58,87,147,39,0,161,167,22,170,217,20,252,9,188,129,104,140,
185,203,164,223,108,37,95,211,133,178,91,70,63,174,94,163,243,117,82,145,51,175,209,17,161,122,154,26,218,21,139,122,240,64,184,163,147,230,219,232,62,25,220,219,208,228,48,159,193,129,80,175,225,29,191,
9,232,0,172,204,169,200,178,66,84,34,18,113,183,224,209,118,4,39,2,88,171,38,10,6,1,59,170,239,195,109,235,219,14,22,13,215,220,162,131,9,93,28,164,124,43,141,195,68,219,74,96,162,220,213,35,167,45,186,
48,249,132,160,142,69,142,50,87,75,97,61,163,72,181,109,213,78,39,74,177,202,122,50,15,244,100,80,170,205,207,185,126,101,122,231,81,195,182,40,223,225,41,238,16,92,30,88,68,74,204,175,158,70,112,165,
81,223,8,114,135,102,183,157,238,7,37,75,22,128,133,58,115,233,171,216,144,73,117,200,168,248,154,70,170,76,204,102,106,136,132,187,58,221,110,163,85,134,121,82,32,196,7,221,255,68,216,124,20,98,46,220,
215,74,99,157,212,35,112,206,204,193,241,124,176,54,165,110,93,219,119,74,27,95,138,49,160,209,237,153,201,20,186,255,168,20,237,226,122,106,131,132,3,136,19,217,78,24,41,149,74,15,180,82,114,14,83,118,
24,67,67,125,98,17,160,171,5,43,80,48,128,15,152,77,107,111,119,81,6,38,138,68,21,92,229,47,40,69,98,241,180,220,104,157,230,53,172,65,255,250,60,79,203,133,190,39,23,76,245,248,222,139,91,101,152,149,
113,36,137,178,223,184,45,238,72,79,68,252,241,64,192,219,216,28,1,138,121,39,173,17,80,38,85,121,224,221,149,114,190,93,152,201,6,109,52,106,52,17,151,147,234,168,109,1,88,176,89,233,20,52,12,71,5,86,
43,187,160,118,41,52,213,25,132,47,166,62,19,72,147,83,197,79,89,231,33,85,222,48,103,48,187,225,154,28,111,234,140,121,225,19,176,88,112,25,152,91,106,62,121,185,5,118,112,3,192,156,175,66,86,72,46,63,
141,134,3,204,184,254,91,8,39,24,21,92,155,56,41,152,50,87,149,238,2,179,104,38,238,54,104,114,138,116,193,153,142,88,2,27,158,140,236,5,206,201,253,98,52,193,43,186,166,185,61,76,130,38,176,80,43,219,
69,110,201,200,96,66,216,203,150,79,207,182,78,50,20,136,219,153,115,85,53,42,64,58,142,141,171,52,24,60,52,92,25,248,112,220,50,185,205,133,1,158,195,245,180,234,241,161,89,176,223,182,0,204,158,146,
38,192,11,222,194,84,80,64,203,25,175,203,27,45,185,202,188,33,165,141,144,24,16,194,50,2,39,8,131,6,4,219,86,7,74,240,191,237,224,0,91,199,170,106,169,196,30,122,25,218,105,199,225,2,65,214,138,40,177,
155,118,24,184,215,214,253,200,213,155,136,84,1,254,66,213,154,200,230,79,177,253,33,27,31,79,116,20,46,13,103,3,201,221,144,28,150,55,200,58,67,145,57,138,155,198,92,139,78,239,214,4,173,167,21,108,182,
31,213,173,145,1,118,230,67,229,45,159,86,156,225,134,110,197,10,230,5,172,222,64,12,1,175,117,211,226,4,46,5,90,185,239,105,11,188,102,21,45,126,198,94,82,34,253,165,214,234,101,215,178,112,157,9,204,
7,38,161,19,232,225,48,72,72,131,100,33,36,232,56,104,173,122,202,201,184,72,101,181,186,29,233,65,185,224,208,15,242,133,31,51,22,159,96,11,5,18,101,131,3,135,232,84,180,106,129,17,73,10,4,244,253,198,
230,251,157,241,173,77,230,246,41,254,117,192,117,53,176,216,146,242,172,176,34,243,221,174,141,72,30,124,6,35,129,138,0,188,64,130,194,163,232,136,73,220,16,17,194,147,248,210,39,83,32,113,236,22,26,
161,38,51,93,10,6,34,76,42,69,134,182,164,237,19,20,96,231,164,186,79,224,154,100,141,34,6,90,105,224,208,155,216,96,146,208,229,31,213,26,175,100,25,155,203,129,44,64,68,120,64,151,226,9,84,102,169,147,
27,32,46,235,152,206,80,86,116,98,203,189,230,137,240,219,145,30,10,141,230,167,252,65,124,171,124,49,120,45,146,2,98,21,133,212,120,7,197,18,179,154,172,38,222,14,254,78,132,22,18,171,20,45,90,143,231,
182,48,180,110,172,219,198,143,70,87,138,148,229,1,32,64,39,170,172,84,242,80,76,66,135,119,212,65,5,93,233,104,216,198,55,116,4,247,152,142,129,56,110,183,17,60,137,84,83,135,16,146,149,94,21,188,183,
67,137,69,148,45,101,159,80,35,80,2,225,66,197,90,173,254,145,105,199,171,232,64,136,192,10,195,191,22,177,109,32,71,170,113,157,205,164,43,114,253,164,178,252,98,242,19,30,202,17,92,4,38,22,130,58,18,
91,40,181,159,27,91,59,251,233,45,65,232,52,26,93,178,205,94,128,140,83,107,73,170,64,197,240,46,84,48,148,64,0,96,248,89,75,138,252,10,99,122,74,141,105,122,233,239,107,7,142,172,0,50,135,74,48,215,11,
92,39,32,111,65,50,231,198,84,60,218,21,109,70,208,7,50,13,6,141,158,89,245,171,84,121,229,15,74,149,223,163,121,147,170,31,8,213,31,116,170,164,240,75,169,74,70,133,215,34,17,72,247,60,103,108,227,50,
209,34,80,207,203,85,124,168,78,127,210,166,119,191,69,234,244,84,81,57,179,48,55,19,75,120,139,142,20,54,1,62,92,39,135,123,167,252,76,121,36,63,22,198,97,74,15,64,205,115,22,166,245,210,223,153,65,50,
162,229,59,237,20,96,4,94,176,5,25,173,108,180,134,95,159,235,197,149,240,116,248,50,6,11,100,219,137,86,36,216,106,104,122,163,1,146,158,18,240,24,44,244,121,155,214,79,4,5,162,206,194,105,5,214,193,
46,244,46,156,7,72,139,46,34,190,109,228,99,39,120,198,56,218,60,34,202,182,117,75,162,170,105,195,65,164,218,219,216,94,11,122,106,135,154,0,3,110,129,91,113,212,60,176,178,80,215,222,65,168,115,201,
50,102,179,184,122,195,35,84,55,249,15,89,148,66,133,7,250,184,158,4,118,170,3,190,235,199,163,46,120,194,129,200,89,109,1,147,163,166,233,27,160,3,101,135,24,69,23,161,215,53,240,185,146,231,234,32,188,
18,95,38,164,145,153,103,203,220,12,142,164,128,201,29,117,62,181,6,187,64,52,44,38,85,11,57,192,123,166,89,196,8,222,115,163,5,16,99,22,205,215,128,126,132,95,21,23,65,101,153,134,187,155,59,104,18,240,
127,0,239,148,29,172,135,11,197,89,80,106,14,9,98,128,124,139,60,24,220,98,54,136,3,108,137,150,233,211,16,200,210,42,17,114,232,106,202,166,48,105,160,94,63,209,13,248,19,183,105,115,143,75,31,73,138,
13,252,110,89,204,224,28,132,63,176,48,206,45,148,167,132,94,33,68,193,10,63,50,128,206,212,179,162,204,67,148,208,166,84,115,16,111,219,136,54,48,185,115,247,64,103,87,116,83,144,116,106,16,12,50,254,
76,216,196,18,237,179,196,244,139,98,59,144,59,76,92,118,88,164,74,139,1,113,47,198,160,110,55,72,186,154,28,130,202,87,96,22,244,200,14,25,12,250,96,180,131,60,7,183,133,29,154,54,255,208,156,189,100,
82,65,43,13,47,155,36,217,122,180,249,231,160,35,248,21,69,144,224,80,76,34,194,122,0,132,112,45,109,131,224,245,52,170,211,250,43,6,23,158,193,145,73,133,184,165,81,140,160,53,229,13,126,245,237,209,
144,16,77,71,220,248,132,2,31,10,173,118,220,166,39,181,12,207,82,117,39,194,79,1,153,190,73,238,0,9,51,117,141,74,1,221,80,0,169,40,87,193,248,93,161,81,15,205,194,27,163,100,105,114,116,23,33,94,129,
215,46,185,72,25,67,196,154,163,80,152,141,219,144,239,242,45,37,81,29,66,196,209,13,209,108,106,206,187,133,179,161,55,104,123,43,29,14,129,159,65,200,199,194,81,146,212,230,58,173,169,146,70,80,192,
158,96,67,145,84,231,98,84,114,130,19,144,53,36,123,21,106,97,161,173,208,185,103,182,112,37,177,48,167,134,110,175,49,6,56,201,27,64,5,183,150,187,0,64,45,208,40,168,249,82,97,220,236,205,40,94,200,132,
182,242,16,204,132,143,174,125,162,227,31,33,254,137,14,95,7,65,26,233,67,109,32,226,87,154,193,45,85,90,194,83,147,145,170,14,64,47,0,75,124,96,194,90,229,51,144,206,5,138,199,142,65,178,200,91,109,123,
36,42,26,78,67,148,30,66,90,201,167,193,90,14,240,6,102,66,51,168,14,209,175,226,10,238,57,33,168,162,164,84,84,90,20,116,108,12,132,118,94,81,171,116,115,201,48,160,183,142,128,237,99,36,154,85,69,81,
246,13,239,147,175,222,209,66,72,87,142,61,16,140,39,165,249,81,214,178,207,4,10,137,31,35,218,79,27,131,25,237,150,66,80,139,196,134,8,134,126,83,148,180,128,170,210,22,197,98,54,70,143,187,11,140,64,
23,248,39,99,193,176,8,40,138,74,77,100,202,62,104,233,138,26,74,224,128,33,6,169,161,204,136,129,227,173,8,1,38,130,0,136,67,216,136,113,161,143,34,83,206,80,112,208,122,130,160,43,130,122,173,48,93,
250,101,91,154,98,77,232,8,79,136,98,3,142,189,252,104,68,117,210,193,164,202,163,199,169,145,147,241,217,200,18,30,96,148,161,239,36,3,4,167,54,92,118,115,248,127,237,79,116,10,146,132,104,113,168,56,
74,64,70,61,9,188,49,122,81,247,42,41,89,34,15,152,129,14,50,12,132,26,114,171,34,2,28,64,14,145,9,144,200,109,152,77,66,166,53,85,36,157,45,32,180,29,161,78,216,10,74,219,193,50,220,61,194,14,3,71,64,
1,64,42,187,214,72,172,137,73,71,51,4,139,12,196,28,163,153,233,239,201,139,14,6,242,186,66,18,142,110,173,128,33,232,3,79,17,10,28,146,56,24,69,176,41,44,192,243,208,157,216,27,50,169,245,171,222,77,
241,24,92,12,216,150,144,113,5,235,234,176,7,187,121,154,13,202,129,130,59,34,15,81,212,165,215,37,109,102,212,122,9,38,71,60,74,77,225,117,233,2,52,228,228,247,234,196,177,90,48,69,220,163,227,165,154,
104,12,12,57,81,38,201,13,77,20,180,218,186,48,100,153,154,26,40,71,38,8,178,11,171,16,234,166,180,38,167,124,180,127,8,181,130,173,189,36,226,113,32,241,230,168,188,211,39,189,34,211,134,136,71,41,141,
88,114,196,203,50,22,123,125,123,197,200,159,105,160,151,32,6,131,198,153,205,68,49,130,161,93,235,88,221,242,10,171,221,124,236,58,109,185,6,226,20,173,150,18,239,103,56,218,210,36,238,197,123,42,247,
24,108,228,213,134,0,255,65,125,30,68,24,37,12,55,227,161,16,114,20,40,161,9,5,202,138,29,232,157,152,148,34,25,236,187,206,16,84,234,121,66,82,246,24,170,69,177,198,129,192,37,128,17,149,141,26,194,43,
186,43,211,208,170,183,247,152,48,184,150,4,28,52,39,22,41,130,42,20,183,232,180,146,46,103,100,44,115,42,32,238,228,26,27,24,205,168,116,36,12,50,9,120,152,114,131,90,49,195,250,72,214,102,84,200,120,
86,37,117,104,134,178,167,144,136,104,213,154,191,173,116,127,142,25,53,43,15,185,163,223,52,62,14,42,169,107,240,167,24,59,74,128,114,150,63,228,190,74,50,104,170,117,72,90,84,235,35,62,97,179,112,123,
77,43,175,52,249,28,48,0,117,123,23,25,144,3,150,87,129,171,8,64,32,71,107,136,30,235,197,236,252,145,202,35,141,81,43,42,13,3,98,77,211,130,19,28,137,163,185,64,133,84,167,12,208,64,5,150,164,43,185,
245,109,65,106,141,128,37,162,67,19,99,87,0,93,4,54,168,8,227,84,50,15,30,97,208,161,40,84,173,74,154,1,204,66,159,34,245,250,75,45,44,144,209,35,73,168,68,164,103,70,33,17,103,212,237,162,20,112,201,
120,27,124,141,55,52,82,144,8,13,226,91,112,35,74,211,241,224,132,147,156,214,48,180,123,76,33,180,64,75,172,144,154,93,220,165,102,188,48,141,38,0,174,25,173,83,177,235,20,59,118,58,33,63,139,232,58,
6,36,197,245,189,56,47,193,107,33,232,139,246,28,21,7,3,133,138,216,241,253,216,113,242,138,110,218,77,162,201,144,58,145,75,27,36,0,117,10,114,116,80,12,52,228,10,140,11,132,196,68,128,219,81,75,12,123,
66,50,5,15,71,104,100,228,240,10,216,102,66,80,167,52,100,85,155,212,180,136,253,1,214,178,238,144,58,14,255,120,133,161,101,56,30,134,2,200,163,151,21,193,93,147,242,229,188,42,194,161,217,80,25,195,
136,127,118,77,94,254,101,77,63,18,121,133,110,168,79,146,0,71,110,72,124,85,216,27,241,214,119,212,186,70,218,202,64,226,246,13,51,224,19,70,187,84,19,120,219,130,115,11,194,144,182,179,155,247,240,27,
206,77,107,79,52,36,222,37,182,69,127,67,13,232,17,135,210,247,42,71,44,213,116,168,17,82,10,254,45,19,51,180,137,234,27,14,236,196,249,209,208,2,231,42,149,169,176,66,250,206,37,106,170,83,138,117,121,
57,112,230,188,101,221,55,153,241,180,27,32,47,49,90,223,214,189,194,146,147,10,191,89,243,63,214,183,1,97,130,251,245,242,3,184,137,178,72,27,43,74,86,131,86,194,108,7,226,41,114,153,164,12,19,156,114,
183,28,222,54,28,158,237,134,133,144,119,218,219,10,120,90,215,96,162,1,83,109,244,141,51,90,39,254,120,139,62,95,203,71,71,80,227,89,203,170,80,37,200,52,58,240,11,62,82,165,186,187,202,0,21,67,29,105,
217,92,114,5,55,131,235,88,101,138,191,189,118,225,229,28,173,14,50,233,76,131,19,80,14,233,75,193,164,118,9,170,86,64,58,124,130,246,246,229,250,53,231,30,191,111,105,18,228,2,70,144,222,243,143,131,
62,81,107,221,82,138,157,148,75,35,108,9,26,128,8,82,192,45,3,9,8,91,55,77,169,94,4,34,239,129,14,71,138,104,209,202,47,84,51,138,166,104,21,19,5,158,193,5,174,220,128,22,250,73,206,184,225,211,128,22,
44,1,212,16,251,76,38,200,98,98,5,116,111,40,31,190,136,181,225,90,96,84,240,182,225,174,117,206,142,146,168,90,221,160,183,117,80,76,190,247,39,91,104,190,251,66,222,69,107,208,87,90,243,115,51,208,37,
147,191,107,238,196,184,208,13,24,56,153,24,151,253,16,125,210,46,72,236,57,228,68,116,220,195,47,134,32,193,34,59,227,183,214,78,120,203,224,215,46,133,95,101,26,105,95,194,0,93,82,35,170,114,47,79,14,
27,151,166,117,6,4,115,78,134,182,68,83,20,38,115,157,20,70,146,119,131,254,158,71,167,156,126,15,168,132,3,1,204,27,126,199,88,200,132,54,15,122,206,40,243,37,87,81,151,65,25,151,92,177,104,192,1,163,
34,212,96,6,143,224,102,9,206,172,90,246,10,168,231,173,117,194,106,137,56,121,219,176,52,210,97,105,239,19,111,81,48,44,102,0,207,40,118,106,97,13,20,134,142,132,148,165,236,101,58,11,96,160,138,157,
90,152,153,28,34,230,58,218,2,157,230,48,94,116,97,171,248,94,20,159,35,70,40,2,172,30,230,144,38,144,146,2,43,3,96,19,165,210,97,105,173,144,193,106,141,238,186,11,7,112,39,2,15,238,180,52,166,167,236,
135,214,205,22,5,249,177,210,167,237,227,74,224,9,181,8,147,169,90,198,129,185,0,7,169,85,233,237,161,83,62,235,132,225,8,193,48,30,120,208,122,138,78,238,89,202,64,103,133,32,133,22,147,206,4,53,200,
14,103,168,115,70,92,130,76,69,124,134,16,102,103,106,24,178,14,248,64,12,102,10,38,113,95,180,202,210,10,30,239,190,231,33,149,65,28,85,225,30,39,207,165,29,63,29,91,155,107,204,70,66,9,40,224,217,185,
71,196,121,35,39,38,68,108,16,20,20,170,213,0,160,19,135,31,102,4,168,72,237,178,248,140,173,95,75,105,67,184,76,76,20,57,69,183,38,157,57,10,64,114,229,70,68,181,68,159,141,147,144,222,83,75,180,76,97,
224,214,97,52,244,15,227,17,44,49,252,14,8,234,120,223,170,3,20,108,220,99,233,196,212,112,218,187,66,228,208,55,40,24,83,209,11,1,238,213,194,41,214,136,49,35,112,193,153,169,53,69,160,132,62,91,240,
145,220,92,212,238,21,218,128,72,55,72,161,196,125,238,129,77,168,108,68,131,213,129,252,10,170,10,212,211,193,75,12,123,162,207,45,146,152,46,70,142,220,213,6,28,13,229,0,146,147,62,153,47,237,28,30,
89,62,167,44,99,37,205,162,32,238,33,92,58,135,50,166,95,209,44,113,63,135,4,104,60,252,3,5,131,153,195,122,29,100,101,95,77,43,120,193,211,0,21,69,130,65,71,202,181,2,101,123,237,126,224,224,241,54,72,
209,134,247,114,85,43,224,200,111,222,75,219,108,90,25,191,200,248,208,62,158,118,70,93,97,121,71,211,193,179,53,239,6,28,14,146,76,237,88,10,228,167,245,121,127,255,227,157,192,43,206,136,54,66,159,180,
164,53,31,52,114,66,207,45,109,26,1,130,104,40,242,23,175,155,212,134,175,141,208,85,186,107,158,168,51,174,3,136,7,0,165,246,149,211,156,218,107,165,219,162,215,66,16,168,182,25,173,86,60,181,28,6,217,
139,107,128,8,64,205,80,165,183,135,162,90,116,130,186,26,255,209,97,188,187,86,79,32,144,195,240,1,45,171,115,5,81,235,204,64,1,154,20,35,145,224,116,60,157,112,202,64,156,216,3,26,190,82,98,237,100,
21,96,77,99,118,152,180,106,97,94,153,191,146,3,157,115,141,90,110,7,144,188,43,149,100,200,13,205,121,109,115,74,204,2,86,217,125,40,173,202,238,120,173,198,170,243,253,93,133,39,3,92,141,82,76,20,155,
207,24,188,172,29,44,218,35,36,92,196,217,137,250,49,180,205,112,85,187,71,96,54,160,172,85,79,45,114,21,29,9,213,126,16,207,71,61,41,247,6,32,50,255,165,186,206,90,147,113,59,9,157,212,43,6,231,2,107,
23,89,108,128,206,169,206,165,214,49,181,212,180,214,20,0,119,46,16,116,234,79,48,178,96,131,76,56,27,201,164,228,59,118,145,46,71,140,98,152,104,43,228,121,71,249,245,164,213,250,44,131,117,224,227,117,
119,103,33,245,248,172,133,86,160,14,220,211,122,2,120,18,200,28,204,60,244,91,77,70,216,187,181,204,10,157,143,187,2,219,181,107,116,207,130,8,244,53,221,64,124,177,255,77,210,42,131,223,40,116,20,73,
159,138,54,44,227,167,117,11,233,135,76,147,194,176,58,115,226,181,229,224,73,221,179,238,27,255,124,94,228,77,121,152,111,231,69,18,253,80,85,230,31,29,23,209,226,60,157,186,151,84,23,42,53,88,57,239,
147,227,160,8,25,110,29,230,136,59,65,82,5,7,3,131,119,7,235,181,61,128,10,32,81,238,253,139,127,247,90,243,55,47,254,221,107,205,7,47,6,22,58,136,235,18,232,210,131,28,124,215,41,99,17,148,139,233,131,
239,14,127,99,174,107,34,122,151,142,19,6,176,72,199,211,239,218,204,245,185,102,110,89,236,179,227,30,49,71,232,154,21,181,65,56,79,70,41,161,228,238,25,134,108,104,89,100,130,82,3,100,128,231,250,117,
58,109,61,100,45,202,76,52,59,50,27,149,175,51,78,109,15,176,83,140,163,189,56,45,151,149,174,237,55,44,184,193,86,250,133,17,195,220,57,137,213,160,197,5,186,130,78,204,30,119,117,55,74,181,32,162,53,
87,26,9,13,37,222,164,194,22,196,204,155,84,32,24,106,179,145,138,13,153,91,229,30,153,150,63,88,170,181,79,136,77,251,71,25,159,80,68,171,247,212,125,188,28,76,121,162,252,33,120,203,16,49,183,14,251,
13,211,82,56,88,181,145,214,241,105,106,25,50,118,192,11,254,43,90,44,1,205,168,42,148,52,35,163,59,112,96,27,97,163,51,42,19,133,11,60,84,217,197,52,146,105,16,156,63,90,231,32,108,242,16,211,175,42,
77,4,94,109,27,23,140,119,215,171,153,166,86,170,44,194,116,75,216,148,173,141,180,73,92,182,186,216,154,161,197,212,117,250,36,100,244,4,147,97,54,110,194,22,117,160,184,210,153,136,24,127,4,59,184,17,
236,149,176,132,156,109,0,189,106,219,8,192,211,242,62,22,2,52,232,90,82,192,108,3,169,122,88,103,76,54,152,78,89,196,169,179,187,120,142,33,76,72,105,67,127,71,39,218,169,141,213,9,88,29,216,176,160,
130,68,26,105,77,230,14,155,209,119,38,40,123,163,19,100,228,176,162,148,215,93,2,162,149,49,31,211,222,227,13,232,214,173,19,210,24,49,174,123,104,61,163,15,145,0,136,132,51,35,8,40,207,161,125,45,25,
49,156,212,66,217,56,198,210,180,237,71,117,66,50,142,122,60,34,193,12,170,105,177,114,128,37,7,197,134,212,223,16,51,10,253,110,187,210,254,65,250,140,144,33,70,229,54,17,86,222,161,107,154,236,87,215,
34,202,228,43,28,157,212,168,67,27,166,32,83,145,206,6,208,212,26,19,224,211,54,220,80,173,90,196,51,185,145,142,10,14,115,131,107,235,213,235,3,18,13,169,136,79,180,218,85,228,167,42,107,175,163,54,134,
76,215,135,240,132,131,56,9,160,40,222,197,147,174,51,40,17,77,130,211,7,210,176,156,157,236,82,234,11,43,123,215,46,104,37,88,94,132,146,210,53,53,246,220,21,94,18,32,151,161,153,129,136,129,2,135,187,
151,86,124,240,122,126,104,141,10,187,166,69,70,251,154,38,89,150,179,118,8,12,243,0,123,213,217,25,21,47,252,17,61,49,205,48,131,206,205,35,223,155,22,43,138,250,66,246,210,143,124,29,136,244,181,131,
232,198,81,0,109,51,219,113,99,114,223,168,187,174,131,49,196,172,225,151,81,25,72,38,119,101,188,214,78,17,50,200,131,182,231,188,78,27,123,170,35,64,85,71,180,180,99,188,12,126,180,169,94,43,74,69,63,
119,50,204,85,78,39,40,195,231,81,74,146,62,198,159,234,132,11,86,122,249,128,232,237,34,59,237,161,34,145,55,78,25,79,123,120,122,109,97,21,254,3,99,197,19,17,171,47,129,171,211,62,109,60,70,249,114,
213,195,84,151,167,134,108,69,196,210,96,174,19,164,106,40,178,190,182,246,84,81,45,21,166,24,33,251,157,3,45,179,181,130,191,135,192,86,39,195,156,54,93,116,54,31,201,119,2,110,35,234,64,38,114,35,104,
101,200,80,83,250,120,14,222,135,168,82,44,187,163,36,145,221,20,140,80,96,151,187,64,230,68,89,215,234,1,153,94,167,27,80,142,69,88,175,53,141,99,251,52,93,71,81,208,83,137,204,32,152,117,78,224,4,45,
54,148,178,236,230,202,125,107,79,81,31,39,169,81,228,28,136,244,145,44,214,162,204,115,44,204,50,12,83,100,133,231,120,118,102,81,170,211,131,7,40,116,20,193,1,248,181,101,113,0,233,63,94,210,124,191,
230,175,151,172,87,220,217,47,93,208,252,58,200,127,239,130,230,243,89,255,221,5,205,159,195,248,181,11,154,175,231,229,247,23,52,95,205,202,159,46,104,190,154,149,63,93,208,252,109,154,63,187,160,249,
119,235,230,117,65,218,132,234,197,15,26,224,200,34,74,237,211,71,193,245,9,213,234,132,157,76,49,74,70,2,5,39,170,125,51,100,133,78,219,161,36,64,176,4,60,65,184,8,70,94,159,218,52,178,211,9,156,43,80,
161,214,247,225,46,109,253,131,149,226,112,45,225,118,209,4,94,116,34,160,128,119,29,76,2,102,130,118,191,49,36,225,30,209,27,199,184,170,35,122,25,243,123,198,209,154,176,4,87,184,59,61,152,209,181,51,
146,183,198,1,14,234,64,14,142,21,81,35,13,94,113,77,58,152,166,143,161,117,36,154,225,69,206,119,121,90,238,162,67,155,40,35,198,55,16,77,83,71,81,171,92,209,141,208,225,165,138,80,185,81,213,178,195,
19,243,161,143,120,62,44,18,241,9,90,239,87,192,17,21,69,31,32,218,178,197,58,205,1,155,70,209,17,46,24,143,254,150,45,189,87,217,250,246,70,227,181,75,254,130,44,45,43,51,19,40,58,119,173,159,216,140,
20,67,204,228,152,181,157,57,181,50,112,82,67,54,97,89,80,41,72,149,249,28,151,213,225,10,109,205,60,203,39,174,180,120,109,37,194,236,89,248,178,254,249,238,194,61,7,143,253,213,228,172,123,142,194,216,
167,154,228,82,59,26,18,126,66,54,49,244,213,160,79,204,23,73,163,138,32,158,166,79,255,114,229,239,87,16,196,86,167,221,8,220,219,214,249,16,157,196,93,161,76,98,164,221,209,156,100,5,181,253,89,240,
111,96,52,250,77,199,43,220,208,66,38,34,149,216,194,77,57,165,130,161,223,45,234,92,157,91,182,107,111,166,223,205,237,39,216,84,246,203,90,54,221,173,223,243,164,9,191,167,132,85,247,156,227,170,46,
248,103,162,41,45,173,48,189,62,116,20,80,134,50,199,247,64,127,46,207,201,217,215,229,117,182,69,215,126,93,89,148,254,235,181,185,228,115,237,43,245,205,115,241,50,66,85,46,250,186,159,138,162,252,70,
120,206,105,35,52,194,214,10,105,183,8,92,157,183,159,151,50,241,252,238,253,173,205,7,247,222,115,191,251,28,66,238,207,113,171,1,185,182,153,239,167,24,208,63,218,32,180,175,78,176,62,63,5,185,125,175,
127,184,251,93,231,114,58,81,177,170,182,35,159,99,112,136,124,109,224,132,251,33,175,207,67,26,63,187,251,235,230,3,54,167,208,114,209,10,158,185,33,215,93,223,197,252,45,226,127,147,79,243,67,66,63,
139,249,183,57,107,139,62,135,167,174,239,188,95,49,215,174,232,167,247,254,32,230,159,70,220,90,42,251,78,250,137,247,111,239,252,65,188,223,69,219,252,174,200,226,7,119,254,126,227,31,98,125,204,141,
245,127,160,186,141,238,252,255,168,238,226,117,70,233,84,121,218,148,151,214,186,16,131,248,225,10,126,28,125,128,145,254,118,250,164,102,192,79,233,136,142,13,65,167,95,48,151,58,63,139,234,117,177,
45,95,3,166,65,131,116,102,36,29,150,64,53,230,141,137,25,188,143,59,235,244,155,142,149,223,108,226,98,244,129,195,25,116,22,88,176,146,184,152,206,139,100,137,186,16,93,241,96,148,1,30,23,142,23,207,
230,170,220,17,112,167,227,178,71,72,246,224,35,0,87,86,210,113,51,45,95,86,57,44,248,11,22,243,250,64,233,40,208,18,116,107,170,4,168,243,177,221,67,228,117,199,103,65,100,218,147,226,219,231,96,52,99,
130,212,75,213,102,174,160,117,47,164,54,46,13,211,11,237,120,157,100,9,114,23,217,145,192,140,3,210,22,52,12,10,72,115,213,179,243,115,170,159,216,95,163,161,165,1,249,111,157,32,184,27,45,138,185,118,
90,202,232,230,96,117,184,187,246,180,224,211,177,147,230,225,181,167,143,214,206,13,63,162,181,127,108,44,58,220,22,125,46,2,172,142,91,235,21,88,118,109,162,235,244,218,177,180,72,242,90,216,198,230,
195,34,90,226,213,249,174,211,162,115,19,79,83,134,157,247,124,202,74,11,105,225,176,177,117,140,30,117,212,196,49,42,12,53,175,63,49,24,157,5,48,255,7,148,69,30,33,189,157,120,97,0,0,1,133,105,67,67,
80,73,67,67,32,112,114,111,102,105,108,101,0,0,120,156,125,145,61,72,195,64,28,197,95,83,181,34,45,14,118,16,113,200,80,157,90,16,21,209,77,170,88,4,11,165,173,208,170,131,201,165,31,66,147,134,36,197,
197,81,112,45,56,248,177,88,117,112,113,214,213,193,85,16,4,63,64,156,28,157,20,93,164,196,255,37,133,22,49,30,28,247,227,221,189,199,221,59,64,104,84,152,106,118,141,1,170,102,25,233,68,92,204,229,87,
196,192,43,122,16,68,8,81,204,72,204,212,147,153,133,44,60,199,215,61,124,124,189,139,241,44,239,115,127,142,144,82,48,25,224,19,137,103,153,110,88,196,235,196,83,155,150,206,121,159,56,204,202,146,66,
124,78,28,53,232,130,196,143,92,151,93,126,227,92,114,88,224,153,97,35,155,158,35,14,19,139,165,14,150,59,152,149,13,149,120,146,56,162,168,26,229,11,57,151,21,206,91,156,213,74,141,181,238,201,95,24,
44,104,203,25,174,211,28,70,2,139,72,34,5,17,50,106,216,64,5,22,98,180,106,164,152,72,211,126,220,195,63,228,248,83,228,146,201,181,1,70,142,121,84,161,66,114,252,224,127,240,187,91,179,56,49,238,38,5,
227,64,247,139,109,127,140,0,129,93,160,89,183,237,239,99,219,110,158,0,254,103,224,74,107,251,171,13,96,250,147,244,122,91,139,28,1,253,219,192,197,117,91,147,247,128,203,29,96,240,73,151,12,201,145,
252,52,133,98,17,120,63,163,111,202,3,3,183,64,223,170,219,91,107,31,167,15,64,150,186,90,186,1,14,14,129,209,18,101,175,121,188,187,183,179,183,127,207,180,250,251,1,211,168,114,206,154,205,228,162,0,
0,0,6,98,75,71,68,0,255,0,255,0,255,160,189,167,147,0,0,0,9,112,72,89,115,0,0,46,35,0,0,46,35,1,120,165,63,118,0,0,0,7,116,73,77,69,7,228,6,12,13,45,18,95,220,1,216,0,0,32,0,73,68,65,84,120,218,237,157,
119,156,85,197,249,255,223,115,239,222,221,101,23,88,202,46,101,1,65,5,91,212,68,84,18,130,74,44,137,53,26,107,98,197,130,154,232,55,106,162,70,212,104,18,123,126,106,196,146,166,160,88,192,130,26,13,
138,189,98,239,98,87,140,244,94,118,129,93,118,247,150,249,253,49,115,150,195,185,115,119,111,93,182,60,239,215,235,190,88,238,57,247,148,153,57,159,243,204,51,207,60,163,38,85,87,253,18,248,59,80,14,
196,17,4,65,16,58,19,10,8,1,151,171,73,213,85,235,128,238,82,38,130,32,8,157,154,133,69,86,253,1,214,1,159,74,153,8,130,32,116,42,190,15,148,1,241,34,223,151,159,16,139,141,209,117,181,82,60,130,32,8,
157,0,213,189,2,194,69,179,129,157,0,252,130,143,214,113,198,175,23,55,190,32,8,66,103,96,82,255,222,205,46,28,48,142,124,65,16,4,161,11,32,130,47,8,130,32,130,47,8,130,32,136,224,11,130,32,8,34,248,130,
32,8,130,8,190,32,8,130,32,130,47,8,130,32,136,224,11,130,32,8,34,248,130,32,8,130,8,190,32,8,130,32,130,47,8,130,32,130,47,8,130,32,136,224,11,130,32,8,34,248,130,32,8,130,8,190,32,8,130,32,130,47,8,
130,32,136,224,11,130,32,8,34,248,130,32,8,130,8,190,32,8,130,32,130,47,8,130,32,136,224,11,130,32,136,224,11,130,32,8,34,248,130,32,8,130,8,190,32,8,130,32,130,47,8,130,32,136,224,11,130,32,8,34,248,
130,32,8,130,8,190,32,8,130,32,130,47,8,130,32,136,224,11,130,32,136,224,11,130,32,8,34,248,130,32,8,130,8,190,32,8,130,32,130,47,8,130,32,136,224,11,130,32,8,34,248,130,32,8,130,8,190,32,8,130,32,130,
47,8,130,32,136,224,11,130,32,8,34,248,130,32,8,34,248,130,32,8,130,8,190,32,8,130,208,65,41,146,34,16,132,142,207,154,53,107,104,106,106,146,130,232,170,150,123,40,68,85,85,149,8,190,32,116,5,166,76,
153,194,141,55,222,136,82,74,10,163,11,178,223,126,251,113,215,93,119,137,224,11,66,87,32,145,72,176,104,209,34,41,136,46,74,44,22,75,175,39,32,69,37,8,130,208,53,16,193,23,4,65,16,193,23,4,65,16,68,240,
5,65,16,4,17,124,65,16,4,161,253,10,190,196,113,9,130,32,116,17,193,175,183,127,23,139,246,11,130,32,116,94,138,0,93,136,3,107,224,86,165,40,175,174,138,4,207,209,184,120,69,236,44,173,11,126,115,171,
86,173,98,205,154,53,82,203,237,8,173,53,125,250,244,161,111,223,190,89,31,67,41,197,228,234,42,52,36,181,45,21,41,142,159,58,119,97,222,27,215,93,219,108,77,162,110,157,115,222,74,124,241,138,216,25,
121,110,207,74,41,238,28,54,152,68,83,99,242,61,22,151,196,78,253,110,129,52,38,33,43,193,47,76,215,65,41,38,13,172,4,104,2,54,248,54,213,151,84,87,85,182,197,205,205,154,53,139,195,15,63,92,106,185,157,
241,232,163,143,230,84,47,127,55,10,24,1,86,98,254,109,110,207,58,218,180,71,237,27,175,191,83,241,227,49,121,187,222,123,118,223,149,68,221,186,193,192,108,160,52,176,57,28,174,174,42,185,69,41,206,201,
163,232,79,234,223,135,68,83,227,238,192,59,129,231,167,68,71,163,37,64,76,90,146,144,177,46,183,209,121,186,249,62,165,109,117,115,225,112,88,106,184,29,146,107,189,156,165,53,42,82,28,3,62,15,180,173,
8,240,211,7,199,236,145,215,235,141,46,156,11,48,6,232,29,56,95,55,160,24,56,178,172,79,121,158,159,204,16,192,37,142,231,103,38,58,33,98,47,180,107,193,223,164,71,42,197,46,228,202,127,230,45,210,192,
44,199,166,253,195,3,250,230,239,68,53,53,16,10,3,92,216,194,94,227,201,183,6,135,66,37,192,47,28,91,254,66,60,46,13,64,232,48,130,47,8,57,51,210,252,51,213,177,105,79,21,82,220,158,167,36,98,255,236,
221,27,160,23,176,107,11,187,237,168,202,122,150,61,113,198,233,121,57,231,35,71,31,5,90,187,124,82,171,129,217,37,35,182,147,6,32,136,224,11,93,135,139,180,6,244,199,192,218,224,54,141,58,37,95,86,126,
164,123,24,224,204,86,118,27,12,84,45,125,96,74,94,206,185,230,149,231,1,198,58,54,125,70,184,40,118,194,172,215,165,1,8,34,248,66,215,161,204,40,59,192,223,28,155,255,64,158,44,124,213,187,95,8,56,36,
141,93,207,87,61,122,229,124,190,199,79,30,135,138,20,3,28,225,216,252,164,174,93,149,144,218,23,58,146,224,107,41,118,33,31,44,88,178,18,192,101,86,111,137,82,3,114,61,254,209,74,65,60,86,2,236,152,198,
238,191,205,71,203,94,241,216,3,0,229,41,206,57,57,186,86,22,57,17,196,194,23,186,32,151,127,246,25,192,42,96,73,96,83,49,176,253,125,99,70,231,116,252,3,140,91,104,52,198,135,239,103,33,240,89,114,119,
64,237,67,77,77,110,61,138,178,158,0,23,56,54,125,2,172,252,141,22,123,73,16,193,23,186,98,227,221,97,7,72,196,215,99,194,51,55,209,77,224,200,198,239,230,228,118,2,227,22,186,220,177,229,58,224,53,199,
247,135,254,203,12,242,102,79,34,14,238,49,131,137,36,196,155,35,116,60,193,239,80,97,153,77,77,77,104,173,91,253,196,227,113,42,42,42,10,122,45,61,123,246,108,87,101,211,218,114,122,197,197,197,5,191,
6,109,4,242,22,199,166,113,40,197,43,151,95,150,203,13,246,38,121,240,84,3,119,2,79,59,126,177,119,164,186,146,248,103,159,101,127,206,112,209,16,32,216,144,26,129,151,79,51,46,172,188,81,82,82,146,86,
219,78,36,18,206,182,221,210,124,138,150,218,106,107,237,34,159,203,52,22,186,13,246,232,209,131,68,34,145,86,57,54,52,52,16,137,68,54,235,51,43,75,28,6,159,230,77,187,204,61,129,239,99,252,169,163,128,
225,152,201,55,10,51,211,113,41,240,5,240,86,40,20,250,162,166,166,230,19,32,225,53,180,104,52,154,237,249,119,115,184,17,150,3,179,195,225,48,137,12,45,61,223,61,237,231,216,188,8,248,34,155,135,204,
94,71,5,240,35,192,31,28,94,148,72,36,158,14,133,66,5,95,99,181,113,121,45,165,213,85,255,117,108,234,14,236,242,205,63,38,126,56,246,138,43,51,62,238,35,71,29,9,176,191,99,211,251,42,18,105,208,9,253,
56,241,164,216,251,157,65,245,188,107,207,221,215,142,95,93,159,241,57,39,85,118,71,149,148,125,15,59,38,237,99,53,74,45,66,107,114,29,140,14,180,239,45,129,237,109,123,251,1,48,196,119,238,117,192,183,
192,155,74,169,15,106,106,106,62,194,230,221,242,234,212,46,171,183,181,253,248,27,229,183,192,119,169,234,190,177,177,209,51,252,14,0,162,1,3,244,19,96,73,62,218,141,239,94,247,194,184,249,252,44,4,190,
204,230,60,129,50,236,7,124,207,234,196,110,192,86,246,153,80,246,69,189,24,227,254,123,187,164,164,228,139,166,166,166,230,222,104,183,110,221,104,104,104,16,193,111,7,98,95,106,173,184,99,108,197,165,
234,9,237,12,252,12,56,215,190,0,214,89,23,192,109,141,141,141,212,212,212,208,167,79,159,108,46,229,16,224,79,108,58,192,189,2,232,31,139,197,8,133,178,234,152,141,0,158,35,121,208,252,37,96,191,72,36,
162,179,121,65,1,103,0,255,47,112,220,37,161,80,104,80,91,212,217,217,90,51,185,186,10,148,186,31,56,54,176,249,124,21,41,57,33,155,227,214,204,122,9,138,138,14,112,108,122,117,245,188,197,186,119,117,
21,128,235,156,23,168,210,242,203,179,234,80,148,148,1,156,236,216,244,44,137,120,99,30,197,126,119,204,96,247,8,171,1,169,14,252,67,224,56,43,230,27,128,191,0,215,107,173,253,150,234,201,192,31,3,245,
127,41,112,109,107,151,3,252,3,24,26,232,253,79,5,78,200,99,19,169,0,94,9,92,159,194,68,65,125,153,67,25,14,176,26,177,175,45,195,84,15,229,15,128,131,236,223,49,160,22,248,13,48,189,190,190,158,72,36,
66,188,13,39,210,137,15,63,185,34,175,183,141,251,88,32,156,65,25,21,89,235,255,86,96,129,82,234,196,222,189,123,163,181,206,38,149,192,68,95,195,244,62,253,128,225,57,88,62,127,114,28,83,217,158,11,77,
77,89,69,127,40,95,99,246,31,243,63,158,203,160,77,234,110,125,13,192,131,142,77,99,8,135,75,200,112,160,115,218,126,251,64,81,24,224,96,199,230,123,214,1,196,162,216,23,93,144,115,179,9,68,187,117,99,
189,254,210,177,249,122,29,139,230,212,182,109,251,222,222,186,162,222,1,118,192,164,162,80,105,234,68,185,189,223,57,192,222,62,227,64,165,104,87,45,186,146,44,119,58,126,179,39,16,209,249,27,156,190,
205,113,125,27,188,54,154,69,25,246,6,38,97,2,5,14,180,61,135,76,52,162,47,240,16,240,137,82,106,255,88,44,134,110,195,129,248,46,31,150,233,171,200,129,152,73,60,23,228,225,176,131,129,123,128,201,94,
215,119,191,253,246,203,228,197,83,3,124,228,216,252,155,140,21,217,8,73,17,238,184,110,207,245,241,147,44,95,144,197,128,107,218,231,115,90,235,108,95,34,25,51,126,109,20,224,99,76,162,62,63,3,209,186,
239,117,25,246,136,54,124,250,17,160,70,2,193,36,127,53,160,63,190,76,107,78,91,182,6,80,115,128,101,73,229,169,66,59,213,204,154,149,209,57,187,245,41,3,119,42,133,21,192,103,91,159,126,118,174,134,204,
88,204,224,246,254,57,22,247,214,192,139,182,247,155,21,77,77,77,204,152,49,131,20,162,187,5,176,109,174,109,226,181,215,94,195,190,164,142,114,108,190,212,239,154,202,160,12,183,0,230,3,167,229,161,217,
238,104,95,190,55,120,231,56,231,156,115,58,165,224,183,155,65,219,94,189,122,249,11,255,43,160,71,43,63,249,8,152,97,223,208,207,0,115,91,217,255,84,224,13,32,252,220,115,207,165,117,223,69,69,69,126,
235,39,200,193,64,216,119,221,173,98,253,236,59,146,236,195,244,115,117,150,69,56,204,118,109,131,188,146,165,219,41,151,55,247,92,204,152,202,38,198,36,176,111,85,166,179,110,77,238,28,87,153,92,135,
222,232,7,13,247,173,90,15,124,237,120,166,198,60,124,196,193,217,156,243,20,199,150,251,72,36,216,251,234,107,114,17,251,179,129,151,211,248,137,215,190,167,167,209,190,31,4,78,39,203,172,157,135,30,
122,40,192,167,152,49,176,32,183,231,218,28,198,140,25,227,185,164,130,221,204,13,192,253,118,252,33,147,50,220,23,51,54,209,189,21,99,246,125,224,49,224,97,224,5,135,65,16,228,124,224,113,128,155,111,
190,89,92,58,133,162,182,182,214,203,149,191,21,102,160,200,37,246,141,192,171,182,155,169,128,93,128,67,109,183,251,0,204,160,151,178,150,119,42,127,224,104,251,38,215,233,116,221,124,3,178,179,28,15,
211,64,160,207,234,213,171,51,177,238,141,107,195,184,167,104,225,26,123,102,225,122,250,157,227,187,55,109,15,101,115,48,209,121,141,153,190,124,148,234,14,236,29,172,26,96,106,201,86,195,155,191,136,
45,153,15,238,137,95,199,169,72,9,31,253,243,31,105,157,238,187,135,31,70,149,116,43,5,118,114,8,200,51,145,65,91,100,92,16,11,23,46,244,254,60,200,231,214,192,33,126,47,217,30,158,191,125,31,227,104,
223,174,24,215,91,49,3,150,185,240,235,20,237,113,80,182,174,14,223,239,78,113,24,152,95,1,203,42,43,91,207,208,238,27,167,216,25,120,30,247,152,103,157,125,57,142,182,122,186,27,112,56,112,52,38,72,98,
128,237,105,92,133,25,40,118,190,255,128,255,120,209,62,34,248,5,192,23,54,246,82,138,93,190,196,12,106,141,197,198,92,127,241,197,23,68,34,17,148,82,132,195,97,191,160,254,203,54,138,131,82,28,107,63,
76,26,247,180,42,212,90,249,159,57,4,191,39,176,93,186,93,81,223,96,208,41,105,236,62,54,93,171,199,199,25,142,239,46,223,28,245,153,88,178,18,66,97,151,137,180,11,80,121,90,154,101,118,239,15,119,247,
122,68,193,248,185,69,160,86,159,240,218,155,205,95,68,87,174,131,120,220,213,19,219,19,165,138,222,61,39,61,55,204,83,71,31,13,90,87,88,129,13,242,238,73,239,125,144,177,27,111,208,160,65,216,227,61,
146,98,183,79,173,59,110,31,204,160,38,74,41,188,200,170,80,40,228,111,171,255,178,251,238,231,232,65,29,145,117,157,25,227,230,53,192,181,154,203,177,121,104,22,174,193,223,27,1,93,91,91,155,150,235,
201,26,130,47,164,216,229,67,204,184,200,1,192,91,193,50,244,25,80,245,192,101,86,79,206,74,113,172,95,132,66,161,113,0,103,159,125,118,193,158,147,46,233,195,47,47,47,247,187,77,92,230,211,115,182,34,
23,156,121,230,153,40,165,80,74,177,195,14,59,120,161,104,205,150,184,183,13,19,94,246,148,125,200,92,221,184,179,128,109,50,16,234,104,138,135,245,60,128,3,15,60,48,93,11,191,7,201,153,30,151,56,118,
255,89,192,165,212,218,131,58,202,97,61,213,105,173,159,223,28,117,122,250,215,95,67,44,154,240,30,188,0,199,124,63,205,227,52,205,255,14,251,0,7,159,141,183,73,196,55,137,181,252,205,146,37,96,30,234,
25,174,158,69,168,50,61,87,82,137,89,40,232,36,199,166,247,209,122,117,166,101,225,235,37,78,195,189,254,196,253,182,55,49,191,162,162,194,223,134,155,69,94,107,221,44,92,118,91,220,10,223,182,152,172,
157,57,107,136,117,77,38,112,207,105,56,58,93,3,41,5,151,164,232,209,220,151,206,143,199,142,109,158,126,113,23,201,99,57,96,198,231,70,2,11,122,247,238,237,44,195,68,34,209,252,189,221,214,0,252,211,
246,6,26,29,199,156,2,108,121,219,109,183,117,42,193,223,236,62,252,245,235,215,99,187,90,46,43,226,117,79,252,70,140,24,193,237,183,167,231,78,84,74,49,111,222,60,48,126,207,177,41,118,123,51,195,75,
253,147,227,187,35,0,102,206,156,153,238,49,92,35,65,174,65,167,195,124,86,77,58,47,146,125,28,155,222,86,134,182,175,212,17,35,72,172,90,229,189,172,131,28,208,99,248,176,86,47,170,246,245,215,189,133,
71,92,121,142,111,212,177,64,217,12,24,128,54,113,212,46,223,205,233,186,41,205,24,107,83,94,23,59,182,92,73,246,130,119,38,102,126,68,144,23,49,161,150,108,183,221,118,172,93,187,54,237,246,125,198,25,
103,128,25,179,216,218,138,87,78,172,91,183,206,251,211,85,126,163,48,17,45,25,97,221,48,17,96,156,99,243,245,0,123,237,181,87,171,199,121,249,229,151,61,119,216,145,142,205,51,129,241,74,41,70,143,30,
77,77,154,233,52,148,82,44,88,176,0,107,148,164,202,113,125,27,160,210,49,188,196,165,147,6,231,158,123,174,95,76,131,214,79,28,59,170,95,84,84,196,156,57,153,77,205,31,54,108,152,23,114,246,149,245,227,
5,233,3,28,156,129,213,242,173,113,37,36,209,106,164,133,157,25,25,2,126,21,216,20,181,221,249,191,7,190,31,2,140,200,64,172,93,130,255,252,230,236,193,13,191,240,98,48,254,212,36,241,208,209,168,158,
58,118,207,22,127,63,253,208,159,97,31,196,234,224,187,0,120,107,250,202,117,73,191,233,189,207,207,60,247,219,134,192,166,1,170,180,108,192,253,251,255,44,157,75,223,14,19,238,231,167,129,68,226,191,
125,246,254,105,54,214,125,49,238,104,179,47,49,131,143,40,165,248,234,171,175,50,58,246,29,119,220,225,253,89,3,252,54,31,117,118,220,113,199,129,25,44,254,220,177,249,239,153,30,207,78,234,234,231,112,
143,197,49,3,169,204,106,37,130,202,26,132,10,51,160,26,228,115,124,225,186,111,189,245,86,70,215,183,197,22,205,14,133,185,184,23,213,57,8,232,157,229,156,24,17,252,32,19,39,78,244,30,8,215,96,209,4,
96,169,82,42,235,201,16,62,11,121,70,10,247,194,73,0,19,38,76,72,199,138,6,120,50,149,149,223,18,43,87,174,244,94,48,67,130,134,21,198,103,122,87,154,221,224,77,253,113,45,207,218,157,182,57,235,118,239,
171,175,1,173,93,201,226,251,235,104,211,54,13,95,126,218,114,153,23,151,146,226,33,188,29,157,224,25,199,139,250,200,233,15,131,214,11,48,225,147,126,122,0,223,171,251,240,157,22,207,57,222,212,243,207,
29,155,102,233,104,35,135,79,187,63,179,238,179,57,222,64,204,172,240,77,170,206,90,253,228,98,61,42,165,184,240,194,11,193,196,162,207,206,181,206,238,191,191,249,254,92,1,0,191,0,42,210,157,89,62,100,
200,16,239,254,79,33,121,12,102,21,38,246,189,213,227,88,151,111,73,10,163,230,146,192,243,153,85,25,218,148,15,55,96,6,125,131,252,181,51,185,116,218,67,28,190,107,48,103,153,173,128,220,125,86,27,125,
158,174,138,59,18,224,154,107,210,14,179,115,185,40,118,7,84,143,30,169,163,72,237,67,189,61,201,121,89,254,229,179,48,214,4,182,29,2,20,247,110,61,1,216,207,29,174,185,57,192,188,109,183,221,182,29,84,
111,243,61,250,249,35,45,9,157,214,16,46,42,193,29,157,243,100,195,146,85,45,255,22,110,114,117,40,109,110,251,148,252,200,172,133,235,122,121,190,48,127,213,250,108,239,223,245,210,250,31,240,106,105,
105,105,206,51,59,239,188,179,121,156,250,196,60,214,217,219,152,69,233,253,20,3,163,210,21,215,249,243,231,123,127,254,197,177,249,252,12,175,199,229,214,91,2,60,158,143,124,56,62,11,222,53,247,194,21,
93,36,22,126,14,184,92,34,143,0,148,149,149,229,243,60,143,177,105,142,25,48,225,145,163,211,105,196,221,186,117,75,229,162,248,1,80,146,202,255,122,216,97,135,181,244,224,95,231,179,120,230,5,182,245,
4,182,94,181,106,85,107,151,230,114,23,92,15,240,245,215,95,111,214,138,213,209,70,112,79,235,63,81,111,72,45,160,55,135,66,160,117,21,16,76,9,209,4,124,124,118,11,86,102,98,217,42,64,221,236,124,49,46,
94,193,196,20,117,125,147,82,168,210,50,0,215,170,235,255,185,34,123,255,253,120,199,119,231,248,92,30,57,177,122,245,106,207,87,158,202,229,152,141,129,84,11,188,27,220,228,9,111,6,110,208,35,29,186,
182,158,52,7,107,125,252,159,227,187,203,160,57,135,80,190,120,213,62,139,65,141,216,181,179,8,254,230,30,180,85,152,216,251,36,139,74,107,205,134,13,27,242,114,146,19,79,108,54,126,38,103,227,146,1,188,
196,74,235,72,14,11,83,36,251,230,55,190,101,30,123,204,171,219,160,171,224,19,123,60,143,91,29,22,213,152,84,47,35,219,163,168,192,166,99,8,136,226,76,218,1,91,141,63,11,204,4,172,164,80,63,213,173,199,
24,190,252,194,249,187,238,3,251,54,247,112,2,155,222,64,235,154,150,242,216,28,252,196,19,160,19,26,87,136,111,117,213,41,229,41,50,76,244,28,208,199,179,238,203,28,189,205,108,223,156,3,73,158,108,212,
152,239,250,177,162,87,135,59,62,63,91,92,86,248,209,180,60,105,208,239,134,81,184,163,157,238,7,184,231,158,123,210,189,142,18,220,17,117,211,243,42,68,166,77,197,112,79,62,219,33,81,128,116,216,93,49,
121,154,178,15,69,144,217,249,180,238,239,187,239,62,238,189,247,94,48,161,154,193,120,245,61,50,60,220,85,216,193,182,64,183,125,74,43,150,78,144,167,188,134,102,45,166,59,29,47,164,179,128,73,90,235,
36,63,165,237,81,108,75,178,127,116,9,176,178,119,174,185,224,243,192,222,215,92,203,220,233,15,68,245,134,250,217,36,143,95,140,157,180,199,168,215,199,59,6,95,81,33,128,63,56,14,249,231,214,242,208,
87,31,116,48,186,110,45,170,123,197,127,29,46,161,99,194,131,134,78,193,229,202,52,17,65,174,72,172,107,114,136,206,113,69,127,124,225,19,152,124,243,20,169,163,210,50,229,11,107,148,236,228,176,172,91,
204,115,109,7,90,73,113,45,211,0,198,141,27,151,238,117,12,113,124,87,107,13,130,104,1,38,71,185,42,102,104,33,234,107,115,8,254,230,246,225,43,146,35,34,0,230,23,40,85,233,255,92,26,145,225,49,222,182,
13,206,239,143,223,1,232,31,10,133,150,165,176,4,92,57,68,158,61,254,248,227,1,24,53,106,20,239,188,243,142,247,192,250,131,250,119,177,229,179,38,197,181,236,69,242,172,221,215,128,134,154,154,26,54,
55,74,41,226,243,230,233,80,191,170,135,72,78,126,118,168,42,233,150,52,120,50,65,41,70,84,87,141,32,57,178,163,6,152,53,126,217,234,86,147,167,140,175,109,100,242,160,126,174,9,58,223,215,13,245,197,
56,227,174,85,79,76,76,246,38,30,34,224,129,104,246,185,239,93,109,107,97,1,139,124,118,94,132,168,168,200,235,53,92,75,242,224,255,177,192,149,225,112,184,169,149,241,135,99,73,30,179,218,0,188,156,161,
120,186,194,65,43,48,153,60,219,138,129,157,197,165,179,217,176,111,102,5,116,115,108,206,123,166,47,219,128,27,73,246,227,119,75,247,24,221,187,119,247,142,225,10,49,249,113,240,1,240,221,227,110,46,
183,213,180,105,230,89,122,247,221,102,119,233,195,142,253,206,72,81,118,224,78,223,123,125,123,170,231,13,49,32,20,114,61,156,63,68,169,200,29,229,155,54,251,193,230,159,227,156,214,107,60,147,108,134,
250,19,199,139,114,32,48,124,114,255,77,109,140,187,71,254,0,140,159,54,248,12,206,5,214,254,114,214,171,217,222,190,43,209,82,33,223,196,139,242,113,16,95,59,126,133,77,115,228,131,205,217,148,202,119,
238,155,72,233,10,186,56,51,139,203,233,222,14,154,113,65,86,59,234,170,62,252,54,185,6,59,176,149,112,244,106,210,62,127,93,93,157,119,12,215,224,237,79,82,116,213,43,73,30,167,152,146,226,65,121,131,
77,23,175,0,227,55,85,142,4,104,229,36,231,78,169,193,100,170,108,55,156,163,53,36,18,113,207,133,21,120,115,253,54,212,115,211,53,10,202,134,14,82,184,67,240,158,233,57,122,175,244,79,172,83,138,206,
37,132,55,237,76,199,150,44,0,227,191,15,86,222,251,196,99,13,189,247,216,51,235,102,231,248,174,144,107,35,230,173,91,108,219,241,98,224,189,192,166,48,118,208,121,251,237,183,79,250,157,157,192,53,210,
97,21,175,195,36,58,204,148,112,59,104,198,18,165,147,43,54,54,61,65,150,25,254,50,197,186,90,92,139,35,100,243,0,186,66,13,15,243,157,199,143,203,223,153,20,185,82,95,95,15,102,50,78,48,220,103,43,160,
187,163,251,236,202,3,114,71,138,151,206,230,197,172,70,117,171,99,203,89,254,71,105,234,79,246,66,235,4,184,103,165,206,56,250,63,143,165,95,223,171,87,130,123,185,197,227,8,133,154,21,255,241,147,199,
121,217,49,93,185,239,255,165,27,115,10,28,112,45,181,85,200,133,9,122,21,224,152,46,103,251,249,0,159,127,254,121,146,81,101,219,222,97,142,223,188,1,52,101,177,46,67,99,59,104,193,5,169,179,46,53,104,
91,85,85,229,117,207,215,59,26,106,175,124,119,125,109,67,172,112,8,126,109,22,135,91,97,197,217,63,40,55,20,147,34,98,169,175,71,225,234,198,46,165,229,84,183,19,129,63,251,254,223,219,158,39,24,38,119,
188,227,197,53,163,61,214,117,247,93,127,196,250,143,222,155,141,137,36,41,247,109,26,128,10,85,191,120,241,197,139,247,185,246,90,26,190,250,28,66,161,195,73,142,4,121,143,77,115,198,180,202,233,13,154,
201,67,7,213,19,139,126,140,89,242,206,223,179,248,233,227,227,78,122,234,176,187,239,97,197,140,135,81,165,229,253,49,41,10,54,237,35,104,253,226,187,53,13,206,184,202,116,237,26,87,211,135,77,252,228,
249,100,235,2,84,223,55,24,23,230,142,129,239,79,194,172,51,209,140,111,162,163,43,58,231,239,128,206,98,93,6,151,14,44,183,6,79,172,141,154,112,65,198,93,186,98,148,142,182,15,69,80,240,71,20,23,23,191,
91,128,69,59,70,58,190,251,34,211,23,135,125,81,221,232,89,212,62,126,7,92,228,179,216,119,119,136,215,39,64,180,133,244,199,55,4,4,31,204,178,117,135,65,115,56,102,111,146,99,212,163,192,231,147,38,77,
106,119,149,252,203,39,103,114,231,150,67,22,235,166,198,213,1,193,47,7,182,249,246,134,235,22,239,115,237,181,94,164,204,117,78,55,76,22,97,113,227,231,47,78,76,170,174,122,61,73,240,97,255,21,79,62,
102,162,164,74,186,225,213,89,128,127,3,252,59,183,40,16,215,139,125,43,128,13,27,54,20,98,17,237,209,121,117,57,132,66,94,143,117,26,112,77,82,79,9,238,45,43,43,211,182,173,123,252,24,227,231,15,138,
118,182,198,200,82,199,119,229,152,89,239,13,116,96,186,226,196,43,157,226,237,57,42,31,147,82,54,26,116,205,15,173,171,219,254,98,150,135,125,158,228,193,229,113,96,6,119,237,236,218,189,29,191,123,24,
208,174,104,30,123,157,117,36,15,10,31,10,132,67,161,144,23,142,57,152,228,172,129,179,129,85,167,159,126,122,251,172,232,134,13,26,119,100,197,241,161,170,230,91,169,198,164,173,245,179,14,165,94,137,
12,30,154,241,57,175,48,255,184,38,249,28,172,138,75,120,240,144,131,32,28,9,225,14,155,125,32,81,187,42,215,219,118,13,238,15,3,186,231,51,33,151,13,38,72,53,246,145,211,115,99,219,164,171,222,246,2,
194,118,92,203,187,6,112,47,53,121,169,247,2,201,130,101,142,231,172,28,51,115,125,147,12,152,133,254,136,224,231,71,240,93,161,100,7,22,224,92,101,182,145,6,185,55,75,247,208,82,146,103,229,245,4,134,
250,226,144,15,113,252,124,114,75,22,149,197,181,46,192,207,125,47,9,87,100,226,95,218,117,69,55,212,129,59,130,104,60,27,31,168,159,56,182,127,66,40,20,59,233,221,247,51,62,231,249,70,172,222,36,57,210,
100,56,74,13,88,247,246,107,144,136,87,147,28,250,183,30,248,234,244,186,156,199,87,155,48,201,251,130,92,156,207,178,181,237,173,175,163,215,151,23,43,31,179,148,224,43,129,77,221,240,185,43,237,53,84,
99,102,158,251,169,7,102,248,122,198,217,240,128,227,187,171,58,186,248,117,213,212,10,143,186,44,48,160,34,31,147,42,124,139,171,184,6,79,191,35,67,223,176,143,6,146,103,221,118,3,182,179,215,93,132,
89,157,43,216,155,72,39,121,138,171,251,123,136,175,60,130,227,2,141,184,19,187,181,27,42,127,126,36,182,172,93,153,24,247,157,100,102,215,186,214,34,124,82,175,93,157,149,242,150,25,51,21,220,121,148,
174,84,145,18,172,64,149,7,182,205,67,235,165,121,186,117,87,106,137,11,128,238,249,104,223,39,158,120,162,39,164,135,82,152,65,91,15,215,74,32,87,131,113,51,218,107,216,205,81,150,255,3,22,248,122,0,
217,224,170,191,131,128,45,243,53,241,202,235,205,164,250,136,224,231,1,107,213,189,98,45,170,32,79,4,220,49,89,97,87,211,41,195,29,179,62,21,240,178,229,229,171,33,158,216,194,3,114,155,239,190,91,42,
147,231,82,116,161,189,135,42,152,74,250,105,128,210,210,210,118,91,215,191,184,231,62,48,145,70,174,148,147,167,217,217,181,174,52,214,147,99,181,217,143,229,52,153,73,83,174,149,176,142,179,17,66,174,
197,232,39,146,191,135,252,49,146,163,117,138,177,57,118,114,105,223,161,80,200,75,81,16,105,169,231,152,43,182,103,249,53,201,169,27,42,128,61,125,147,252,92,61,151,63,249,122,0,89,9,49,102,156,205,149,
54,226,242,92,203,208,215,243,80,192,127,109,125,121,159,25,184,51,135,118,88,193,223,172,51,109,135,13,27,230,253,233,18,227,61,60,215,78,30,214,211,156,142,123,129,239,123,97,147,108,121,153,242,41,
201,131,74,94,244,204,57,142,174,237,187,25,28,59,40,82,35,236,61,184,114,255,60,7,249,73,198,85,72,202,127,176,171,231,174,10,86,232,174,152,65,233,224,36,184,143,128,101,191,206,225,129,222,235,206,
201,96,162,170,22,39,139,174,218,5,247,82,152,147,226,75,115,246,223,243,204,51,207,128,137,2,123,219,177,249,38,236,60,10,187,218,84,198,248,66,117,11,58,235,212,38,14,140,2,207,58,54,31,102,221,62,67,
73,14,167,93,149,162,7,159,54,54,140,83,227,78,247,125,50,102,205,223,172,203,208,231,38,189,4,147,239,234,48,223,231,16,204,2,241,157,70,240,55,107,192,246,220,185,115,189,63,159,194,61,120,59,19,147,
47,38,99,209,247,237,63,33,197,67,125,31,240,117,182,131,49,190,94,193,212,20,86,126,255,192,119,203,129,37,25,156,207,149,226,247,18,76,228,79,144,167,236,67,217,174,249,213,211,207,130,82,239,147,28,
78,55,56,133,235,227,6,114,76,90,245,189,83,78,133,68,98,189,125,121,248,41,194,189,208,250,11,104,205,25,121,176,240,15,56,224,0,239,207,84,51,76,223,6,182,88,179,102,77,70,237,59,28,14,19,112,239,29,
93,200,122,243,69,203,253,53,133,232,122,109,211,217,163,205,161,7,237,55,198,254,228,240,4,104,43,200,63,202,180,12,3,26,177,27,238,49,129,167,113,47,65,42,22,126,182,216,240,196,122,220,57,175,1,222,
199,38,112,218,103,159,214,131,16,2,11,62,223,154,66,72,22,67,171,41,89,210,109,136,207,58,202,241,175,36,103,93,156,78,122,254,251,230,247,33,201,235,241,158,233,16,252,249,192,255,10,148,123,168,0,45,
46,209,128,117,215,249,40,195,70,93,248,104,0,94,31,159,7,75,91,155,137,95,46,113,119,13,226,63,145,88,179,50,111,183,107,211,91,127,131,59,25,92,185,221,182,75,186,70,77,56,28,246,199,239,95,137,123,
18,96,254,45,67,99,168,204,39,57,170,173,47,38,250,109,111,135,182,60,158,99,15,26,128,157,118,106,206,223,182,103,224,89,243,172,167,55,177,75,164,166,43,250,190,253,246,198,228,159,114,241,107,64,23,
106,34,227,102,139,195,159,92,93,89,56,43,126,201,42,174,108,161,18,124,93,170,167,129,191,1,191,119,60,20,179,129,27,94,120,225,133,137,4,242,133,212,212,212,184,186,115,163,49,81,121,251,165,56,237,
169,64,83,75,139,150,164,67,207,158,61,89,187,118,237,59,142,158,210,64,167,181,154,193,195,165,181,94,143,241,93,246,223,212,13,145,20,215,127,53,29,137,120,28,194,69,127,198,237,175,247,179,130,80,104,
161,206,67,90,218,217,43,106,249,126,117,229,115,160,26,105,125,214,228,75,59,254,249,186,188,221,110,101,101,165,39,46,183,96,22,211,216,62,217,181,196,7,214,48,249,187,214,122,81,208,176,112,196,235,
143,182,98,31,204,218,186,6,119,50,194,124,9,62,152,104,176,160,229,117,51,102,69,55,63,139,128,15,243,33,150,159,126,218,28,221,250,145,237,249,254,222,241,114,153,134,89,91,227,6,173,245,167,105,148,
225,96,204,224,249,185,41,78,123,50,48,207,183,148,100,167,176,240,203,1,141,10,21,234,179,120,216,144,1,161,12,26,211,249,142,174,183,199,5,214,237,51,11,19,55,221,3,40,234,213,171,87,200,190,44,171,
109,67,88,130,153,198,157,74,236,143,2,158,25,57,114,100,214,3,73,30,54,111,72,141,189,166,150,248,28,88,62,99,70,198,115,79,30,104,101,123,2,184,175,80,81,4,133,224,180,87,223,196,190,192,151,183,178,
235,35,196,99,49,242,32,24,183,104,237,217,133,119,182,178,107,35,232,143,71,95,116,81,33,196,178,17,147,85,53,85,84,216,197,190,246,125,148,109,223,225,72,36,18,194,12,202,14,177,237,123,169,109,223,
251,58,122,173,247,22,170,222,180,214,222,220,146,143,73,158,65,220,159,228,188,65,231,229,187,12,109,59,63,159,228,1,106,175,145,140,195,76,108,252,6,51,19,183,31,80,100,203,48,140,9,155,62,210,150,223,
130,22,196,126,58,112,247,226,197,139,189,197,226,59,141,224,23,220,158,203,194,130,216,133,150,147,44,237,129,153,188,180,22,19,231,220,128,25,80,90,132,153,253,58,160,133,223,158,130,93,77,235,195,15,
63,204,231,125,182,22,3,63,13,224,240,195,15,207,244,184,183,183,178,253,51,160,41,199,144,183,182,101,155,109,209,235,214,208,66,55,218,227,90,221,80,159,183,211,198,76,180,206,163,180,236,198,156,216,
6,78,206,93,112,207,30,245,183,239,233,182,125,71,109,251,110,178,238,148,27,73,30,27,194,26,57,187,145,93,154,144,12,58,103,113,236,57,158,107,205,147,229,61,103,121,117,129,108,156,172,118,6,238,60,
73,30,195,49,169,28,150,249,202,48,106,175,253,97,90,158,145,124,43,118,32,120,208,160,65,5,109,8,93,53,14,223,37,250,191,196,76,229,110,45,87,134,194,157,145,48,217,61,96,124,223,83,2,231,201,23,47,224,
78,148,229,117,55,95,246,61,48,105,97,123,3,186,149,135,235,45,32,22,152,218,222,238,217,238,194,203,193,149,61,211,215,139,7,150,239,112,193,165,121,59,231,153,198,58,252,152,150,83,111,255,85,215,175,
43,88,219,246,249,193,7,2,175,167,243,179,52,218,247,124,76,234,136,37,109,168,33,191,111,101,251,227,144,247,37,74,73,36,18,94,25,38,172,117,254,27,220,11,143,7,137,208,122,128,74,220,186,201,206,41,
144,70,136,224,183,34,250,151,98,66,17,115,77,8,54,193,30,231,189,66,84,164,207,55,248,82,11,15,237,187,153,230,77,241,173,133,219,210,160,220,164,142,88,199,123,92,118,25,152,56,231,84,76,37,30,227,199,
151,92,154,223,19,107,189,130,212,139,132,124,67,184,168,102,240,137,227,219,170,24,246,194,132,29,231,226,87,252,61,102,249,191,21,109,92,133,75,61,35,38,5,247,0,121,91,162,180,5,75,255,95,152,196,130,
185,62,7,207,219,227,92,222,86,98,15,133,30,180,77,36,32,28,94,218,130,37,90,160,134,145,121,225,41,165,152,58,117,42,199,29,119,220,92,204,12,194,29,49,190,187,31,2,91,144,60,155,207,207,74,76,132,203,
19,152,129,91,13,38,158,183,0,201,216,252,17,19,255,197,248,104,253,78,129,48,38,170,161,41,211,204,136,62,191,252,251,152,193,219,224,96,99,2,120,167,3,191,219,87,218,151,228,176,64,153,105,224,249,110,
59,236,156,255,51,234,4,168,240,95,48,174,178,96,88,211,83,69,125,43,245,1,183,222,214,38,6,141,54,121,160,159,198,248,234,79,197,248,159,71,208,242,234,74,107,109,219,126,22,19,6,25,245,142,105,219,203,
106,140,111,218,31,22,147,215,172,179,197,197,197,222,115,52,17,51,174,16,124,192,55,0,175,20,82,52,227,241,56,74,41,42,42,42,168,169,169,89,136,137,240,187,24,19,186,185,151,213,136,150,2,243,235,49,
227,37,31,96,220,100,239,129,153,111,208,150,209,110,5,19,252,184,214,92,175,20,149,131,250,109,73,27,135,98,118,223,121,151,117,115,72,200,0,0,12,116,73,68,65,84,172,194,44,142,63,254,120,78,56,225,4,
230,205,155,199,144,33,67,62,197,248,223,189,40,149,10,76,42,216,106,219,224,154,48,83,184,231,219,7,185,1,223,248,65,161,223,216,35,71,142,228,131,15,62,152,4,220,237,176,238,179,78,225,106,179,21,206,
199,134,237,57,92,69,140,26,53,170,67,170,125,124,241,10,194,131,250,237,239,234,217,170,162,72,211,113,47,190,156,247,115,174,88,186,154,126,195,6,207,212,209,166,173,147,159,3,21,63,249,163,79,24,215,
70,247,175,148,226,186,235,174,227,34,51,64,124,39,112,23,102,226,89,137,21,210,225,152,25,213,113,235,174,249,6,179,136,72,131,191,77,5,218,246,45,192,63,2,247,22,207,231,117,251,66,44,103,216,23,22,
14,67,164,77,168,173,173,245,191,236,86,2,191,181,58,90,106,203,114,43,204,132,176,176,45,147,69,86,39,106,173,102,52,249,159,181,182,14,126,40,152,224,135,128,139,204,205,116,168,116,162,90,107,182,216,
98,139,141,230,114,56,220,20,141,70,155,148,82,235,73,177,156,155,82,138,72,36,146,115,236,111,38,124,248,225,135,40,165,18,228,121,177,6,187,120,185,166,125,44,2,145,87,236,196,166,104,91,158,115,130,
57,167,78,245,28,156,214,198,101,48,97,194,4,38,76,152,208,92,221,209,104,180,190,168,168,168,30,19,94,57,219,101,217,150,149,149,57,123,170,86,248,99,180,65,142,120,159,31,189,93,180,75,255,75,175,172,
172,44,86,95,95,191,222,134,53,175,192,49,203,89,107,77,69,69,133,23,101,183,217,232,138,249,240,51,238,202,165,147,98,181,45,197,94,16,242,69,1,242,227,119,57,188,0,134,118,183,234,91,10,67,92,16,4,65,
232,2,136,224,11,130,32,136,224,11,130,32,8,34,248,130,32,8,130,8,190,32,8,130,208,126,5,223,139,153,13,211,1,70,153,5,65,16,132,236,5,223,139,107,149,16,77,65,16,132,78,46,248,130,32,8,130,8,190,32,8,
130,32,130,47,8,130,32,136,224,11,130,32,8,34,248,130,32,8,194,102,66,34,115,4,65,16,58,43,241,24,20,69,30,199,172,113,177,74,44,124,65,16,156,204,155,55,15,173,117,243,167,189,225,191,182,116,174,207,
203,99,239,125,170,171,171,59,125,29,254,224,143,87,66,44,122,25,241,216,41,196,99,23,116,106,11,63,211,21,159,4,169,23,97,35,118,93,8,111,241,159,21,237,244,50,79,199,184,166,255,157,206,11,2,179,216,
203,86,192,220,69,139,22,109,80,157,124,178,233,110,231,157,199,110,231,157,215,252,255,78,45,248,123,236,177,7,159,127,254,185,60,185,237,204,42,171,170,170,146,130,232,56,28,1,220,79,54,235,134,182,
221,245,21,165,35,248,150,109,48,11,189,252,4,120,165,171,85,102,167,22,252,170,170,42,17,23,65,200,141,246,190,178,79,130,204,150,56,92,9,220,129,89,95,182,203,33,131,182,130,208,9,123,81,62,250,97,214,
91,93,129,89,236,155,162,162,34,226,241,120,186,191,15,251,190,47,2,98,173,185,65,236,239,189,53,93,19,190,69,212,177,61,133,112,58,199,105,225,126,74,172,96,103,51,176,176,4,56,3,224,242,203,47,207,246,
252,197,246,56,113,128,146,146,18,231,18,144,105,214,203,106,96,61,64,56,28,38,145,40,236,242,188,50,104,43,8,157,147,221,49,11,103,47,5,230,2,107,129,235,32,237,49,148,74,43,168,247,251,44,253,40,102,
161,238,116,152,13,92,229,248,254,167,246,197,211,43,45,139,180,168,217,38,221,10,152,111,133,118,158,21,201,31,145,249,2,230,219,218,251,26,121,197,21,87,180,186,243,252,249,243,189,63,71,96,214,180,
94,106,175,35,10,92,6,208,216,152,209,50,187,35,128,229,190,122,89,3,252,19,104,241,37,44,130,47,8,66,18,86,52,186,3,47,3,51,173,40,142,4,78,6,46,2,110,78,243,80,107,129,163,128,191,217,255,31,12,252,
10,104,74,243,247,97,127,239,192,135,178,158,133,86,205,123,165,148,183,86,244,78,192,183,86,32,143,4,198,2,19,48,62,248,93,50,180,244,51,26,139,24,50,100,8,24,127,255,215,192,27,192,207,129,125,108,89,
94,1,220,5,112,245,213,87,167,115,184,30,192,59,152,69,206,127,108,175,253,120,224,84,96,170,184,116,4,65,200,204,130,11,133,0,202,236,231,10,224,11,235,78,248,72,41,85,4,12,183,162,167,91,18,90,160,73,
107,253,136,239,235,103,219,58,162,197,231,2,121,4,248,12,216,11,76,184,232,208,161,67,103,1,239,89,17,254,180,16,231,183,247,91,12,76,177,47,151,163,125,155,95,6,94,183,159,105,151,92,114,201,115,151,
94,122,105,58,130,223,11,184,9,120,203,126,247,49,16,177,150,127,132,2,143,153,136,224,11,66,39,162,169,169,137,226,226,226,58,107,161,63,5,252,3,120,72,41,53,23,184,187,35,221,139,21,252,129,86,12,119,
2,24,49,98,4,115,230,204,241,182,189,105,69,95,21,240,252,125,129,161,192,255,193,198,241,15,187,237,3,235,154,57,26,120,46,141,67,214,2,171,128,251,108,189,60,8,124,211,86,214,61,136,75,71,16,58,21,253,
250,245,3,168,3,190,103,197,228,124,224,59,107,209,255,13,227,155,111,51,205,78,243,187,150,24,102,255,93,157,72,36,152,51,103,14,96,6,56,45,179,11,124,15,197,246,223,207,96,163,159,221,90,255,13,152,
241,136,190,105,30,171,206,190,188,62,0,206,198,184,137,52,38,164,180,63,64,113,113,177,8,190,32,8,233,81,83,83,227,253,185,16,51,64,186,53,38,34,228,8,96,60,198,127,92,214,6,151,162,82,232,75,73,134,
199,89,238,9,175,223,165,228,27,120,30,90,224,251,240,78,52,196,255,101,207,158,61,193,184,96,138,177,81,54,105,244,22,192,12,210,30,130,113,173,245,3,14,176,61,132,183,129,30,233,68,251,136,224,11,130,
224,231,135,62,203,119,61,38,36,243,63,192,105,86,184,250,180,193,53,188,7,28,238,137,157,79,240,142,100,227,42,123,233,240,173,117,133,220,224,79,141,96,197,127,11,96,95,178,11,207,108,149,94,189,122,
121,2,189,12,56,215,47,220,181,181,181,216,94,212,32,140,235,44,29,182,3,62,178,189,172,58,91,47,207,88,193,31,138,241,241,139,224,11,130,144,17,95,89,11,242,137,192,51,126,178,21,207,218,54,184,134,191,
218,107,152,226,251,238,28,224,36,54,174,163,221,34,145,72,196,251,243,48,251,162,120,200,183,121,91,76,66,176,66,247,150,234,129,63,216,30,210,223,124,162,63,6,248,208,126,30,72,115,64,123,5,198,69,245,
32,155,142,59,252,26,227,219,175,23,193,23,4,33,109,122,244,232,1,80,131,9,163,220,198,138,107,189,181,130,183,180,226,185,46,131,67,134,179,188,148,217,192,41,152,80,78,109,63,227,48,185,111,210,114,
41,249,220,54,175,96,194,23,127,226,59,214,151,152,40,164,153,100,230,38,82,89,104,223,61,192,177,192,9,190,243,191,6,220,142,9,123,165,162,162,162,197,3,88,23,208,42,224,64,160,10,51,127,192,171,151,
145,246,248,53,221,187,119,47,104,251,144,40,29,65,232,68,172,95,111,220,201,137,68,226,165,80,40,180,13,102,54,103,21,102,194,82,12,96,197,138,140,242,160,61,100,63,42,93,215,137,231,122,177,214,253,
20,123,254,13,108,244,117,79,78,55,251,166,239,88,211,236,167,151,189,167,165,118,151,91,125,66,158,206,65,191,240,68,127,250,244,233,105,157,191,188,188,156,245,235,215,63,0,60,128,153,227,208,19,88,
236,223,167,53,214,173,91,199,156,57,115,24,62,124,248,155,192,206,24,255,127,127,107,245,55,6,235,79,44,124,65,16,210,22,201,178,178,50,79,40,27,128,5,158,216,43,165,188,72,158,86,41,45,45,69,41,229,
125,116,38,113,248,74,41,74,74,74,252,174,140,245,158,239,93,41,69,40,20,242,111,79,235,88,54,237,64,141,39,246,87,93,117,85,243,241,194,225,112,171,98,95,82,82,226,191,31,142,57,230,152,180,206,95,87,
87,71,36,18,241,202,115,189,39,246,222,125,164,203,136,17,35,40,45,45,245,142,19,197,12,172,55,106,173,155,203,186,208,136,133,47,8,157,144,198,198,198,140,196,40,213,49,252,100,154,19,191,169,169,169,
69,17,203,36,37,65,83,83,147,63,20,51,137,116,114,208,100,152,2,97,19,98,177,152,179,60,51,45,147,124,212,139,88,248,130,32,8,130,8,190,32,8,130,32,130,47,8,130,32,130,47,8,130,32,136,224,11,130,208,14,
105,143,139,140,11,237,175,254,37,74,71,16,58,1,149,149,149,156,124,242,201,82,16,93,148,161,67,211,75,41,164,38,85,87,205,197,228,113,248,68,199,162,59,143,95,182,70,74,79,16,4,161,19,34,46,29,65,16,
4,17,124,65,16,4,65,4,95,16,4,65,16,193,23,4,65,16,68,240,5,65,16,4,17,124,65,16,4,65,4,95,16,4,65,16,193,23,4,65,16,68,240,5,65,16,4,17,124,65,16,4,17,124,65,16,4,161,19,35,201,211,4,65,40,24,119,132,
21,161,202,94,157,230,126,116,67,29,227,107,155,68,240,5,65,16,60,254,123,202,201,44,127,252,33,66,3,251,149,160,245,207,128,112,103,184,47,213,189,215,107,147,7,246,93,89,178,213,54,156,240,250,155,34,
248,130,32,8,43,102,62,142,234,214,125,52,90,191,209,233,110,46,20,62,165,241,127,223,76,233,144,151,46,77,83,16,132,188,83,210,45,4,76,235,164,119,247,15,66,161,14,233,167,18,193,23,4,33,255,36,226,37,
64,89,39,189,187,16,208,189,35,94,184,184,116,4,65,0,96,3,112,183,82,68,6,245,207,94,23,66,42,81,183,96,105,162,124,80,191,56,240,1,208,55,197,158,165,192,78,29,184,184,58,228,154,146,34,248,130,32,0,
112,163,82,12,28,88,9,58,241,17,144,200,226,16,197,196,185,169,124,64,223,127,199,23,45,111,10,15,172,60,16,149,106,87,53,4,152,47,165,46,130,47,8,194,102,32,12,88,129,254,94,14,135,169,66,65,61,208,35,
30,199,41,248,42,4,33,85,34,37,46,130,47,8,66,39,160,199,160,126,37,104,253,34,80,149,98,151,72,71,191,199,233,71,28,206,218,215,95,134,112,129,100,84,107,34,67,134,1,16,93,56,207,177,61,65,100,240,48,
34,125,250,82,247,225,219,156,240,244,139,148,142,220,85,4,95,16,132,54,71,1,91,2,3,59,235,13,214,190,246,34,42,82,178,51,240,71,160,33,239,29,46,165,222,142,46,152,123,11,0,161,208,157,64,79,223,246,
16,42,60,57,186,112,222,147,209,5,223,161,34,37,135,76,253,249,65,207,147,136,55,84,140,217,139,163,30,126,84,4,95,16,4,33,111,111,180,80,24,160,26,56,186,64,167,40,69,169,91,208,26,224,96,160,95,96,251,
171,198,101,22,2,184,16,244,52,66,161,189,107,223,156,245,254,211,191,253,63,14,184,245,182,164,3,134,32,245,176,138,32,8,130,208,97,232,1,188,7,234,190,69,143,62,212,119,114,117,21,74,169,36,11,127,131,
253,59,162,66,33,38,85,246,144,98,19,132,46,72,85,223,238,249,182,255,196,152,220,60,28,15,28,143,82,39,76,26,220,255,193,59,148,138,157,110,122,9,20,1,113,187,83,53,161,240,221,170,164,155,20,151,32,
116,93,162,121,58,78,12,184,158,212,19,148,42,129,223,74,113,23,148,251,72,36,126,23,170,174,26,11,212,121,130,239,249,241,123,2,39,73,25,9,130,144,43,77,139,150,199,138,7,244,253,155,219,198,87,160,212,
150,34,248,109,194,174,192,33,192,131,158,224,71,165,76,4,65,200,39,38,166,95,181,214,3,16,218,134,90,239,143,34,140,191,167,26,227,203,23,159,155,32,116,109,54,0,57,231,253,13,155,56,252,153,164,142,
195,47,150,162,46,124,71,11,184,21,173,159,110,22,252,85,139,87,124,92,12,31,107,41,28,65,232,210,20,3,165,213,149,249,178,251,20,176,61,157,56,14,191,157,243,41,48,182,124,167,93,86,31,251,204,179,156,
230,9,254,31,180,72,189,32,8,112,173,82,148,74,49,116,116,86,2,191,71,235,123,117,67,29,191,122,250,25,126,229,219,40,233,145,5,65,16,114,235,201,180,197,177,91,211,106,13,60,5,244,143,46,94,113,239,158,
55,76,100,252,234,250,164,157,100,166,173,32,8,133,162,83,251,233,117,221,90,84,207,62,239,1,7,178,49,188,61,159,98,191,92,199,99,118,50,109,232,168,64,121,42,224,75,29,139,65,60,138,42,45,31,135,214,
243,18,203,86,242,235,22,188,54,34,248,130,32,20,194,56,141,129,190,26,51,251,211,101,141,246,5,206,237,200,119,56,126,93,140,155,148,90,17,130,167,11,242,66,1,206,179,226,61,81,169,87,130,93,137,251,
128,119,19,9,47,26,106,94,58,199,20,193,23,4,33,239,110,136,218,69,203,98,61,251,245,186,201,233,239,8,133,33,20,26,222,209,5,31,224,119,109,52,6,122,158,227,60,217,20,158,8,190,32,8,0,212,208,156,157,
107,50,217,173,232,20,1,62,68,107,122,15,31,166,18,245,117,169,114,53,104,54,205,252,40,180,213,219,88,75,148,142,32,8,150,229,90,243,214,169,167,100,245,91,173,19,244,30,182,37,163,254,252,23,166,14,
238,95,138,214,223,210,114,88,102,71,157,247,211,8,108,125,218,162,229,139,58,218,133,139,133,47,8,66,51,253,148,226,208,187,166,228,197,152,164,115,103,227,237,144,247,37,97,153,130,32,228,159,120,124,
3,176,164,147,222,93,61,74,45,19,193,23,4,65,0,244,134,245,0,251,2,51,59,217,173,189,7,140,34,22,237,144,57,200,254,63,137,4,226,108,36,92,20,118,0,0,0,0,73,69,78,68,174,66,96,130,0,0 };

const char* DrumSnapperAudioProcessorEditor::logoSmall_png = (const char*)resource_NewComponent2_logoSmall_png;
const int DrumSnapperAudioProcessorEditor::logoSmall_pngSize = 19731;