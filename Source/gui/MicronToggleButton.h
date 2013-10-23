/*
  ==============================================================================

    MicronToggleButton.h
    Created: 25 Sep 2013 7:02:00pm
    Author:  Jules

  ==============================================================================
*/

#ifndef MICRONTOGGLEBUTTON_H_INCLUDED
#define MICRONTOGGLEBUTTON_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
	MicronToggleButton:
		Like a ToggleButton except with customized appearance and disabled key-focus.
*/
class MicronToggleButton    : public ToggleButton
{
public:
    explicit MicronToggleButton (const String& buttonText);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicronToggleButton)
};


#endif  // MICRONTOGGLEBUTTON_H_INCLUDED
