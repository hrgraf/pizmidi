# pizmidi
Simple and free MIDI plug-ins based on VST2 SDK and pizmidi framework/examples.
Without dedicated GUI, but parameters can get edited from the VST host.

Currently includes:
  * midiUnifyChannel (a simple template for new modules)
  * midiProgramChange for Arturia MIDI keyboards
  * midiFromJoystick (translates XInput to MIDI)
  * midiUartBridge for Arduino

## midiUnifyChannel 
A simple template for new VST plug-ins. 
Maps all incoming MIDI messages to the selected channel.

## midiProgramChange for Arturia MIDI keyboards
This is a program change VST plug-in for Arturia MIDI keyboards. 
While their MIDI keyboards allow user configurations, it seems that it is not possible to generate standard MIDI program changes.

You might be able to remap MIDI events in your VST Host (DAW), but as the "Preset" knob only sends relative changes, it is not so trivial.
This VST plug-in maps the default Arturia MIDI CC commands of the "Preset" knob to MIDI program changes while keeping track of the absolute program number.

How to use/operate:
  * rotating the "Preset" knob sends program changes instead
  * pressing down the "Preset" knob changes to the first program (reset)
  * all other MIDI events get passed thru

No configuration needed, no GUI, just wire in your VST host this VST plug-in after the Arturia MIDI keyboard.

But what if 128 programs are not enough for you? You need bank select commands. Fortunately, the Arturia MIDI keyboard has a "Category" knob.
So this VST plug-in maps also the default Arturia MIDI CC commands of the "Category" knob to MIDI bank select commands (7-bit LSB) while keeping track of the absolute bank number.

(The expected MIDI CC ID for CategoryEnc/CategoryBtn/PresetEnc/PresetBtn are 112/113/114/115. This is the default configuration at least on my Arturia Laboratory MIDI keyboard.)

## midiFromJoystick (translates XInput to MIDI)
Use your Joystick/Gamepad/Game controller to generate MIDI events (Note On/Off, Pitch Bench, Sustain, Portamento, Expression Controller, Program Change, Bank Select),
directly in your DAW / VST host.

Proof-of-concept for Xbox360 controllers (XInput) with a hardwired mapping: 
  * play notes with buttons A/B/X/Y plus the DPAD (8 notes from C4 to C5)
  * pitch bend with left thumb stick Y-axis
  * program changes with left shoulder / right shoulder / start button  (to decrement/increment/reset)
  
If your Joystick/Gamepad/Game controller is not XInput compatible, use the free and configurable XOutput tool.

## midiUartBridge for Arduino
This is a MIDI <-> USB <-> UART bridge implemented as VST2 plug-in.
A more robust alternative to tools like the commonly used "Hairless MIDI to Serial Bridge".

Using a virtual UART over USB, as typically used to program and communicate with Arduino Uno and similar devices,
it allows to send and receive MIDI events over USB, without any MIDI circuit/interface on the Arduino.

A simple example for an Arduino Uno, working out of the box, is provided [here](doc/ArduMidiTest.ino).
The example works also on Arduino Leonardo compatible boards (e.g. Pro Micro), but since they implement USB-MIDI
directly, the midiUartBridge or other hairy tools are not needed...

## Download / install / use
Download the Windows 10 VST2 plug-ins as either 32-bit or 64-bit DLL (binary) at https://github.com/hrgraf/pizmidi/releases.
Copy the DLL and the .ini file to your VST Plug-in directory (for 64-bit e.g. to C:\Program Files\VSTPlugins).
Use your favorite DAW, or any VST host, e.g. the free VSTHost or SAVIHost.
