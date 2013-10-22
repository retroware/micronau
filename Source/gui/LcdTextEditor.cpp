/*
  ==============================================================================

    LcdTextEditor.cpp
    Created: 26 Sep 2013 12:40:07am
    Author:  Jules

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "LcdTextEditor.h"

//==============================================================================
LcdTextEditor::LcdTextEditor(const String& componentName, juce_wchar passwordCharacter) : TextEditor(componentName, passwordCharacter)
{
	// TODO: find out the full charset allowed in micron program names.
	const String& allowedCharacters = String::empty;
    setInputRestrictions(14, allowedCharacters);
    setOpaque (false);
	addListener(this);
}

void LcdTextEditor::textEditorReturnKeyPressed (TextEditor& editor)
{
	Component::unfocusAllComponents();
}

void LcdTextEditor::textEditorEscapeKeyPressed (TextEditor& editor)
{
	Component::unfocusAllComponents();
}
