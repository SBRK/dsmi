#include "psg.h"

#include "linear_freq_table.h"

/*
This rather basic code that turns midi messages into PSG sounds.
It is meant as a starting point for making your own cool PSG clients.
*/

/*
  SCHANNEL_CR reference from GBAtek (always useful :-)

  Bit0-6    Volume       (0..127=silent..loud)
  Bit7      Not used     (always zero)
  Bit8-9    Data Shift   (0=Normal, 1=Div2, 2=Div4, 3=Div16)
  Bit10-14  Not used     (always zero)
  Bit15     Hold         (0=Nothing, 1=Hold)               (?)
  Bit16-22  Panning      (0..127=left..right) (64=half volume on both speakers)
  Bit23     Not used     (always zero)
  Bit24-26  Wave Duty    (0..7) ;HIGH=(N+1)*12.5%, LOW=(7-N)*12.5% (PSG only)
  Bit27-28  Repeat Mode  (0=Manual, 1=Loop Infinite, 2=One-Shot, 3=Prohibited)
  Bit29-30  Format       (0=PCM8, 1=PCM16, 2=IMA-ADPCM, 3=PSG/Noise)
  Bit31     Start/Status (0=Stop, 1=Start/Busy)
*/

#define SOUND_PSG_WAVE_DUTY(n)	((n)<<24)

ChannelState channels[8];

extern const u32 linear_freq_table[LINEAR_FREQ_TABLE_SIZE];

void psg_init()
{
	int i;
	for(i=0;i<8;++i) {
		channels[i].vol = 127;
		channels[i].envelope = 0;
		channels[i].duty = 0;
		channels[i].state = CS_INACTIVE;
		channels[i].freq = 0;
		channels[i].sustain = 0;
	}
}

void psg_update()
{
	u8 i;
	for(i=0;i<8;++i)
	{
		switch(channels[i].state)
		{
			case CS_ATTACK:
				channels[i].envelope += DELTA_ATTACK;
				if(channels[i].envelope >= HEIGHT_ATTACK) {
					channels[i].envelope = HEIGHT_ATTACK;
					channels[i].state = CS_DECAY;
				}
				break;
			
			case CS_DECAY:
				channels[i].envelope -= DELTA_DECAY;
				if(channels[i].envelope <= HEIGHT_DECAY) {
					channels[i].envelope = HEIGHT_DECAY;
					channels[i].state = CS_SUSTAIN;
					channels[i].sustain = 0;
				}
				break;
			
			case CS_SUSTAIN:
				channels[i].sustain++;
				if(channels[i].sustain == LENGTH_SUSTAIN) {
					channels[i].state = CS_RELEASE;
				}
				break;
			
			case CS_RELEASE:
				if(channels[i].envelope < DELTA_RELEASE) {
					channels[i].state = CS_INACTIVE;
					break;
				}
				channels[i].envelope -= DELTA_RELEASE;
				break;
			
			default:
				break;
		}
		
		u8 channel = i+8; // PSG channels start with 8
		u8 duty = i+1; // The duty rises with the channel index
		if(channels[i].state != CS_INACTIVE)
		{
			SCHANNEL_TIMER(channel) = SOUND_FREQ((int)channels[i].freq);
			SCHANNEL_CR(channel) =
					SOUND_VOL(channels[i].envelope)
					| SOUND_PAN(64)
					| SOUND_FORMAT_PSG
					| SOUND_PSG_WAVE_DUTY(duty)
					| SCHANNEL_ENABLE;
		}
	}
}

void psg_midimsg(u8 msg, u8 data1, u8 data2)
{
	u8 cmd = msg & 0xF0;
	u8 channel = msg & 0x0F;
	
	if(channel > 7)
		return;
	
	switch(cmd) {
		case NOTE_ON: {
			u8 note = data1;
			u8 velocity = data2;
			
			psg_noteon(channel, note, velocity);
			break;
		}
			
		case NOTE_OFF: {
			psg_noteoff(channel);
			break;
		}
	}
}

void psg_noteon(u8 channel, u8 note, u8 velocity)
{
	channels[channel].envelope = 0;
	channels[channel].state = CS_ATTACK;
	channels[channel].freq = linear_freq_table[note];
}

void psg_noteoff(u8 channel)
{
	if(channels[channel].state != CS_INACTIVE) {
		channels[channel].state = CS_RELEASE;
	}
}
