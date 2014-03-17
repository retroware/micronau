/*
 
 This file originally derived from Micronizer, A real-time program editor for the Alesis Micron
 Copyright (C) 2006 Hector Urtubia
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 
*/

#include "IonSysex.h"
#include <CoreFoundation/CoreFoundation.h>
#include <math.h>
#include "../JuceLibraryCode/JuceHeader.h"
#include "tinyxml.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include <map>
#include "mapping.h"

#define SYSEX_PROGRAM_SIZE 434

static UInt32 checksum(unsigned char *buff, UInt32 len);

void logDebug(const char *s)
{
//	fprintf(stdout, "%s\n", s);
}

static bool encodeToMidi(vector<unsigned char> &raw, vector <unsigned char> &encoded)
{
    unsigned char *rawBuf = new unsigned char[raw.size()];
    int encodedBufSize = raw.size() + (raw.size() / 7);
    unsigned char *encodedBuf = new unsigned char[encodedBufSize];
    unsigned char *rawPtr, *encodedPtr;
    unsigned int i = 0, j = 0;

    rawPtr = rawBuf;
    encodedPtr = encodedBuf;

    for(i = 0; i < raw.size(); i++) rawBuf[i] = raw[i];
	for (i = 0; i < encodedBufSize; i++) {
		encodedBuf[i] = 0;
	}
    for(i = 0; (i * 7) <  raw.size(); i++){
        unsigned char *highbits = encodedPtr;
        encodedPtr++;
        for(j = 0; j < 7; j++){
            *highbits = *highbits | ((*rawPtr >> 7) & 0x01);
            *encodedPtr = *rawPtr & 0x7f;
            rawPtr++;
            encodedPtr++;
            if(j != 6) *highbits = *highbits << 1;
        }
    }

    encoded.clear();
    for(int i = 0; i < encodedBufSize; i++){
        encoded.push_back(encodedBuf[i]);
    }

    delete encodedBuf;
    delete rawBuf;
    return true;
}

// gets rid of the byte with the set of most significant bits for the next 7 bytes.
// SHRINK
static bool decodeFromMidi(vector<unsigned char> &encoded, vector<unsigned char> &raw)
{
    if (encoded.size() % 8) {
        return false;
    }
    unsigned char *encodedBuf = new unsigned char[encoded.size()];
    int rawBufSize = encoded.size() - (encoded.size() / 8);
    unsigned char *rawBuf = new unsigned char[rawBufSize];

    unsigned char *encodedPtr = encodedBuf;
    unsigned char *rawPtr = rawBuf;

    for(unsigned int i = 0; i < encoded.size(); i++) encodedBuf[i] = encoded[i];

    // read chunks of 8, output chunks of 7
    for(unsigned int i = 0; i * 8 < encoded.size(); i++){
        unsigned char highbits = *encodedPtr;
        encodedPtr++;
        for(int j = 0; j < 7; j++){
            *rawPtr = *encodedPtr | ((highbits << (j + 1)) & 0x80);
            encodedPtr++;
            rawPtr++;
        }
    }

    raw.clear();
    for(int i = 0; i < rawBufSize; i++) raw.push_back(rawBuf[i]);

    delete encodedBuf;
    delete rawBuf;
    return true;
}


static vector<string> init_string_vector(const char *strorig)
{
   char *ptr1, *ptr2, *ptr3;
   char *buf = NULL;
   int curBufLen = 0;
   vector<string>ret;
   bool done = false;
   char *str = strdup(strorig);
   ptr1 = ptr2 = str;
   while(!done){
      // get rid of trailing space
      if(*ptr2 == ' ') while(*ptr2 == ' ') ptr2++;
      // anchor the first pointer
      ptr1 = ptr2;
      // find the next comma or the end of the string
      while(*ptr2 != ',' && *ptr2 != '\0')
         ptr2++;
      if(*ptr2 == '\0')
         done = true;
      // take out trailing space
      ptr3 = ptr2;
      while(*(ptr3 - 1) == ' ')
         ptr3--;
      // check the length of the string
      // and reallocate buffer if necessary
      if((ptr3 - ptr1) > curBufLen){
         curBufLen = (ptr3 - ptr1) + 1;
         if(buf != NULL) delete buf;
         buf = new char[curBufLen];
      }
      *ptr3 = '\0';
      // put the string on the vector
      ret.push_back(ptr1);
      if(!done)
         ptr2++;
   }
   if(buf != NULL) delete buf;
   free(str);
   return ret;
}

static void hexdump(char *ptr, int n)
{
   for (int i = 0; i < n; i++){
      unsigned int a = ptr[i];
      a = a & 0x000000FF;
      printf("%02x ", a);
   }
   cout << endl;
}

void tomSecSec(char *buf, float v)
{
	if (v < 1000) {
		sprintf(buf, "%.2f msec\n", v);
	} else {
		sprintf(buf, "%.2f sec\n", v/1000);
	}
}

void hzToString(char *buf, float v)
{
	if (v < 1000) {
		sprintf(buf, "%.3f hz\n", v);
	} else {
		sprintf(buf, "%.3f khz\n", v/1000);
	}
}

void intToLeBuff(unsigned int v, unsigned char *buff)
{
	for (int i = 0; i < 4; i++) {
		buff[i] = v & 0xff;
		v >>= 8;
	}
}

///// ListItemParameter

ListItemParameter::ListItemParameter(const char *name) :
m_name(name),
m_enabled(true),
m_hasSpecialNrpnValue(false),
m_nrpnValue(0)
{}

const char * ListItemParameter::getName()
{
    return m_name;
}

bool ListItemParameter::isDisabled()
{
    return !m_enabled;
}

void ListItemParameter::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool ListItemParameter::hasSpecialNrpnValue()
{
    return m_hasSpecialNrpnValue;
}

void ListItemParameter::setSpecialNrpnValue(int nrpn)
{
    m_nrpnValue = nrpn;
    m_hasSpecialNrpnValue = true;
}

int ListItemParameter::getNrpnValue()
{
    return m_nrpnValue;
}

///// IonSysexParam

IonSysexParam::IonSysexParam(const char *name)
{
   m_name = name;
   m_conv = NONE;
   m_nrpn = -1;
   m_offset = -1;
   m_valueSet = false;
   m_cntrlOffset = 0;
   m_paramName = NULL;
   m_defaultValue = 0;
   m_textValue = String("");

   init_mapping();   
 }

