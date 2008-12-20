/*
  Structures and functions to allow the ARM9 to send commands to the
  ARM7. Based on code from the MOD player example posted to the GBADEV
  forums.
  Chris Double (chris.double@double.co.nz)
  http://www.double.co.nz/nintendo_ds
*/
#include <nds.h>
#include <string.h>
#include <stdio.h>

#include "../../generic/command.h"

void CommandInit() {
	memset(commandControl, 0, sizeof(CommandControl));
}

void CommandMidiMsg(unsigned char msg, unsigned char data1, unsigned char data2)
{
	Command* command = &commandControl->command[commandControl->currentCommand];
	command->destination = DST_ARM7;
	MidiMsgCommand* mm = &command->midimsg;
	
	command->commandType = MIDI_MSG; 
	mm->msg = msg;
	mm->data1 = data1;
	mm->data2 = data2;
	
	commandControl->currentCommand++;
	commandControl->currentCommand %= MAX_COMMANDS;
}

void CommandProcessCommands()
{
	static int currentCommand = 0;
	while(currentCommand != commandControl->currentCommand) {
		Command* command = &commandControl->command[currentCommand];
		
		if(command->destination == DST_ARM9) {
		
			switch(command->commandType) {
				default:
					break;
			}
		}
			
		currentCommand++;
		currentCommand %= MAX_COMMANDS;
	}
}
