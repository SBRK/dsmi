/*
  Functions for the ARM7 to process the commands from the ARM9.Based
  on code from the MOD player example posted to the GBADEV forums.
  Chris Double (chris.double@double.co.nz)
  http://www.double.co.nz/nintendo_ds
*/
#include <nds.h>

#include "../../generic/command.h"
#include "psg.h"
#include "linear_freq_table.h"

static void RecvCommandMidiMsg(MidiMsgCommand* mm)
{
	psg_midimsg(mm->msg, mm->data1, mm->data2);
}

void CommandProcessCommands()
{
	static int currentCommand = 0;
	while(currentCommand != commandControl->currentCommand)
	{
		Command* command = &commandControl->command[currentCommand];
		
		if(command->destination == DST_ARM7) {
		
			switch(command->commandType) {
				case MIDI_MSG:
					RecvCommandMidiMsg(&command->midimsg);
					break;
				default:
					break;
			}
		
		}
		currentCommand++;
		currentCommand %= MAX_COMMANDS;
	}
}
