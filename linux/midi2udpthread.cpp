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

#include "midi2udpthread.h"

#include <arpa/inet.h>

Midi2UdpThread::Midi2UdpThread(QObject *parent)
	:QThread(parent)
{
	abort = false;
}

Midi2UdpThread::~Midi2UdpThread()
{
	mutex.lock();
	abort = true;
	mutex.unlock();
	wait();
	freeSeq();
}
	
bool Midi2UdpThread::go()
{
	// Initialize midi port
	bool res = initSeq();
	if(res == false) {
		return false;
	}
	
	// start expecing MIDI events
	npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
	pfd = (struct pollfd *)malloc(npfd * sizeof(struct pollfd));
	snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
	
	// run thread
	if(!isRunning()) {
		start(LowPriority);
	}
	
	return true;
}

void Midi2UdpThread::add_ip(string ip)
{
	ds_ips.insert(ip); // Duplicates won't be added
}

void Midi2UdpThread::run()
{
	QUdpSocket *udpSocket;
	udpSocket = new QUdpSocket(0);
	
	forever {
		
		if (abort) {
			delete udpSocket;
			
			return;
		}
		
		if (poll(pfd, npfd, 250) > 0) {
			
			printf("midi2udp: got midi event!\n");
			
			// Get MIDI event
			snd_seq_event_input(seq_handle, &midi_event);
			
			int res = snd_midi_event_decode(eventparser, midimsg, MIDI_MESSAGE_LENGTH, midi_event);
			
			if( res < 0 ) {
				printf("midi2udp: Error decoding midi event!\n");
			} else {
				// Send it over UDP
				for(set<string>::iterator ip_it = ds_ips.begin(); ip_it != ds_ips.end(); ++ip_it)
				{
					QString to_((*ip_it).c_str());
					QHostAddress to(to_);
					udpSocket->writeDatagram((char*)midimsg, MIDI_MESSAGE_LENGTH, to, DS_PORT);
				}
			}
			
			snd_seq_free_event(midi_event);
			
			snd_midi_event_reset_decode(eventparser);
		}
	}
}

bool Midi2UdpThread::initSeq()
{
	if(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0) < 0) {
    	printf("midi2udp: Error opening ALSA sequencer.\n");
    	return false;
  	}
	
	snd_seq_set_client_name(seq_handle, "DSMIDIWIFI MIDI2UDP");
	
	char portname[64] = "DSMIDIWIFI MIDI2UDP IN";
	
	int res = midi_in_port = snd_seq_create_simple_port(seq_handle, portname, SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
              SND_SEQ_PORT_TYPE_APPLICATION);
	
	if(res < 0) {
		printf("midi2udp: Error creating MIDI port!\n");
		
		snd_seq_close(seq_handle);
		return false;
	}
	
	res = snd_midi_event_new(MIDI_MESSAGE_LENGTH, &eventparser);
	if(res != 0) {
		printf("midi2udp: Error making midi event parser!\n");
		
		snd_seq_close(seq_handle);
		return false;
	}
	snd_midi_event_init(eventparser);
	
	midi_event = (snd_seq_event_t*)malloc(sizeof(snd_seq_event_t));
	
	return true;
}

void Midi2UdpThread::freeSeq()
{
	int res = snd_seq_close(seq_handle);
	if( res < 0 ) {
		printf("midi2udp: Error closing socket!\n");
	}
}
