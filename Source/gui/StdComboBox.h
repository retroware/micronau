/*
  ==============================================================================

    StdComboBox.h
    Created: 23 Sep 2013 5:02:29pm
    Author:  Jules

  ==============================================================================
*/

#ifndef STDCOMBOBOX_H_INCLUDED
#define STDCOMBOBOX_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
	StdComboBox:
		Like a ComboBox but with customized appearance and disabled key-focus.
*/
class StdComboBox    : public ComboBox
{
public:
    explicit StdComboBox(const String& componentName = String::empty);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StdComboBox)
};


#endif  // STDCOMBOBOX_H_INCLUDED
