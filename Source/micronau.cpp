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
    midi_out_idx = -1;
    set_midi_index(MIDI_OUT_IDX, MidiOutput::getDefaultDeviceIndex());
    out_channel = 0;
    
    midi_in = NULL;
    midi_in_idx = -1;
    set_midi_index(MIDI_IN_IDX, MidiInput::getDefaultDeviceIndex());
}

MicronauAudioProcessor::~MicronauAudioProcessor()
{
    midi_in->stop();
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

float MicronauAudioProcessor::getParameterMin (int index)
{
    return nrpns[index]->getMin();
}

float MicronauAudioProcessor::getParameterMax (int index)
{
    return nrpns[index]->getMax();
;
}

float MicronauAudioProcessor::getParameterDefault (int index)
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
void MicronauAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

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

void MicronauAudioProcessor::send_nrpn(int nrpn, int value)
{
    unsigned char midiChannel = 176 + out_channel;
    unsigned char nrpnMSB;
    unsigned char nrpnLSB;
    unsigned char dataMSB;
    unsigned char dataLSB;
    MidiMessage *msg;
    
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
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void MicronauAudioProcessor::sync_via_sysex()
{
    unsigned char sysex_buf[434];

    memset(sysex_buf, 0, 434);
	params->getAsSysexMessage(sysex_buf);

    MidiMessage sysexe_msg(sysex_buf, sizeof(sysex_buf));
    midi_out->sendMessageNow(sysexe_msg);
}

void MicronauAudioProcessor::init_from_sysex(unsigned char *sysex)
{
	int i;
	if (params->parseParamsFromContent(sysex, 432)) {
		return;
	}

	for (i = 0; i < params->numParams(); i++) {
        IonSysexParam *param;
		param = params->getParam(i);
		if ((param->getNrpn() != -1)  && (param->getNrpn() > 150)) {
//			SetParameter(param->getNrpn(), kAudioUnitScope_Global, 0, param->getValue(), 0);
		}
	}
}
    
void MicronauAudioProcessor::handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message)
{
    if (source != midi_in) {
        return;
    }
    if (message.isSysEx()) {
        int x = message.getSysExDataSize();
        const uint8 *data = message.getSysExData();
        init_from_sysex((unsigned char *) data);
    }
}

void MicronauAudioProcessor::set_midi_index(int in_out, int idx)
{
    switch (in_out) {
        case MIDI_OUT_IDX:
            if (idx != midi_out_idx) {
                if (midi_out != NULL) {
                    delete midi_out;
                }
                midi_out = MidiOutput::openDevice(idx);
                midi_out_idx = idx;
            }
            break;
        case MIDI_IN_IDX:
            if (idx != midi_in_idx) {
                if (midi_in != NULL) {
                    midi_in->stop();
                    delete midi_in;
                }
                midi_in = MidiInput::openDevice(idx, this);
                midi_in_idx = idx;
                midi_in->start();
            }
            break;
    }
    return;
}

int MicronauAudioProcessor::get_midi_index(int in_out)
{
    switch (in_out) {
        case MIDI_OUT_IDX:
            return midi_out_idx;
            break;
        case MIDI_IN_IDX:
            return midi_in_idx;
            break;
    }
    return -1;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MicronauAudioProcessor();
}
