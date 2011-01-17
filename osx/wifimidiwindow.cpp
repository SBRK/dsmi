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

#include "wifimidiwindow.h"

#include <QPixmap>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>

WifiMidiWindow::WifiMidiWindow(QWidget *parent)
	:QWidget(parent)
{
	midi2udp = 0;
	udp2midi = 0;
	
	setFixedSize(163, 80);
	setWindowTitle("DSMI");
	
	setWindowIcon(QIcon(":/images/icon.icns"));
	
	QLabel *label_logo = new QLabel(this);
	label_logo->setGeometry(10, 10, 143, 61);
	label_logo->setPixmap(QPixmap(":/images/logo.png"));
	label_logo->show();
	
	
	midi2udp = new Midi2Udp();
	bool res = midi2udp->go();
	if(res == true) {
		printf("midi2udp initialized\n");
	} else {
		QMessageBox::critical(0, "Error!", "Error initializing midi2udp!");
		delete midi2udp;
		midi2udp = 0;
	}
	
	udp2midi = new Udp2Midi();
	res = udp2midi->go(midi2udp);
	if(res == true) {
		printf("udp2midi initialized\n");
	} else {
		QMessageBox::critical(0, "Error!", "Error initializing udp2midi!");
		delete udp2midi;
		udp2midi = 0;
	}
}


