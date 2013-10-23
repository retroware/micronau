/*
  ==============================================================================

    LcdLabel.h
    Created: 25 Sep 2013 10:14:20pm
    Author:  Jules

  ==============================================================================
*/

#ifndef LCDLABEL_H_INCLUDED
#define LCDLABEL_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
	LcdLabel:
		Like a Label except with customized appearance.
*/
class LcdLabel    : public Label
{
public:
	explicit LcdLabel(const String& componentName = String::empty,
						const String& labelText = String::empty);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LcdLabel)
};


#endif  // LCDLABEL_H_INCLUDED
