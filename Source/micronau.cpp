/*
 This file is part of micronau.
 Copyright (c) 2013 - David Smitley
 
 Permission is granted to use this software under the terms of the GPL v2 (or any later version)

 Details can be found at: www.gnu.org/licenses
*/

/*
TODO:

    sysex load
    handle incoming nrpn messages
    presest save/restore
    handle adding/deleteing midi ports
 */

#include "micronau.h"
#include "micronauEditor.h"

//==============================================================================
MicronauAudioProcessor::MicronauAudioProcessor()
{
    params = new IonSysexParams();
    
    int idx = 0;
    for (int i = 0; i < params->numParams(); i++) {
        IonSysexParam *param = params->getParam(i);
        if (param->hasNrpn()) {
            nrpns.add(param);
            param_by_nrpn.set(param->getNrpn(), new ext_param(param, idx));
            idx++;
        }

    }

    midi_out = NULL;
    midi_out_port = "None";
    set_midi_port(MIDI_OUT_IDX, midi_out_port);
    set_midi_chan(0);
    
    midi_in = NULL;
    midi_in_port = "None";
    set_midi_port(MIDI_IN_IDX, midi_in_port);

    set_prog_name("micronAU");
}

MicronauAudioProcessor::~MicronauAudioProcessor()
{
    if (midi_in != NULL) {
        midi_in->stop();
    }
}

//==============================================================================
const String MicronauAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int MicronauAudioProcessor::getNumParameters()
{
    return nrpns.size();
}

float MicronauAudioProcessor::getParameter (int index)
{
    return nrpns[index]->getValue();
}

void MicronauAudioProcessor::setParameter (int index, float newValue)
{
    IonSysexParam *param;
    param = nrpns[index];
    int nrpn_num;
    int value = (int) newValue;
   
    // XXX - handle the tracking matrix
    
    param->setValue(value);

    // handle custom list values
    if (param->getList().size()) {
        ListItemParameter lip =  param->getList()[(int) value];
        if (lip.hasSpecialNrpnValue()) {
            value = lip.getNrpnValue();
        }
    }

    if (param->isFxSelector()) {
        send_nrpn(param->fxSelectorToNrpn()-512, (int) newValue);

        // we changed the fx type so update all the corresponding nrpns
        int fxMinNrpn = param->fxMin();
        int fxMaxNrpn = param->fxMax();
        int fxnrpn;
        for(fxnrpn = fxMinNrpn; fxnrpn < (fxMinNrpn+fxMaxNrpn); fxnrpn++) {
            if (param_by_nrpn.contains(fxnrpn)) {
                IonSysexParam *fxparam = param_by_nrpn[fxnrpn]->param;
                nrpn_num = params->fx1fx2NrpnNum(fxparam)-512;
                int fxvalue = fxparam->getNrpnValue();
                send_nrpn(nrpn_num, fxvalue);
            }
        }
        
        return;
    }

    // "normal" parameter
    nrpn_num = params->fx1fx2NrpnNum(param);
    if (nrpn_num == NO_NRPN) {
        return;
    }
    if (nrpn_num >= 512) {
        nrpn_num -= 512;
    }
    send_nrpn(nrpn_num, (int) value);
    
    // update gui
    // send nrpn
}

const String MicronauAudioProcessor::getParameterName (int index)
{
    return nrpns[index]->getName();
}

const String MicronauAudioProcessor::getParameterText (int index)
{
    return nrpns[index]->getTextValue();
}

float MicronauAudioProcessor::getParameterMinValue (int index)
{
    return nrpns[index]->getMin();
}

float MicronauAudioProcessor::getParameterMaxValue (int index)
{
    return nrpns[index]->getMax();
;
}

float MicronauAudioProcessor::getParameterDefaultValue (int index)
{
    return nrpns[index]->getDefaultValue();
}

int MicronauAudioProcessor::getParameterNumSteps (int index)
{
    return nrpns[index]->getMax() - nrpns[index]->getMin();
}

const String MicronauAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String MicronauAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool MicronauAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool MicronauAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool MicronauAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MicronauAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MicronauAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double MicronauAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MicronauAudioProcessor::getNumPrograms()
{
    return 0;
}

int MicronauAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MicronauAudioProcessor::setCurrentProgram (int index)
{
}

const String MicronauAudioProcessor::getProgramName (int index)
{
    return String::empty;
}

void MicronauAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void MicronauAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void MicronauAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void MicronauAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // update parameters with incoming midi msgs
    
    
    // silence all output channels
    for (int i = 0; i < getNumOutputChannels(); ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }
}

//==============================================================================
bool MicronauAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* MicronauAudioProcessor::createEditor()
{
    return new MicronauAudioProcessorEditor (this);
}

//==============================================================================
int MicronauAudioProcessor::index_of_nrpn(int nrpn)
{
    return param_by_nrpn[nrpn]->index;
}

IonSysexParam *MicronauAudioProcessor::param_of_nrpn(int nrpn)
{
    return param_by_nrpn[nrpn]->param;
}


void MicronauAudioProcessor::sync_via_nrpn()
{
    if (midi_out == NULL) {
        return;
    }

    send_bank_patch();
    
    int l = nrpns.size();
    for (unsigned int i = 0; i < l; i++) {
        IonSysexParam *param = nrpns[i];
        int nrpn_num = params->fx1fx2NrpnNum(param);
        if (params->shouldSkipFx1(param) || params->shouldSkipFx2(param)) {
            continue;
        }
        if (nrpn_num >= 2048) {
            continue;
        }
        /* XXX
        if (nrpn_num < 300) {
            if (nrpn_num == 100) {
                sendMIDI(0xb0, 0, 0);
                sendMIDI(0xb0, 32, get_bank_num());
                continue;
            }
            if (nrpn_num == 101) {
                sendMIDI(0xc0, get_patch_num(), 0);
                continue;
            }
            continue;
        }
        */
        if (nrpn_num >= 512) {
            nrpn_num -= 512;
        }
        send_nrpn(nrpn_num, param->getNrpnValue());
    }
    return;
}

void MicronauAudioProcessor::send_bank_patch()
{
    if (midi_out == NULL) {
        return;
    }
    
    unsigned char cmd = 0xb0 + get_midi_chan();
    int bank, prog;
    bank = param_of_nrpn(100)->getValue();
    prog = param_of_nrpn(101)->getValue();
    
    MidiMessage *msg;
    if (bank > 0) {
        bank = bank - 1;

        // bank msb
        msg = new MidiMessage(cmd, 0, 0);
        midi_out->sendMessageNow(*msg);
        delete msg;
        
        // bank lsb
        msg = new MidiMessage(cmd, 32, bank);
        midi_out->sendMessageNow(*msg);
        delete msg;
    }

    if (prog > 0) {
        prog = prog - 1;
        cmd = 0xc0 + get_midi_chan();
        msg = new MidiMessage(cmd, prog);
        midi_out->sendMessageNow(*msg);
        delete msg;
    }
}

void MicronauAudioProcessor::send_nrpn(int nrpn, int value)
{
    unsigned char midiChannel = 176 + get_midi_chan();
    unsigned char nrpnMSB;
    unsigned char nrpnLSB;
    unsigned char dataMSB;
    unsigned char dataLSB;
    MidiMessage *msg;
    
    if (midi_out == NULL) {
        return;
    }
    
    if ((nrpn >= 100) && (nrpn <= 101)) {
        send_bank_patch();
        return;
    }
    
    nrpnMSB = (nrpn >> 7) & (0x7f);
    nrpnLSB = (nrpn & 0xff) & (0x7f);
    dataMSB = (value >> 7) & (0x7f);
    dataLSB = (value & 0xff) & (0x7f);
    
    msg = new MidiMessage(midiChannel, 0x63, nrpnMSB);
    midi_out->sendMessageNow(*msg);
    delete msg;
    
    msg = new MidiMessage(midiChannel, 0x62, nrpnLSB);
    midi_out->sendMessageNow(*msg);
    delete msg;
    
    msg = new MidiMessage(midiChannel, 0x6, dataMSB);
    midi_out->sendMessageNow(*msg);
    delete msg;

    msg = new MidiMessage(midiChannel, 0x26, dataLSB);
    midi_out->sendMessageNow(*msg);
    delete msg;
}

void MicronauAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    preset *p = (preset *) data;
    String s;

	if (!params->parseParamsFromContent(p->sysex, SYSEX_LEN)) {
		return;
	}

    set_midi_chan(p->midi_out_chan);
    param_of_nrpn(100)->setValue(p->bank);
    param_of_nrpn(101)->setValue(p->patch);
   
    s = String(CharPointer_UTF8((const char *) p->midi_in_port));
    set_midi_port(MIDI_IN_IDX, s);
    s = String(CharPointer_UTF8((const char *) p->midi_out_port));
    set_midi_port(MIDI_OUT_IDX, s);
    
