/*******************
  dsmi OSC example
            by fishuyo

Source code licensed
under the GPL.
*******************/

#include <nds.h>
#include <nds/arm9/console.h>

#include <stdio.h>
#include <stdarg.h>

#include "libdsmi.h"

#include "dsmi_logo_ds.h"

touchPosition touch;
int touch_was_down = 0;
int kaos_touch = 0, kaos_x = 85, kaos_y = 85, old_kaos_x = 85, old_kaos_y = 85;
int slider_touch1 = 0, slider_val1 = 160, old_slider_val1 = 160;
int slider_touch2 = 0, slider_val2 = 160, old_slider_val2 = 160;
int slider_touch3 = 0, slider_val3 = 160, old_slider_val3 = 160;

u16 *sub_vram = (u16*)BG_BMP_RAM_SUB(2);
u16 *bg3vram = (u16*)BG_BMP_RAM(0);


void drawKaos(){
	int i;
	for(i=10; i <= 160; i++){
	  bg3vram[i + 256 * 10] = RGB15(25,25,25)|BIT(15);
	  bg3vram[i + 256 * 160] = RGB15(25,25,25)|BIT(15);
	}
	for(i=10; i <= 160; i++){
	  bg3vram[10 + 256 * i] = RGB15(25,25,25)|BIT(15);
	  bg3vram[160 + 256 * i] = RGB15(25,25,25)|BIT(15);
	}
	for(i = old_kaos_x - 1; i <= old_kaos_x + 1 ; i++){
	  bg3vram[ i + 256 * (old_kaos_y - 1)] = RGB15(0,0,0);
	  bg3vram[ i + 256 * old_kaos_y] = RGB15(0,0,0);
	  bg3vram[ i + 256 * (old_kaos_y + 1)] = RGB15(0,0,0);
	}
	for(i = kaos_x - 1; i <= kaos_x + 1 ; i++){
	  bg3vram[ i + 256 * (kaos_y - 1)] = RGB15(25,7,0)|BIT(15);
	  bg3vram[ i + 256 * kaos_y] = RGB15(25,7,0)|BIT(15);
	  bg3vram[ i + 256 * (kaos_y + 1)] = RGB15(25,7,0)|BIT(15);
	}
}
void drawSlider1(){
	int i;
	for(i=170; i <= 190; i++){
	  bg3vram[i + 256 * 10] = RGB15(25,25,25)|BIT(15);
	  bg3vram[i + 256 * 160] = RGB15(25,25,25)|BIT(15);
	}
	for(i=10; i <= 160; i++){
	  bg3vram[170 + 256 * i] = RGB15(25,25,25)|BIT(15);
	  bg3vram[190 + 256 * i] = RGB15(25,25,25)|BIT(15);
	}

	for(i=171; i < 190; i++){
	  bg3vram[ i + 256 * old_slider_val1] = RGB15(0,0,0);
	  bg3vram[ i + 256 * (old_slider_val1 + 1)] = RGB15(0,0,0);
	  bg3vram[ i + 256 * slider_val1] = RGB15(25,7,0)|BIT(15);
	  bg3vram[ i + 256 * (slider_val1 + 1)] = RGB15(25,7,0)|BIT(15);
	}
}
void drawSlider2(){
	int i;
	for(i=200; i <= 220; i++){
	  bg3vram[i + 256 * 10] = RGB15(25,25,25)|BIT(15);
	  bg3vram[i + 256 * 160] = RGB15(25,25,25)|BIT(15);
	}
	for(i=10; i <= 160; i++){
	  bg3vram[200 + 256 * i] = RGB15(25,25,25)|BIT(15);
	  bg3vram[220 + 256 * i] = RGB15(25,25,25)|BIT(15);
	}

	for(i=201; i < 220; i++){
	  bg3vram[ i + 256 * old_slider_val2] = RGB15(0,0,0);
	  bg3vram[ i + 256 * (old_slider_val2 + 1)] = RGB15(0,0,0);
	  bg3vram[ i + 256 * slider_val2] = RGB15(25,7,0)|BIT(15);
	  bg3vram[ i + 256 * (slider_val2 + 1)] = RGB15(25,7,0)|BIT(15);
	}
}
void drawSlider3(){
	int i;
	for(i=230; i <= 250; i++){
	  bg3vram[i + 256 * 10] = RGB15(25,25,25)|BIT(15);
	  bg3vram[i + 256 * 160] = RGB15(25,25,25)|BIT(15);
	}
	for(i=10; i <= 160; i++){
	  bg3vram[230 + 256 * i] = RGB15(25,25,25)|BIT(15);
	  bg3vram[250 + 256 * i] = RGB15(25,25,25)|BIT(15);
	}

	for(i=231; i < 250; i++){
	  bg3vram[ i + 256 * old_slider_val3] = RGB15(0,0,0);
	  bg3vram[ i + 256 * (old_slider_val3 + 1)] = RGB15(0,0,0);
	  bg3vram[ i + 256 * slider_val3] = RGB15(25,7,0)|BIT(15);
	  bg3vram[ i + 256 * (slider_val3 + 1)] = RGB15(25,7,0)|BIT(15);
	}
}

