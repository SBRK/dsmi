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

#include <windows.h>
#include <mmsystem.h>
#include <winsock.h>

#include <set>
#include <string>

#include "settings.h"

using namespace std;

class Midi2Udp
{
	
	public:
		Midi2Udp();
		~Midi2Udp();
		
		bool go(int port);
		bool changePort(int port);
	
		void midiMessage(UINT wMsg, DWORD dwParam1, DWORD dwParam2);
		void midiSysExMessage(UINT wMsg, MIDIHDR *pmhdr, unsigned long timestamp);
	
		void broadcastMessage(unsigned char *msg, unsigned int len);
	
		void add_ip(string ip);
		
	private:
		bool initSeq(int port);	
		void freeSeq();
		
		bool initUdp();
		void closeUdp();
		
		void stop();
	
		bool prepareSysExBuffer(char *sysExBuffer, MIDIHDR *sysExMidiHeader);
	
		// Midi stuff
		unsigned char midimsg[MAX_MESSAGE_LENGTH];
		HMIDIIN midiIn;
		MIDIHDR midihdr;
		char sysExBuffers[N_SYSEX_BUFFERS][MAX_SYSEX_LENGTH];
		MIDIHDR sysExMidiHeaders[N_SYSEX_BUFFERS];
		
		// Network
		int sock;
		struct sockaddr_in dest;
		set<unsigned long> ds_ips;
		
		bool closing;
};

#endif
