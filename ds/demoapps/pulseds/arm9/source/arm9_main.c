/*****************
Pulse DS
         by 0xtob

 => MIDI in,
<= PSG sounds out.

License: GPL
******************/

#include <nds.h>
#include <nds/arm9/console.h>

#include <stdio.h>

#include "libdsmi.h"

#include "dsmi_logo_ds.h"
#include "../../generic/command.h"

u16 *sub_vram = (u16*)BG_BMP_RAM_SUB(2);

int main(void)
{
	powerON(POWER_ALL);
	irqInit();
	
	lcdMainOnBottom();
	
	irqEnable(IRQ_VBLANK);
	
	videoSetMode(MODE_5_2D | DISPLAY_BG0_ACTIVE);
	videoSetModeSub(MODE_5_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG2_ACTIVE);
	vramSetBankC(VRAM_C_SUB_BG);
	
	// Init console
	SUB_BG0_CR = BG_MAP_BASE(4)| BG_TILE_BASE(0) | BG_PRIORITY(0);
	BG_PALETTE_SUB[255] = RGB15(31,15,0);
	consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(4), (u16*)CHAR_BASE_BLOCK_SUB(0), 16);
	
	// Gfx on sub
	SUB_BG2_CR = BG_BMP16_256x256 | BG_MAP_BASE(2) | BG_PRIORITY(1);
	SUB_BG2_XDX = 1 << 8;
	SUB_BG2_XDY = 0;
	SUB_BG2_YDX = 0;
	SUB_BG2_YDY = 1 << 8;
	
	u16 i;
	for(i=0; i<256*192; ++i)
		sub_vram[i] = dsmi_logo_ds[i];
	
	CommandInit();
	
	iprintf("\x1b[12;12HPulse DS\n");
	
	dsmi_setup_wifi_support();
	
	iprintf("\x1b[15;0H\x1b[KConnecting\n");
	
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
		
		// 1. Query and play until there are no more new messages
		while(dsmi_read(&message, &data1, &data2))
		{
			printf("0x%x 0x%x 0x%x\n", message, data1, data2);
			CommandMidiMsg(message, data1, data2); // Send the message to the arm7
		}
		
		// 2. Wait
		swiWaitForVBlank();
	}

	return 0;
}
