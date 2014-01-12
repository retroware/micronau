/*
 This file is part of micronau.
 Copyright (c) 2013 - David Smitley
 
 Permission is granted to use this software under the terms of the GPL v2 (or any later version)
 
 Details can be found at: www.gnu.org/licenses
*/

#include "micronau.h"
#include "micronauEditor.h"
#include "gui/MicronSlider.h"
#include "gui/MicronToggleButton.h"
#include "gui/MicronTabBar.h"
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
        create_env(i, 455, 310);
    }
    
    create_prefilt(290, 140);
    create_postfilt(713, 140);
    create_filter(410, 135);
    create_mod(0, 55, 25);
    create_fm(0, 377);
    create_xyz(900, 270);
    create_lfo(270, 440);
    
    sync_nrpn = new TextButton("sync nrpn");
    sync_nrpn->addListener(this);
    sync_nrpn->setBounds(910, 120, 30, 20);
    addAndMakeVisible(sync_nrpn);
    
    sync_sysex = new TextButton("sync sysex");
    sync_sysex->addListener(this);
    sync_sysex->setBounds(960, 120, 30, 20);
    addAndMakeVisible(sync_sysex);
    
    mod1_6 = new TextButton("7 - 12");
    mod1_6->addListener(this);
    mod1_6->setBounds(10, 65, 40, 15);
    addAndMakeVisible(mod1_6);
    mod7_12 = new TextButton("1 - 6");
    mod7_12->addListener(this);
    mod7_12->setBounds(10, 65, 40, 15);
    addChildComponent(mod7_12);
    
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
    
    logo = Drawable::createFromImageData (BinaryData::logo_svg, BinaryData::logo_svgSize);

    // This is where our plugin's editor size is set.
    setSize (1060, 670);

    startTimer (50);
}

MicronauAudioProcessorEditor::~MicronauAudioProcessorEditor()
{
}

void MicronauAudioProcessorEditor::add_knob(int nprn, int x, int y, const char *text, Component *parent = NULL) {
    ext_slider *s;
    s = new ext_slider(owner, nprn);
    sliders.add(s);
    s->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);

    if (text) {
        s->setTextBoxStyle(Slider::TextBoxBelow, true, 40, 15);
        s->setLabel(text);
    }

    s->addListener (this);
    s->setBounds(x, y, 40, 40);
    if (parent) {
        parent->addAndMakeVisible(s);
    } else {
        addAndMakeVisible(s);
    }
}

void MicronauAudioProcessorEditor::add_box(int nrpn, int x, int y, int width, const char *text, int loc, Component *parent = NULL){
    ext_combo *c;
    
    c = new ext_combo(owner, nrpn);
    c->setBounds(x, y, width, 15);
    c->addListener(this);
    if (parent) {
        parent->addAndMakeVisible(c);
    } else {
        addAndMakeVisible(c);
    }
    boxes.add(c);
    
    if (text != NULL) {
        Label *l;
        switch (loc) {
            case 0:
                // to the right
                l = new back_label(text, x + width + 4, y, 55, 15);
                l->setJustificationType (Justification::centredLeft);
                break;
            case 1:
                // below
                l = new back_label(text, x, y + 15 + 2, width, 15);
                break;
            case 2:
                l = new back_label(text, x - 3 - 55, y, 55, 15);
                l->setJustificationType (Justification::centredRight);
                break;
        }
        if (parent) {
            parent->addAndMakeVisible(l);
        } else {
            addAndMakeVisible(l);
        }
    }
}

void MicronauAudioProcessorEditor::add_button(int nrpn, int x, int y, const char *text)
{
    ext_button *b = new ext_button(owner, nrpn);
    buttons.add(b);
    b->setBounds(x, y, 20, 20);
    b->addListener (this);
    addAndMakeVisible(b);
    
    if (text) {
        Label *l = new back_label(text, x + 17, y + 3, 55, 15);
        l->setJustificationType (Justification::centredLeft);
        addAndMakeVisible(l);
    }
}

void MicronauAudioProcessorEditor::create_mod(int n, int x, int y)
{
    int y_off = 12;
    for (n = 0; n < 2; n++) {
        Component *c = new Component();
        mods[n] = c;
        for (int i = 0; i < 6; i++) {
            String s = "mod ";
            s += (i + (n*6)) + 1;
            Label *l = new back_label(s, 40 + (i*118), 0, 40, 15);
            c->addAndMakeVisible(l);
            add_knob((i*4)+(n*24)+694, (i*118), 0 + y_off, "level", c);
            add_knob((i*4)+(n*24)+695, (i*118), 40 + y_off, "offset", c);

            add_box((i*4) + (n*24) + 692, 40 + (i*118), 4 + y_off, 75, "source", 1, c);
            add_box((i*4) + (n*24) + 693, 40 + (i*118), 4 + 40 + y_off, 75, "destination", 1, c);
        }
        c->setBounds(x, y, 700, 100);
    }
    addAndMakeVisible(mods[0]);
    addChildComponent(mods[1]);
}

