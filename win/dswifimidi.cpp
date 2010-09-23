// DSMIDIWiFi Server - Forwards MIDI messages between local appliations
// and Nintendo DS handhelds in the local network.
//
// This is part of DSMI - DS Music Interface - http://dsmi.tobw.net
//
// Copyright (C) 2007 Tobias Weyand (me@dsmi.tobw.net)
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include <QApplication>

#include "wifimidiwindow.h"
#include "midiports.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void syntax() {
	printf("Syntax:\n");
	printf("    List available ports:\n        DSMIDIWiFi.exe -list\n\n");
	printf("    Launch with specific input/output ports:\n        DSMIDIWiFi.exe -in <in-port> -out <out-port>\n");
}

int main(int argc, char *argv[])
{
	unsigned int inport = 0, outport = 0;
	bool inport_set = false, outport_set = false;
	
	if( (argc == 2) && ( (strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0) ) ) {
		syntax();
		exit(0);
	} else if( (argc == 2) && (strcmp(argv[1], "-list") == 0) ) {
		printf("\n");
		vector<string> inports = getInputPorts();
		if(inports.size() > 0) {
			printf("Input ports:\n");
			for(unsigned int i=0; i<inports.size(); ++i) {
				printf("    %d: %s\n", i+1, inports[i].c_str());	
			}
		} else {
			printf("No input ports found.\n");
		}
		printf("\n");
		
		vector<string> outports = getOutputPorts();
		if(outports.size() > 0) {
			printf("Output ports:\n");
			for(unsigned int i=0; i<outports.size(); ++i) {
				printf("    %d: %s\n", i+1, outports[i].c_str());	
			}
		} else {
			printf("No output ports found.\n");
		}
		
		exit(0);
    } else if(argc > 1) {
		for(int i=1; i<argc; ++i) {
			if(strcmp(argv[i], "-in") == 0) {
				if(argc == i+1) {
					printf("Expected to see an input port number after -in.\n\n");
					syntax();
					exit(0);
				}
				inport = atoi(argv[i+1]);
				inport_set = true;
				i++;
			} else if(strcmp(argv[i], "-out") == 0) {
				if(argc == i+1) {
					printf("Expected to see an output port number after -out.\n\n");
					syntax();
					exit(0);
				}
				outport = atoi(argv[i+1]);
				outport_set = true;
				i++;
			} else {
				printf("Unknown argument: %s\n\n", argv[i]);
				syntax();
				exit(0);
			}
		}
	}

	if(outport_set) {
		vector<string> outports = getOutputPorts();
		
		if(outport < 1) {
			printf("Outport must be greater than 0.\n");
			exit(0);
		}
		if(outport > outports.size()) {
			printf("You selected outport %u, but only %u output ports are available!\n", outport, outports.size());	
			exit(0);
		}
	}

	if(inport_set) {
		vector<string> inports = getInputPorts();
		
		if(inport < 1) {
			printf("Inport must be greater than 0.\n");
			exit(0);
		}
		if(inport > inports.size()) {
			printf("You selected inport %u, but only %u input ports are available!\n", inport, inports.size());	
			exit(0);
		}
	}

	QApplication app(argc, argv);
	WifiMidiWindow wmw; // Create main window
	
	if(inport_set) {
		wmw.setInputPort(inport);
	}
	
	if(outport_set) {
		wmw.setOutputPort(outport);
	}
	
	wmw.setWindowFlags(Qt::WindowMinimizeButtonHint); // Disable maximize button
    wmw.show();
    return app.exec();
}
