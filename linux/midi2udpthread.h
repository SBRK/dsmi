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

#ifndef _MIDI2UDPTHREAD_
#define _MIDI2UDPTHREAD_

#include <QThread>
#include <QMutex>
#include <QtNetwork>

#include <set>
#include <string>

#include <alsa/asoundlib.h>

#include "settings.h"

using namespace std;

class Midi2UdpThread: public QThread
{
	Q_OBJECT
	
	public:
		Midi2UdpThread(QObject *parent = 0);
		~Midi2UdpThread();
	
		bool go();
		
		void add_ip(string ip);
		
	protected:
		void run();
	
	private:
		bool initSeq();	
		void freeSeq();
		
		// Thread stuff
		QMutex mutex;
		bool abort;
	
		// Midi stuff
                unsigned char midimsg[MAX_MIDI_MESSAGE_LENGTH];
		snd_seq_t *seq_handle;
		int midi_in_port;
		snd_midi_event_t *eventparser;
		snd_seq_event_t *midi_event;
		
		int npfd;
		struct pollfd *pfd;
		
		set<string> ds_ips;
};

#endif