String IonSysexParam::getTextValue()
{
    return m_textValue;
}

bool IonSysexParam::setTextValue(const char *str)
{
	char buff[15];
	strncpy(buff, str, 14);
	buff[14] = 0;
    m_textValue = String(buff);
    return true;
}

String IonSysexParam::getParamName()
{
	return String(m_paramName);
}

void IonSysexParam::setParamName(const char *str)
{
    m_paramName = str;
}

String IonSysexParam::getConvertedValue(SInt32 val)
{
    char buf[128];
	val = m_value + m_cntrlOffset;
	if (getList().size() != 0) {
		return String(getList()[val].getName());
	}
	else {
		switch(m_conv){
			case BANK:
				if (val == 0) {
                    sprintf(buf,"None");
                } else {
                    sprintf(buf,"%d", (int)(val - 1));
                }
				break;
			case PERCENT:
				sprintf(buf,"%d%%",(int)val);
				break;
			case WET_DRY:
				sprintf(buf, "%d%% dry, %d%% wet", (int)(50-val), (int)(50+val));
				break;
			case POST_BAL:
				if (val == 0) {
					strcpy(buf, "Center");
					break;
				}
				if (val < 0) {
					sprintf(buf, "%d%% left", (int)-val);
				} else {
					sprintf(buf, "%d%% right", (int)val);
				}
				break;
			case PRE_BAL:
				if (val < 0) {
					sprintf(buf, "%d%% f1, %d%% f2", (int)(-val+50), (int)(50+val));
				} else {
					sprintf(buf, "%d%% f1, %d%% f2", (int)(50-val), (int)(val+50));
				}
				break;
			case TENTHS_OF_PERCENT:
				sprintf(buf,"%.1f%%",((val)/10.0));
				break;
			case PITCH_FINE:
				sprintf(buf,"%.1f",((val)/10.0));
				break;
			case BALANCE:
				sprintf(buf,"%d",(int)(50 - val));
				break;
			case FX1_FX2_BALANCE:
				
				sprintf(buf,"%d%% fx1 %d%% fx2",(int)(50 - val),(int)(100-(50-val)));
				break;
			case FILTER_OFFSET_FREQ:
				hzToString(buf, (float) (20 * pow((float) 1000, (float) val/1023)));
				break;
			case FILTER_OFFSET_OCT:
				sprintf(buf,"%.2f Oct",val/100.0);
				break;
			case FILTER_FREQ:
				if(val == 920) strcpy(buf,"10.000 KHz");
				else{
					float hz = 20 * pow((float) 1000, (float) val/1023);
					hzToString(buf, hz);
				}
				break;
			case ENV_TIME:
				if (val == 256) {
					strcpy(buf, "Hold");
				} else {
					tomSecSec(buf,  0.5 * pow((float) (30000/0.5), (float) val/255));
				}
				break;
			case PORTA_TIME:
				tomSecSec(buf,  10 * pow((float) 1000, (float) val/127));
				break;
			case LFO_FREQ:
				hzToString(buf, (float) 0.01 * pow((float) (1000/.01), (float) val/1023));
				break;
			case FX_LFO_FREQ:
				hzToString(buf, (float) 0.01 * pow((float) (4.8/.01), (float) val/127));
				break;
			case MS:
				sprintf(buf,"%d ms",(int) val);
				break;
			case OCTAVE:
				if ((val == 1) || (val == -1)) {
					sprintf(buf,"%d octave",(int) val);
				} else {
					sprintf(buf,"%d octaves",(int) val);
				}	
				break;

			case SEMITONE:
				if ((val == 1) || (val == -1)) {
					sprintf(buf,"%d semitone",(int) val);
				} else {
					sprintf(buf,"%d semitones",(int) val);
				}	
				break;
			case RELEASE_TIME:
				tomSecSec(buf, (float) 2 * pow((float) 30000/2, (float) val/255));
				break;
			case EXT_IN:
				if (val == 0) {
					strcpy(buf, "L to f1, R to f2");
					break;
				}
				if (val == -100) {
					strcpy(buf, "L and R to f1");
					break;
				}
				if (val == 100) {
					strcpy(buf, "L and R to f2");
					break;
				}
				if (val < 0) {
					sprintf(buf, "L and %d%% R to f1", (int)-val);
				} else {
					sprintf(buf, "R and %d%% L to f2", (int)val);
				}
				break;
					
			default:
				sprintf(buf,"%d", (int)val);
				break;
	   }
	}
	return String(buf);
}

int IonSysexParam::getValue()
{
    if(m_valueSet)  {
		return m_value;
	}
    m_value = m_min;
    m_valueSet = true;
    return m_value;
}

int IonSysexParam::getBitWidth()
{
   int l_min = m_min;
   if ((m_nrpn >= FX1_FIRST_NRPN) && (m_nrpn < FX1_LAST_NRPN)) {
	 return 8;
   }
   if ((m_nrpn >= FX2_FIRST_NRPN) && (m_nrpn < FX2_LAST_NRPN)) {
	 return 16;
   }
   switch(m_offset) {
   case 282:
	 return 2;
   case 152:
	 return 1;
   case 278:
	 return 1;
   }
   if (m_offset == 282) {
	  return 2;
   }
   if (m_offset == 2240) {
      l_min = -100;
   }
   int biggest = m_max + 1 > l_min ? m_max + 1 : - l_min;
   int bits = int(log((float)biggest)/log(2.0) + 0.9999);
   if(l_min < 0) bits++;
   return bits;
}

int getValueOfByte(char b){
   int res = 0;
   for(int i = 0; i < 8; i++){
      res += (int)(((b >> i) & 0x01) * (1 << i));
   }
   return res;
}


bool IonSysexParam::setNameFromContent(unsigned char *content)
{
   char name[15];
   memset(name, 0, 15);
   memcpy(name,&content[m_offset/8],15);
   name[14] = 0;
   m_textValue = String(name);
   m_valueSet = true;
   return true;
}

void IonSysexParam::setValue(int value)
{
    m_value = value;
    m_valueSet = true;
    return;
}

void IonSysexParam::setNrpn(int nrpn)
{
    m_nrpn = nrpn;
}

