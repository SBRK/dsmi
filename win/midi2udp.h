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
	
		void add_ip(string ip);
	
	private:
		bool initSeq(int port);	
		void freeSeq();
		
		bool initUdp();
		void closeUdp();
		
		void stop();
	
		// Midi stuff
		unsigned char midimsg[MIDI_MESSAGE_LENGTH];
		HMIDIIN midiIn;
		MIDIHDR midihdr;
		unsigned char SysExBuffer[256];
		
		// Network
		int sock;
		struct sockaddr_in dest;
		set<unsigned long> ds_ips;
};

#endif
