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
    
    //Print out the list of present endpoints
    printMIDIList();
    
    //Set up active sources and their callback
    setupActiveMIDISources();

    CFRunLoopRun();
    
    return 0;
}


