/*******************
 KAOS-DS
            by 0xtob

Source code licensed
under the GPL.
*******************/

#include <nds.h>
#include <nds/arm9/console.h>

#include <stdio.h>
#include <stdarg.h>

#include "libdsmi.h"

#include "dsmi_logo_ds.h"

#define PEN_DOWN (~IPC->buttons & (1 << 6))

touchPosition touch;
int touch_was_down = 0;

u8 channel = 0;

u16 *sub_vram = (u16*)BG_BMP_RAM_SUB(2);

void sgIP_dbgprint(char * txt, ...) {
        char buffer[256];
        va_list args;
        va_start(args,txt);
        vsprintf(buffer,txt,args);
        iprintf(buffer);
}

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
	touch = touchReadXY();
	
	if(!touch_was_down && PEN_DOWN) {
		touch_was_down = 1;
		kaos_event(touch.px, touch.py);
	}
	else if(touch_was_down && !PEN_DOWN)
	{
		touch_was_down = 0;
	}

	if(touch_was_down && PEN_DOWN) {
		kaos_event(touch.px, touch.py);
	}

	u16 keys = keysDown();
	
	if(keys & KEY_X)
	{
		kaos_x();
	}
	
	if(keys & KEY_Y)
	{
		kaos_y();
	}

	if(keys & KEY_UP)
	{
		if(channel < 15)
			channel++;
		
		printChannel();
	}
	
	if(keys & KEY_DOWN)
	{
		if(channel > 0)
			channel--;
		
		printChannel();
	}
}

int main(void)
{
	powerON(POWER_ALL);
	irqInit();
	
	lcdMainOnBottom();
	
	irqEnable(IRQ_VBLANK);

	// Set modes
	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG2_ACTIVE);
	
	// Set banks
	vramSetMainBanks(VRAM_A_MAIN_BG_0x06000000, VRAM_B_MAIN_BG_0x06020000,
		VRAM_C_SUB_BG_0x06200000 , VRAM_D_LCD);
	
	// Text bg on sub
	SUB_BG0_CR = BG_MAP_BASE(4) | BG_TILE_BASE(0) | BG_PRIORITY(0);
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
	
	iprintf("\x1b[12;12HKaos-DS\n");
	
	dsmi_setup_wifi_support();
	
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
	
	while(1)
	{
		VblankHandler();

		swiWaitForVBlank();
	}
	
}