void MicronauAudioProcessorEditor::create_osc(int n)
{
    int x, y, y_base;
    const char *labels[] = {"shape","octave", "semi", "fine", "wheel"};
    Label *l;
    
    x = 15;
    y = 130;
    y_base = y + n * 65;
    
    for (int i = 0; i < 5; i++) {
        add_knob((n*6)+i+524, x + (i*40), y_base + 20, labels[i]);
    }

    add_box((n*6)+523, x + 50, y_base, 55, "waveform", 0);

    String s = "osc";
    s += (n+1);
    l = new back_label(s, x, y_base, 55, 15);
    addAndMakeVisible(l);

//    l = new back_label("waveform", x+105, y_base, 55, 15);
//    addAndMakeVisible(l);

}

void MicronauAudioProcessorEditor::create_prefilt(int x, int y)
{
    const char *labels[] = {"osc1", "osc2", "osc3", "ring", "noise", "ext in"};
    Label *l;
    for (int i = 0; i < 6; i++) {
        add_knob(541 + i, x, y + (i * 40), NULL);
        add_knob(547 + i, x + 50, y + (i * 40), NULL);
        l = new back_label(labels[i], x-40, y + i*40 + 12, 40, 15);
        addAndMakeVisible(l);
    }

    l = new back_label("level", x, y - 15, 40, 15);
    addAndMakeVisible(l);

    l = new back_label("balance", x+40, y - 15, 60, 15);
    addAndMakeVisible(l);
}

void MicronauAudioProcessorEditor::create_postfilt(int x, int y)
{
    const char *labels[] = {"filter1", "filter2", "prefilter"};
    Label *l;
    for (int i = 0; i < 3; i++) {
        add_knob(566 + i, x, y + (i * 40), NULL);
        add_knob(569 + i, x + 40, y + (i * 40), NULL);
        l = new back_label(labels[i], x-40, y + i*40 + 12, 40, 15);
        addAndMakeVisible(l);
    }
    add_box(573, x+100, y+5, 58, "polarity", 1);
    add_box(572, x+100, y+45, 58, "input", 1);
    
    l = new back_label("level", x, y - 15, 40, 15);
    addAndMakeVisible(l);
    
    l = new back_label("balance", x+40, y - 15, 60, 15);
    addAndMakeVisible(l);
}

void MicronauAudioProcessorEditor::create_filter(int x, int y)
{
    for (int i = 0; i < 2; i++) {
        int x_offset = 50;
        
        add_knob((i*6)+556, x + x_offset, y+(i*80), "cutoff");
        add_knob((i*6)+557, x + x_offset + 40, y+(i*80), "res");
        add_knob((i*6)+559, x + x_offset + 80, y+(i*80), "envamt");
        add_knob((i*6)+558, x + x_offset + 120, y+(i*80), "keytrk");

        add_box((i*6)+555, x + 88, y + 42 + (i * 80), 135, NULL, 0);
    }
    add_knob(553, x, y+17, "f1>f2");
    add_knob(670, x, y+62, "offset");

    add_button(560, x + 10, y + 100, NULL);
}

void MicronauAudioProcessorEditor::create_env(int n, int x, int y)
{
    int v_space  = 65;
    const char *labels[] = {"attack", "decay", "sustain", "time", "release"};
    int base_nrpns[] = {578, 580, 583, 582, 584};
    int x_offset = x + 35;
    for (int i = 0; i < 5; i++) {
        add_knob(base_nrpns[i] + (n * 13), x_offset + (i * 40), y + (n * v_space), labels[i]);
    }

    add_box(579 + (n*13), x_offset, y + 40 + (n * v_space), 35, NULL, 0);
    add_box(581 + (n*13), x_offset + 40, y + 40 + (n * v_space), 35, NULL, 0);
    add_box(585 + (n*13), x_offset + 160, y + 40 + (n * v_space), 35, NULL, 0);
    add_button(590 + (n*13), x_offset + 80,  y + 35 + (n * v_space), "pedal");
    
    add_box(588 + (n*13), x + 255 + 57, y + (n * v_space), 28, "free run", 0);
    add_box(587 + (n*13), x + 255 + 33, y + 19 + (n * v_space), 52, "reset", 0);
    add_box(589 + (n*13), x + 255 + 40, y + 38 + (n * v_space), 45, "loop", 0);

    Label *l;
    l = new back_label("amp", x, y+6, 40, 15);
    addAndMakeVisible(l);
    l = new back_label("filter", x, y+6+v_space, 40, 15);
    addAndMakeVisible(l);
    l = new back_label("env3", x, y+6+(v_space * 2), 40, 15);
    addAndMakeVisible(l);
}

