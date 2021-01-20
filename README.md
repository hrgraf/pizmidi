# pizmidi
Simple MIDI tools based on VST2 SDK and pizmidi framework/examples

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

Download the Windows 10 VST2 plug-in as either 32-bit or 64-bit DLL (binary) at https://github.com/hrgraf/pizmidi/releases.

