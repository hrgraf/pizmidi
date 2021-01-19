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

(The expected MIDI CC ID for PresetEnc is 114, for PresetBtn the ID is 115. This is the default configuration at least on my Arturia Laboratory MIDI keyboard.)

