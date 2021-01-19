# pizmidi
Simple MIDI tools based on VST2 SDK and pizmidi framework/examples

## midiProgramChange for Arturia MIDI keyboards
This is a MIDI CC to program change converter for Arturia MIDI keyboards.
While their MIDI keyboards allow user configurations, it seems it is not possible to generate standard MIDI program changes.

This VST plug-in works with the Arturia default "Preset" knob configuration:
  * rotating the "Preset" knob sends program changes instead
  * pressing down the "Preset" knob changes to the first program (reset)
  * all other MIDI events get passed thru

No configuration needed, no GUI, just wire in your VST host this VST plug-in after the MIDI keyboard.

(The expected MIDI CC ID for PresetEnc is 114, for PresetBtn the ID is 115. This is the default configuration on my Arturia Laboratory MIDI keyboard.)

