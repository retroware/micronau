/*
  ==============================================================================

    MicronToggleButton.cpp
    Created: 25 Sep 2013 7:02:00pm
    Author:  Jules

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "MicronToggleButton.h"

//==============================================================================
MicronToggleButton::MicronToggleButton(const String& buttonText) : ToggleButton(buttonText)
{
    setWantsKeyboardFocus(false);
}
