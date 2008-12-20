#ifndef _PSG_H_
#define _PSG_H_

#include <nds.h>

#define NOTE_ON 0x90  // note on is 0x9n where n is the MIDI Channel, data1 is note number, data2 is key velocity 
#define NOTE_OFF 0x80 // note off is 0x8n where n is the MIDI channel, data1 is note number, data2 is release velocity
#define MIDI_CC 0xB0  // midi CC is 0xBn where n is the MIDI channel, data1 is the control number, data2 is the control value 0-127

#define CS_INACTIVE	0
#define CS_ATTACK	1
#define CS_DECAY	2
#define CS_SUSTAIN	3
#define CS_RELEASE	4

#define DELTA_ATTACK	40
#define HEIGHT_ATTACK	127
#define DELTA_DECAY	40
#define HEIGHT_DECAY	32
#define LENGTH_SUSTAIN	15
#define DELTA_RELEASE	2

typedef struct
{
	// State variables (modified by the user)
	u8 vol; // Channel volume
	u8 duty;
	u32 freq;
	
	// Changing variables (modified by the program)
	u8 envelope; // Current envelope height
	u16 sustain; // Number of frames waited in sustain
	u8 state;
	
} ChannelState;

void psg_init();
void psg_update();
void psg_midimsg(u8 msg, u8 data1, u8 data2);
void psg_noteon(u8 channel, u8 note, u8 velocity);
void psg_noteoff(u8 channel);

#endif
