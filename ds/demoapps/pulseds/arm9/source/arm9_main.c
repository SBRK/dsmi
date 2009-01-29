/*****************
Pulse DS
         by 0xtob

 => MIDI in,
<= PSG sounds out.

License: GPL
******************/

#include <nds.h>
#include <stdio.h>
#include <libdsmi.h>

#include "dsmi_logo_ds.h"
#include "../../generic/midimsg.h"

void midiToArm7(u8 message, u8 data1, u8 data2)
{
	MidiMsg midimsg;
	midimsg.msg = message;
	midimsg.data1 = data1;
	midimsg.data2 = data2;
	fifoSendDatamsg(FIFO_MIDI, sizeof(midimsg), (u8*)&midimsg);
}

int main(void)
{
	lcdMainOnBottom();
	irqEnable(IRQ_VBLANK);
	
	soundEnable();
	
	vramSetBankC(VRAM_C_SUB_BG);
	
	videoSetModeSub(MODE_5_2D);
	PrintConsole *pc = consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 4, 0, false);
	BG_PALETTE_SUB[255] = RGB15(31,15,0);
	bgSetPriority(pc->bgId, 0);
	
	int bg2 = bgInitSub(2, BgType_Bmp16, BgSize_B16_256x256, 2, 0);
	bgSetPriority(bg2, 1);
	u16 *bg2vram = bgGetGfxPtr(bg2);
	
	dmaCopy(dsmi_logo_ds, bg2vram, 256*192);
	
	iprintf("\x1b[12;12HPulse DS\n");
	
	// Connect
	int res = dsmi_connect();
	while(res == 0) {
		iprintf("\x1b[15;0H\x1b[KCould not connect. We apologize for the inconvecience.\n");
		while(1);
	}
	
	iprintf("\x1b[15;0H\x1b[KRunning.\n");
	
	u8 message, data1, data2;
	while(1)
	{
		// We query every 60th of a second (notice the swiWaitForVBlank).
		// For lower latency you should query more frequently.
		
		// Query and play until there are no more new messages
		while(dsmi_read(&message, &data1, &data2)) {
			printf("0x%x 0x%x 0x%x\n", message, data1, data2);
			midiToArm7(message, data1, data2);
		}
		
		swiWaitForVBlank();
	}
	
	return 0;
}
