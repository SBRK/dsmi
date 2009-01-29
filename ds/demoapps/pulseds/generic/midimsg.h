#ifndef MIDIMSG_H
#define MIDIMSG_H

#define FIFO_MIDI FIFO_USER_01

typedef struct
{
	unsigned char msg, data1, data2;
} MidiMsg;

#endif
