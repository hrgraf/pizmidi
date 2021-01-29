// -----------------------------------------------------------------------------
// Arduino MIDI Test by H.R.Graf
//
// Works out-of-the-box on Arduino Uno / Leonardo compatible boards
// over USB (No MIDI DIN circuit needed)
//
// Received Note On/Off control built-in LED and are sent back on channel 2
// with slight change in pitch (to demonstrate active functionality).
//
// On the Arduino Uno, the serial interface is used for the USB communication,
// which on the PC is accessible as virtual COM port. There, some additional
// SW is required to translate between UART and MIDI.
// The baud rate of 115200 is compatible to both
// - Hairless MIDI to Serial Bridge
// - my pizmidi/midiUartBridge (recommended!)
//
// On the Arduino Leonardo, the USB MIDI functionality is built-in and the 
// serial interface is available for serial monitor / debug messages.
// -----------------------------------------------------------------------------

#define BAUD_RATE 115200

#if defined(ARDUINO_AVR_LEONARDO)
  #define LED_BLINK LED_BUILTIN
  //#define LED_BLINK 17 // RX LED on Pro Micro
  
  #define DEBUG_INIT(x) Serial.begin(x)
  #define DEBUG(x) Serial.print(x)
  
  #include <USB-MIDI.h>
  USBMIDI_CREATE_DEFAULT_INSTANCE();  
  //MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDICoreSerial);  
  
#else // USB <-> UART <-> MIDI
  #define LED_BLINK LED_BUILTIN
  
  #define DEBUG_INIT(x) // not available
  #define DEBUG(x)
  
  #include <MIDI.h>

  struct CustomBaud : public midi::DefaultSettings{
      static const long BaudRate = BAUD_RATE; // e.g. Hairless MIDI to Serial Bridge
  };
  MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, CustomBaud);
#endif

// -----------------------------------------------------------------------------

void handleNoteOn(byte channel, byte key, byte velocity)
{
    // Do whatever you want when a note is pressed.

    digitalWrite(LED_BLINK, velocity ? HIGH : LOW);
    MIDI.sendNoteOn(key+1, velocity, 2);
    DEBUG("NoteOn: ");
    DEBUG(key);
    DEBUG(" @ ");
    DEBUG(velocity);
    DEBUG("\n");
}

void handleNoteOff(byte channel, byte key, byte velocity)
{
    // Do something when the note is released.
    // Note that NoteOn messages with 0 velocity are interpreted as NoteOffs.
    
    digitalWrite(LED_BLINK, LOW);
    MIDI.sendNoteOff(key+1, velocity, 2);
    DEBUG("NoteOff: ");
    DEBUG(key);
    DEBUG(" @ ");
    DEBUG(velocity);
    DEBUG("\n");
}

// -----------------------------------------------------------------------------

void setup()
{
    DEBUG_INIT(BAUD_RATE);
    DEBUG("ArduMidiTest");
    
    pinMode(LED_BLINK, OUTPUT);
    digitalWrite(LED_BLINK, LOW);
    
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.turnThruOff();
    
    DEBUG("Ready");
}

void loop()
{
    MIDI.read();
}

// -----------------------------------------------------------------------------
