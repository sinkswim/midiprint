//
//  midi_util.c
//  MidiPrint
//


#include "midi_util.h"
#include <CoreMIDI/CoreMIDI.h>

#define SYSEX_LENGTH 1024

int min(int a,  int b){
    return (a>b)?b:a;
}

//called whenever there is incoming MIDI from connected sources, see doc for MIDIReadProc
//pktlist
//The incoming MIDI message(s).
//
//readProcRefCon
//The refCon you passed to MIDIInputPortCreate or MIDIDestinationCreate
//
//srcConnRefCon
//A refCon you passed to MIDIPortConnectSource, which identifies the source of the data.
void midiInputCallback (const MIDIPacketList *list, void *procRef, void *srcRef){
    
    bool continueSysEx = false;
    UInt16 nBytes;
    const MIDIPacket *packet = &list->packet[0];
    unsigned char sysExMessage[SYSEX_LENGTH];
    unsigned int sysExLength = 0;
    for (unsigned int i = 0; i < list->numPackets; i++) {
        nBytes = packet->length;
        // Check if this is the end of a continued SysEx message
        if (continueSysEx) {
            unsigned int lengthToCopy = min(nBytes, SYSEX_LENGTH - sysExLength);
            // Copy the message into our SysEx message buffer,
            // making sure not to overrun the buffer
            memcpy(sysExMessage + sysExLength, packet->data, lengthToCopy);
            sysExLength += lengthToCopy;
            // Check if the last byte is SysEx End.
            continueSysEx = (packet->data[nBytes - 1] == 0xF7);
            if (!continueSysEx || sysExLength == SYSEX_LENGTH) {
                // We would process the SysEx message here, as it is we're just ignoring it
                
                sysExLength = 0;
            }
        } else {
            UInt16 iByte, size;
            
            iByte = 0;
            while (iByte < nBytes) {
                size = 0;
                
                // First byte should be status
                unsigned char status = packet->data[iByte];
                if (status < 0xC0) {
                    size = 3;
                } else if (status < 0xE0) {
                    size = 2;
                } else if (status < 0xF0) {
                    size = 3;
                } else if (status == 0xF0) {
                    // MIDI SysEx then we copy the rest of the message into the SysEx message buffer
                    unsigned int lengthLeftInMessage = nBytes - iByte;
                    unsigned int lengthToCopy = min(lengthLeftInMessage, SYSEX_LENGTH);
                    
                    memcpy(sysExMessage + sysExLength, packet->data, lengthToCopy);
                    sysExLength += lengthToCopy;
                    
                    size = 0;
                    iByte = nBytes;
                    
                    // Check whether the message at the end is the end of the SysEx
                    continueSysEx = (packet->data[nBytes - 1] != 0xF7);
                } else if (status < 0xF3) {
                    size = 3;
                } else if (status == 0xF3) {
                    size = 2;
                } else {
                    size = 1;
                }
                
                unsigned char messageType = status & 0xF0;
                unsigned char messageChannel = status & 0xF;
                
                switch (messageType) {
                    case 0x80:
                        printf("Note off: %d, %d\n", packet->data[iByte + 1], packet->data[iByte + 2]);
                        break;
                        
                    case 0x90:
                        printf("Note on: %d, %d\n", packet->data[iByte + 1], packet->data[iByte + 2]);
                        break;
                        
                    case 0xA0:
                        printf("Aftertouch: %d, %d\n", packet->data[iByte + 1], packet->data[iByte + 2]);
                        break;
                        
                    case 0xB0:
                        printf("Control change: %d, %d\n", packet->data[iByte + 1], packet->data[iByte + 2]);
                        break;
                        
                    case 0xC0:
                        printf("Program change: %d\n", packet->data[iByte + 1]);
                        break;
                        
                    case 0xD0:
                        printf("Change aftertouch: %d\n", packet->data[iByte + 1]);
                        break;
                        
                    case 0xE0:
                        printf("Pitch wheel: %d, %d\n", packet->data[iByte + 1], packet->data[iByte + 2]);
                        break;
                        
                    default:
                        printf("Uncommon message (0xF1 to 0xFF)\n");
                        break;
                }
                
                iByte += size;
            }
        }
        
        packet = MIDIPacketNext(packet);
    }
    printf("---------------------\n");
    
}

// Returns the display name of a given MIDIObjectRef as a CFStringRef
CFStringRef getDisplayName(MIDIObjectRef object){
    CFStringRef name = nil;
    OSStatus status = MIDIObjectGetStringProperty(object, kMIDIPropertyName, &name);
    if (status != noErr) {
        printf("failed to get display name: %s\n", midiErrorToString(status));
        return nil;
    }
    return name;
}

//decodes a MIDIObjectType
char* midiTypeToString(MIDIObjectType midiType) {
    switch (midiType) {
        case kMIDIObjectType_Other: return "other";
        case kMIDIObjectType_Device: return "device";
        case kMIDIObjectType_Entity: return "entity";
        case kMIDIObjectType_Source: return "source";
        case kMIDIObjectType_Destination: return "destination";
        default: return "unknown";
    }
}

//decodes an OSStatus
char* midiErrorToString(OSStatus statusCode) {
    switch (statusCode) {
        case kMIDIInvalidClient: return "invalid client";
        case kMIDIInvalidPort: return "invalid port";
        case kMIDIWrongEndpointType: return "wrong endpoint type";
        case kMIDINoConnection: return "no connection";
        case kMIDIUnknownEndpoint: return "unknown endpoint";
        case kMIDIUnknownProperty: return "unknown property";
        case kMIDIWrongPropertyType: return "wrong property type";
        case kMIDINoCurrentSetup: return "no current setup";
        case kMIDIMessageSendErr: return "message send err";
        case kMIDIServerStartErr: return "server start err";
        case kMIDISetupFormatErr: return "setup format err";
        case kMIDIWrongThread: return "wrong thread";
        case kMIDIObjectNotFound: return "object not found";
        case kMIDIIDNotUnique: return "ID not unique";
        case kMIDINotPermitted: return "not permitted";
        default: {
            char *formatted = (char*)calloc(256, 1);
            snprintf(formatted, 256, "UNKNOWN ERROR (%d)", statusCode);
            return formatted;
        }
    }
}

