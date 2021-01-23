/*-----------------------------------------------------------------------------
MidiFromJoystick
original framework by Reuben Vinal
specific implementation by H.R.Graf
-----------------------------------------------------------------------------*/

// No MFC
#define WIN32_LEAN_AND_MEAN

// We need the Windows Header and the XInput Header
#include <windows.h>
#include <XInput.h>
#include <stdio.h>

#pragma comment(lib, "Xinput.lib")

#include "../common/PizMidi.h"

enum
{
    kChannel,
    kPower,

    kNumParams,
    kNumPrograms = 4
};

//-------------------------------------------------------------------------------------------------------
class MidiFromJoystickProgram {
    friend class MidiFromJoystick;
public:
    MidiFromJoystickProgram();
    ~MidiFromJoystickProgram() {}
private:
    float fChannel;
    float fPower;
    char name[kVstMaxProgNameLen];
};

//-------------------------------------------------------------------------------------------------------
class MidiFromJoystick : public PizMidi
{
public:
    MidiFromJoystick(audioMasterCallback audioMaster);
    ~MidiFromJoystick();

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

    MidiFromJoystickProgram *programs;
};


//-------------------------------------------------------------------------------------------------------
AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
    return new MidiFromJoystick(audioMaster);
}

MidiFromJoystickProgram::MidiFromJoystickProgram()
{
    // default Program Values
    fChannel = 0.0f;
    fPower = 1.0f;

    // default program name
    strcpy(name, "Default");
}

