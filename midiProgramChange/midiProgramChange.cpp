#include "MidiProgramChange.hpp"

//-------------------------------------------------------------------------------------------------------
AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
    return new MidiProgramChange(audioMaster);
}

MidiProgramChangeProgram::MidiProgramChangeProgram()
{
    // default Program Values
    fMirror = MIDI_TO_FLOAT(60);
    fPower = 1.0f;

    // default program name
    strcpy(name, "Default");
}

//-----------------------------------------------------------------------------
MidiProgramChange::MidiProgramChange(audioMasterCallback audioMaster)
    : PizMidi(audioMaster, kNumPrograms, kNumParams), programs(0)
{
    programs = new MidiProgramChangeProgram[numPrograms];

    if (programs) {
        CFxBank* defaultBank = new CFxBank(kNumPrograms, kNumParams);
        if (readDefaultBank(PLUG_NAME, defaultBank)) {
            if ((VstInt32)defaultBank->GetFxID() == PLUG_IDENT) {
                for (int i = 0; i < kNumPrograms; i++) {
                    programs[i].fMirror = defaultBank->GetProgParm(i, 0);
                    programs[i].fPower = defaultBank->GetProgParm(i, 1);
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
                    programs[i].fMirror = MIDI_TO_FLOAT(0);
                    sprintf(programs[i].name, "Zero");
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
    if (programs) delete[] programs;
}

//------------------------------------------------------------------------
void MidiProgramChange::setProgram(VstInt32 program)
{
    MidiProgramChangeProgram* ap = &programs[program];

    curProgram = program;
    setParameter(kMirror, ap->fMirror);
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
    case kMirror:   fMirror = ap->fMirror = value;	break;
    case kPower:    fPower  = ap->fPower  = value;  break;
    default: break;
    }
}

//-----------------------------------------------------------------------------------------
float MidiProgramChange::getParameter(VstInt32 index) {
    float v = 0;

    switch (index) {
    case kMirror:    v = fMirror;  break;
    case kPower:     v = fPower;   break;
    default: break;
    }
    return v;
}

//-----------------------------------------------------------------------------------------
void MidiProgramChange::getParameterName(VstInt32 index, char *label) {
    switch (index) {
    case kMirror:   strcpy(label, "Center Note"); break;
    case kPower:    strcpy(label, "Power");       break;
    default: break;
    }
}

//-----------------------------------------------------------------------------------------
void MidiProgramChange::getParameterDisplay(VstInt32 index, char *text) {
    switch (index) {
    case kMirror: strcpy(text, getNoteName(FLOAT_TO_MIDI(fMirror), bottomOctave)); break;
    case kPower:
        if (fPower < 0.5f) strcpy(text, "off");
        else strcpy(text, "on"); break;
    default: break;
    }
}

void MidiProgramChange::processMidiEvents(VstMidiEventVec *inputs, VstMidiEventVec *outputs, VstInt32 sampleFrames)
{
    short mirror = FLOAT_TO_MIDI(fMirror);

    // process incoming events (of first input)
    for (unsigned int i = 0; i < inputs[0].size(); i++) 
    {
        //copying event "i" from input (with all its fields)
        VstMidiEvent me = inputs[0][i];
        short change = 0;

        short status = me.midiData[0] & 0xf0;   // scraping  channel
        short channel = me.midiData[0] & 0x0f;  // isolating channel (0-15)
        //short data1 = me.midiData[1] & 0x7f;
        //short data2 = me.midiData[2] & 0x7f;

        if (status == MIDI_CONTROLCHANGE)
        {
            short id = me.midiData[1] & 0x7f;
            short val = me.midiData[2] & 0x7f;

            if (id == CC_ID_TURN_PRESET)
            {
                if (val == CC_VAL_TURN_INC)
                {
                    change = 1;
                    mirror++;
                }
                else if (val == CC_VAL_TURN_DEC)
                {
                    change = 1;
                    mirror--;
                }
            }
            else if (id == CC_ID_PRESS_PRESET)
            {
                if (val == CC_VAL_PRESS_DOWN)
                {
                    change = 1;
                    mirror = 0;
                }
            }
        }

        if ((fPower >= 0.5f) && change)
        {
            me.midiData[0] = MIDI_PROGRAMCHANGE | channel;
            me.midiData[1] = mirror & 127;
            me.midiData[2] = 0;
            me.midiData[3] = 0;
        }

        outputs[0].push_back(me);
    }

    fMirror = MIDI_TO_FLOAT(mirror);
}
