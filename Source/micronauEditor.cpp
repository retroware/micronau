/*
 This file is part of micronau.
 Copyright (c) 2013 - David Smitley
 
 Permission is granted to use this software under the terms of the GPL v2 (or any later version)
 
 Details can be found at: www.gnu.org/licenses
*/

#include "micronau.h"
#include "micronauEditor.h"
#include "gui/MicronSlider.h"
#include "gui/LcdLabel.h"
#include "gui/StdComboBox.h"
#include "gui/LookAndFeel.h"


//==============================================================================
MicronauAudioProcessorEditor::MicronauAudioProcessorEditor (MicronauAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter)
{
	LookAndFeel::setDefaultLookAndFeel( PluginLookAndFeel::getInstance() );

	background = ImageCache::getFromMemory (BinaryData::background_png,
                                     BinaryData::background_pngSize);

    owner = ownerFilter;
    
    // create all of the gui elements and hook them up to the processor
    for (int i = 0; i < 5; i++) {
        ext_slider *s;
        float min, max;
        
        sliders[i] = s = new ext_slider(owner, i+523);
        s->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        s->addListener (this);
        min = s->get_min();
        max = s->get_max();
        s->setRange (min, max, 1);
        addAndMakeVisible(s);
    }

    sync_nrpn = new TextButton("sync nrpn");
    sync_nrpn->addListener(this);
    addAndMakeVisible(sync_nrpn);
    
    sync_sysex = new TextButton("sync sysex");
    sync_sysex->addListener(this);
    addAndMakeVisible(sync_sysex);
    
	param_display = new LcdLabel("panel", "micronAU\nretroware");
    param_display->setJustificationType (Justification::centredLeft);
    param_display->setEditable (false, false, false);
    param_display->setColour (TextEditor::textColourId, Colours::black);
    param_display->setColour (TextEditor::backgroundColourId, Colour (0x00000000));
    param_display->setFont (Font (18.00f, Font::plain));
	addAndMakeVisible(param_display);

    midi_in_menu = new StdComboBox ();
    midi_in_menu->setEditableText (false);
    StringArray x = MidiInput::getDevices();
    for (int i = 0; i < x.size(); i++) {
        midi_in_menu->addItem(x[i], i+1);
    }
    midi_in_menu->setSelectedItemIndex(owner->get_midi_index(MIDI_IN_IDX));
    midi_in_menu->addListener(this);
    addAndMakeVisible (midi_in_menu);

    midi_out_menu = new StdComboBox ();
    midi_out_menu->setEditableText (false);
    x = MidiOutput::getDevices();
    for (int i = 0; i < x.size(); i++) {
        midi_out_menu->addItem(x[i], i+1);
    }
    midi_out_menu->setSelectedItemIndex(owner->get_midi_index(MIDI_OUT_IDX));
    midi_out_menu->addListener(this);
    addAndMakeVisible (midi_out_menu);
    
    // This is where our plugin's editor size is set.
    setSize (1000, 300);

    //
    startTimer (50);
}

MicronauAudioProcessorEditor::~MicronauAudioProcessorEditor()
{
}

//==============================================================================
void MicronauAudioProcessorEditor::paint (Graphics& g)
{
	g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), RectanglePlacement(RectanglePlacement::stretchToFit));
    g.setColour (Colours::black);
}

void MicronauAudioProcessorEditor::resized()
{
    for (int i = 0; i < 5; i++) {
        Slider *s = sliders[i];
        s->setBounds(20+i*180, 60, 150, 40);
    }

	param_display->setBounds(360,110,150,45);

    sync_nrpn->setBounds(20, 120, 30, 20);
    sync_sysex->setBounds(70, 120, 30, 20);

    midi_in_menu->setBounds(120, 120, 100, 20);
    midi_out_menu->setBounds(240, 120, 100, 20);
}

void MicronauAudioProcessorEditor::timerCallback()
{
    // update gui if parameters have changed
    for (int i = 0; i < 5; i++) {
        sliders[i]->setValue(sliders[i]->get_value(), dontSendNotification);
    }

    // update midi menus
}

void MicronauAudioProcessorEditor::sliderValueChanged (Slider *slider)
{
    ext_slider *s = dynamic_cast<ext_slider*>( slider );
	if (s)
	{
		// NOTE: must use the down-casted pointer for getValue/setValue, as they aren't virtual in JUCE's Slider base class.
		s->set_value(s->getValue());
		param_display->setText(s->get_name() + "\n" + s->get_txt_value(s->getValue()), dontSendNotification);
	}
}

void MicronauAudioProcessorEditor::sliderDragStarted (Slider* slider)
{	// when user just touches a slider, update its value so it may be seen in the parameter display box.
	sliderValueChanged(slider);
}


void MicronauAudioProcessorEditor::buttonClicked (Button* button)
{
    if (button == sync_nrpn) {
        owner->sync_via_nrpn();
    }
    if (button == sync_sysex) {
        owner->sync_via_sysex();
    }
}

void MicronauAudioProcessorEditor::comboBoxChanged (ComboBox* box)
{
    if (box == midi_out_menu) {
        owner->set_midi_index(MIDI_OUT_IDX, box->getSelectedItemIndex());
    }
    if (box == midi_in_menu) {
        owner->set_midi_index(MIDI_IN_IDX, box->getSelectedItemIndex());
    }
}