//-----------------------------------------------------------------------------
MidiFromJoystick::MidiFromJoystick(audioMasterCallback audioMaster)
    : PizMidi(audioMaster, kNumPrograms, kNumParams), programs(0)
{
    programs = new MidiFromJoystickProgram[numPrograms];

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
MidiFromJoystick::~MidiFromJoystick() {
    if (programs) 
        delete[] programs;
}

//------------------------------------------------------------------------
void MidiFromJoystick::setProgram(VstInt32 program)
{
    MidiFromJoystickProgram* ap = &programs[program];

    curProgram = program;
    setParameter(kChannel, ap->fChannel);
    setParameter(kPower, ap->fPower);
}

//------------------------------------------------------------------------
void MidiFromJoystick::setProgramName(char *name)
{
    vst_strncpy(programs[curProgram].name, name, kVstMaxProgNameLen);
}

//------------------------------------------------------------------------
void MidiFromJoystick::getProgramName(char *name)
{
    strcpy(name, programs[curProgram].name);
}

//-----------------------------------------------------------------------------------------
bool MidiFromJoystick::getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text)
{
    if (index < kNumPrograms)
    {
        strcpy(text, programs[index].name);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------------
void MidiFromJoystick::setParameter(VstInt32 index, float value) {

    MidiFromJoystickProgram* ap = &programs[curProgram];

    switch (index) {
    case kChannel: fChannel = ap->fChannel = value; break;
    case kPower:    fPower  = ap->fPower  = value;  break;
    }
}

//-----------------------------------------------------------------------------------------
float MidiFromJoystick::getParameter(VstInt32 index) {
    float v = 0;

    switch (index) {
    case kChannel:   v = fChannel; break;
    case kPower:     v = fPower;   break;
    }
    return v;
}

//-----------------------------------------------------------------------------------------
void MidiFromJoystick::getParameterName(VstInt32 index, char *label) {
    switch (index) {
    case kChannel:  strcpy(label, "Channel Out"); break;
    case kPower:    strcpy(label, "Power");       break;
    }
}

//-----------------------------------------------------------------------------------------
void MidiFromJoystick::getParameterDisplay(VstInt32 index, char *text) {
    switch (index) {
    case kChannel: sprintf(text, "%d", FLOAT_TO_CHANNEL015(fChannel) + 1); break;
    case kPower:   strcpy(text, (fPower < 0.5f) ? "off" : "on"); break;
    }
}

//-----------------------------------------------------------------------------------------

typedef struct JoyNote
{
    WORD button;
    BYTE vel;
    BYTE key;
};

static const JoyNote joyNote[] =
{
  { XINPUT_GAMEPAD_A             , 0x40, 0x30 }, // C4
  { XINPUT_GAMEPAD_B             , 0x40, 0x32 }, // D4
  { XINPUT_GAMEPAD_X             , 0x40, 0x34 }, // E4
  { XINPUT_GAMEPAD_Y             , 0x40, 0x35 }, // F4
  { XINPUT_GAMEPAD_DPAD_DOWN     , 0x40, 0x37 }, // G4
  { XINPUT_GAMEPAD_DPAD_RIGHT    , 0x40, 0x39 }, // A4
  { XINPUT_GAMEPAD_DPAD_LEFT     , 0x40, 0x3B }, // H4
  { XINPUT_GAMEPAD_DPAD_UP       , 0x40, 0x3C }, // C5
  { 0, 0, 0 } // terminator
};

/*
XINPUT_GAMEPAD_START,
XINPUT_GAMEPAD_BACK,
XINPUT_GAMEPAD_LEFT_THUMB,
XINPUT_GAMEPAD_RIGHT_THUMB,
XINPUT_GAMEPAD_LEFT_SHOULDER,
XINPUT_GAMEPAD_RIGHT_SHOULDER,
*/


void MidiFromJoystick::processMidiEvents(VstMidiEventVec *inputs, VstMidiEventVec *outputs, VstInt32 sampleFrames)
{
    static DWORD pktNum = 0;
    static WORD lastButtons = 0;
    static SHORT lastY = 0;
    static SHORT progNum = 0;

    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(XINPUT_STATE));

    if (XInputGetState(0, &state) == ERROR_SUCCESS)
    {
        if (state.dwPacketNumber != pktNum) // changed
        {
            WORD newButtons = state.Gamepad.wButtons;

            // output enabled
            if (fPower >= 0.5f)
            {
                WORD changed = newButtons ^ lastButtons;

                VstMidiEvent me;
                me.deltaFrames = 0;

                SHORT channel = FLOAT_TO_CHANNEL015(fChannel) & 0x0F; //outgoing midi channel

                // handle notes
                for (DWORD i = 0; ; i++)
                {
                    JoyNote note = joyNote[i];
                    DWORD button = note.button;
                    if (!button) // terminator found
                        break;

                    if (button & changed)
                    {
                        me.midiData[0] = MIDI_NOTEON + channel;
                        me.midiData[1] = note.key;
                        me.midiData[2] = (button & newButtons) ? note.vel : 0x00; // velocity
                        outputs[0].push_back(me);
                    }
                }

                // handle program change
                {
                    SHORT delta = 0;
                    if (XINPUT_GAMEPAD_LEFT_SHOULDER & changed & newButtons)
                        delta = -1;
                    if (XINPUT_GAMEPAD_RIGHT_SHOULDER & changed & newButtons)
                        delta = 1;
                    if (XINPUT_GAMEPAD_START & changed & newButtons)
                        delta = -128;

                    if (delta)
                    {
                        progNum += delta;
                        if (progNum > 127)
                            progNum = 127;
                        if (progNum < 0)
                            progNum = 0;

                        // program change
                        me.midiData[0] = MIDI_PROGRAMCHANGE | channel;
                        me.midiData[1] = progNum & 0x7F;
                        me.midiData[2] = 0;
                        outputs[0].push_back(me);
                    }
                }


                // handle controllers
                SHORT y = state.Gamepad.sThumbLY / 4; // 14bit
                if ((y < 10) && (y > -10))
                    y = 0;
                y += (0x40<<7); // zero offset
                if (y != lastY)
                {
                    me.midiData[0] = MIDI_PITCHBEND + channel;
                    me.midiData[1] =  y       & 0x7F; // lsb
                    me.midiData[2] = (y >> 7) & 0x7F; //msb
                    outputs[0].push_back(me);
                }
                lastY = y;

            }

            lastButtons = newButtons;
        }
    }
}

//-----------------------------------------------------------------------------------------