bool IonSysexParam::writeNameToBuffer(unsigned char *buffer)
{
	char str[16];
	bzero(str, sizeof(str));
	m_textValue.copyToUTF8(str, sizeof(str));
//	const char *str = CFStringGetCStringPtr(m_textValue, 0);
	bzero(&buffer[m_offset/8], 15);
	memcpy((char *) &buffer[m_offset/8], str, 14);
	bzero(&buffer[296], 15);
	memcpy(&buffer[296], str, 14);
	return true;
}

#define getBit(x) ((content[x/8] >> (x % 8)) & 1)

static void setBit(unsigned char *buffer, int offset, short val)
{
	unsigned char b, mask;
	int delta = offset % 8;
	b = buffer[offset/8];
	mask = ~(1 << delta);
	b = (b & mask) | ((val & 1) << delta);
	buffer[offset/8] = b;
}

bool IonSysexParam::writeValueToBuffer(unsigned char *buffer)
{
	int i;
    if(m_conv == NAME){
        return true;
    }
	if (m_offset < 0) {
		return true;
	}
	/*
		if (id >= 844) {
			// this will check whether the id is it currently selected fx and if so return true
			if (!shouldWrite(id)) {
				return true;
			}
		}
	*/
    int bits = getBitWidth();
    int byte_offset = m_offset / 8;
    unsigned char tmp;
//    printf("writing %s. Bits: %d, Byte Offset: %d value %d\n", m_name, bits, byte_offset, m_value);

    short s_value = (short) m_value;
	s_value += getCntrlOffset();
    if(s_value < m_min)
        s_value = m_min;
    if(s_value > m_max)
        s_value = m_max;
	
	if (m_offset == 282) {
		// fm algorithm lin/exp
		setBit(buffer, 295, ((s_value >= 3) ? 1 : 0));
		if (s_value >=3) {
			s_value -= 3;
		}
	}

	if (m_offset == 122) {
		// unison 
		if (s_value > 0) {
			// unison on
			setBit(buffer, 121, 0);
//			s_value -= 1;
		} 	else {
			// unison off
			setBit(buffer, 121, 1);
		}
	}
	
 	if (m_offset == 152) {
		// portamento 
		if (s_value > 0) {
			// portamento on
			setBit(buffer, 135, 0);
			s_value -= 1;
		} 	else {
			setBit(buffer, 135, 1);
		}
	}

   // osc sync param
    if (m_offset == 278) {
		if (s_value > 0) {
			s_value--;
			int sync_type = (s_value >> 1) & 1;
			sync_type = (sync_type == 0) ? 1 : 0;
			int sync_route = s_value & 1;
			setBit(buffer, 280, sync_type);
			setBit(buffer, 279, sync_route);
			s_value = 0;
		} else {
			s_value = 1;
		}
    }

    // fx2, sync param
   if (m_offset == 1112) {
	  s_value += 12;
   }
	// fx wet/dry mix
   if (m_offset == 2240) {
//	 s_value = (s_value*2)-100;
	 s_value = (s_value*2);
	 bits = 8;
   }

  // mod src 1336, 1344, 1352, ....
  for (i = 0; i < 12; i++) {
	  if (m_offset == (1336+(i*8))) {
		s_value = mod_src_n_to_s[s_value];
	  }
	  // remap mod destinations 1432, 1440, ....
	  if (m_offset == (1432+(i*8))) {
		s_value = mod_dst_n_to_s[s_value];
	  }
  }
  // filter offset 608, 616
  for (i = 0; i < 2; i++) {
	  if (m_offset == (608+(i*8))) {
		s_value = filter_n_to_s[s_value];
	  }
  }
  // s&h source 1168
  if (m_offset == 1168) {
	s_value = sh_n_to_s[s_value];
  }
  // tracking source 1912
  if (m_offset == 1912) {
	s_value = tracking_n_to_s[s_value];
  }
  if (m_offset == 122) {
		if (s_value > 0) {
			setBit(buffer, 121, 0);
			s_value--;
		} else {
			setBit(buffer, 121, 1);
		}
	}

   if(bits >= 8 || m_min < 0){
        // write low byte
        tmp = (unsigned char)(s_value & 0x00ff);
        buffer[byte_offset] = tmp;
        logDebug(">=8 || m_min < 0");

    }

    if(bits > 8){
        tmp = (unsigned char)((s_value >> 8) & 0x00ff);
        buffer[byte_offset - 1] = tmp;
        logDebug(">=8 || m_min < 0");
    }
    if((bits < 8) && (m_min >= 0)){
        unsigned char mask = ((1<<bits) - 1) << (m_offset % 8);
        tmp = buffer[byte_offset];
        tmp = (tmp & (~mask & 0xff)) | (s_value << (m_offset % 8));
        buffer[byte_offset] = tmp;
        logDebug(">=8 || m_min < 0");
    }
   return true;
}

