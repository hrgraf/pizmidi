/*-----------------------------------------------------------------------------
MidiProgramChange
original framework by Reuben Vinal
specific implementation by H.R.Graf
-----------------------------------------------------------------------------*/
#ifndef __MidiProgramChange_H
#define __MidiProgramChange_H

#include "../common/PizMidi.h"

enum
{
    kPower,

    kNumParams,
    kNumMidiCh = 16,
    kNumPrograms = 1
};

class MidiProgramChangeProgram {
    friend class MidiProgramChange;
public:
    MidiProgramChangeProgram();
    ~MidiProgramChangeProgram() {}
private:
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
    float fPower;

    short progNum[kNumMidiCh]; // individual for every MIDI channel
    short bankNum[kNumMidiCh]; // individual for every MIDI channel

    virtual void processMidiEvents(VstMidiEventVec *inputs, VstMidiEventVec *outputs, VstInt32 sampleFrames);

    MidiProgramChangeProgram *programs;
};

#endif
