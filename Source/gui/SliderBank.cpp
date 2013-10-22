/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
#include "MicronSlider.h"
//[/Headers]

#include "SliderBank.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
SliderBank::SliderBank ()
{
    addAndMakeVisible (slider9 = new MicronSlider ("new slider"));
    slider9->setRange (-100, 100, 0.1);
    slider9->setSliderStyle (Slider::LinearVertical);
    slider9->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    slider9->addListener (this);

    addAndMakeVisible (slider10 = new MicronSlider ("new slider"));
    slider10->setRange (-100, 100, 0.1);
    slider10->setSliderStyle (Slider::LinearVertical);
    slider10->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    slider10->addListener (this);

    addAndMakeVisible (slider11 = new MicronSlider ("new slider"));
    slider11->setRange (-100, 100, 0.1);
    slider11->setSliderStyle (Slider::LinearVertical);
    slider11->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    slider11->addListener (this);

    addAndMakeVisible (slider12 = new MicronSlider ("new slider"));
    slider12->setRange (-100, 100, 0.1);
    slider12->setSliderStyle (Slider::LinearVertical);
    slider12->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    slider12->addListener (this);

    addAndMakeVisible (slider13 = new MicronSlider ("new slider"));
    slider13->setRange (-100, 100, 0.1);
    slider13->setSliderStyle (Slider::LinearVertical);
    slider13->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    slider13->addListener (this);

    addAndMakeVisible (slider14 = new MicronSlider ("new slider"));
    slider14->setRange (-100, 100, 0.1);
    slider14->setSliderStyle (Slider::LinearVertical);
    slider14->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    slider14->addListener (this);

    addAndMakeVisible (slider15 = new MicronSlider ("new slider"));
    slider15->setRange (-100, 100, 0.1);
    slider15->setSliderStyle (Slider::LinearVertical);
    slider15->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    slider15->addListener (this);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (400, 300);


    //[Constructor] You can add your own custom stuff here..

	lockedSliderBeingDragged = 0;
	prevSlider = 0;
	resettingValuesToDefault = false;
	setBroadStrokesMode(true);
	resetDoubleClickTimer();

	// disable any clicks from getting to children.
	setInterceptsMouseClicks (true, false);
	for (int i = getNumChildComponents(); --i >= 0;)
		getChildComponent(i)->setInterceptsMouseClicks(false, false);

	updateMaxBoundX();

    //[/Constructor]
}

SliderBank::~SliderBank()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    slider9 = nullptr;
    slider10 = nullptr;
    slider11 = nullptr;
    slider12 = nullptr;
    slider13 = nullptr;
    slider14 = nullptr;
    slider15 = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void SliderBank::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void SliderBank::resized()
{
    slider9->setBounds (4, 4, 10, 140);
    slider10->setBounds (20, 4, 10, 140);
    slider11->setBounds (36, 4, 10, 140);
    slider12->setBounds (52, 4, 10, 140);
    slider13->setBounds (68, 4, 10, 140);
    slider14->setBounds (84, 4, 10, 140);
    slider15->setBounds (100, 4, 10, 140);
    //[UserResized] Add your own custom resize handling here..
	updateMaxBoundX();
    //[/UserResized]
}

void SliderBank::updateMaxBoundX()
{
	maxBoundX = 0;
	for (int i = getNumChildComponents(); --i >= 0;)
	{
		MicronSlider* slider = dynamic_cast<MicronSlider*>(getChildComponent(i));
		if (slider)
		{
			const int sliderRightEdge = slider->getBounds().getRight() + 5;
			if (sliderRightEdge > maxBoundX)
				maxBoundX = sliderRightEdge;
		}
	}
}