bool IonSysexParam::setValueFromContent(unsigned char *content)
{
   int i;
   if (m_conv == TEXT_LABEL) {
	  return true;
   }
   if(m_conv == NAME){
      return true;
   }
   if (m_offset == -1) {
      return true;
   }

   int bits = getBitWidth();
 
   // handle the weird fx mix
   int l_min = m_min;
   int l_max = m_max;
   if (m_offset == 2240) {
	  l_min = -100;
	  l_max = 100;
	  bits = 8;
   }
  int byte_offset = m_offset / 8;
   int result;
   // Read low byte
   result = (int)content[byte_offset];
   if(bits > 8){
      // Read high byte
      bits = 16;
      result += 256 * (int)content[byte_offset - 1];
   }
   else if(bits < 8){
      if(l_min < 0)
         bits = 8;
      else{
         result >>= m_offset % 8;
         result &= ((1 << bits) - 1);
      }
   }
   if(l_min < 0 && (result & (1 << (bits - 1)))){
      result = -(-result & ((1 << bits) - 1));
   }

   // fx2, sync param
   if (m_offset == 1112) {
	  result -= 12;
   }

   if(result < l_min)
      result = l_min;
   if(result > l_max)
      result = l_max;

   // handle weird cases
   // handle the fm params - don't change order in XML file
   if (m_offset == 282) {
	int fm_type = getBit(295)*3;
	result = (result & 0x3)  + fm_type;
   }
 
   // osc sync param - don't change order in XML file
   if (m_offset == 278) {
	// 0: off, 1: hard 2->1, 2: hard 2+3->1, 3: soft 2->1, 4: soft 2+3->1
	int sync_onoff =  getBit(278);
	int sync_type = getBit(280);
	sync_type = (sync_type == 0) ? 1 : 0;
	int sync_route = getBit(279);
	if (sync_onoff == 1) {
	    result = 0;
	} else {
		result = sync_type*2 + sync_route + 1;
	}
   }

   // portamento params
   if (m_offset == 152) {
	if (getBit(135) == 0) {
		result = result+1;
	} else {
		result = 0;
	}
   }

   // unison
   if (m_offset == 122) {
	if (getBit(121) == 0) {
		result = result+1;
	} else {
		result = 0;
	}
   }
   
   // fx wet dry all screwed up!
   if (m_offset == 2240) {
//	 result = (result+100)/2;
	 result = result/2;
   }
   // fx1, sync param
   if (m_offset == 2304) {
	  // printf("%d\n", result);
	  // result -= 12;
   }

  // remap mod destinations
  for (i = 0; i < 12; i++) {
	  if (m_offset == (1432+(i*8))) {
		result = mod_dst_s_to_n[result];
	  }
	  if (m_offset == (1336+(i*8))) {
		result = mod_src_s_to_n[result];
	  }
  }
  for (i = 0; i < 2; i++) {
	  if (m_offset == (608+(i*8))) {
		result = filter_s_to_n[result];
	  }
  }
  if (m_offset == 1168) {
	result = sh_s_to_n[result];
  }
  // tracking source 1912
  if (m_offset == 1912) {
	result = tracking_s_to_n[result];
  }
   m_value = result - m_cntrlOffset;
   //cout << "m_value was " << result << endl;
   //
   m_valueSet = true;
   return true;
}

IonSysexParam::Conversion IonSysexParam::getConversionType()
{
   return m_conv;
}

vector<ListItemParameter> & IonSysexParam::getList()
{
   return m_list;
}

void IonSysexParam::appendToList(ListItemParameter value)
{
    m_list.push_back(value);
}

/*
bool IonSysexParam::setList(string list)
{
   m_list = init_string_vector(list.c_str());
   m_conv = LIST;
   return true;
}
*/

bool IonSysexParam::setConversion(Conversion conv)
{
   m_conv = conv;
   return true;
}

bool IonSysexParam::setMin(int min)
{
   m_min = min;
   
   return true;
}

bool IonSysexParam::setMax(int max)
{
   m_max = max;
   return true;
}

bool IonSysexParam::setOffset(int offset)
{
   m_offset = offset;
   return true;
}

void IonSysexParam::setCntrlOffset(int offset)
{
   m_cntrlOffset = offset;
}

int IonSysexParam::getCntrlOffset()
{
	return m_cntrlOffset;
}

int IonSysexParam::getNrpn() 
{ 
    return m_nrpn; 
}

int IonSysexParam::getNrpnValue()
{
    int v = getValue();
    if(m_conv == LIST){
        if(m_list[v].hasSpecialNrpnValue()){
            return m_list[v].getNrpnValue();
        }
    }
    return m_value;
}

bool IonSysexParam::hasNrpn() 
{ 
    return (m_nrpn > 0 ? true : false); 
}

bool IonSysexParam::isFxSelector() {
	if ((m_nrpn == FX1_SELECTOR) || (m_nrpn == FX2_SELECTOR)) {
		return true;
	}
	return false;
}

bool IonSysexParam::isTrackingGenValue(){
	return (m_nrpn >= 633 && m_nrpn <= 633+32);
}

SInt32 IonSysexParam::fxSelectorToNrpn() {
	switch (m_nrpn) {
        case FX1_SELECTOR:
            return 743;
            break;
        case FX2_SELECTOR:
            return 757;
            break;
        default:
            return -1;
	}
	return -1;
}


IonSysexParam * IonSysex::getParamByName(string paramName)
{
#if 0
    for(int i = 0; i < params.params.size(); i++){
		if (!params.params[i]->getParamName().empty() &&
			(params.params[i]->getParamName() == paramName)) {
			return params.params[i];
		}
        if(paramName == params.params[i]->getName())
            return params.params[i];
    }
#endif
    return NULL;
}

void IonSysexParam::printDebug()
{
   cout << "name: " << m_name << endl;
   cout << "min,max:" << m_min << "," << m_max << endl;
   if(m_list.size() > 0){
      //cout << "list: " << m_list[0];;
      for(unsigned int j = 1; j < m_list.size() ; j++){
//         cout << ", " << m_list[j];
      }
      cout << endl;
      //cout << "value:" << m_list[m_value] << endl;
   }else if(m_conv == NAME){
      cout << "value: " << String(m_textValue)  << endl;
   }else{
      cout << "value:" << m_value << endl;
   }
   cout << "--------------------------------" << endl;
}

////// IonSysexParams

