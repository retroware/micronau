/*
  ==============================================================================

    StdComboBox.cpp
    Created: 23 Sep 2013 5:02:29pm
    Author:  Jules

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "StdComboBox.h"

//==============================================================================
StdComboBox::StdComboBox(const String& componentName) : ComboBox(componentName)
{
    setWantsKeyboardFocus(false);
}

