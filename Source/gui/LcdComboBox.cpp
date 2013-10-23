/*
  ==============================================================================

    LcdComboBox.cpp
    Created: 20 Sep 2013 12:33:24am
    Author:  Jules

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "LcdComboBox.h"
#include "LookAndFeel.h"

//==============================================================================
LcdComboBox::LcdComboBox(const String& componentName) : ComboBox(componentName)
{
    setWantsKeyboardFocus(false);
}