touchPosition touch;

void VblankHandler()
{
	scanKeys();
	touchRead(&touch);

	u16 held = keysHeld();
	
	if(!touch_was_down && (held & KEY_TOUCH)) {
		
		touch_was_down = 1;
		
		dsmi_osc_new("/ds/touch/pendown");
		dsmi_osc_addintarg( 1);
		dsmi_osc_send();
		
		if( touch.px > 170 && touch.px < 190 && touch.py > 10 && touch.py < 160){
		  
		  slider_touch1 = 1;
		  old_slider_val1 = slider_val1;
 		  slider_val1 = touch.py;
		  
		  dsmi_osc_new( "/ds/slider1");
		  dsmi_osc_addfloatarg( (float)(160 - slider_val1) / 150.0f );
		  dsmi_osc_send();
		  drawSlider1();
		}
		if( touch.px > 200 && touch.px < 220 && touch.py > 10 && touch.py < 160){
		  
		  slider_touch2 = 1;
		  old_slider_val2 = slider_val2;
 		  slider_val2 = touch.py;
		  
		  dsmi_osc_new( "/ds/slider2");
		  dsmi_osc_addfloatarg( (float)(160 - slider_val2) / 150.0f );
		  dsmi_osc_send();
		  drawSlider2();
		}
		if( touch.px > 230 && touch.px < 250 && touch.py > 10 && touch.py < 160){
		  
		  slider_touch3 = 1;
		  old_slider_val3 = slider_val3;
 		  slider_val3 = touch.py;
		  
		  dsmi_osc_new( "/ds/slider3");
		  dsmi_osc_addfloatarg( (float)(160 - slider_val3) / 150.0f );
		  dsmi_osc_send();
		  drawSlider3();
		}
		if( touch.px > 10 && touch.px < 160 && touch.py > 10 && touch.py < 160){
		  
		  kaos_touch = 1;
		  old_kaos_x = kaos_x;
		  old_kaos_y = kaos_y;
		  kaos_x = touch.px;
 		  kaos_y = touch.py;
		  
		  dsmi_osc_new( "/ds/kaos/x");
		  dsmi_osc_addfloatarg( (float)(kaos_x - 10) / 150.0f );
		  dsmi_osc_send();
		  
		  dsmi_osc_new( "/ds/kaos/y");
		  dsmi_osc_addfloatarg( (float)(160 - kaos_y) / 150.0f );
		  dsmi_osc_send();
		  
		  drawKaos();
		}
		dsmi_osc_new( "/ds/touch/x");
		dsmi_osc_addfloatarg( (float) touch.px);
		dsmi_osc_send();
		
		dsmi_osc_new( "/ds/touch/y");
		dsmi_osc_addfloatarg( (float) touch.py);
		dsmi_osc_send();
	}
	
	else if(touch_was_down && !(held & KEY_TOUCH))
	{
		touch_was_down = 0;
		slider_touch1 = 0;
		slider_touch2 = 0;
		slider_touch3 = 0;
		kaos_touch = 0;
		dsmi_osc_new("/ds/touch/pendown");
		dsmi_osc_addintarg( 0);
		dsmi_osc_send();
		
	}

	if(touch_was_down && (held & KEY_TOUCH)) {
		
		if( slider_touch1) {
		  old_slider_val1 = slider_val1;
		  slider_val1 = touch.py;
		  if (slider_val1 > 160) slider_val1 = 160;
		  if (slider_val1 < 10) slider_val1 = 10;
		  
		  dsmi_osc_new( "/ds/slider1");
		  dsmi_osc_addfloatarg( (float)(160 - slider_val1) / 150.0f );
		  dsmi_osc_send();
		  drawSlider1();
		}
		if( slider_touch2) {
		  old_slider_val2 = slider_val2;
		  slider_val2 = touch.py;
		  if (slider_val2 > 160) slider_val2 = 160;
		  if (slider_val2 < 10) slider_val2 = 10;
		  
		  dsmi_osc_new( "/ds/slider2");
		  dsmi_osc_addfloatarg( (float)(160 - slider_val2) / 150.0f );
		  dsmi_osc_send();
		  drawSlider2();
		}
		if( slider_touch3) {
		  old_slider_val3 = slider_val3;
		  slider_val3 = touch.py;
		  if (slider_val3 > 160) slider_val3 = 160;
		  if (slider_val3 < 10) slider_val3 = 10;
		  
		  dsmi_osc_new( "/ds/slider3");
		  dsmi_osc_addfloatarg( (float)(160 - slider_val3) / 150.0f );
		  dsmi_osc_send();
		  drawSlider3();
		}
		if( kaos_touch ) { 
		  old_kaos_x = kaos_x;
		  old_kaos_y = kaos_y;
		  kaos_x = touch.px;
 		  kaos_y = touch.py;
		  if (kaos_x > 160) kaos_x = 160;
		  if (kaos_x < 10) kaos_x = 10;
		  if (kaos_y > 160) kaos_y = 160;
		  if (kaos_y < 10) kaos_y = 10;
		 
		  dsmi_osc_new( "/ds/kaos/x");
		  dsmi_osc_addfloatarg( (float)(kaos_x - 10) / 150.0f );
		  dsmi_osc_send();
		  
		  dsmi_osc_new( "/ds/kaos/y");
		  dsmi_osc_addfloatarg( (float)(160 - kaos_y) / 150.0f);
		  dsmi_osc_send();
		  
		  drawKaos();
		}
		
		
	}
	
}

