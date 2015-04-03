//
//  main.c
//  MidiPrint
//

#include <stdio.h>
#include <stdlib.h>
#include <CoreServices/CoreServices.h>
#include <CoreMIDI/CoreMIDI.h>
#include "midi_util.h"



int main(int argc, const char * argv[]) {
    printf("Welcome to MidiPrint\n\n");
    
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
    
    /////
    ItemCount numSources = MIDIGetNumberOfSources();
    printf("number of sources: %lu\n\n", numSources);
    MIDIClientRef midiClients[numSources];
    MIDIPortRef inputPorts[numSources];
    MIDIEndpointRef endpoints[numSources];
    OSStatus result;
    for(int i = 0; i < numSources; i++){
        result = MIDIClientCreate(CFSTR("MIDI client"), NULL, NULL, &midiClients[i]);
        if (result != noErr) {
            printf("Error creating MIDI client: %s - %s", GetMacOSStatusErrorString(result), GetMacOSStatusCommentString(result));
            return 1;
        }
        result = MIDIInputPortCreate(midiClients[i], CFSTR("Input"), midiInputCallback, NULL, &inputPorts[i]);
        endpoints[i] = MIDIGetSource(i);
        result = MIDIPortConnectSource(inputPorts[i], endpoints[i], NULL);
        if (result != noErr) {
            printf("Error connecting port and source: %s - %s", GetMacOSStatusErrorString(result), GetMacOSStatusCommentString(result));
            return 1;
        }
        
    }
    //////


    CFRunLoopRun();
    
    return 0;
}


