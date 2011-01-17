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

static void	MIDIReceiveCallback(const MIDIPacketList *pktlist, void *refCon, void *connRefCon)
{
	Midi2Udp *midi2udp = (Midi2Udp*)refCon;
	
	MIDIPacket *packet = (MIDIPacket*)pktlist->packet;
	
	for (unsigned int j = 0; j < pktlist->numPackets; ++j)
	{
		midi2udp->midiMessage(packet->data, packet->length);

		packet = MIDIPacketNext(packet);
	}
}

Midi2Udp::Midi2Udp()
	:midi_client(NULL), midi_dest(NULL)
{
	
}

Midi2Udp::~Midi2Udp()
{
	stop();
}

bool Midi2Udp::go()
{
	// Initialize Network
	if(!initUdp())
	{
		printf("midi2udp: udp init failed!\n");
		return false;
	}

	// Initialize MIDI port
	if (!initSeq())
	{
		printf("midi2udp: seq init failed!\n");
		closeUdp();
		return false;
	}
	
	return true;
}

bool Midi2Udp::initSeq()
{
	OSStatus status;
	status = MIDIClientCreate(CFSTR("DSMIDIWiFi"), NULL, NULL, &midi_client);
	if(status != 0) {
		printf("midi2udp: Error making MIDI client!\n");
		return false;
	}
	
	status = MIDIDestinationCreate(midi_client, CFSTR("DSMIDIWiFi-in"), MIDIReceiveCallback, this, &midi_dest);
	if(status != 0) {
		printf("midi2udp: Error making MIDI destination!\n");
		MIDIClientDispose(midi_client);
		return false;
	}
	
	return true;
}

void Midi2Udp::freeSeq()
{
	MIDIEndpointDispose(midi_dest);
	MIDIClientDispose(midi_client);
	midi_dest = NULL;
	midi_client = NULL;
}

bool Midi2Udp::initUdp()
{
	sock = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in sin;

	sin.sin_family = AF_INET;
	sin.sin_port = htons(PC_SENDER_PORT);
	sin.sin_addr.s_addr = INADDR_ANY;
	
	int res = bind(sock, (struct sockaddr*)&sin, sizeof(sin));

	if(res == -1) {
		printf("midi2udp: Failed to bind to port %d!\n", PC_PORT);
		return false;
	}
	
	dest.sin_family = AF_INET;
	dest.sin_port = htons(DS_PORT);
	
	return true;
}

void Midi2Udp::closeUdp()
{
	int res = close(sock);
	if(res != 0) {
		printf("midi2udp: Error closing socket!\n");
	}
}

void Midi2Udp::stop()
{
	freeSeq();
	closeUdp();
}

void Midi2Udp::midiMessage(const Byte *msg, size_t length)
{
	printf("midi2udp: Got midi msg: ");
	for(size_t i=0; i<length; ++i) {
		printf("0x%x ", msg[i]);
	}
	printf("\n");
	
	for(set<unsigned long>::iterator ip_it = ds_ips.begin(); ip_it != ds_ips.end(); ++ip_it)
	{
		dest.sin_addr.s_addr = *ip_it;
		
		int res = sendto(sock, (char*)msg, length, 0, (struct sockaddr*)&dest, sizeof(dest));
	
		if( res == -1) {
			printf("midi2udp: Error sending! (%s)\n", strerror(errno));
		}
	}
}

void Midi2Udp::add_ip(string ip)
{
	ds_ips.insert(inet_addr(ip.c_str())); // Duplicates won't be added
}