IonSysexParams::IonSysexParams()
{
	fx1Param = 0;
	fx2Param = 0;
   /*
   IonSysexParam *param;

   // Add "name" parameter by hand
   params.push_back(new IonSysexParam("name"));
   param = params[params.size() - 1];
   param->setConversion(IonSysexParam::NAME);
   param->setOffset(0);
   categoriesMap["Main"].push_back(param);
   addCategoryName("Main");

#include "params.def"
*/
    initFromXml();

}
void IonSysexParams::initFromXml()
{
    int sz;
    const char *xml = BinaryData::getNamedResource("parameters_xml", sz);
    TiXmlDocument doc;
    if(!doc.Parse(xml)){
        logDebug("Unable to parse parameters.xml");
        return;
    }
    TiXmlElement *rootElement = doc.RootElement();
	TiXmlNode *pParamNode = NULL;
    for(pParamNode = rootElement->FirstChild(); pParamNode != 0; pParamNode = pParamNode->NextSibling()) {
		IonSysexParam *param;
		const char *paramName;
		bool isList = false;
		if(string(pParamNode->Value()) != "parameter"){
			logDebug("One of the root children was not a parameter node. Aborting");
			return;
		}
		TiXmlAttribute *pParamAttrib = pParamNode->ToElement()->FirstAttribute();
		if(string(pParamAttrib->Name()) != "name"){
			logDebug("First attribute of parameter element not name. Aborting");
			return;
		}else{
			paramName = pParamAttrib->Value();
		}
		params.push_back(new IonSysexParam(strdup(paramName)));
		param = params[params.size() - 1];
		pParamAttrib = pParamAttrib->Next();
		while(pParamAttrib != 0){
			if(string(pParamAttrib->Name()) == "conversion"){
				string convString = string(pParamAttrib->Value());
				if(convString == "NONE")
					param->setConversion(IonSysexParam::NONE);
				else if(convString == "LIST"){
					param->setConversion(IonSysexParam::LIST);
					isList = true;
				} 
				else if(convString == "PERCENT")
					param->setConversion(IonSysexParam::PERCENT);
				else if(convString == "TENTHS_OF_PERCENT")
					param->setConversion(IonSysexParam::TENTHS_OF_PERCENT);
				else if(convString == "INT32")
					param->setConversion(IonSysexParam::INT32);
				else if(convString == "INT16")
					param->setConversion(IonSysexParam::INT16);
				else if(convString == "ENV_TIME")
					param->setConversion(IonSysexParam::ENV_TIME);
				else if(convString == "FX1_FX2_BALANCE")
					param->setConversion(IonSysexParam::FX1_FX2_BALANCE);
				else if(convString == "FILTER_FREQ")
					param->setConversion(IonSysexParam::FILTER_FREQ);
				else if(convString == "PITCH_FINE")
					param->setConversion(IonSysexParam::PITCH_FINE);
				else if(convString == "PORTA_TIME")
					param->setConversion(IonSysexParam::PORTA_TIME);
				else if(convString == "LFO_FREQ")
					param->setConversion(IonSysexParam::LFO_FREQ);
				else if(convString == "FX_LFO_FREQ")
					param->setConversion(IonSysexParam::FX_LFO_FREQ);
				else if(convString == "MS")
					param->setConversion(IonSysexParam::MS);
				else if(convString == "RELEASE_TIME")
					param->setConversion(IonSysexParam::RELEASE_TIME);
				else if(convString == "FILTER_OFFSET_FREQ")
					param->setConversion(IonSysexParam::FILTER_OFFSET_FREQ);
				else if(convString == "FILTER_OFFSET_OCT")
					param->setConversion(IonSysexParam::FILTER_OFFSET_OCT);
				else if(convString == "BALANCE")
					param->setConversion(IonSysexParam::BALANCE);
				else if(convString == "TENTHS")
					param->setConversion(IonSysexParam::TENTHS);
				else if(convString == "NAME")
					param->setConversion(IonSysexParam::NAME);
				else if(convString == "WET_DRY")
					param->setConversion(IonSysexParam::WET_DRY);
				else if(convString == "PRE_BAL")
					param->setConversion(IonSysexParam::PRE_BAL);
				else if(convString == "POST_BAL")
					param->setConversion(IonSysexParam::POST_BAL);
				else if(convString == "BANK")
					param->setConversion(IonSysexParam::BANK);
				else if(convString == "EXT_IN")
					param->setConversion(IonSysexParam::EXT_IN);
				else if(convString == "OCTAVE")
					param->setConversion(IonSysexParam::OCTAVE);
				else if(convString == "SEMITONE")
					param->setConversion(IonSysexParam::SEMITONE);
				else if(convString == "TEXT_LABEL")
					param->setConversion(IonSysexParam::TEXT_LABEL);
			}else if(string(pParamAttrib->Name()) == "min"){
				int min;
				if(pParamAttrib->QueryIntValue(&min) != TIXML_SUCCESS){
					logDebug("Error parsing min attribute");
				}
				param->setMin(min);
			}else if(string(pParamAttrib->Name()) == "max"){
				int max;
				if(pParamAttrib->QueryIntValue(&max) != TIXML_SUCCESS){
					logDebug("Error parsing max attribute");
				}
				param->setMax(max);
			}else if(string(pParamAttrib->Name()) == "sysexoffset"){
				int sysexoffset;
				if(pParamAttrib->QueryIntValue(&sysexoffset) != TIXML_SUCCESS){
					logDebug("Error parsing sysexoffset attribute");
				}
				param->setOffset(sysexoffset);
			} else if(string(pParamAttrib->Name()) == "defaultval"){
				int defaultval;
				if(pParamAttrib->QueryIntValue(&defaultval) != TIXML_SUCCESS){
					logDebug("Error parsing defaultval attribute");
				}
				param->setDefaultValue(defaultval);
			}else if(string(pParamAttrib->Name()) == "nrpn"){
				int nrpn;
				if(pParamAttrib->QueryIntValue(&nrpn) != TIXML_SUCCESS){
					logDebug("Error parsing nrpn attribute");
				}
                if (nrpn == FX1_SELECTOR) {
                    fx1Param = param;
                }
                if (nrpn == FX2_SELECTOR) {
                    fx2Param = param;
                }
				param->setNrpn(nrpn);
			}else if(string(pParamAttrib->Name()) == "cntrloffset"){
				int offset;
				// how much to add to nrpn value before sending to controller.
				// added to help the fact that bcr2000 doesn't handle negative value well
				if(pParamAttrib->QueryIntValue(&offset) != TIXML_SUCCESS){
				   logDebug("Error parsing nrpn attribute");
				}
				param->setCntrlOffset(offset);
			} else if(string(pParamAttrib->Name()) == "paramname"){
				// how much to add to nrpn value before sending to controller.
				// added to help the fact that bcr2000 doesn't handle negative value well
			   param->setParamName(pParamAttrib->Value());
			}
			

			pParamAttrib = pParamAttrib->Next();

		}
		// Read the listitems
		if(isList){
			TiXmlNode *pListItemNode = NULL;
			for(pListItemNode = pParamNode->FirstChild(); pListItemNode != 0; pListItemNode = pListItemNode->NextSibling()){
				if(string(pListItemNode->Value()) != "listitem"){
					logDebug("One of the root children was not a parameter node. Aborting");
					return;
				}
				TiXmlAttribute *pListItemAttrib = pListItemNode->ToElement()->FirstAttribute();
				if(pListItemAttrib == NULL){
					logDebug("listitem element with no attributes");
				}else{
					const char *listItemName = "";
					bool disableListItem = false;
					bool hasSpecialNrpn = false;
					int nrpnValue = 0;

					while(pListItemAttrib != NULL){
						if(string(pListItemAttrib->Name()) == "name"){
							listItemName = strdup(pListItemAttrib->Value());
						}else if(string(pListItemAttrib->Name()) == "disable"){
							if(string(pListItemAttrib->Value()) == "true")
								disableListItem = true;
						}else if(string(pListItemAttrib->Name()) == "nrpnvalue"){
							if(pListItemAttrib->QueryIntValue(&nrpnValue) != TIXML_SUCCESS){
								logDebug("nrpn attribute ok, but couldn't parse value");
							}else{
								hasSpecialNrpn = true;
							}

						}
						pListItemAttrib = pListItemAttrib->Next();
					}
					ListItemParameter listItemParam(listItemName);
					if(disableListItem)
						listItemParam.setEnabled(false);
					if(hasSpecialNrpn)
						listItemParam.setSpecialNrpnValue(nrpnValue);

					param->appendToList(listItemParam);    
				}
			}
		}
	}
} 


