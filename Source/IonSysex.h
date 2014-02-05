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

#ifndef _IONSYSEX_H_
#define _IONSYSEX_H_

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <MacTypes.h>
#include "../JuceLibraryCode/JuceHeader.h"

#define FX1_SELECTOR 800
#define FX2_SELECTOR 801
#define FX1_FIRST_NRPN 834
#define FX1_LAST_NRPN FX1_FIRST_NRPN+10*7
#define FX2_FIRST_NRPN 920
#define FX2_LAST_NRPN FX2_FIRST_NRPN+5*6
#define NO_NRPN 4096
//typedef unsigned int UInt32;
//typedef int SInt32;

// TODO: remove this "using namespace" from here
using namespace std;

struct _SysexHeader{
   unsigned char hdr[5];
};


typedef struct _SysexHeader SysexHeader;


struct _ProgramHeader{
   char tag[8];                                 // 8
   unsigned int n_checksum; // 1s comp          // 4
   unsigned char version[8];                    // 8
   unsigned char date[12];                      // 12
   unsigned char time[12];                      // 12
   unsigned int n_length;                       // 4
   unsigned char matchId;                       // 1
   unsigned char dirty;                         // 1
   unsigned char padding[6];                    // 6
   unsigned char crap[8];                       // 8
};

typedef struct _ProgramHeader ProgramHeader;

class IonSysex;
class IonSysexParams;

class ListItemParameter{
    public:
        ListItemParameter(const char *name);
        const char *getName();
        bool isDisabled();
        void setEnabled(bool enabled);
        bool hasSpecialNrpnValue();
        int  getNrpnValue();
        void setSpecialNrpnValue(int nrpn);
    private:
        const char *m_name;
        bool m_enabled;
        bool m_hasSpecialNrpnValue;
        int m_nrpnValue;
};

class IonSysexParam {
   public:
      IonSysexParam(const char *name);

      enum Conversion{
         NONE = 0,
         LIST,
         PERCENT,
         TENTHS_OF_PERCENT,
         INT32,
         INT16,
         INT8,
         ENV_TIME,
         FX1_FX2_BALANCE,
         FILTER_FREQ,
         PITCH_FINE,
         PORTA_TIME,
         LFO_FREQ,
         RELEASE_TIME,
         FILTER_OFFSET_FREQ,
         FILTER_OFFSET_OCT,
         BALANCE,
         TENTHS,
         NAME,
		 TEXT_LABEL,
		 WET_DRY,
		 PRE_BAL,
		 POST_BAL,
		 EXT_IN,
		 FX_LFO_FREQ,
		 MS,
		 OCTAVE,
		 SEMITONE
      };

      const char *getName() { return m_name; }
      Conversion getConversionType();
      vector<ListItemParameter> &getList();
      int getBitWidth();
      int getValue();
      void setValue(int value);
      int getNrpnValue();
	  int getDefaultValue() { return m_defaultValue;}
 	  void setDefaultValue(int v) { m_defaultValue = v;}
      String getConvertedValue(SInt32 val);
      String getTextValue();
	  bool setTextValue(const char *);
      void setParamName(const char *);
	  String getParamName();
	  bool setValueFromContent(unsigned char *content);
      bool writeValueToBuffer(unsigned char *buffer);
      bool writeNameToBuffer(unsigned char *buffer);
      void printDebug();

      int getMin() { return m_min - m_cntrlOffset; };
      int getMax() { return m_max - m_cntrlOffset; };
      int getNrpn();
	  int getCntrlOffset();
	  bool hasNrpn();
      bool isFxSelector();
	  SInt32 fxSelectorToNrpn();
	  SInt32 fxMin();
	  SInt32 fxMax();
   
      friend class IonSysexParams;
      friend class IonSysex;
      

   private:
      bool setNameFromContent(unsigned char *content);
	  unsigned char progName[16];
     // bool setList(string list);
      void appendToList(ListItemParameter value);
      void setNrpn(int nrpn);
      bool setConversion(Conversion conv);
      bool setMin(int min);
      bool setMax(int max);
      bool setOffset(int offset);
	  void setCntrlOffset(int cntrloffset);
      int m_offset;
      vector<ListItemParameter> m_list;
      bool m_hasMin;
      bool m_hasMax;
      bool m_valueSet;
      int m_value;
      const char *m_name;
      String m_textValue;
	  const char *m_paramName;
      int m_min;
      int m_max;
      int m_nrpn;
	  int m_cntrlOffset;
	  int m_defaultValue;
      Conversion m_conv;
};

/* The content goes from 
 from         (77)
 to    0x1b0  (433)
       0x1b1  (434)      -> F7 (sysex footer)
       */

// parameter descriptions loaded from XML
class IonSysexParams{
   public:
      IonSysexParams();
      bool parseParamsFromContent(unsigned char *content, int contentSize);
      void fillBuffer(unsigned char *buffer);
	  IonSysexParam *getParam(UInt32 idx);
	  UInt32 numParams() {return params.size();}
      ~IonSysexParams();
      bool getAsSysexMessage(unsigned char *sysBuf);
    
      String get_prog_name() {return m_prog_name;}
      void set_prog_name(String s) {m_prog_name = s;}

      // returns adjusted nrpn number if this is an fx parameter, otherwise returns normal nrpn
      SInt32 fx1fx2NrpnNum(IonSysexParam *p);
      bool shouldSkipFx1(IonSysexParam *p);
      bool shouldSkipFx2(IonSysexParam *p);

    private:
      SysexHeader sysexHeader;
      ProgramHeader programHeader;
      void initFromXml();
      vector<IonSysexParam*> params;
	  IonSysexParam *fx1Param;
	  IonSysexParam *fx2Param;
      String m_prog_name;
};

// parameters loaded from sysex
class IonSysex{
   public:
      IonSysex();
      IonSysex(string fileName);
	  IonSysex(unsigned char* buf, UInt32 len);
      bool Save();
      bool SaveAs();
      bool Print();
      string GetError();
      const vector<string> & getCategoryNames();
      IonSysexParam * getParamByName(string parameterName);
      bool HasErrors();
      bool parseBuffer(unsigned char *buffer);

      bool WriteXMLDefinition();
  private:
      bool parse(FILE *fp);
      string filePath;
      string strerror;
      SysexHeader sysexHeader;
      ProgramHeader programHeader;
      IonSysexParams params;
      bool hasErrors;
      int fileSize;
};

bool IonSysexTests();

#endif
