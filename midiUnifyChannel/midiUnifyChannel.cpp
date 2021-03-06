/*-----------------------------------------------------------------------------
MidiUnifyChannel
original framework by Reuben Vinal
specific implementation by H.R.Graf
-----------------------------------------------------------------------------*/
#include "../common/PizMidi.h"

enum
{
    kChannel,
    kPower,

    kNumParams,
    kNumPrograms = 4
};

//-------------------------------------------------------------------------------------------------------
class MidiUnifyChannelProgram {
    friend class MidiUnifyChannel;
public:
    MidiUnifyChannelProgram();
    ~MidiUnifyChannelProgram() {}
private:
    float fChannel;
    float fPower;
    char name[kVstMaxProgNameLen];
};

//-------------------------------------------------------------------------------------------------------
class MidiUnifyChannel : public PizMidi
{
public:
    MidiUnifyChannel(audioMasterCallback audioMaster);
    ~MidiUnifyChannel();

    virtual void   setProgram(VstInt32 program);
    virtual void   setProgramName(char *name);
    virtual void   getProgramName(char *name);
    virtual bool   getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text);

    virtual void   setParameter(VstInt32 index, float value);
    virtual float  getParameter(VstInt32 index);
    virtual void   getParameterDisplay(VstInt32 index, char *text);
    virtual void   getParameterName(VstInt32 index, char *text);

protected:
    float fChannel;
    float fPower;

    virtual void processMidiEvents(VstMidiEventVec *inputs, VstMidiEventVec *outputs, VstInt32 sampleFrames);

    MidiUnifyChannelProgram *programs;
};


//-------------------------------------------------------------------------------------------------------
AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
    return new MidiUnifyChannel(audioMaster);
}

MidiUnifyChannelProgram::MidiUnifyChannelProgram()
{
    // default Program Values
    fChannel = 0.0f;
    fPower = 1.0f;

    // default program name
    strcpy(name, "Default");
}

//-----------------------------------------------------------------------------
MidiUnifyChannel::MidiUnifyChannel(audioMasterCallback audioMaster)
    : PizMidi(audioMaster, kNumPrograms, kNumParams), programs(0)
{
    programs = new MidiUnifyChannelProgram[numPrograms];

    if (programs) {
        CFxBank* defaultBank = new CFxBank(kNumPrograms, kNumParams);
        if (readDefaultBank(PLUG_NAME, defaultBank)) {
            if ((VstInt32)defaultBank->GetFxID() == PLUG_IDENT) {
                for (int i = 0; i < kNumPrograms; i++) {
                    programs[i].fChannel = defaultBank->GetProgParm(i, 0);
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
                    sprintf(programs[i].name, "Output on channel 10");
                    programs[i].fChannel = CHANNEL_TO_FLOAT015(9);
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
MidiUnifyChannel::~MidiUnifyChannel() {
    if (programs) 
        delete[] programs;
}

//------------------------------------------------------------------------
void MidiUnifyChannel::setProgram(VstInt32 program)
{
    MidiUnifyChannelProgram* ap = &programs[program];

    curProgram = program;
    setParameter(kChannel, ap->fChannel);
    setParameter(kPower, ap->fPower);
}

//------------------------------------------------------------------------
void MidiUnifyChannel::setProgramName(char *name)
{
    vst_strncpy(programs[curProgram].name, name, kVstMaxProgNameLen);
}

//------------------------------------------------------------------------
void MidiUnifyChannel::getProgramName(char *name)
{
    strcpy(name, programs[curProgram].name);
}

//-----------------------------------------------------------------------------------------
bool MidiUnifyChannel::getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text)
{
    if (index < kNumPrograms)
    {
        strcpy(text, programs[index].name);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------------
void MidiUnifyChannel::setParameter(VstInt32 index, float value) {

    MidiUnifyChannelProgram* ap = &programs[curProgram];

    switch (index) {
    case kChannel: fChannel = ap->fChannel = value; break;
    case kPower:    fPower  = ap->fPower  = value;  break;
    }
}

//-----------------------------------------------------------------------------------------
float MidiUnifyChannel::getParameter(VstInt32 index) {
    float v = 0;

    switch (index) {
    case kChannel:   v = fChannel; break;
    case kPower:     v = fPower;   break;
    }
    return v;
}

//-----------------------------------------------------------------------------------------
void MidiUnifyChannel::getParameterName(VstInt32 index, char *label) {
    switch (index) {
    case kChannel:  strcpy(label, "Channel Out"); break;
    case kPower:    strcpy(label, "Power");       break;
    }
}

//-----------------------------------------------------------------------------------------
void MidiUnifyChannel::getParameterDisplay(VstInt32 index, char *text) {
    switch (index) {
    case kChannel: sprintf(text, "%d", FLOAT_TO_CHANNEL015(fChannel) + 1); break;
    case kPower:   strcpy(text, (fPower < 0.5f) ? "off" : "on"); break;
    }
}

void MidiUnifyChannel::processMidiEvents(VstMidiEventVec *inputs, VstMidiEventVec *outputs, VstInt32 sampleFrames)
{
    // process incoming events (of first input)
    for (unsigned int i = 0; i < inputs[0].size(); i++) 
    {
        //copying event "i" from input (with all its fields)
        VstMidiEvent me = inputs[0][i];

        short status = me.midiData[0] & 0xF0;   // scraping  channel
        //short channel = me.midiData[0] & 0x0F;  // isolating channel (0-15)
        //short data1 = me.midiData[1] & 0x7F;
        //short data2 = me.midiData[2] & 0x7F;

        // modify event
        short channel = FLOAT_TO_CHANNEL015(fChannel) & 0x0F; //outgoing midi channel
        me.midiData[0] = status | channel;

        // output event
        if (fPower >= 0.5f)
                outputs[0].push_back(me);
    }
}
