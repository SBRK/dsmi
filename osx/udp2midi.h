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

#ifndef _UDP2MIDI_
#define _UDP2MIDI_

#include <QThread>
#include <QMutex>
#include <QtNetwork>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <CoreMIDI/MIDIServices.h>
#include <CoreFoundation/CFRunLoop.h>

#include <vector>
#include <string>

#include "settings.h"

#include "midi2udp.h"

class Udp2Midi: public QThread
{
	Q_OBJECT
	
	public:
		Udp2Midi(QObject *parent = 0);
		~Udp2Midi();
		
		bool go(Midi2Udp *midi2udp);
		
	protected:
        void run();
		
	private:
		bool initSeq();	
		void freeSeq();
		bool sendMessage(unsigned char *message, int length);
	
		void stop();
	
		Midi2Udp *midi2udp;
	
		// Thread stuff
		QMutex mutex;
		bool abort;
		
		// Midi stuff
		unsigned char midimsg[MAX_MESSAGE_LENGTH];
		MIDIClientRef midi_client;
		MIDIEndpointRef midi_source;
		
};

#endif
