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

#include <QMessageBox>

#include <stdio.h>

Udp2Midi::Udp2Midi(QObject *parent)
	:QThread(parent)
{

}

Udp2Midi::~Udp2Midi()
{
	stop();
}

bool Udp2Midi::go(int port, Midi2Udp *midi2udp)
{
	this->midi2udp = midi2udp;
	
	// Initialize midi port
	bool res = initSeq(port);
	if(res == false) {
		return false;
	}
	
	abort = false;
	
	// run 
	if(!isRunning()) {
		printf("start!\n");
		start(LowPriority);
	}
	
	return true;
}

bool Udp2Midi::changePort(int port)
{
	stop();
	return go(port, midi2udp);
}

void Udp2Midi::run()
{
	QUdpSocket *udpSocket = new QUdpSocket();
	
    bool bres = udpSocket->bind(PC_PORT);
	
	if( bres == false ) {
		char msg[256];
		sprintf(msg, "Could not bind to port %d!", PC_PORT);
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
				printf("Error receiving data!\n");
				continue;
			}
			
			string from_ip = from_address.toString().toStdString();
			midi2udp->add_ip(from_ip);
			
			// Send to MIDI
			if((msglen == 3)&&(midimsg[0] == 0)&&(midimsg[1] == 0)&&(midimsg[2] == 0)) {
				printf("keepalive from %s\n", from_ip.c_str());
			} else {
				printf("Sending event\n");
		
				if( !sendMessage(midimsg, msglen) ) {
					printf("Error sending MIDI message\n");
				}
			}
		}
		
	}
}

bool Udp2Midi::initSeq(int port)
{
	MMRESULT res = midiOutOpen(&midiOut, port, 0, 0, CALLBACK_NULL);
	
	if( res != MMSYSERR_NOERROR ) {
		QMessageBox::critical(0, "Error!", "Error opening MIDI output.");
    	return false;
	}
	
	return true;
}

void Udp2Midi::freeSeq()
{
	midiOutClose(midiOut);
}

bool Udp2Midi::sendMessage(unsigned char *message, size_t length)
{
	printf("got udp msg: ");
	for(size_t i=0; i<length; ++i) {
		printf("0x%x ", message[i]);
	}
	printf("\n");
	
	union {
        DWORD dwData; 
        BYTE bData[4]; 
    } u; 
	u.dwData = 0;
	
	MMRESULT res;
	
	if(length <= 3) { // Short MIDI message
		for(int i=0; i<3; ++i) {
			u.bData[i] = message[i];
		}
		
		res = midiOutShortMsg(midiOut, u.dwData);
	} else { // Long MIDI message (SysEx)
		MIDIHDR mhdr;
		mhdr.lpData = (char*)message;
		mhdr.dwBufferLength = length;
		mhdr.dwBytesRecorded = length;
		mhdr.dwUser = 0;
		mhdr.dwFlags = 0;
		mhdr.dwOffset = 0;
		res = midiOutPrepareHeader(midiOut, &mhdr, sizeof(mhdr));
		if(res != MMSYSERR_NOERROR) {
			return false;
		}
		res = midiOutLongMsg(midiOut, &mhdr, sizeof(mhdr));
		if(res != MMSYSERR_NOERROR) {
			return false;
		}
		res = midiOutUnprepareHeader(midiOut, &mhdr, sizeof(mhdr));
	}
	
	if(res == MMSYSERR_NOERROR) {
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
