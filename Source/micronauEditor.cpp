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
    for (int i = 0; i < 3; i++) {
        create_osc(i);
    }
    
    create_prefilt(290, 140);
    
    create_mod(0, 55, 25);
    
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
    midi_in_menu->addListener(this);
    addAndMakeVisible (midi_in_menu);

    midi_out_menu = new StdComboBox ();
    midi_out_menu->setEditableText (false);
    midi_out_menu->addListener(this);
    addAndMakeVisible (midi_out_menu);
    
    // This is where our plugin's editor size is set.
    setSize (1060, 670);

    //
    startTimer (50);
}

MicronauAudioProcessorEditor::~MicronauAudioProcessorEditor()
{
}

void MicronauAudioProcessorEditor::create_mod(int n, int x, int y)
{
    for (int i = 0; i < 6; i++) {
        ext_slider *s;

        s = new ext_slider(owner, (i*4)+(n*24)+694);
        sliders.add(s);
        s->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle(Slider::TextBoxBelow, true, 40, 15);
        s->setLabel("level");
        s->addListener (this);
        s->setBounds(x + (i*118), y, 40, 40);
        addAndMakeVisible(s);

        s = new ext_slider(owner, (i*4)+(n*24)+695);
        sliders.add(s);
        s->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle(Slider::TextBoxBelow, true, 40, 15);
        s->setLabel("offset");
        s->addListener (this);
        s->setBounds(x + (i*118), y+40, 40, 40);
        addAndMakeVisible(s);
        
        ext_combo *c;
        Label *l;
        
        c = new ext_combo(owner, (i*4) + (n*6) + 692);
        c->setBounds(x + 40 + (i*118), y+4, 75, 15);
        c->addListener(this);
        addAndMakeVisible(c);
        boxes.add(c);
        l = new back_label("source", x + 40 + (i*118), y + 4 + 15, 75, 15);
        addAndMakeVisible(l);

        c = new ext_combo(owner, (i*4) + (n*6) + 693);
        c->setBounds(x + 40 + (i*118), y + 4 + 40, 75, 15);
        c->addListener(this);
        addAndMakeVisible(c);
        boxes.add(c);
        l = new back_label("destination", x + 40 + (i*118), y + 4 + 15 + 40, 75, 15);
        addAndMakeVisible(l);
    }
}

void MicronauAudioProcessorEditor::create_osc(int n)
{
    int x, y, y_base;
    Label *l;
    
    x = 15;
    y = 130;
    y_base = y + n * 65;
    
    for (int i = 0; i < 5; i++) {
        ext_slider *s;
        
        s = new ext_slider(owner, (n*6)+i+524);
        sliders.add(s);
        s->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle(Slider::TextBoxBelow, true, 40, 15);
        s->addListener (this);
 
        switch (i) {
            case 0:
                s->setLabel("shape");
                break;
            case 1:
                s->setLabel("octave");
                break;
            case 2:
                s->setLabel("semi");
                break;
            case 3:
                s->setLabel("fine");
                break;
            case 4:
                s->setLabel("wheel");
                break;
        }

        s->setBounds(x + (i*40), y_base + 20, 40, 40);
        addAndMakeVisible(s);
    }

    String s = "osc";
    s += (n+1);
    l = new back_label(s, x, y_base, 55, 15);
    addAndMakeVisible(l);

    l = new back_label("waveform", x+105, y_base, 55, 15);
    addAndMakeVisible(l);

    ext_combo *c = new ext_combo(owner, (n*6)+523);
    c->setBounds(x + 50, y_base, 55, 15);
    c->addListener(this);
    boxes.add(c);
    
    addAndMakeVisible(c);
}

