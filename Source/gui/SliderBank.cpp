/*
  ==============================================================================

    SliderBank.cpp
    Created: 28 Jan 2013 9:19:00pm
    Author:  Jules

  ==============================================================================
*/

#include "MicronSlider.h"
#include "SliderBank.h"
#include "micronauEditor.h"

//==============================================================================
SliderBank::SliderBank (MicronauAudioProcessor *owner, MicronauAudioProcessorEditor *parent)
{
	for (int i = 0; i < 33; ++i)
	{
		const int SLIDER_SPACING = 11;
		const int SLIDER_WIDTH = 11;
		const int SLIDER_HEIGHT = 100;

		ext_slider *newSlider = new ext_slider(owner, 633+i);
        parent->addSlider(newSlider);
        newSlider->addListener(parent);
		newSlider->setBounds (i*SLIDER_SPACING, 0, SLIDER_WIDTH, SLIDER_HEIGHT);
		newSlider->setRange (-100, 100, 0.1);
		newSlider->setSliderStyle (Slider::LinearVertical);
		newSlider->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
		newSlider->addListener (this);

        sliders[i] = newSlider;
		addAndMakeVisible ( newSlider );
	}

    setSize (400, 300);

	lockedSliderBeingDragged = 0;
	prevSlider = 0;
	resettingValuesToDefault = false;
	setBroadStrokesMode(true);
	resetDoubleClickTimer();

	// disable any clicks from getting to children.
	setInterceptsMouseClicks (true, false);
	for (int i = getNumChildComponents(); --i >= 0;)
		getChildComponent(i)->setInterceptsMouseClicks(false, false);

	updateBoundsX();
}

SliderBank::~SliderBank()
{
}

//==============================================================================
void SliderBank::paint (Graphics& g)
{
}

void SliderBank::resized()
{
	updateBoundsX();
}

void SliderBank::sliderValueChanged (Slider* sliderThatWasMoved)
{
}

void SliderBank::updateBoundsX()
{
       minBoundX = 1000000;
       maxBoundX = 0;
       for (int i = getNumChildComponents(); --i >= 0;)
       {
               MicronSlider* slider = dynamic_cast<MicronSlider*>(getChildComponent(i));
               if (slider && slider->isVisible())
               {
                       const int sliderLeftEdge = slider->getBounds().getX() - 5;
                       const int sliderRightEdge = slider->getBounds().getRight() + 5;
                       if (sliderLeftEdge < minBoundX)
                               minBoundX = sliderLeftEdge;
                       if (sliderRightEdge > maxBoundX)
                               maxBoundX = sliderRightEdge;
               }
       }
}

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
		if (slider && slider->isVisible())
		{
			const int SLIGHT_X_ADJUSTMENT = 2;
			float sliderDist = abs( slider->getBounds().getCentre().getX()+SLIGHT_X_ADJUSTMENT - pos.getX() );
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
		if ( event.getPosition().getX() >= minBoundX && event.getPosition().getX() < maxBoundX )
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

void SliderBank::hide_12_16(bool is_12)
{
    int i;
    for (i = 0; i < 33; i++) {
        sliders[i]->setVisible(true);
    }
    if (is_12) {
        for (i = 0; i < 4; i++) {
            sliders[i]->setVisible(false);
            sliders[32 - i]->setVisible(false);
        }
    }

	updateBoundsX();
}
