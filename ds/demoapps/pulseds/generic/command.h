#if !defined(COMMAND_H)
#define COMMAND_H

#define DST_ARM7	0
#define DST_ARM9	1

#define DEBUGSTRSIZE 40

#include <nds.h>

/*
  Structures and functions to allow the ARM9 to send commands to the
  ARM7. Based on code from the MOD player example posted to the GBADEV
  forums.
*/

/* Enumeration of commands that the ARM9 can send to the ARM7 */
#define	MIDI_MSG 	0

typedef unsigned int	CommandType;

typedef struct
{
	unsigned char msg, data1, data2;
} MidiMsgCommand;

/* The ARM9 fills out values in this structure to tell the ARM7 what
   to do. */
typedef struct {
	u8 destination;
	CommandType commandType;
	union {
		void* data;
		MidiMsgCommand midimsg;
	};
} Command;

/* Maximum number of commands */
#define MAX_COMMANDS 40

/* A structure shared between the ARM7 and ARM9. The ARM9
   places commands here and the ARM7 reads and acts upon them.
*/
typedef struct {
	Command command[MAX_COMMANDS];
	int currentCommand;
	int return_data;
} CommandControl;

/* Address of the shared CommandControl structure */
#define commandControl ((CommandControl*)((uint32)(IPC) + sizeof(TransferRegion)))

#if defined(ARM9)
void CommandInit();
void CommandMidiMsg(unsigned char msg, unsigned char data1, unsigned char data2);
#endif

void CommandProcessCommands();

#if defined(ARM7)

#endif

#endif
