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
#include "gui/MicronTabBar.h"
#include "gui/SliderBank.h"

class LcdLabel;
class StdComboBox;
class SliderBank;
class LcdTextEditor;

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
        if (list_items.size() != 0) {
            vector<ListItemParameter>::const_iterator i;
            for (int i = 0; i != list_items.size(); i++) {
                addItem(list_items[i].getName(), i+1);
            }
        } else {
            addItem("--", 1000);
            for (int i = param->getMin(); i < param->getMax(); i++) {
                addItem(String(i+1), i+1);
            }
        }
        nrpn = nrpn_num;
    }
    void set_value(int v){plugin->setParameterNotifyingHost(idx, v);}
    int get_value(){ return param->getValue();}
    int get_min() { return param->getMin();}
    int get_max() { return param->getMax();}
    int get_nrpn() {return nrpn;}
    const String get_name () { return param->getName();}
    const String get_txt_value (int v) { return param->getConvertedValue(v);}
    vector<ListItemParameter> & get_list_item_names() {return param->getList();}
    
private:
    IonSysexParam *param;
    MicronauAudioProcessor *plugin;
    int idx;
    int nrpn;
};

class ext_button : public MicronToggleButton
{
public:
    ext_button(MicronauAudioProcessor *owner, int nrpn_num, LookAndFeel *lf) : MicronToggleButton(""), plugin(owner) {
        param = owner->param_of_nrpn(nrpn_num);
        idx = owner->index_of_nrpn(nrpn_num);
        if (lf) {
            setLookAndFeel(lf);
        }
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
										public AudioProcessorListener,
                                        public SliderListener,
                                        public ButtonListener,
                                        public ComboBoxListener,
                                        public TextEditorListener,
                                        public Timer
{
public:
    MicronauAudioProcessorEditor (MicronauAudioProcessor* ownerFilter);
    ~MicronauAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics& g);
    void timerCallback();
	void updateGuiComponents();
    void sliderValueChanged (Slider* slider);
	void sliderDragStarted (Slider* slider);
	void mouseDown(const MouseEvent& event);
    KeyboardFocusTraverser* createFocusTraverser();
    void buttonClicked (Button* button);
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    void textEditorTextChanged (TextEditor &t);
    void addSlider(ext_slider *s) {sliders.add(s);}

    void audioProcessorParameterChanged (AudioProcessor* processor, int parameterIndex, float newValue) { paramHasChanged = true; }
	void audioProcessorChanged (AudioProcessor* processor) { paramHasChanged = true; }

private:

	// layout of all the major component groups
	enum
	{
		// group box margins
		GROUP_BOX_MARGIN_X = 10,
		GROUP_BOX_MARGIN_Y = 20,

		// top and left edges
		LEFT_X = 15,
		TOP_Y = 20,

		// component groups...
		
		MODMAT_X = LEFT_X,
		MODMAT_Y = TOP_Y,
		MODMAT_W = 755,
		MODMAT_H = 100,

		OSCS_X = MODMAT_X,
		OSCS_Y = 145,
		OSCS_W = 210,
		OSCS_H = 200,

		ENVS_X = 460,
		ENVS_Y = 315,
		ENVS_W = 400,
		ENVS_H = 200,

		PREFILT_X = 255,
		PREFILT_Y = OSCS_Y,
		PREFILT_W = 140,
		PREFILT_H = 250,

		FILT_X = 420,
		FILT_Y = PREFILT_Y,
		FILT_W = 230,
		FILT_H = 145,

		POSTFILT_X = 680,
		POSTFILT_Y = FILT_Y,
		POSTFILT_W = 200,
		POSTFILT_H = FILT_H,

		LFO_X = PREFILT_X,
		LFO_Y = 440,
		LFO_W = 185,
		LFO_H = 225,
		
		FM_X = OSCS_X,
		FM_Y = 377,
		FM_W = OSCS_W,
		FM_H = 76,

		VOICE_X = FM_X,
		VOICE_Y = FM_Y + 107,
		VOICE_W = FM_W,
		VOICE_H = 77,

		PORTA_X = VOICE_X,
		PORTA_Y = VOICE_Y + 107,
		PORTA_W = VOICE_W,
		PORTA_H = 55,
		
		XYZ_X = 900,
		XYZ_Y = 270,
		XYZ_W = 150,
		XYZ_H = 70,

		OUTPUT_X = XYZ_X,
		OUTPUT_Y = XYZ_Y + 115,
		OUTPUT_W = XYZ_W,
		OUTPUT_H = 135,

		TRACKING_X = 930,
		TRACKING_Y = ENVS_Y + 245,
		TRACKING_W = 120,
		TRACKING_H = 70,
        
        FX_X = ENVS_X,
        FX_Y = TRACKING_Y,
        FX_W = 440,
        FX_H = 105
	};

    void add_knob(int nrpn, int x, int y, const char *text, Component *parent);
    void add_box(int nprn, int x, int y, int width, const char *text, int loc, Component *parent);
    void add_button(int nrpn, int x, int y, const char *text, bool invert, Component *parent);
    void update_tracking();
    
	void create_group_box(const char* labelText, int x, int y, int w, int h);
    void create_osc(int x, int y);
    void create_prefilt(int x, int y);
    void create_postfilt(int x, int y);
    void create_mod(int x, int y);
    void create_filter(int x, int y);
    void create_env(int x, int y);
    void create_fm(int x, int y);
    void create_voice(int x, int y);
    void create_portamento(int x, int y);
    void create_xyz(int x, int y);
    void create_output(int x, int y);
    void create_tracking(int x, int y);
    void create_lfo(int x, int y);
	void create_fx_and_tracking_tabs(int x, int y);
    void create_fx1(int x, int y, Component* parent);
    void create_fx2(int x, int y, Component* parent);


    void update_midi_menu(int in_out, bool init);
    void select_item_by_name(int in_out, String nm);

	Image background;
    ScopedPointer<Drawable> logo;
    ScopedPointer<LookAndFeel> inverted_button_lf;
    OwnedArray<GroupComponent> group_boxes;

    ScopedPointer<TextButton> sync_nrpn;
    ScopedPointer<TextButton> sync_sysex;
    ScopedPointer<TextButton> mod1_6;
    ScopedPointer<TextButton> mod7_12;
    ScopedPointer<LcdComboBox> midi_in_menu;
    ScopedPointer<LcdComboBox> midi_out_menu;
    ScopedPointer<LcdComboBox> midi_out_chan;

    ScopedPointer<LcdLabel> param_display;
    ScopedPointer<LcdTextEditor> prog_name;

    MicronauAudioProcessor *owner;
	bool paramHasChanged; // using this flag to avoid repeatedly updating program name which interferes with editing of the name

    ScopedPointer<Component> mods[2];

	ScopedPointer<MicronTabBar> fx_and_tracking_tabs;
    ScopedPointer<Component> fx1[7];
    ScopedPointer<Component> fx2[7];
	SliderBank* trackgen;

    // prototype
    OwnedArray<ext_slider> sliders;
    OwnedArray<ext_combo> boxes;
    OwnedArray<ext_button> buttons;
};


#endif  // __PLUGINEDITOR_H_74E5CE11__
