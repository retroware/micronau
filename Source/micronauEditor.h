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
class LcdLabel;
class StdComboBox;

class ext_slider : public MicronSlider
{
public:
    ext_slider(MicronauAudioProcessor *owner, int nrpn_num) : plugin(owner) {
        param = owner->param_of_nrpn(nrpn_num);
        idx = owner->index_of_nrpn(nrpn_num);
    }
    void set_value(int v){plugin->setParameterNotifyingHost(idx, v);}
    int get_value(){ return param->getValue();}
    int get_min() { return param->getMin();}
    int get_max() { return param->getMax();}
    const String get_name () { return param->getName();}
    const String get_txt_value (int v) { return param->getConvertedValue(v);}

	// this setRange override adjusts mouse drag sensitivity so that smaller ranges are more sensitive than larger ranges.
	void setRange (double newMin, double newMax, double newInt) { MicronSlider::setRange (newMin, newMax, newInt); setMouseDragSensitivity( 20.0*(4.0+log10(newMax - newMin)) ); }

private:
    IonSysexParam *param;
    MicronauAudioProcessor *plugin;
    int idx;
};

//==============================================================================
/**
*/
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
    void update_midi_menu(int in_out);
    void select_item_by_name(int in_out, String nm);

	Image background;

    ScopedPointer<TextButton> sync_nrpn;
    ScopedPointer<TextButton> sync_sysex;
    ScopedPointer<StdComboBox> midi_in_menu;
    ScopedPointer<StdComboBox> midi_out_menu;

    ScopedPointer<LcdLabel> param_display;

    MicronauAudioProcessor *owner;

    // prototype
    ext_slider *sliders[5];
};


#endif  // __PLUGINEDITOR_H_74E5CE11__
