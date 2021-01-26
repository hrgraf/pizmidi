/*-----------------------------------------------------------------------------
MidiUartBridge
original framework by Reuben Vinal
specific implementation by H.R.Graf
-----------------------------------------------------------------------------*/
#include "../common/PizMidi.h"
#include <stdlib.h>

enum
{
    kChannel,
    kComPort,
    kPower,

    kNumParams,
    kNumPrograms = 4
};

//-------------------------------------------------------------------------------------------------------

static short getMidiEvLen(short status)
{
    short len = 0;
    switch (status & 0xF0)
    {
    case MIDI_NOTEOFF:         len = 2; break;
    case MIDI_NOTEON:          len = 3; break;
    case MIDI_POLYKEYPRESSURE: len = 3; break;
    case MIDI_PROGRAMCHANGE:   len = 2; break;
    case MIDI_CHANNELPRESSURE: len = 2; break;
    case MIDI_PITCHBEND:       len = 3; break;
    }
    return len;
}

static void closeComPort(HANDLE hCom)
{
    if (hCom == INVALID_HANDLE_VALUE)
        return;

    CloseHandle(hCom);
}

static HANDLE openComPort(short nr)
{
    char name[10] = { 0 }; // com port id
    snprintf(name, sizeof(name), "\\\\.\\COM%d", nr);

    HANDLE hCom = ::CreateFile(name, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    if (hCom == INVALID_HANDLE_VALUE)
        return hCom;

    //Setting the Parameters for the SerialPort
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    GetCommState(hCom, &dcbSerialParams);

    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    SetCommState(hCom, &dcbSerialParams);

    //Setting Timeouts for non-blocking read
    COMMTIMEOUTS timeouts;
    memset((void *)&timeouts, 0, sizeof(timeouts));
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    SetCommTimeouts(hCom, &timeouts);

    return hCom;
}

static bool sendComPort(HANDLE hCom, char *msg, short msglen)
{
    DWORD len = 0;
    if ((!WriteFile(hCom, msg, msglen, &len, NULL)) || (len != msglen))
        return false;

    return true;
}

static short recvComPort(HANDLE hCom, char *msg, short maxlen)
{
    DWORD len = 0;
    if (!ReadFile(hCom, msg, maxlen, &len, NULL))
        return 0;

    return (short)len;
}



//-------------------------------------------------------------------------------------------------------
static short reqComPort = 0;

static short getComPortNr(float fComPort)
{
    short nr = 5; // fixme
    reqComPort = nr; // save
    return nr;
}

static char *getComPortName(float fComPort)
{
    static char buf[8];
    short nr = getComPortNr(fComPort);
    if (nr > 0)
        snprintf(buf, sizeof(buf), "COM%d", nr);
    else
        strcpy(buf, "NONE");
    return buf;
}

//-------------------------------------------------------------------------------------------------------
class MidiUartBridgeProgram {
    friend class MidiUartBridge;
public:
    MidiUartBridgeProgram();
    ~MidiUartBridgeProgram() {}
private:
    float fChannel;
    float fComPort;
    float fPower;
    char name[kVstMaxProgNameLen];
};

//-------------------------------------------------------------------------------------------------------
class MidiUartBridge : public PizMidi
{
public:
    MidiUartBridge(audioMasterCallback audioMaster);
    ~MidiUartBridge();

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
    float fComPort;
    float fPower;

    virtual void processMidiEvents(VstMidiEventVec *inputs, VstMidiEventVec *outputs, VstInt32 sampleFrames);

    MidiUartBridgeProgram *programs;

private:
    HANDLE hCom;
};


//-------------------------------------------------------------------------------------------------------
AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
    return new MidiUartBridge(audioMaster);
}

MidiUartBridgeProgram::MidiUartBridgeProgram()
{
    // default Program Values
    fChannel = 0.0f;
    fComPort = 1.0f;
    fPower = 1.0f;

    // default program name
    strcpy(name, "Default");

    // activate UART
    getComPortNr(fComPort);
}

