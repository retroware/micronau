/*
  ==============================================================================

    LcdTextEditor.h
    Created: 26 Sep 2013 12:40:07am
    Author:  Jules

  ==============================================================================
*/

#ifndef LCDTEXTEDITOR_H_INCLUDED
#define LCDTEXTEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
	LcdTextEditor:
		Like a TextEditor except with customized appearance and 
		behavior for the micronAU plugin.
*/
class LcdTextEditor    : public TextEditor, public TextEditorListener
{
public:
    explicit LcdTextEditor(const String& componentName = String::empty,
                         juce_wchar passwordCharacter = 0);

	// we will listen to ourself so that we can lose focus on return or escape.
	void textEditorReturnKeyPressed (TextEditor&);
	void textEditorEscapeKeyPressed (TextEditor&);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LcdTextEditor)
};


#endif  // LCDTEXTEDITOR_H_INCLUDED