void MicronauAudioProcessorEditor::create_fm(int x, int y)
{
    // fm
    add_box(520, x + 20, y + 0, 90, "sync", 1);
    add_box(522, x + 20, y + 35, 90, "algorithm", 1);
    add_box(554, x + 123, y + 20, 45, "noise", 1);
    add_knob(521, x + 180, y + 14, "amount");

    // voice
    y += 107;
    add_box(512, x + 20, y + 0, 60, "poly", 0);
    add_box(513, x + 20, y + 25, 60, "unison", 0);
    add_box(518, x + 20, y + 50, 105, "pitch wheel", 0);
    add_knob(519, x + 145, y, "drift");
    add_knob(514, x + 180, y, "detune");

    // portamento
    y += 120;
    add_box(515, x + 20, y + 0, 60, "mode", 0);
    add_box(516, x + 20, y + 25, 90, "type", 0);
    add_knob(517, x + 180, y, "time");

}

void MicronauAudioProcessorEditor::create_xyz(int x, int y)
{
    Label *l;
    // x, y, z
    add_box(411, x, y + 0, 110, "X", 0);
    add_box(412, x, y + 20, 110, "Y", 0);
    add_box(413, x, y + 40, 110, "Z", 0);
    
    y += 115;
    // output
    add_knob(742, x + 35, y, "fx1/fx2");
    add_knob(577, x + 75, y, "wet/dry");
    add_knob(575, x + 75, y+45, "level");
    add_knob(576, x + 75, y+90, "level");
    add_box(574, x-15, y + 50, 85, "drive", 1);
    l = new back_label("effects", x - 8, y + 7, 50, 15);
    addAndMakeVisible(l);
    l = new back_label("program", x + 28, y + 95, 50, 15);
    addAndMakeVisible(l);
    
    y += 175;
    // tracking
    add_box(630, x, y, 60, "input", 0);
    add_box(631, x, y + 20, 60, "preset", 0);
    add_box(632, x, y + 40, 25, "points", 0);
}

void MicronauAudioProcessorEditor::create_lfo(int x, int y)
{
    const char *labels[] = {"lfo1", "lfo2", "s&h"};
    
    for (int i = 0; i < 3; i++) {
        add_box(671 + i, x, y + (i*70) + 3, 65, labels[i], 2);
        add_knob(618 + (i*4), x+70, y + (i*70), "rate");
        add_knob(620 + (i*4), x+110, y + (i*70), "m1");
        add_box(619 + (i*4), x+78, y + (i*70) + 40, 60, "reset", 2);
        add_button(617 + (i*4), x, y + 23 + (i* 70), "sync");
    }
    add_box(628, x, y+200, 100, "input", 0);
}

//==============================================================================
void MicronauAudioProcessorEditor::paint (Graphics& g)
{
	g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), RectanglePlacement(RectanglePlacement::stretchToFit));
    g.setColour (Colours::black);

    jassert (logo != 0);
    if (logo != 0)
        logo->drawWithin (g, Rectangle<float> (790, 20, 68, 40), RectanglePlacement::stretchToFit, 1.000f);
}

void MicronauAudioProcessorEditor::resized()
{
	param_display->setBounds(885,15,150,45);

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
        return;
    }
    if (button == sync_sysex) {
        owner->sync_via_sysex();
        return;
    }
    // switch to 7-12
    if (button == mod1_6) {
        mods[0]->setVisible(false);
        mods[1]->setVisible(true);
        mod1_6->setVisible(false);
        mod7_12->setVisible(true);
    }
    
    // switch to 1-6
    if (button == mod7_12) {
        mods[0]->setVisible(true);
        mods[1]->setVisible(false);
        mod1_6->setVisible(true);
        mod7_12->setVisible(false);
    }

    ext_button *b = dynamic_cast<ext_button*>( button );
	if (b)
	{
        int v = b->getToggleState();
		b->set_value(v);
		param_display->setText(b->get_name() + "\n" + b->get_txt_value(v), dontSendNotification);
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
		param_display->setText(b->get_name() + "\n" + b->get_txt_value(b->getSelectedItemIndex()), dontSendNotification);
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