bool IonSysexParams::parseParamsFromContent(unsigned char *ptr, int contentSize)
{
   char buf[4];
   // header
   memcpy(buf, ptr, sizeof(buf));
   ptr += sizeof(buf);
   if(buf[0] != 0x00 ||
      buf[1] != 0x00 ||
      buf[2] != 0x0e ||
      buf[3] != 0x22)
   {
      logDebug("Invalid Sysex Header");
      return false;
   }

   // opcode
   unsigned int n;
   memcpy(&n, ptr, sizeof(unsigned int));
   ptr += sizeof(unsigned int);
   
   // program header
   memcpy(&programHeader, ptr, sizeof(ProgramHeader));
   ptr += sizeof(ProgramHeader);

   // now comes the rest
   // First, decode (shrink) then parse
   vector <unsigned char> encoded;
   vector <unsigned char> decoded;
   for(int j = 0; j < 360; j++) encoded.push_back(ptr[j]);
   decodeFromMidi(encoded, decoded);
   unsigned char *decodedContent = new unsigned char[decoded.size()];
   for (unsigned int i = 0; i < decoded.size(); i++) {
	 decodedContent[i] = decoded[i];
   }

    for(unsigned int i = 0; i < params.size(); i++){
      params[i]->setValueFromContent(decodedContent);
	  if (params[i]->getNrpn() == FX1_SELECTOR) {
		 fx1Param = params[i];
	   }
	  if (params[i]->getNrpn() == FX2_SELECTOR) {
		 fx2Param = params[i];
	   }
//     params[i].printDebug();
   }
    
   {
        char name[15];
        memset(name, 0, 15);
        memcpy(name,&decodedContent[0],15);
        name[14] = 0;
        m_prog_name = String(name);
   }

   delete decodedContent;
   return true;
}

// we assign each fx parameter a different nrpn so that we can have a different GUI element controlling it.
// this function maps those nrpns back to the ones expected by the micron
SInt32 IonSysexParams::fx1fx2NrpnNum(IonSysexParam *p)
{
	SInt32 nrpn = p->getNrpn();
	SInt32 fx = p->fxSelectorToNrpn();
	if (fx != -1) {
		// paramater is a selector
		return fx;
	}
	if ((nrpn >= FX1_FIRST_NRPN) && (nrpn < FX1_LAST_NRPN)) {
		if ((fx1Param) && (fx1Param->getValue())) {
			return nrpn-fx1Param->getValue()*10-100+10;
		}
        // fx bypassed so no nrpn
		return NO_NRPN;
	}
	if ((nrpn >= FX2_FIRST_NRPN) && (nrpn < FX2_LAST_NRPN)) {
		if ((fx2Param) && (fx2Param->getValue())) {
			return nrpn-fx2Param->getValue()*5-157;
		}
        // fx bypassed so no nrpn
		return NO_NRPN;
	}
	return nrpn;
}

SInt32 IonSysexParam::fxMin()
{
	if (m_nrpn == FX1_SELECTOR) {
		return m_value*10+FX1_FIRST_NRPN;
	}
	if (m_nrpn == FX2_SELECTOR) {
		return m_value*5+FX2_FIRST_NRPN;
	}
	return -1;
}

SInt32 IonSysexParam::fxMax()
{
	if (m_nrpn == FX1_SELECTOR) {
		return 10;
	}
	if (m_nrpn == FX2_SELECTOR) {
		return 5;
	}
	return -1;
}

bool IonSysexParams::shouldSkipFx1(IonSysexParam *p) {
	SInt32 nrpn = p->getNrpn();
	SInt32 selectedFxType = fx1Param->getValue()*10 + FX1_FIRST_NRPN;
	if ((nrpn < FX1_FIRST_NRPN) || (nrpn > FX1_LAST_NRPN)) {
		return false;
	}
	if ((nrpn < selectedFxType) || (nrpn >= (selectedFxType + 10))) {
		// we are an fx1 parameter but not the selected one
		return true;
	}
	return false;
}

bool IonSysexParams::shouldSkipFx2(IonSysexParam *p) {
	SInt32 nrpn = p->getNrpn();
	if (!fx2Param || (fx2Param->getValue() == 0)) {
		return false;
	}
	SInt32 selectedFxType = (fx2Param->getValue()-1)*5 + FX2_FIRST_NRPN;
	if ((nrpn < FX2_FIRST_NRPN) || (nrpn > FX2_LAST_NRPN)) {
		return false;
	}
	if ((nrpn < selectedFxType) || (nrpn >= (selectedFxType + 5))) {
		// we are an fx2 parameter but not the selected one
		return true;
	}
	return false;
}

IonSysexParams::~IonSysexParams()
{
        for(unsigned int i = 0; i < params.size(); i++) delete params[i];
}

IonSysexParam *IonSysexParams::getParam(UInt32 idx)
{
	return params[idx];
}

