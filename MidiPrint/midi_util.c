//
//  midi_util.c
//  MidiPrint
//


#include "midi_util.h"
#include <CoreMIDI/CoreMIDI.h>

#define SYSEX_LENGTH 1024


//Print out the list of present endpoints
void printMIDIList(){
    ItemCount numOfDevices = MIDIGetNumberOfDevices();
    for (int i = 0; i < numOfDevices; i++) {
        MIDIDeviceRef midiDevice = MIDIGetDevice(i);
        CFStringRef deviceName = getDisplayName(midiDevice);
        ItemCount numSources = MIDIEntityGetNumberOfSources(midiDevice);
    
        CFShow(deviceName);
        printf("numSources=%lu\n", numSources);
    
        for (int j = 0; j < numSources; j++) {
            MIDIEndpointRef endpoint = MIDIEntityGetSource(midiDevice, j);
            CFStringRef endpointName = getDisplayName(endpoint);
            printf("endpoint: ");
            fflush(stdout);
            CFShow(endpointName);
        }
        printf("---------------\n");
    }
    printf("---------------\n");
}

//Set up active sources and their callback
void setupActiveMIDISources(){
    ItemCount numSources = MIDIGetNumberOfSources();
    printf("Number of active sources: %lu\n\n", numSources);
    MIDIClientRef midiClients[numSources];
    MIDIPortRef inputPorts[numSources];
    MIDIEndpointRef endpoints[numSources];
    OSStatus result;
    for(int i = 0; i < numSources; i++){
        result = MIDIClientCreate(CFSTR("MIDI client"), NULL, NULL, &midiClients[i]);
        if (result != noErr) {
            printf("Error creating MIDI client"); // %s - %s", GetMacOSStatusErrorString(result), GetMacOSStatusCommentString(result));
        }
        endpoints[i] = MIDIGetSource(i);
        result = MIDIInputPortCreate(midiClients[i], CFSTR("Input"), midiInputCallback, NULL, &inputPorts[i]);
        if (result != noErr) {
            printf("Error creating input port"); // %s - %s", GetMacOSStatusErrorString(result), GetMacOSStatusCommentString(result));
        }
    
        result = MIDIPortConnectSource(inputPorts[i], endpoints[i], &endpoints[i]);
        if (result != noErr) {
            printf("Error connecting port and source"); // %s - %s", GetMacOSStatusErrorString(result), GetMacOSStatusCommentString(result));
        }
    
    }
}

int min(int a,  int b){
    return (a>=b)?b:a;
}

//Called whenever there is incoming MIDI from connected sources, see doc for MIDIReadProc
void midiInputCallback (const MIDIPacketList *list, void *procRef, void *srcRef){
    printf("Message from: ");
    MIDIObjectRef midiObject = *(MIDIObjectRef*)srcRef;
    CFShow(getDisplayName(midiObject));
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
                
                // First byte is the status byte
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
                unsigned char messageChannel = (status & 0x0F) + 0x31;
                printf("on channel: %c\n",messageChannel);
                
                //Check http://www.midi.org/techspecs/midimessages.php for a list of midi messages
                switch (messageType) {
                    case 0x80:
                        printf("Note off: %d, %d\n", packet->data[iByte + 1], packet->data[iByte + 2]);
                        break;
                        
                    case 0x90:
                        printf("Note on: %d, %d\n", packet->data[iByte + 1], packet->data[iByte + 2]);
                        break;
                        
                    case 0xA0:
                        printf("Polyphonic key pressure (aftertouch): %d, %d\n", packet->data[iByte + 1], packet->data[iByte + 2]);
                        break;
                        
                    case 0xB0:
                        printf("Control change: %d, %d\n", packet->data[iByte + 1], packet->data[iByte + 2]);
                        break;
                        
                    case 0xC0:
                        printf("Program change: %d\n", packet->data[iByte + 1]);
                        break;
                        
                    case 0xD0:
                        printf("Channel pressure (aftertouch): %d\n", packet->data[iByte + 1]);
                        break;
                        
                    case 0xE0:
                        printf("Pitch bend change: %d, %d\n", packet->data[iByte + 1], packet->data[iByte + 2]);
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

