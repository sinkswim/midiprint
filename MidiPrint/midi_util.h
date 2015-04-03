//
//  midi_util.h
//  MidiPrint
//

#ifndef __MidiPrint__midi_util__
#define __MidiPrint__midi_util__

#include <stdio.h>

#include <CoreMIDI/CoreMIDI.h>

void midiInputCallback (const MIDIPacketList *list, void *procRef, void *srcRef);
CFStringRef getDisplayName(MIDIObjectRef object);
char* midiTypeToString(MIDIObjectType midiType);
char* midiErrorToString(OSStatus statusCode);

#endif /* defined(__MidiPrint__midi_util__) */
