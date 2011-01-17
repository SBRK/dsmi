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

#ifndef _MIDI2UDP_
#define _MIDI2UDP_

#include <set>
#include <string>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <CoreMIDI/MIDIServices.h>
#include <CoreFoundation/CFRunLoop.h>

#include "settings.h"

using namespace std;

class Midi2Udp
{
	
	public:
		Midi2Udp();
		~Midi2Udp();
		
		bool go();
	
		void midiMessage(const Byte *msg, size_t length);
	
		void add_ip(string ip);
	
	private:
		bool initSeq();	
		void freeSeq();
		
		bool initUdp();
		void closeUdp();
		
		void stop();
	
		// Midi stuff
		unsigned char midimsg[MAX_MESSAGE_LENGTH];
		MIDIClientRef midi_client;
		MIDIEndpointRef midi_dest;
		
		// Network
		int sock;
		struct sockaddr_in dest;
		set<unsigned long> ds_ips;
};

#endif