/*
    if ((p->bank != 0) && (p->patch != 0)) {
        sync_via_nrpn();
    }
*/
}

void MicronauAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    unsigned char sysex_buf[SYSEX_LEN + 2];
    preset p;
    String s;
    
    memset(sysex_buf, 0, SYSEX_LEN + 2);
	params->getAsSysexMessage(sysex_buf);
    memcpy(p.sysex, sysex_buf+1, SYSEX_LEN);

    p.midi_out_chan = get_midi_chan();
    p.bank = param_of_nrpn(100)->getValue();
    p.patch = param_of_nrpn(101)->getValue();

    s = get_midi_port(MIDI_IN_IDX);
    s.copyToUTF8(((CharPointer_UTF8::CharType *) p.midi_in_port), MAX_MIDI_PORT_NAME);
    s = get_midi_port(MIDI_OUT_IDX);
    s.copyToUTF8(((CharPointer_UTF8::CharType *) p.midi_out_port), MAX_MIDI_PORT_NAME);

    destData.append(&p, sizeof(p));
}

void MicronauAudioProcessor::sync_via_sysex()
{
    unsigned char sysex_buf[SYSEX_LEN + 2];

    if (midi_out == NULL) {
        return;
    }
    
    memset(sysex_buf, 0, SYSEX_LEN + 2);
	params->getAsSysexMessage(sysex_buf);

    MidiMessage sysexe_msg(sysex_buf, sizeof(sysex_buf));
    midi_out->sendMessageNow(sysexe_msg);
}

void MicronauAudioProcessor::init_from_sysex(unsigned char *sysex)
{
	int i;
	if (!params->parseParamsFromContent(sysex, SYSEX_LEN)) {
		return;
	}

	for (i = 0; i < nrpns.size(); i++) {
        IonSysexParam *param = nrpns[i];
		param = params->getParam(i);
        sendParamChangeMessageToListeners(i, param->getValue());
	}
}
    
void MicronauAudioProcessor::handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message)
{
    if (source != midi_in) {
        return;
    }
    if (message.isSysEx()) {
        const uint8 *data = message.getSysExData();
        init_from_sysex((unsigned char *) data);
    }
}

void MicronauAudioProcessor::set_midi_port(int in_out, String p)
{
    int idx;
    switch (in_out) {
        case MIDI_OUT_IDX:
            if (p != midi_out_port) {
                if (midi_out != NULL) {
                    delete midi_out;
                }
                midi_out_port = p;
                idx = midi_find_port_by_name(in_out, midi_out_port);
                if (idx == -1) {
                    midi_out = NULL;
                } else {
                    midi_out = MidiOutput::openDevice(idx);
                }
            }
            break;
        case MIDI_IN_IDX:
            if (p != midi_in_port) {
                if (midi_in != NULL) {
                    midi_in->stop();
                    delete midi_in;
                }
                midi_in_port = p;
                idx = midi_find_port_by_name(in_out, midi_in_port);
                if (idx == -1) {
                    midi_in = NULL;
                } else {
                    midi_in = MidiInput::openDevice(idx, this);
                    midi_in->start();
                }
            }
            break;
    }
    return;
}

void MicronauAudioProcessor::set_midi_chan(unsigned int chan)
{
    midi_out_channel = chan;
}

unsigned int MicronauAudioProcessor::get_midi_chan()
{
    return midi_out_channel;
}

String MicronauAudioProcessor::get_midi_port(int in_out)
{
    switch (in_out) {
        case MIDI_OUT_IDX:
            return midi_out_port;
            break;
        case MIDI_IN_IDX:
            return midi_in_port;
            break;
    }
    return "None";
}

String MicronauAudioProcessor::get_prog_name()
{
    return params->get_prog_name();
}

int MicronauAudioProcessor::midi_find_port_by_name(int in_out, String nm)
{
    StringArray devices;
    int i;
    switch (in_out) {
        case MIDI_OUT_IDX:
            devices = MidiOutput::getDevices();
            break;
        case MIDI_IN_IDX:
            devices = MidiInput::getDevices();
            break;
    }
    for (i = 0; i < devices.size(); i++) {
        if (devices[i] == nm) {
            return i;
        }
    }
    return -1;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MicronauAudioProcessor();
}