//-----------------------------------------------------------------------------
MidiUartBridge::MidiUartBridge(audioMasterCallback audioMaster)
    : PizMidi(audioMaster, kNumPrograms, kNumParams), programs(0)
{
    hCom = INVALID_HANDLE_VALUE;

    programs = new MidiUartBridgeProgram[numPrograms];

    if (programs) {
        CFxBank* defaultBank = new CFxBank(kNumPrograms, kNumParams);
        if (readDefaultBank(PLUG_NAME, defaultBank)) {
            if ((VstInt32)defaultBank->GetFxID() == PLUG_IDENT) {
                for (int i = 0; i < kNumPrograms; i++) {
                    programs[i].fChannel = defaultBank->GetProgParm(i, 0);
                    programs[i].fComPort = defaultBank->GetProgParm(i, 1);
                    programs[i].fPower = defaultBank->GetProgParm(i, 2);
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
                    sprintf(programs[i].name, "Channel 10");
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
MidiUartBridge::~MidiUartBridge() {
    closeComPort(hCom);
    hCom = INVALID_HANDLE_VALUE;

    if (programs) 
        delete[] programs;
}

//------------------------------------------------------------------------
void MidiUartBridge::setProgram(VstInt32 program)
{
    MidiUartBridgeProgram* ap = &programs[program];

    curProgram = program;
    setParameter(kChannel, ap->fChannel);
    setParameter(kComPort, ap->fComPort);
    setParameter(kPower, ap->fPower);
}

//------------------------------------------------------------------------
void MidiUartBridge::setProgramName(char *name)
{
    vst_strncpy(programs[curProgram].name, name, kVstMaxProgNameLen);
}

//------------------------------------------------------------------------
void MidiUartBridge::getProgramName(char *name)
{
    strcpy(name, programs[curProgram].name);
}

//-----------------------------------------------------------------------------------------
bool MidiUartBridge::getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text)
{
    if (index < kNumPrograms)
    {
        strcpy(text, programs[index].name);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------------
void MidiUartBridge::setParameter(VstInt32 index, float value) {

    MidiUartBridgeProgram* ap = &programs[curProgram];

    switch (index) {
    case kChannel: fChannel = ap->fChannel = value; break;
    case kComPort: fComPort = ap->fComPort = value; break;
    case kPower:    fPower  = ap->fPower  = value;  break;
    }
}

//-----------------------------------------------------------------------------------------
float MidiUartBridge::getParameter(VstInt32 index) {
    float v = 0;

    switch (index) {
    case kChannel:   v = fChannel; break;
    case kComPort:   v = fComPort; break;
    case kPower:     v = fPower;   break;
    }
    return v;
}

//-----------------------------------------------------------------------------------------
void MidiUartBridge::getParameterName(VstInt32 index, char *label) {
    switch (index) {
    case kChannel:  strcpy(label, "Channel Out"); break;
    case kComPort:  strcpy(label, "COM Port");    break;
    case kPower:    strcpy(label, "Power");       break;
    }
}

//-----------------------------------------------------------------------------------------
void MidiUartBridge::getParameterDisplay(VstInt32 index, char *text) {
    switch (index) {
    case kChannel: sprintf(text, "%d", FLOAT_TO_CHANNEL015(fChannel) + 1); break;
    case kComPort: strcpy(text, getComPortName(fComPort));  break;
    case kPower:   strcpy(text, (fPower < 0.5f) ? "off" : "on"); break;
    }
}

//-----------------------------------------------------------------------------------------

void MidiUartBridge::processMidiEvents(VstMidiEventVec *inputs, VstMidiEventVec *outputs, VstInt32 sampleFrames)
{
    static short curComPort = 0;
    short uartChannel = (FLOAT_TO_CHANNEL015(fChannel) & 0x0F); // midi channel to send to uart

    if (reqComPort != curComPort) // change com port
    {
        if (curComPort) // close existing port
        {
            closeComPort(hCom);
            hCom = INVALID_HANDLE_VALUE;
            curComPort = 0;
        }

        if (reqComPort) // open requested port
        {
            hCom = openComPort(reqComPort);
            if (hCom == INVALID_HANDLE_VALUE)
            {
                VstMidiEvent me;
                memset(&me, 0, sizeof(me));
                me.midiData[0] = MIDI_NOTEOFF | uartChannel; // "Error Message"
                outputs[0].push_back(me);

            }
            else
                curComPort = reqComPort;
        }
    }

    // process incoming events (of first input)
    for (unsigned int i = 0; i < inputs[0].size(); i++) 
    {
        //copying event "i" from input (with all its fields)
        VstMidiEvent me = inputs[0][i];

        short status  = me.midiData[0] & 0xF0;  // scraping  channel
        short channel = me.midiData[0] & 0x0F;  // isolating channel (0-15)
        //short data1 = me.midiData[1] & 0x7F;
        //short data2 = me.midiData[2] & 0x7F;
        if ((channel == uartChannel) && (fPower >= 0.5f) && (hCom != INVALID_HANDLE_VALUE))
        {
            short len = getMidiEvLen(status);
            if (len > 0)
            {
                sendComPort(hCom, me.midiData, len);
            }
        }
    }

    // process incoming UART data
    static char recvBuf[10];
    static short recvPos = 0;
    
    if (recvPos < sizeof(recvBuf))
    {
        short len = recvComPort(hCom, &recvBuf[recvPos], sizeof(recvBuf) - recvPos);
        recvPos += len;
    }

    if (fPower >= 0.5f)
    {
        short i = 0;
        while (i < recvPos)
        {
            if (recvBuf[i] & 0x80) // status
            {
                short len = getMidiEvLen(recvBuf[i]);
                if (len > 0)
                {
                    if ((i + len) > recvPos)
                        break; // not enough data yet

                    VstMidiEvent me;
                    memset(&me, 0, sizeof(me));
                    for (int j = 0; j < len; j++)
                        me.midiData[j] = recvBuf[i + j];
                    outputs[0].push_back(me);

                    i += len;
                }
                else
                    i++; // skip
            }
            else
                i++; // skip
        }

        if (i == recvPos)
        {
            recvPos = 0; // all consumed
        }
        else if (i > 0) // partly consumed
        {
            recvPos -= i;

            // shift down
            for (int j = 0; j < recvPos; j++)
                recvBuf[j] = recvBuf[j + i];
        }
    }
    else // ignore recv data
    {
        recvPos = 0;
    }
}