bool IonSysexParams::getAsSysexMessage(unsigned char* sysexBuf)
{
    unsigned char *bufPtr = sysexBuf;
    SysexHeader sysexHeader;
	UInt32 cs;
    // Buffer is 315 chars, which is encoded
    // The raw buffer is 315 + (315 / 7) bytes long
    unsigned char *rawContent = new unsigned char[350];
    for(int i = 0; i < 350; i++) rawContent[i] = 0;
    // first write to a buffer (of 315) then expand
    for(unsigned int i = 0; i < params.size(); i++){
		if (shouldSkipFx1(params[i])) {
			continue;
		}
		if (shouldSkipFx2(params[i])) {
			continue;
		}
        params[i]->writeValueToBuffer(rawContent);
    }
	// if not 1, then we add a new program
	rawContent[293] = 1;
     {
        char str[16];
        bzero(str, sizeof(str));
        m_prog_name.copyToUTF8(str, sizeof(str));
        //	const char *str = CFStringGetCStringPtr(m_textValue, 0);
        bzero(&rawContent[0], 15);
        memcpy((char *) &rawContent[0], str, 14);
        bzero(&rawContent[296], 15);
        memcpy(&rawContent[296], str, 14);
    }
	cs = checksum(rawContent, 315);
	
	sysexHeader.hdr[0] = 0xf0 ;
    sysexHeader.hdr[1] = 0x00;
    sysexHeader.hdr[2] = 0x00;
    sysexHeader.hdr[3] = 0x0e;
    sysexHeader.hdr[4] = 0x22;
    memcpy(bufPtr, &sysexHeader, sizeof(SysexHeader));
    bufPtr += sizeof(SysexHeader);
    
    unsigned int h_opcode = 1;
    unsigned int n_opcode = 1;
    n_opcode = /* TODO: some function of */ h_opcode;
	intToLeBuff(n_opcode, bufPtr);
    bufPtr += sizeof(unsigned int);
    
    ProgramHeader programHeader;
    /* TODO: fill program header */
	bzero(&programHeader, sizeof(programHeader));
	memcpy(programHeader.tag, "Q01SYNTH", 8);
	programHeader.n_checksum = htonl(cs);
	memcpy(programHeader.version, "\x76\x31\x2e\x30\xff\xff\xff\xff", 8);
	bzero(programHeader.date, 12);
	bzero(programHeader.time, 12);
	memcpy(&programHeader.n_length, "\x00\x00\x01\x3b", 4);
	programHeader.matchId = 0xff;
	programHeader.dirty = 0;
	memset(programHeader.padding, 0xff, 6);
	vector<unsigned char> rawheader;
	vector<unsigned char> encodedheader;
	for (unsigned int j = 0; j < 56; j++) {
		rawheader.push_back(*(((unsigned char *) &programHeader)+ j));
	}
	encodeToMidi(rawheader, encodedheader);
	for (unsigned int j = 0; j < encodedheader.size(); j++) {
		bufPtr[j] = encodedheader[j];
	}
	
	bufPtr += sizeof(ProgramHeader);
    vector <unsigned char> raw;
    vector <unsigned char> encoded;
    for(int j = 0; j < 315; j++) raw.push_back(rawContent[j]);
    encodeToMidi(raw, encoded);
    for(unsigned int j = 0; j < encoded.size(); j++) bufPtr[j] = encoded[j];
	
	sysexBuf[SYSEX_PROGRAM_SIZE - 1] = 0xf7;
    
	return true;
}

////////// IonSysex




IonSysex::IonSysex()
{
   strerror = "";
}

IonSysex::IonSysex(unsigned char* buf, UInt32 len)
{
   hasErrors = false;
   if(len != SYSEX_PROGRAM_SIZE){
	  logDebug("Incorrect message size");
   }
  if(!parseBuffer(buf)){
      logDebug("Error parsing buffer");
      hasErrors = true;
   }
}

IonSysex::IonSysex(string filePath)
{
   hasErrors = false;
   // File size first
   struct stat fstat;
   stat(filePath.c_str(), &fstat);
   fileSize = fstat.st_size;
   //cout << "File Size is: " << fileSize;
   // Now open and parse
   FILE *fp = NULL;
   if((fp = fopen(filePath.c_str(), "r")) == NULL) {
	   logDebug("can't open xml");
       return;
   }
   if(!parse(fp)){
      hasErrors = true;
      logDebug("troubles parsing");
   }
   fclose(fp);
}

bool IonSysex::Save()
{
   return true;
}

bool IonSysex::SaveAs()
{
   return true;
}

bool IonSysex::parseBuffer(unsigned char *buffer)
{
   unsigned char *ptr = buffer;

   // header
   memcpy(&sysexHeader, ptr, sizeof(SysexHeader));
   ptr += sizeof(SysexHeader);
   if(sysexHeader.hdr[0] != 0xf0 ||
         sysexHeader.hdr[1] != 0x00 ||
         sysexHeader.hdr[2] != 0x00 ||
         sysexHeader.hdr[3] != 0x0e ||
         sysexHeader.hdr[4] != 0x22)
   {
      logDebug("Invalid Sysex Header");
      return false;
   }

   // opcode
   unsigned int n;
   memcpy(&n, ptr, sizeof(unsigned int));
   ptr += sizeof(unsigned int);
   
   // program header
   memcpy(&programHeader, ptr, sizeof(ProgramHeader));
   ptr += sizeof(ProgramHeader);

   // now comes the rest
   params.parseParamsFromContent(ptr, 360);
   if(ptr[360] != 0xf7){
      logDebug("Bad sysex footer");
      return false;
   }

   return true;
}

bool IonSysex::parse(FILE *fp)
{

    /*

    1 - 73 : Header
    74 - 433 : Content
    434 : Sysex footer (f7)

    */

   logDebug("Entering parse");
   // Check Sysex Header
   if(fread(&sysexHeader, sizeof(SysexHeader), 1, fp) != 1) {
	   logDebug("Error reading sysexHeader");
      return false;
   }
   if(sysexHeader.hdr[0] != 0xf0 ||
         sysexHeader.hdr[1] != 0x00 ||
         sysexHeader.hdr[2] != 0x00 ||
         sysexHeader.hdr[3] != 0x0e ||
         sysexHeader.hdr[4] != 0x22)
   {
      logDebug("Invalid Sysex Header");
      return false;
   }
   // Check Opcode
   unsigned int n_opcode;
   if(fread(&n_opcode, sizeof(unsigned int), 1, fp) != 1){
	  logDebug("Error reading opcode");
      return false;
   }

   //h_opcode = ntohl(n_opcode);

   // Get Header
   logDebug("Getting Header");
   if(fread(&programHeader, sizeof(ProgramHeader), 1, fp) != 1){
      logDebug("Error reading program Header");
      return false;
   }
   // Get the rest and parse it
   int restOfFile = 361;

   /*
   if(restOfFile < 400){
       strerror =  "File too small";
       return false;
   }
   */

   unsigned char *buffer = new unsigned char[restOfFile];
   if(fread(buffer, 1, restOfFile, fp) != restOfFile){
        logDebug("Error reading content");
        return false;
   }
   if(buffer[360] != 0xf7){
       logDebug("Bad sysex footer");
	   return false;
   }
   logDebug("Sending to parse from content");
   params.parseParamsFromContent(buffer, restOfFile - 1);
   delete buffer;
   logDebug("Done Parsing");
   return true;
}

