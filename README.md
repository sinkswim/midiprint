# midiprint
A Mac OS X command line tool for displaying current MIDI devices and monitoring MIDI messages on the system.

## compiling
`cd Midiprint`

`sudo clang -framework Foundation -framework CoreMidi main.c midi_util.c -o midiprint`

## running
Within Midiprint folder:
`./midiprint`


