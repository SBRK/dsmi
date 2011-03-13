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

#include "midi2udp.h"

#include <QMessageBox>

static void CALLBACK midiin_callback(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	switch(wMsg) {
		case MIM_DATA: {
			Midi2Udp *midi2udp = (Midi2Udp*)dwInstance;
			
			midi2udp->midiMessage(wMsg, dwParam1, dwParam2);
			break;
		}
		
		case MIM_LONGDATA:
		case MIM_LONGERROR:
		{
			Midi2Udp *midi2udp = (Midi2Udp*)dwInstance;
			MIDIHDR *pmhdr = (MIDIHDR*)dwParam1;
			if(pmhdr->dwBytesRecorded == 0) {
				break;
			}
			midi2udp->midiSysExMessage(wMsg, pmhdr, dwParam2);
			if(wMsg == MIM_LONGERROR) {
				printf("SysEx message incomplete!\n");
			}
			break;
		}
		
		case MIM_OPEN: {
			printf("MIDI Input opened\n");
			break;
		}
		
		case MIM_CLOSE: {
			printf("MIDI Input closed\n");
			break;
		}
		
		default: {
			printf("got unhandled MIDI message\n");
			break;
		}
	}
}

Midi2Udp::Midi2Udp()
{

}

Midi2Udp::~Midi2Udp()
{
	stop();
}

bool Midi2Udp::go(int port)
{
	// Initialize Network
	if(!initUdp())
	{
		printf("udp init failed!\n");
		return false;
	}

	// Initialize MIDI port
	if (!initSeq(port))
	{
		printf("seq init failed!\n");
		closeUdp();
		return false;
	}
	
	return true;
}

bool Midi2Udp::changePort(int port)
{
	stop();
	return go(port);
}

bool Midi2Udp::initSeq(int port)
{
	closing = false;
	
	MMRESULT res = midiInOpen(&midiIn, port, (DWORD)midiin_callback, (DWORD)this, CALLBACK_FUNCTION | MIDI_IO_STATUS);
	
	if(res != MMSYSERR_NOERROR) {
		return false;
	}
	
	for(unsigned int i=0; i<N_SYSEX_BUFFERS; ++i) {
		res = prepareSysExBuffer(sysExBuffers[i], &sysExMidiHeaders[i]);
		if(!res) {
			printf("Error preparing SysEx Buffer\n");
			return false;
		}
	}
	
	res = midiInStart(midiIn);
	if(res != MMSYSERR_NOERROR) {
		return false;
	}
	
	return true;
}

void Midi2Udp::freeSeq()
{
	closing = true;
	midiInReset(midiIn);
	for(unsigned int i=0; i<N_SYSEX_BUFFERS; ++i) {
		midiInUnprepareHeader(midiIn, &sysExMidiHeaders[i], sizeof(MIDIHDR));
	}
	midiInClose(midiIn);
}

bool Midi2Udp::initUdp()
{
	WSADATA wsaData;
    WORD wVersionRequested;
    int err;
	
    wVersionRequested = MAKEWORD(2, 0);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        printf("Couldn't not find a usable WinSock DLL.n");
        return false;
    }
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in sin;

	sin.sin_family = AF_INET;
	sin.sin_port = htons(PC_SENDER_PORT);
	sin.sin_addr.s_addr = INADDR_ANY;
	
	int res = bind(sock, (struct sockaddr *)&sin, sizeof(sin));

	if(res == -1) {
		printf("Failed to bind to port %d!\n", PC_SENDER_PORT);
		return false;
	}
	
	dest.sin_family = AF_INET;
	dest.sin_port = htons(DS_PORT);
	
    u_long dir_bcast_addr = ~0;
	dest.sin_addr.s_addr = dir_bcast_addr;
	
	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);
	err = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&bOptVal, bOptLen);
	
	if(err != 0) {
		printf("Error enabling broadcast\n");
		return false;
	}
	
	return true;
}

void Midi2Udp::closeUdp()
{
	int res = closesocket(sock);
	if(res != 0) {
		printf("Error closing socket!\n");
	}
	WSACleanup();
}

void Midi2Udp::stop()
{
	freeSeq();
	closeUdp();
}

bool Midi2Udp::prepareSysExBuffer(char *sysExBuffer, MIDIHDR *sysExMidiHeader)
{
	sysExMidiHeader->lpData = sysExBuffer;
	sysExMidiHeader->dwBufferLength = MAX_SYSEX_LENGTH;
	sysExMidiHeader->dwFlags = 0;
	int res = midiInPrepareHeader(midiIn, sysExMidiHeader, sizeof(MIDIHDR));
	if(res != MMSYSERR_NOERROR) {
		printf("Error preparing SysEx Buffer\n");
		return false;
	}
	res = midiInAddBuffer(midiIn, sysExMidiHeader, sizeof(MIDIHDR));
	if(res != MMSYSERR_NOERROR) {
		printf("Error adding SysEx Buffer\n");
		return false;
	}
	return true;
}

void Midi2Udp::midiMessage(UINT wMsg, DWORD dwParam1, DWORD dwParam2)
{
	unsigned char *msg = (unsigned char*)&dwParam1;
	
	// MIDI Messages (non-SysEx) can be 2 or 3 bytes long). Unfortunately Windows
	// does not tell us the length of the message, so we have to determine the
	// length looking at the status byte.
	unsigned int len = 3;
	unsigned char status = msg[0];
	// Aftertouch
	if( ((status & (0xF0)) == 0xC0) || ((status & (0xF0)) == 0xC0) ) {
		len = 2;
	// Time code quarter frame || song select
	} else if( (status == 0xF1) || (status == 0xF3) ) {
		len = 2;
	// Undefined (Reserved) (x2) || Tune Request || End of Exclusive
	} else if( (status == 0xF4) || (status == 0xF5) || (status == 0xF6) || (status == 0xF7) ) {
		len = 1;
	// Stsem Real-Time Messages
	} else if(status >= 0xF8) {
		len = 1;
	}
	
	printf("got midi msg: %x ", msg[0]);
	for(unsigned int i=1; i<len; ++i) {
		printf("%x ", msg[i]);
	}
	printf("\n");
	
	broadcastMessage(msg, len);
}

void Midi2Udp::midiSysExMessage(UINT wMsg, MIDIHDR *pmhdr, unsigned long timestamp)
{
	size_t len = pmhdr->dwBytesRecorded;
	unsigned char *msg = (unsigned char*)pmhdr->lpData;
	printf("got midi sysex msg: ");
	for(unsigned int i=0; i<len; ++i) {
		printf("0x%x ", msg[i]);
	}
	printf("\n");
	
	broadcastMessage(msg, len);
	
	if(!closing) {
		prepareSysExBuffer(pmhdr->lpData, pmhdr);
	}
}

void Midi2Udp::broadcastMessage(unsigned char *msg, unsigned int len)
{
	for(set<unsigned long>::iterator ip_it = ds_ips.begin(); ip_it != ds_ips.end(); ++ip_it) {
		dest.sin_addr.s_addr = *ip_it;
		int res = sendto(sock, (char*)msg, len, 0, (struct sockaddr *)&dest, sizeof(dest));
		//printf("sent to %x\n", *ip_it);
		
		if( res == SOCKET_ERROR) {
			int err = WSAGetLastError();
			printf("Error sending: %d\n", err);
		}
	}
}

void Midi2Udp::add_ip(string ip)
{
	ds_ips.insert(inet_addr(ip.c_str())); // Duplicates won't be added
}
