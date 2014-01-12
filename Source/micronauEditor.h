/*
 This file is part of micronau.
 Copyright (c) 2013 - David Smitley
 
 Permission is granted to use this software under the terms of the GPL v2 (or any later version)
 
 Details can be found at: www.gnu.org/licenses
*/

#ifndef __PLUGINEDITOR_H_74E5CE11__
#define __PLUGINEDITOR_H_74E5CE11__

#include "../JuceLibraryCode/JuceHeader.h"
#include "micronau.h"
#include "gui/MicronSlider.h"
#include "gui/LcdComboBox.h"
#include "gui/MicronToggleButton.h"

class LcdLabel;
class StdComboBox;

class ext_slider : public MicronSlider
{
public:
    ext_slider(MicronauAudioProcessor *owner, int nrpn_num) : plugin(owner) {
        param = owner->param_of_nrpn(nrpn_num);
        idx = owner->index_of_nrpn(nrpn_num);
        setRange (param->getMin(), param->getMax(), 1);
    }
    void set_value(int v){plugin->setParameterNotifyingHost(idx, v);}
    int get_value(){ return param->getValue();}
    const String get_name () { return param->getName();}
    const String get_txt_value (int v) { return param->getConvertedValue(v);}
	// this setRange override adjusts mouse drag sensitivity so that smaller ranges are more sensitive than larger ranges.
	void setRange (double newMin, double newMax, double newInt) { MicronSlider::setRange (newMin, newMax, newInt); setMouseDragSensitivity( 20.0*(4.0+log10(newMax - newMin)) ); }

private:
    IonSysexParam *param;
    MicronauAudioProcessor *plugin;
    int idx;
};

class ext_combo : public LcdComboBox
{
public:
    ext_combo(MicronauAudioProcessor *owner, int nrpn_num) : plugin(owner) {
        param = owner->param_of_nrpn(nrpn_num);
        idx = owner->index_of_nrpn(nrpn_num);
 
        vector<ListItemParameter> list_items = param->getList();
        vector<ListItemParameter>::const_iterator i;
        for (int i = 0; i != list_items.size(); i++) {
            addItem(list_items[i].getName(), i+1);
        }
        
    }
    void set_value(int v){plugin->setParameterNotifyingHost(idx, v);}
    int get_value(){ return param->getValue();}
    int get_min() { return param->getMin();}
    int get_max() { return param->getMax();}
    const String get_name () { return param->getName();}
    const String get_txt_value (int v) { return param->getConvertedValue(v);}
    vector<ListItemParameter> & get_list_item_names() {return param->getList();}
    
private:
    IonSysexParam *param;
    MicronauAudioProcessor *plugin;
    int idx;
};

class ext_button : public MicronToggleButton
{
public:
    ext_button(MicronauAudioProcessor *owner, int nrpn_num) : MicronToggleButton(""), plugin(owner) {
        param = owner->param_of_nrpn(nrpn_num);
        idx = owner->index_of_nrpn(nrpn_num);
    }
    void set_value(int v){plugin->setParameterNotifyingHost(idx, v);}
    int get_value(){ return param->getValue();}
    const String get_name () { return param->getName();}
    const String get_txt_value (int v) { return param->getConvertedValue(v);}
    
private:
    IonSysexParam *param;
    MicronauAudioProcessor *plugin;
    int idx;
};

class back_label : public Label
{
public:
    back_label(String s, int x, int y, int w, int h) : Label() {
        setText(s, dontSendNotification);
        setFont (Font ("Arial", 12.00f, Font::bold));
        setBounds(x, y, w, h);
        setJustificationType(Justification::centred);
    }
};

//==============================================================================
class MicronauAudioProcessorEditor  : public AudioProcessorEditor,
                                        public SliderListener,
                                        public ButtonListener,
                                        public ComboBoxListener,
                                        public Timer
{
public:
    MicronauAudioProcessorEditor (MicronauAudioProcessor* ownerFilter);
    ~MicronauAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics& g);
    void resized();
    void timerCallback();
    void sliderValueChanged (Slider* slider);
	void sliderDragStarted (Slider* slider);
    void buttonClicked (Button* button);
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    
private:
    void add_knob(int nrpn, int x, int y, const char *text, Component *parent);
    void add_box(int nprn, int x, int y, int width, const char *text, int loc, Component *parent);
    void add_button(int nrpn, int x, int y, const char *text);
    
    void create_osc(int n);
    void create_prefilt(int x, int y);
    void create_postfilt(int x, int y);
    void create_mod(int n, int x, int y);
    void create_filter(int x, int y);
    void create_env(int n, int x, int y);
    void create_fm(int x, int y);
    void create_xyz(int x, int y);
    void create_lfo(int x, int y);

    void update_midi_menu(int in_out);
    void select_item_by_name(int in_out, String nm);

	Image background;

    ScopedPointer<TextButton> sync_nrpn;
    ScopedPointer<TextButton> sync_sysex;
    ScopedPointer<TextButton> mod1_6;
    ScopedPointer<TextButton> mod7_12;
    ScopedPointer<StdComboBox> midi_in_menu;
    ScopedPointer<StdComboBox> midi_out_menu;

    ScopedPointer<LcdLabel> param_display;

    MicronauAudioProcessor *owner;
    ScopedPointer<Component> mods[2];

    // prototype
    Array<ext_slider *> sliders;
    Array<ext_combo *> boxes;
    Array<ext_button *> buttons;
};


#endif  // __PLUGINEDITOR_H_74E5CE11__
