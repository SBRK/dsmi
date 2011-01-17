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

#include "udp2midi.h"

#include <string>
#include <iostream>

#include <QMessageBox>

#include <CoreMIDI/MIDIServices.h>
#include <CoreFoundation/CFRunLoop.h>

#include <stdio.h>

using namespace std;

Udp2Midi::Udp2Midi(QObject *parent)
	:QThread(parent), midi_client(NULL), midi_source(NULL)
{

}

Udp2Midi::~Udp2Midi()
{
	stop();
}

bool Udp2Midi::go(Midi2Udp *midi2udp)
{
	this->midi2udp = midi2udp;
	
	// Initialize midi port
	bool res = initSeq();
	if(res == false) {
		return false;
	}
	
	abort = false;
	
	// run 
	if(!isRunning()) {
		printf("udp2midi: start!\n");
		start(LowPriority);
	}
	
	return true;
}

void Udp2Midi::run()
{
	QUdpSocket *udpSocket = new QUdpSocket();
	
    bool bres = udpSocket->bind(PC_PORT);
	
	if( bres == false ) {
		char msg[256];
		sprintf(msg, "udp2midi: Could not bind to port %d!", PC_PORT);
		QMessageBox::critical(0, "Error!", msg);
		return;
	}
	
	forever {
		
		if (abort) {
			delete udpSocket;
			
			return;
		}
		
		if( udpSocket->waitForReadyRead(250) == true ) {
			
			// Receive from UDP
			size_t msglen = udpSocket->pendingDatagramSize();			
			QHostAddress from_address;
			int res = udpSocket->readDatagram((char*)midimsg, msglen, &from_address);
			
			if( res == -1 ) {
				printf("udp2midi: Error receiving data!\n");
				continue;
			}
			
			if( (msglen == 3) && (midimsg[0] == 0) && (midimsg[1] == 0) && (midimsg[2] == 0) ) {
			
				string from_ip = from_address.toString().toStdString();
				cout << "udp2midi: keepalive from " << from_ip << endl;
				midi2udp->add_ip(from_ip);
			
			} else {
			
				// Send to MIDI
				printf("udp2midi: Sending event\n");
		
				if( !sendMessage(midimsg, msglen) ) {
					printf("udp2midi: Error sending MIDI message\n");
				}
			}
		}
		
	}
}

bool Udp2Midi::initSeq()
{
	OSStatus status;
	status = MIDIClientCreate(CFSTR("DSMIDIWiFi"), NULL, NULL, &midi_client);
	if(status != 0) {
		printf("udp2midi: Failed to create MIDI client!\n");
		return false;
	}
	
	status = MIDISourceCreate(midi_client, CFSTR("DSMIDIWiFi-out"), &midi_source);
	if(status != 0) {
		printf("udp2midi: Failed to create MIDI source!\n");
		MIDIClientDispose(midi_client);
		return false;
	}
	
	return true;
}

void Udp2Midi::freeSeq()
{
	MIDIEndpointDispose(midi_source);
	MIDIClientDispose(midi_client);
}

bool Udp2Midi::sendMessage(unsigned char *message, int length)
{
	printf("udp2midi: got udp msg: ");
	for(int i=0; i<length; ++i) {
		printf("0x%x ", message[i]);
	}
	printf("\n");
		   
	
	Byte buffer[64];
	MIDIPacketList *pktlist = (MIDIPacketList *)buffer;
	MIDIPacket *curPacket = MIDIPacketListInit(pktlist);
		
	curPacket = MIDIPacketListAdd(pktlist, sizeof(buffer), curPacket, 0, length, message); 

	OSStatus status;
	status = MIDIReceived(midi_source, pktlist);
	
	if(status == 0) {
		return true;
	} else {
		return false;
	}
}

void Udp2Midi::stop()
{
	mutex.lock();
	abort = true;
	mutex.unlock();
	wait();
	freeSeq();
}
