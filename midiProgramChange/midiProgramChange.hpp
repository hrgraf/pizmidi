/*-----------------------------------------------------------------------------
MidiProgramChange
by Reuben Vinal
-----------------------------------------------------------------------------*/
#ifndef __MidiProgramChange_H
#define __MidiProgramChange_H

#include "../common/PizMidi.h"

// Arturia Control Change Constants

#define CC_ID_TURN_PRESET       114
#define CC_ID_PRESS_PRESET      115

#define CC_VAL_TURN_INC           1
#define CC_VAL_TURN_DEC          65

#define CC_VAL_PRESS_DOWN       127
#define CC_VAL_PRESS_RELEASE      0

enum
{
    kMirror,
    kPower,

    kNumParams,
    kNumPrograms = 16
};

class MidiProgramChangeProgram {
    friend class MidiProgramChange;
public:
    MidiProgramChangeProgram();
    ~MidiProgramChangeProgram() {}
private:
    float fMirror;
    float fPower;
    char name[kVstMaxProgNameLen];
};

class MidiProgramChange : public PizMidi
{
public:
    MidiProgramChange(audioMasterCallback audioMaster);
    ~MidiProgramChange();

    virtual void   setProgram(VstInt32 program);
    virtual void   setProgramName(char *name);
    virtual void   getProgramName(char *name);
    virtual bool   getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text);

    virtual void   setParameter(VstInt32 index, float value);
    virtual float  getParameter(VstInt32 index);
    virtual void   getParameterDisplay(VstInt32 index, char *text);
    virtual void   getParameterName(VstInt32 index, char *text);

protected:
    float fMirror;
    float fPower;

    virtual void processMidiEvents(VstMidiEventVec *inputs, VstMidiEventVec *outputs, VstInt32 sampleFrames);

    MidiProgramChangeProgram *programs;
};

#endif