bool IonSysex::HasErrors()
{
    return hasErrors;
}

bool IonSysexTests()
{
    //IonSysex test;
    //test.WriteXMLDefinition();
    logDebug("Testing decode and encode functions: ");
    vector <unsigned char> a,b,c;
    for(int i = 0; i < 315; i ++) a.push_back((unsigned char)i);
    encodeToMidi(a, b);
    decodeFromMidi(b, c);
    if(a.size() != c.size()){
        logDebug("a and c not same size\n");
        return false;
    }
    for(int i = 0; i < a.size(); i++) if(a[i] != c[i]) {
		logDebug("Mismatching element");
        return false;
    }
    // test headers
    // test contents
    // test name
    IonSysex is("test.syx");
    return true;
}

bool IonSysex::WriteXMLDefinition()
{
#if 0
    TiXmlDocument doc;
    TiXmlDeclaration *decl = new TiXmlDeclaration( "1.0", "", "");
    doc.LinkEndChild(decl);
    TiXmlElement *rootElement = new TiXmlElement("parameters");
    IonSysexParam *param = NULL;
    vector<string> categoryNames = params.getCategoryNames();
    for(unsigned k = 0; k < categoryNames.size(); k++){
        TiXmlElement *categoryElement = new TiXmlElement("category");
        categoryElement->SetAttribute("name",categoryNames[k].c_str());
        for(unsigned int i = 0; i < getParamsForCategory(categoryNames[k]).size(); i++){
            param = getParamsForCategory(categoryNames[k])[i];
            TiXmlElement *nodeElement = new TiXmlElement("parameter");
            nodeElement->SetAttribute("name", param->getName().c_str());
            switch(param->getConversionType()){
                case IonSysexParam::NONE:
                    nodeElement->SetAttribute("conversion", "NONE");
                    break;
                case IonSysexParam::LIST:
                    nodeElement->SetAttribute("conversion", "LIST");
                    break;
                case IonSysexParam::PERCENT:
                    nodeElement->SetAttribute("conversion", "PERCENT");
                    break;
                case IonSysexParam::TENTHS_OF_PERCENT:
                    nodeElement->SetAttribute("conversion", "TENTHS_OF_PERCENT");
                    break;
                case IonSysexParam::INT32:
                    nodeElement->SetAttribute("conversion", "INT32");
                    break;
                case IonSysexParam::INT16:
                    nodeElement->SetAttribute("conversion", "INT16");
                    break;
                case IonSysexParam::ENV_TIME:
                    nodeElement->SetAttribute("conversion", "ENV_TIME");
                    break;
                case IonSysexParam::FX1_FX2_BALANCE:
                    nodeElement->SetAttribute("conversion", "FX1_FX2_BALANCE");
                    break;
                case IonSysexParam::FILTER_FREQ:
                    nodeElement->SetAttribute("conversion", "FILTER_FREQ");
                    break;
                case IonSysexParam::PITCH_FINE:
                    nodeElement->SetAttribute("conversion", "PITCH_FINE");
                    break;
                case IonSysexParam::PORTA_TIME:
                    nodeElement->SetAttribute("conversion", "PORTA_TIME");
                    break;
                case IonSysexParam::LFO_FREQ:
                    nodeElement->SetAttribute("conversion", "LFO_FREQ");
                    break;
                case IonSysexParam::FX_LFO_FREQ:
                    nodeElement->SetAttribute("conversion", "FX_LFO_FREQ");
                    break;
                case IonSysexParam::MS:
                    nodeElement->SetAttribute("conversion", "MS");
                    break;
                case IonSysexParam::RELEASE_TIME:
                    nodeElement->SetAttribute("conversion", "RELEASE_TIME");
                    break;
                case IonSysexParam::FILTER_OFFSET_FREQ:
                    nodeElement->SetAttribute("conversion", "FILTER_OFFSET_FREQ");
                    break;
                case IonSysexParam::BALANCE:
                    nodeElement->SetAttribute("conversion", "BALANCE");
                    break;
                case IonSysexParam::TENTHS:
                    nodeElement->SetAttribute("conversion", "TENTHS");
                    break;
                 case IonSysexParam::OCTAVE:
                    nodeElement->SetAttribute("conversion", "OCTAVE");
                    break;
                case IonSysexParam::SEMITONE:
                    nodeElement->SetAttribute("conversion", "SEMITONE");
                    break;
               case IonSysexParam::NAME:
                    nodeElement->SetAttribute("conversion", "NAME");
                    break;
				default:
					logDebug("unknown type\n");
					break;
            }
            if(param->getConversionType() != IonSysexParam::NAME){
                nodeElement->SetAttribute("min", param->getMin());
                nodeElement->SetAttribute("max", param->getMax());
            }
            if(param->getConversionType() == IonSysexParam::LIST){
                /*
                   for(unsigned int j = 0; j <  param->m_list.size(); j++){
                   TiXmlElement *listElement = new TiXmlElement("listitem");
                   listElement->SetAttribute("name", param->m_list[j].c_str());
                   nodeElement->LinkEndChild(listElement);
                   }
                 */

            }
            nodeElement->SetAttribute("sysexoffset", param->m_offset);
            if(param->hasNrpn()){
                nodeElement->SetAttribute("nrpn", param->getNrpn());
            }


            categoryElement->LinkEndChild(nodeElement);
        }
        rootElement->LinkEndChild(categoryElement);
    }
    doc.LinkEndChild(rootElement);
    doc.SaveFile("parameters.xml"); 
#endif
    return true;
}

UInt32 toBigEndian(unsigned char *b)
{
  return (*b << 24) | (*(b+1) << 16) | (*(b+2) << 8) | *(b+3);
}

static UInt32 checksum(unsigned char *buff, UInt32 len)
{
	UInt32 cs = 0;
	UInt32 i;
	
	for (i = 0; i < len; i += 4) {
		UInt32 oldcs = cs;
		cs += toBigEndian(&buff[i]);
		if (cs < oldcs) {
		}
	}
	return cs * -1;
}
