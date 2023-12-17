# midiprint
A Mac OS X command line tool for displaying current MIDI devices and messages on the system.

## compiling
cd Midiprint

sudo clang -framework Foundation -framework CoreMidi main.c midi_util.c -o midiprint

## running
./midiprint


