/*
  ==============================================================================

    SliderBank.h
    Created: 28 Jan 2013 9:19:00pm
    Author:  Jules
	
  ==============================================================================
*/

#ifndef __JUCE_HEADER_4F1A0AB4C98078C6__
#define __JUCE_HEADER_4F1A0AB4C98078C6__

#include "JuceHeader.h"
#include "micronauEditor.h"
class MicronSlider;
class MicronauAudioProcessorEditor;
class ext_slider;

//==============================================================================
/*

    SliderBank:
		Component that contains several sliders and allows the user to
		'paint' over them to set many in a single mouse drag.

*/

class SliderBank  : public Component, public SliderListener
{
public:
    //==============================================================================
    SliderBank (MicronauAudioProcessor *owner, MicronauAudioProcessorEditor *parent);
    ~SliderBank();

    //==============================================================================

	// enable/disable broad stroke 'painting' of slider values
	void setBroadStrokesMode(bool enableBroadStrokes);

    void paint (Graphics& g);
    void resized();
    void sliderValueChanged (Slider* sliderThatWasMoved);

    void hide_12_16(bool is_12);
    
private:
	void mouseWheelMove (const MouseEvent&, const MouseWheelDetails&);
	void mouseDown (const MouseEvent& event);
	void mouseUp (const MouseEvent& event);
    void mouseDrag (const MouseEvent& event);

	void updateMaxBoundX();
	MicronSlider* findSliderAtPos(const Point<int>& pos);
	void resetDoubleClickTimer();

	bool broadStrokesEnabled; // mode for broad-stroke 'painting' of slider values

	MicronSlider* lockedSliderBeingDragged; // for locking onto a particular slider for adjustment
	MicronSlider* prevSlider; // for tracking while 'painting' sliders
	bool resettingValuesToDefault; // for painting slider default-resets with double-click and drag
	Time prevMouseDownTime; // for double-click detection (built-in doubleclick doesn't allow us to correctly drag afterwards).
	int maxBoundX; // so we don't keep painting sliders too far off the right edge.

    ext_slider *sliders[33];
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderBank)
};

#endif   // __JUCE_HEADER_4F1A0AB4C98078C6__