int main(void)
{
	
	lcdMainOnBottom();

	// Set modes
	videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_5_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG2_ACTIVE);
	
	// Set banks
	vramSetMainBanks(VRAM_A_MAIN_BG_0x06000000, VRAM_B_MAIN_BG_0x06020000,
		VRAM_C_SUB_BG_0x06200000 , VRAM_D_LCD);
	
	// Gfx on main
	REG_BG3CNT = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY(0);
	REG_BG3PA = 1 << 8;
	REG_BG3PB = 0;
	REG_BG3PC = 0;
	REG_BG3PD = 1 << 8;
	
	// Text bg on sub
	REG_BG0CNT_SUB = BG_MAP_BASE(4) | BG_TILE_BASE(0) | BG_PRIORITY(0);
	BG_PALETTE_SUB[255] = RGB15(31,15,0);
	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 4, 0, false, true);
	
	// Gfx on sub
	REG_BG2CNT_SUB = BG_BMP16_256x256 | BG_MAP_BASE(2) | BG_PRIORITY(1);
	REG_BG2PA_SUB = 1 << 8;
	REG_BG2PB_SUB = 0;
	REG_BG2PC_SUB = 0;
	REG_BG2PD_SUB = 1 << 8;
	
	u16 i;
	for(i=0; i<256*192; ++i)
		sub_vram[i] = dsmi_logo_ds[i];
	drawKaos();
	drawSlider1();
	drawSlider2();
	drawSlider3();
	
	iprintf("\x1b[12;12HOSC Demo\n");
	
	iprintf("\x1b[15;0H\x1b[KConnecting\n");
	int res = dsmi_connect();
	
	if(res == 1) {
		iprintf("\x1b[15;0H\x1b[KOK\n");
	} else {
		iprintf("\x1b[15;0H\x1b[KOh no, could not connect!\n");
		while(1);
	}
	
	
	while(1)
	{
		VblankHandler();

		swiWaitForVBlank();
	}
	
}
