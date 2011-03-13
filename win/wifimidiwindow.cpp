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
#include "midiports.h"

#include <QMessageBox>
#include <QLabel>
#include <QString>

using namespace std;

WifiMidiWindow::WifiMidiWindow(QWidget *parent)
	:QWidget(parent), lbOutputPorts(0), lbInputPorts(0), midi2udp(0), udp2midi(0)
{
	setFixedSize(555, 140);
	setWindowTitle("DSMIDIWiFi");
	
	setWindowIcon(QIcon(":DSMIDIWIFI.ico"));
	
	QLabel *labelInputPorts = new QLabel(this);
	labelInputPorts->setText("Input ports");
	labelInputPorts->setGeometry(10, 10, 180, 20);
	labelInputPorts->show();
	
	QLabel *labelOutputPorts = new QLabel(this);
	labelOutputPorts->setText("Output ports");
	labelOutputPorts->setGeometry(200, 10, 180, 20);
	labelOutputPorts->show();
	
	lbInputPorts = new QListWidget(this);
	lbInputPorts->setGeometry(10, 35, 180, 90);
	lbInputPorts->setEnabled(true);
	lbInputPorts->show();
	
	vector<string> inports = getInputPorts();
	
	for(unsigned int i=0; i<inports.size(); ++i) {
		lbInputPorts->addItem(QString(inports.at(i).c_str()));
	}
	lbInputPorts->setCurrentRow(0);
	
	connect(lbInputPorts, SIGNAL(currentRowChanged(int)), this, SLOT(inputPortChanged(int)));
	
	lbOutputPorts = new QListWidget(this);
	lbOutputPorts->setGeometry(200, 35, 180, 90);
	lbOutputPorts->setEnabled(true);
	lbOutputPorts->show();
	
	vector<string> outports = getOutputPorts();
	for(unsigned int i=0; i<outports.size(); ++i) {
		lbOutputPorts->addItem(QString(outports.at(i).c_str()));
	}
	lbOutputPorts->setCurrentRow(0);
	
	connect(lbOutputPorts, SIGNAL(currentRowChanged(int)), this, SLOT(outputPortChanged(int)));
	
	// Quit if no ports exist
	if( (inports.size() == 0) && (outports.size() == 0) )
	{
		QMessageBox::critical(0, "Problem!", "No MIDI input or output ports found. Exiting.");
		exit(1);
	}
	
	QLabel *label_logo = new QLabel(this);
	label_logo->setGeometry(400, 50, 143, 61);
	label_logo->setPixmap(QPixmap(":/images/logo.png"));
	label_logo->show();
	
	
	
	int port = lbInputPorts->currentRow();
	
	midi2udp = new Midi2Udp();
	bool res = midi2udp->go(port);
	
	if(res == true) {
		printf("midi2udp inited\n");
	} else {
		QMessageBox::critical(0, "Error!", "Error initing MIDI2UDP!");
		
		delete midi2udp;
		midi2udp = 0;
	}
	
	
	port = lbOutputPorts->currentRow();
	
	udp2midi = new Udp2Midi();
	printf("udp2midi created\n");
	
	res = udp2midi->go(port, midi2udp);
	
	if(res == true) {
		printf("udp2midi inited\n");
	} else {
		QMessageBox::critical(0, "Error!", "Error initing UDP2MIDI!");
		
		delete udp2midi;
		udp2midi = 0;
	}
}

WifiMidiWindow::~WifiMidiWindow()
{
	printf("closing\n");
	delete midi2udp;
	delete udp2midi;
}

void WifiMidiWindow::setInputPort(int port) {
	lbInputPorts->setCurrentRow(port - 1);
}

void WifiMidiWindow::setOutputPort(int port) {
	lbOutputPorts->setCurrentRow(port - 1);
}

void WifiMidiWindow::inputPortChanged(int port)
{
	if(midi2udp != 0) {
		bool res = midi2udp->changePort(port);
		if(res == true) {
			printf("Input port changed to %d\n", port + 1);
		} else {
			QMessageBox::critical(0, "Error!", "Error changing input port!");
		}
	}
}

void WifiMidiWindow::outputPortChanged(int port)
{
	if(udp2midi != 0) {
		bool res = udp2midi->changePort(port);
		if(res == true) {
			printf("Output port changed to %d\n", port+1);
		} else {
			QMessageBox::critical(0, "Error!", "Error changing output port!");
		}
	}
}