void MicronauAudioProcessorEditor::create_prefilt(int x, int y)
{
    Label *l;
    for (int i = 0; i < 6; i++) {
        ext_slider *s;
        String sl;
        
        s = new ext_slider(owner, 541 + i);
        sliders.add(s);
        s->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
        s->addListener (this);
        s->setBounds(x, y + i*40, 40, 40);
        addAndMakeVisible(s);

        s = new ext_slider(owner, 547 + i);
        sliders.add(s);
        s->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
        s->addListener (this);        
        s->setBounds(x + 50, y + i*40, 40, 40);
        addAndMakeVisible(s);
        
        switch (i) {
            case 0:
                sl = String("osc1");
                break;
            case 1:
                sl = String("osc2");
                break;
            case 2:
                sl = String("osc2");
                break;
            case 3:
                sl = String("ring");
                break;
            case 4:
                sl = String("noise");
                break;
            case 5:
                sl = String("ext in");
                break;
        };
        l = new back_label(sl, x-40, y + i*40 + 12, 40, 15);
        addAndMakeVisible(l);
    }

    l = new back_label("level", x, y - 15, 40, 15);
    addAndMakeVisible(l);

    l = new back_label("balance", x+40, y - 15, 60, 15);
    addAndMakeVisible(l);
}

//==============================================================================
void MicronauAudioProcessorEditor::paint (Graphics& g)
{
	g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), RectanglePlacement(RectanglePlacement::stretchToFit));
    g.setColour (Colours::black);
}

void MicronauAudioProcessorEditor::resized()
{
	param_display->setBounds(885,15,150,45);

    sync_nrpn->setBounds(910, 120, 30, 20);
    sync_sysex->setBounds(960, 120, 30, 20);

    midi_in_menu->setBounds(910, 70, 100, 20);
    midi_out_menu->setBounds(910, 95, 100, 20);
}

void MicronauAudioProcessorEditor::timerCallback()
{
    // update gui if parameters have changed
    for (int i = 0; i < sliders.size(); i++) {
        sliders[i]->setValue(sliders[i]->get_value(), dontSendNotification);
    }

    for (int i = 0; i < boxes.size(); i++) {
        boxes[i]->setSelectedItemIndex(boxes[i]->get_value(), dontSendNotification);
    }
    
    update_midi_menu(MIDI_IN_IDX);
    update_midi_menu(MIDI_OUT_IDX);
}

void MicronauAudioProcessorEditor::update_midi_menu(int in_out)
{
    ComboBox *menu;
    StringArray x;
    switch (in_out) {
        case MIDI_IN_IDX:
            x = MidiInput::getDevices();
            menu = midi_in_menu;
            break;
        case MIDI_OUT_IDX:
            x = MidiOutput::getDevices();
            menu = midi_out_menu;
            break;
        default:
            return;
    }

    bool midi_changed = false;
    if (x.size() + 1 != menu->getNumItems()) {
        midi_changed = true;
    } else {
        for (int i = 0; i < x.size(); i++) {
            if (x[i] != menu->getItemText(i+1)) {
                midi_changed = true;
                break;
            }
        }
    }
    if (midi_changed) {
        int idx = menu->getSelectedItemIndex();
        String current_midi;
        if (idx == -1) {
            current_midi = "None";
        } else {
            current_midi = menu->getItemText(idx);
        }
        menu->clear();
        menu->addItem("None", 1000);
        for (int i = 0; i < x.size(); i++) {
            menu->addItem(x[i], i+1);
        }
        select_item_by_name(in_out, current_midi);
    } else {
        select_item_by_name(in_out, owner->get_midi_port(in_out));
    }
}

void MicronauAudioProcessorEditor::sliderValueChanged (Slider *slider)
{
    ext_slider *s = dynamic_cast<ext_slider*>( slider );
	if (s)
	{
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
    int idx = box->getSelectedItemIndex();
    if (box == midi_out_menu) {
        owner->set_midi_port(MIDI_OUT_IDX, box->getItemText(idx));
    }
    if (box == midi_in_menu) {
        owner->set_midi_port(MIDI_IN_IDX, box->getItemText(idx));
    }
    ext_combo *b = dynamic_cast<ext_combo *>( box );
	if (b)
	{
		b->set_value(b->getSelectedItemIndex());
	}
}

void MicronauAudioProcessorEditor::select_item_by_name(int in_out, String nm)
{
    int i;
    ComboBox *menu;
    switch (in_out) {
        case MIDI_IN_IDX:
            menu = midi_in_menu;
            break;
        case MIDI_OUT_IDX:
            menu = midi_out_menu;
            break;
        default:
            return;
    }
    for (i = 0; i < menu->getNumItems(); i++) {
        if (menu->getItemText(i) == nm) {
            menu->setSelectedItemIndex(i);
            return;
        }
    }
    menu->setSelectedItemIndex(0);
}

