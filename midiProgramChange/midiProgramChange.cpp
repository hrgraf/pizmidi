#include "MidiProgramChange.hpp"

// Arturia Control Change Constants

#define CC_ID_TURN_CAT          112
#define CC_ID_PRESS_CAT         113
#define CC_ID_TURN_PRESET       114
#define CC_ID_PRESS_PRESET      115

#define CC_VAL_TURN_INC           1
#define CC_VAL_TURN_DEC          65

#define CC_VAL_PRESS_DOWN       127
#define CC_VAL_PRESS_RELEASE      0

//-------------------------------------------------------------------------------------------------------
AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
    return new MidiProgramChange(audioMaster);
}

MidiProgramChangeProgram::MidiProgramChangeProgram()
{
    // default Program Values
    fPower = 1.0f;

    // default program name
    strcpy(name, "Default");
}

//-----------------------------------------------------------------------------
MidiProgramChange::MidiProgramChange(audioMasterCallback audioMaster)
    : PizMidi(audioMaster, kNumPrograms, kNumParams), programs(0)
{
    // reset internal state
    for (int i = 0; i < kNumMidiCh; i++)
    {
        progNum[i] = 0;
        bankNum[i] = 0;
    }

    programs = new MidiProgramChangeProgram[numPrograms];

    if (programs) {
        CFxBank* defaultBank = new CFxBank(kNumPrograms, kNumParams);
        if (readDefaultBank(PLUG_NAME, defaultBank)) {
            if ((VstInt32)defaultBank->GetFxID() == PLUG_IDENT) {
                for (int i = 0; i < kNumPrograms; i++) {
                    programs[i].fPower = defaultBank->GetProgParm(i, 0);
                    strcpy(programs[i].name, defaultBank->GetProgramName(i));
                }
            }
        }
        else {
            // built-in programs
            for (int i = 0; i < kNumPrograms; i++) {
                switch (i)
                {
                case 0:
                    sprintf(programs[i].name, "Arturia Midi Keyboard");
                    break;
                default:
                    sprintf(programs[i].name, "Program %d", i + 1);
                    break;
                }
            }
        }
        setProgram(0);
    }

    init();
}


//-----------------------------------------------------------------------------------------
MidiProgramChange::~MidiProgramChange() {
    if (programs) 
        delete[] programs;
}

//------------------------------------------------------------------------
void MidiProgramChange::setProgram(VstInt32 program)
{
    MidiProgramChangeProgram* ap = &programs[program];

    curProgram = program;
    setParameter(kPower, ap->fPower);
}

//------------------------------------------------------------------------
void MidiProgramChange::setProgramName(char *name)
{
    vst_strncpy(programs[curProgram].name, name, kVstMaxProgNameLen);
}

//------------------------------------------------------------------------
void MidiProgramChange::getProgramName(char *name)
{
    strcpy(name, programs[curProgram].name);
}

//-----------------------------------------------------------------------------------------
bool MidiProgramChange::getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text)
{
    if (index < kNumPrograms)
    {
        strcpy(text, programs[index].name);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------------
void MidiProgramChange::setParameter(VstInt32 index, float value) {

    MidiProgramChangeProgram* ap = &programs[curProgram];

    switch (index) {
    case kPower:    fPower  = ap->fPower  = value;  break;
    }
}

//-----------------------------------------------------------------------------------------
float MidiProgramChange::getParameter(VstInt32 index) {
    float v = 0;

    switch (index) {
    case kPower:     v = fPower;   break;
    }
    return v;
}

//-----------------------------------------------------------------------------------------
void MidiProgramChange::getParameterName(VstInt32 index, char *label) {
    switch (index) {
    case kPower:    strcpy(label, "Power");       break;
    }
}

//-----------------------------------------------------------------------------------------
void MidiProgramChange::getParameterDisplay(VstInt32 index, char *text) {
    switch (index) {
    case kPower:
        if (fPower < 0.5f) 
            strcpy(text, "off");
        else
            strcpy(text, "on"); 
        break;
    }
}

void MidiProgramChange::processMidiEvents(VstMidiEventVec *inputs, VstMidiEventVec *outputs, VstInt32 sampleFrames)
{
    // process incoming events (of first input)
    for (unsigned int i = 0; i < inputs[0].size(); i++) 
    {
        //copying event "i" from input (with all its fields)
        VstMidiEvent me = inputs[0][i];

        short status = me.midiData[0] & 0xf0;   // scraping  channel
        short channel = me.midiData[0] & 0x0f;  // isolating channel (0-15)
        //short data1 = me.midiData[1] & 0x7f;
        //short data2 = me.midiData[2] & 0x7f;

        short filter = 0;
        short change = 0;
        short num    = progNum[channel];
        short bank   = bankNum[channel];

        if (status == MIDI_CONTROLCHANGE)
        {
            short id  = me.midiData[1] & 0x7f;
            short val = me.midiData[2] & 0x7f;

            if (id == CC_ID_TURN_PRESET)
            {
                if (val == CC_VAL_TURN_INC)
                {
                    change = 1;
                    if (num < 127)
                      num++;
                }
                else if (val == CC_VAL_TURN_DEC)
                {
                    change = 1;
                    if (num > 0)
                        num--;
                }
            }
            else if (id == CC_ID_PRESS_PRESET)
            {
                if (val == CC_VAL_PRESS_DOWN)
                {
                    change = 1;
                    num    = 0;
                }
                else if (val == CC_VAL_PRESS_RELEASE)
                {
                    filter = 1;
                }
            }
            else if (id == CC_ID_TURN_CAT)
            {
                if (val == CC_VAL_TURN_INC)
                {
                    change = 2;
                    num    = 0;
                    if (bank < 127)
                        bank++;
                }
                else if (val == CC_VAL_TURN_DEC)
                {
                    change = 2;
                    num = 0;
                    if (bank > 0)
                        bank--;
                }
            }
            else if (id == CC_ID_PRESS_CAT)
            {
                if (val == CC_VAL_PRESS_DOWN)
                {
                    change = 2;
                    num    = 0;
                    bank   = 0;
                }
                else if (val == CC_VAL_PRESS_RELEASE)
                {
                    filter = 1;
                }
            }

            progNum[channel] = num; // write back
            bankNum[channel] = bank;
        }

        if (change && (fPower >= 0.5f))
        {
            if (change == 2) // bank change
            {
                me.midiData[0] = MIDI_CONTROLCHANGE | channel;
                me.midiData[1] = MIDI_BANK_CHANGE | MIDI_LSB;
                me.midiData[2] = bank & 127;
                me.midiData[3] = 0;
                outputs[0].push_back(me);
            }

            // program change
            me.midiData[0] = MIDI_PROGRAMCHANGE | channel;
            me.midiData[1] = num & 127;
            me.midiData[2] = 0;
            me.midiData[3] = 0;
        }

        if (! filter)
            outputs[0].push_back(me);
    }
}