void SliderBank::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == slider9)
    {
        //[UserSliderCode_slider9] -- add your slider handling code here..
        //[/UserSliderCode_slider9]
    }
    else if (sliderThatWasMoved == slider10)
    {
        //[UserSliderCode_slider10] -- add your slider handling code here..
        //[/UserSliderCode_slider10]
    }
    else if (sliderThatWasMoved == slider11)
    {
        //[UserSliderCode_slider11] -- add your slider handling code here..
        //[/UserSliderCode_slider11]
    }
    else if (sliderThatWasMoved == slider12)
    {
        //[UserSliderCode_slider12] -- add your slider handling code here..
        //[/UserSliderCode_slider12]
    }
    else if (sliderThatWasMoved == slider13)
    {
        //[UserSliderCode_slider13] -- add your slider handling code here..
        //[/UserSliderCode_slider13]
    }
    else if (sliderThatWasMoved == slider14)
    {
        //[UserSliderCode_slider14] -- add your slider handling code here..
        //[/UserSliderCode_slider14]
    }
    else if (sliderThatWasMoved == slider15)
    {
        //[UserSliderCode_slider15] -- add your slider handling code here..
        //[/UserSliderCode_slider15]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void SliderBank::setBroadStrokesMode(bool enableBroadStrokes)
{
	broadStrokesEnabled = enableBroadStrokes;
}

MicronSlider* SliderBank::findSliderAtPos(const Point<int>& pos)
{
	MicronSlider* closestSlider = 0;
	float closestDist = 1e6;
	for (int i = getNumChildComponents(); --i >= 0;)
	{
		MicronSlider* slider = dynamic_cast<MicronSlider*>(getChildComponent(i));
		if (slider)
		{
			float sliderDist = abs( slider->getBounds().getCentre().getX() - pos.getX() );
//			if ( slider == prevSlider )
//				sliderDist *= 0.75f;

			if ( sliderDist < closestDist )
			{
				closestDist = sliderDist;
				closestSlider = slider;
			}
		}
	}
	return closestSlider;
}

void SliderBank::mouseWheelMove (const MouseEvent& event, const MouseWheelDetails& wheelDetails)
{
	MicronSlider* slider = findSliderAtPos(event.getPosition());
	if (slider)
		slider->mouseWheelMove(event.getEventRelativeTo(slider), wheelDetails);
}

void SliderBank::mouseDown (const MouseEvent& event)
{
	if (event.eventTime - prevMouseDownTime < RelativeTime(MicronSlider::doubleClickDuration))
	{ // double-click, initiate default-values painting
		resettingValuesToDefault = true;
		resetDoubleClickTimer();
	}
	else
	{ // first click, prime for double-click detection
		prevMouseDownTime = event.eventTime;
	}

	// possibly lock onto a particular slider for dragging.
	if (event.mods.isRightButtonDown() || ! broadStrokesEnabled)
		lockedSliderBeingDragged = findSliderAtPos(event.getPosition());

	// let mouseDrag work out which slider to click and manage tracking from there.
	mouseDrag(event);
}

void SliderBank::mouseUp (const MouseEvent& event)
{	// reset all tracking state except for double-click detection
	resettingValuesToDefault = false;
	lockedSliderBeingDragged = 0;
	prevSlider = 0;
}

void SliderBank::resetDoubleClickTimer()
{	// ensure next click does not look like another double-click
	prevMouseDownTime = Time::getCurrentTime() - RelativeTime(MicronSlider::doubleClickDuration + 1.0);
}

void SliderBank::mouseDrag (const MouseEvent& event)
{
	// choose slider to operate on
	MicronSlider* slider = 0;
	if (lockedSliderBeingDragged)
		slider = lockedSliderBeingDragged;
	else
		if ( event.getPosition().getX() >= 0 && event.getPosition().getX() < maxBoundX )
			slider = findSliderAtPos(event.getPosition());

	if ( ! slider )
		return; // no slider

	if (slider != prevSlider)
	{ // different slider from last time
		if (resettingValuesToDefault)
		{ // reset slider to default value
			bool dummy;
			slider->setValue(slider->getDoubleClickReturnValue(dummy));
		}
		else
		{ // set slider value at mouse position
			slider->resetDoubleClickTimer(); // ensure we don't reset sliders just by dragging back and forth over them
			slider->mouseDown(event.getEventRelativeTo(slider));
		}
	}
	else
	{ // same slider as last time
		if ( ! resettingValuesToDefault )
			slider->mouseDrag(event.getEventRelativeTo(slider));
	}

	prevSlider = slider;
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="SliderBank" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="400" initialHeight="300">
  <BACKGROUND backgroundColour="ffffff"/>
  <SLIDER name="new slider" id="14574dce7fda0989" memberName="slider9"
          virtualName="MicronSlider" explicitFocusOrder="0" pos="4 4 10 140"
          min="-100" max="100" int="0.10000000000000000555" style="LinearVertical"
          textBoxPos="NoTextBox" textBoxEditable="1" textBoxWidth="80"
          textBoxHeight="20" skewFactor="1"/>
  <SLIDER name="new slider" id="68d6dc2e2abe9f73" memberName="slider10"
          virtualName="MicronSlider" explicitFocusOrder="0" pos="20 4 10 140"
          min="-100" max="100" int="0.10000000000000000555" style="LinearVertical"
          textBoxPos="NoTextBox" textBoxEditable="1" textBoxWidth="80"
          textBoxHeight="20" skewFactor="1"/>
  <SLIDER name="new slider" id="44febf80755cd85c" memberName="slider11"
          virtualName="MicronSlider" explicitFocusOrder="0" pos="36 4 10 140"
          min="-100" max="100" int="0.10000000000000000555" style="LinearVertical"
          textBoxPos="NoTextBox" textBoxEditable="1" textBoxWidth="80"
          textBoxHeight="20" skewFactor="1"/>
  <SLIDER name="new slider" id="7589d7b6435fc8d9" memberName="slider12"
          virtualName="MicronSlider" explicitFocusOrder="0" pos="52 4 10 140"
          min="-100" max="100" int="0.10000000000000000555" style="LinearVertical"
          textBoxPos="NoTextBox" textBoxEditable="1" textBoxWidth="80"
          textBoxHeight="20" skewFactor="1"/>
  <SLIDER name="new slider" id="45889a66d0206446" memberName="slider13"
          virtualName="MicronSlider" explicitFocusOrder="0" pos="68 4 10 140"
          min="-100" max="100" int="0.10000000000000000555" style="LinearVertical"
          textBoxPos="NoTextBox" textBoxEditable="1" textBoxWidth="80"
          textBoxHeight="20" skewFactor="1"/>
  <SLIDER name="new slider" id="35593a3592a633ea" memberName="slider14"
          virtualName="MicronSlider" explicitFocusOrder="0" pos="84 4 10 140"
          min="-100" max="100" int="0.10000000000000000555" style="LinearVertical"
          textBoxPos="NoTextBox" textBoxEditable="1" textBoxWidth="80"
          textBoxHeight="20" skewFactor="1"/>
  <SLIDER name="new slider" id="8376187fa24822a1" memberName="slider15"
          virtualName="MicronSlider" explicitFocusOrder="0" pos="100 4 10 140"
          min="-100" max="100" int="0.10000000000000000555" style="LinearVertical"
          textBoxPos="NoTextBox" textBoxEditable="1" textBoxWidth="80"
          textBoxHeight="20" skewFactor="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
