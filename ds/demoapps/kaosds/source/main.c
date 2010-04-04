/*******************
 KAOS-DS
            by 0xtob

Source code licensed
under the GPL.

http://dsmi.tobw.net
*******************/

#include <nds.h>
#include <stdio.h>

#include <libdsmi.h>

#include "dsmi_logo_ds.h"

u16 *sub_vram = NULL;
u8 channel = 0;

void kaos_event(u8 x, u8 y)
{
	u8 kaos_x = x * 128 / 256;
	if(kaos_x > 127) kaos_x = 127;
	
	u8 kaos_y = (191 - y) * 128 / 192;
	if(kaos_y > 127) kaos_y = 127;
	
	dsmi_write(0xB0 | channel, 0x00, kaos_x);
	dsmi_write(0xB0 | channel, 0x01, kaos_y);
}

void kaos_x()
{
	dsmi_write(0xB0 | channel, 0x00, 0x00);
}

void kaos_y()
{
	dsmi_write(0xB0 | channel, 0x01, 0x00);
}

void printChannel()
{
	iprintf("\x1b[15;0H\x1b[KUsing MIDI channel %u\n", channel);
}

void VblankHandler()
{
	scanKeys();
	u16 keysdown = keysDown();
	u16 keysheld = keysHeld();
	touchPosition touch;
	touchRead(&touch);
	
	if(keysheld & KEY_TOUCH) {
		kaos_event(touch.px, touch.py);
	}

	if(keysdown & KEY_X) {
		kaos_x();
	}
	
	if(keysdown & KEY_Y) {
		kaos_y();
	}

	if(keysdown & KEY_UP) {
		if(channel < 15)
			channel++;
		
		printChannel();
	}
	
	if(keysdown & KEY_DOWN) {
		if(channel > 0)
			channel--;
		
		printChannel();
	}
}

int main(void)
{
	lcdMainOnBottom();

	// Set banks
	vramSetMainBanks(VRAM_A_MAIN_BG_0x06000000, VRAM_B_MAIN_BG_0x06020000,
		VRAM_C_SUB_BG_0x06200000 , VRAM_D_LCD);

	// Set modes
	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);

	// Text bg on sub
	PrintConsole *pc = consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 4, 0, false, true);
	bgSetPriority(pc->bgId, 0);
	BG_PALETTE_SUB[255] = RGB15(31,15,0);

	// gfx on sub display
	int sub_bg2 = bgInitSub(2, BgType_Bmp16, BgSize_B16_256x256, 2, 0);
	bgSetPriority(sub_bg2, 1);
	sub_vram = (u16*)bgGetGfxPtr(sub_bg2);

	u16 i;
	for(i=0; i<256*192; ++i) {
		sub_vram[i] = dsmi_logo_dsBitmap[i] | BIT(15);
	}

	iprintf("\x1b[12;12HKaos-DS\n");
	
	iprintf("\x1b[15;0H\x1b[KConnecting\n");
	int res = dsmi_connect();
	
	if(res == 1) {
		iprintf("\x1b[15;0H\x1b[KOK\n");
	} else {
		iprintf("\x1b[15;0H\x1b[KOh no, could not connect!\n");
		while(1);
	}
	
	printChannel();
	
	iprintf("\nX sends a single X axis event.\n\
Y sends a single Y axis event.\n\
Up/down change MIDI channels");

	while(1) {
		VblankHandler();
		swiWaitForVBlank();
	}

	return 0;
}
