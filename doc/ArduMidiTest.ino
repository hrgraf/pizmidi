// -----------------------------------------------------------------------------
// Arduino MIDI Test by H.R.Graf
// - received Note On/Off control built-in LED and are sent back on channel 2
//   with slight change in pitch (to demonstrate active functionality)
// - using Baudrate of 115200 over USB
// - compatible to "Hairless MIDI to Serial Bridge"
// - compatible to my pizmidi/midiUartBridge (recommended!)
// -----------------------------------------------------------------------------

#include <MIDI.h>

struct CustomBaud : public midi::DefaultSettings{
    static const long BaudRate = 115200; // e.g. Hairless MIDI to Serial Bridge
};
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, CustomBaud);

// -----------------------------------------------------------------------------

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
    // Do whatever you want when a note is pressed.

    digitalWrite(LED_BUILTIN, HIGH);
    MIDI.sendNoteOn(pitch+1, velocity, 2);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
    // Do something when the note is released.
    // Note that NoteOn messages with 0 velocity are interpreted as NoteOffs.
    
    digitalWrite(LED_BUILTIN, LOW);
    MIDI.sendNoteOff(pitch+1, velocity, 2);
}

// -----------------------------------------------------------------------------

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.turnThruOff();
}

void loop()
{
    MIDI.read();
}

// -----------------------------------------------------------------------------
