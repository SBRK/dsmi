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

#include "udp2midithread.h"

#include <iostream>

Udp2MidiThread::Udp2MidiThread(QObject *parent)
	:QThread(parent)
{
	abort = false;
}

Udp2MidiThread::~Udp2MidiThread()
{
	mutex.lock();
	abort = true;
	mutex.unlock();
	wait();
	freeSeq();
}

bool Udp2MidiThread::go(Midi2UdpThread *midi2udp)
{
	this->midi2udp = midi2udp;
	
	// Initialize midi port
	bool res = initSeq();
	if(res == false) {
		return false;
	}
	
	// run thread
	if(!isRunning()) {
		start(LowPriority);
	}
	
	return true;
}

void Udp2MidiThread::run()
{
	QUdpSocket *udpSocket = new QUdpSocket();
	bool bres = udpSocket->bind(PC_PORT);
	if( bres == false ) {
		char msg[256];
		sprintf(msg, "Could not bind to port %d!\n", PC_PORT);
		printf(msg);
		return;
	}
	
	forever {
		
		if (abort) {
			delete udpSocket;
			
			return;
		}
		
		if( udpSocket->waitForReadyRead(250) == true ) {
			
			// Receive from UDP
			if( udpSocket->pendingDatagramSize() > MIDI_MESSAGE_LENGTH) {
				printf("udp2midi: received a message of %d bytes, but max length is %d byte\n", (int)udpSocket->pendingDatagramSize(), MIDI_MESSAGE_LENGTH);
			}
			
			QHostAddress from_address;
			int res = udpSocket->readDatagram((char*)midimsg, MIDI_MESSAGE_LENGTH, &from_address);
		
			if( res == -1 ) {
				printf("udp2midi: Error receiving data!\n");
			}
			
			if( (midimsg[0] == 0) && (midimsg[1] == 0) && (midimsg[2] == 0) ) {

				string from_ip = from_address.toString().toStdString();
			
				printf("Keepalive from: %s\n", from_ip.c_str());

				midi2udp->add_ip(from_ip);
			
			} else {

				// Send to MIDI
				printf("udp2midi: Sending event: 0x%x 0x%x 0x%x\n", midimsg[0], midimsg[1], midimsg[2]);
		
				res = snd_midi_event_encode(eventparser, midimsg, MIDI_MESSAGE_LENGTH, midi_event);
				
				if( res < 0) {
					printf("Error encoding midi event!\n");
				}
				
				snd_midi_event_reset_encode(eventparser);
				
				if(midi_event->type == SND_SEQ_EVENT_NOTEON) {
					printf("udp2midi: Note on: %d, channel %d\n", midi_event->data.note.note, midi_event->data.control.channel);
				} else if(midi_event->type == SND_SEQ_EVENT_NOTEOFF){
					printf("udp2midi: Note off: %d, channel %d\n", midi_event->data.note.note, midi_event->data.control.channel);
				}
				
				snd_seq_ev_set_subs(midi_event);
				snd_seq_ev_set_direct(midi_event);
				snd_seq_ev_set_source(midi_event, midi_out_port);
				
				snd_seq_event_output_direct(seq_handle, midi_event);
				
				snd_seq_free_event(midi_event);
			}
		}
	}
}

bool Udp2MidiThread::initSeq()
{
	if(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
    	printf("udp2midi: Error opening ALSA sequencer.\n");
    	return false;
  	}
	
	snd_seq_set_client_name(seq_handle, "DSMIDIWIFI UDP2MIDI");
	
	char portname[64] = "DSMIDIWIFI UDP2MIDI OUT";
	
	int res = midi_out_port = snd_seq_create_simple_port(seq_handle, portname, SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
              SND_SEQ_PORT_TYPE_APPLICATION);
	
	if(res < 0) {
		printf("udp2midi: Error creating MIDI port!\n");
		
		snd_seq_close(seq_handle);
		return false;
	}
	
	res = snd_midi_event_new(MIDI_MESSAGE_LENGTH, &eventparser);
	if(res != 0) {
		printf("udp2midi: Error making midi event parser!\n");
		
		snd_seq_close(seq_handle);
		return false;
	}
	snd_midi_event_init(eventparser);
	
	midi_event = (snd_seq_event_t*)malloc(sizeof(snd_seq_event_t));
	
	return true;
}

void Udp2MidiThread::freeSeq()
{
	int res = snd_seq_close(seq_handle);
	if( res < 0 ) {
		printf("udp2midi: Error closing socket!\n");
	}
}
