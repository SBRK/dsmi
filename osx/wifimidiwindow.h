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

#ifndef _WIFIMIDIWINDOW_
#define _WIFIMIDIWINDOW_

#include <QWidget>

#include "midi2udp.h"
#include "udp2midi.h"

class WifiMidiWindow: public QWidget
{
	Q_OBJECT
	
	public:
		WifiMidiWindow(QWidget *parent = 0);
		
	private:
		Midi2Udp *midi2udp;
		Udp2Midi *udp2midi;
};

#endif
