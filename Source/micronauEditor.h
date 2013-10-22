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


class ext_slider : public Slider
{
public:
    ext_slider(MicronauAudioProcessor *owner, Slider *s, int nrpn_num) : s(s),  plugin(owner) {
        param = owner->param_of_nrpn(nrpn_num);
        idx = owner->index_of_nrpn(nrpn_num);
    }
    void set_value(int v){plugin->setParameterNotifyingHost(idx, v);}
    int get_value(){ return param->getValue();}
    int get_min() { return param->getMin();}
    int get_max() { return param->getMax();}
    const String get_name () { return param->getName();}
    const String get_txt_value (int v) { return param->getConvertedValue(v);}
    
private:
    Slider *s;
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
    void sliderValueChanged (Slider*);
    void buttonClicked (Button* button);
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    
private:
    ScopedPointer<TextButton> sync_nrpn;
    ScopedPointer<TextButton> sync_sysex;
    ScopedPointer<ComboBox> midi_in_menu;
    ScopedPointer<ComboBox> midi_out_menu;

    ScopedPointer<Label> param_name;
    ScopedPointer<Label> param_value;

    MicronauAudioProcessor *owner;

    // prototype
    ext_slider *sliders[5];
};


#endif  // __PLUGINEDITOR_H_74E5CE11__
