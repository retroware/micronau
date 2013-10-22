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

#ifndef __JUCE_HEADER_4F1A0AB4C98078C6__
#define __JUCE_HEADER_4F1A0AB4C98078C6__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
class MicronSlider;
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Introjucer.

    SliderBank:
		Component that contains several sliders and allows the user to 
		'paint' over them to set many in a single mouse drag.
                                                                    //[/Comments]
*/
class SliderBank  : public Component,
                    public SliderListener
{
public:
    //==============================================================================
    SliderBank ();
    ~SliderBank();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.

	// enable/disable broad stroke 'painting' of slider values
	void setBroadStrokesMode(bool enableBroadStrokes);

private:
	void mouseWheelMove (const MouseEvent&, const MouseWheelDetails&);
	void mouseDown (const MouseEvent& event);
	void mouseUp (const MouseEvent& event);
    void mouseDrag (const MouseEvent& event);

	void updateMaxBoundX();
	MicronSlider* findSliderAtPos(const Point<int>& pos);
	void resetDoubleClickTimer();

public:
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void sliderValueChanged (Slider* sliderThatWasMoved);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.

	bool broadStrokesEnabled; // mode for broad-stroke 'painting' of slider values

	MicronSlider* lockedSliderBeingDragged; // for locking onto a particular slider for adjustment
	MicronSlider* prevSlider; // for tracking while 'painting' sliders
	bool resettingValuesToDefault; // for painting slider default-resets with double-click and drag
	Time prevMouseDownTime; // for double-click detection (built-in doubleclick doesn't allow us to correctly drag afterwards).
	int maxBoundX; // so we don't keep painting sliders too far off the right edge.

    //[/UserVariables]

    //==============================================================================
    ScopedPointer<MicronSlider> slider9;
    ScopedPointer<MicronSlider> slider10;
    ScopedPointer<MicronSlider> slider11;
    ScopedPointer<MicronSlider> slider12;
    ScopedPointer<MicronSlider> slider13;
    ScopedPointer<MicronSlider> slider14;
    ScopedPointer<MicronSlider> slider15;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderBank)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_4F1A0AB4C98078C6__
