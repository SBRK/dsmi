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

#ifndef _UDP2MIDITHREAD_
#define _UDP2MIDITHREAD_

#include <QThread>
#include <QMutex>
#include <QtNetwork>

#include <alsa/asoundlib.h>

#include "settings.h"

#include "midi2udpthread.h"

class Udp2MidiThread: public QThread
{
	Q_OBJECT
	
	public:
		Udp2MidiThread(QObject *parent = 0);
		~Udp2MidiThread();
		
		bool go(Midi2UdpThread *midi2udp);
		
	protected:
        void run();
		
	private:
		bool initSeq();
		void freeSeq();
		
		Midi2UdpThread *midi2udp;
		
		// Thread stuff
		QMutex mutex;
		bool abort;
		
		// Midi stuff
                unsigned char midimsg[MAX_MIDI_MESSAGE_LENGTH];
		snd_seq_t *seq_handle;
		int midi_out_port;
		snd_midi_event_t *eventparser;
		snd_seq_event_t *midi_event;
		
};

#endif
