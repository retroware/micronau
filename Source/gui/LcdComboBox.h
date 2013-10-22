/*
  ==============================================================================

    LcdComboBox.h
    Created: 20 Sep 2013 12:33:24am
    Author:  Jules

  ==============================================================================
*/

#ifndef LCDCOMBOBOX_H_INCLUDED
#define LCDCOMBOBOX_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

class PluginLookAndFeel;

//==============================================================================
/*
	LcdComboBox:
		Like a ComboBox but with customized backlit LCD appearance and disabled key-focus.
*/
class LcdComboBox    : public ComboBox
{
public:
    explicit LcdComboBox(const String& componentName = String::empty);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LcdComboBox)
};


#endif  // LCDCOMBOBOX_H_INCLUDED
