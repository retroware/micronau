/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "Fx2Panel.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
#include "LcdComboBox.h"
//[/MiscUserDefs]

//==============================================================================
Fx2Panel::Fx2Panel ()
{
    addAndMakeVisible (comboBox = new LcdComboBox ("new combo box"));
    comboBox->setEditableText (false);
    comboBox->setJustificationType (Justification::centred);
    comboBox->setTextWhenNothingSelected (String::empty);
    comboBox->setTextWhenNoChoicesAvailable ("(no choices)");
    comboBox->addItem ("bypass", 1);
    comboBox->addItem ("mono delay", 2);
    comboBox->addItem ("stereo delay", 3);
    comboBox->addItem ("etc.", 4);
    comboBox->addListener (this);

    addAndMakeVisible (label = new Label ("new label",
                                          "type"));
    label->setFont (Font (15.00f, Font::plain));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (label2 = new Label ("new label",
                                           "fx2/delay"));
    label2->setFont (Font (22.00f, Font::plain));
    label2->setJustificationType (Justification::centredLeft);
    label2->setEditable (false, false, false);
    label2->setColour (TextEditor::textColourId, Colours::black);
    label2->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

Fx2Panel::~Fx2Panel()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    comboBox = nullptr;
    label = nullptr;
    label2 = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void Fx2Panel::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void Fx2Panel::resized()
{
    comboBox->setBounds (44, 12, 80, 16);
    label->setBounds (8, 8, 39, 24);
    label2->setBounds (8, 48, 100, 20);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void Fx2Panel::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == comboBox)
    {
        //[UserComboBoxCode_comboBox] -- add your combo box handling code here..
        //[/UserComboBoxCode_comboBox]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="Fx2Panel" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="4" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ffffff"/>
  <COMBOBOX name="new combo box" id="c55ad942730330b3" memberName="comboBox"
            virtualName="LcdComboBox" explicitFocusOrder="0" pos="44 12 80 16"
            editable="0" layout="36" items="bypass&#10;mono delay&#10;stereo delay&#10;etc."
            textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <LABEL name="new label" id="f02d1c5ad67c9689" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="8 8 39 24" edTextCol="ff000000" edBkgCol="0"
         labelText="type" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <LABEL name="new label" id="f0adbe0a7b83a794" memberName="label2" virtualName=""
         explicitFocusOrder="0" pos="8 48 100 20" edTextCol="ff000000"
         edBkgCol="0" labelText="fx2/delay" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="22"
         bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
