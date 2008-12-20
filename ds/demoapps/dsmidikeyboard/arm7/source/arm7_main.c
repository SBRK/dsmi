#include <nds.h>

#include <dswifi7.h>

#define abs(x)	((x)>=0?(x):-(x))

int vcount;
touchPosition first,tempPos;

//---------------------------------------------------------------------------------
void VcountHandler() {
//---------------------------------------------------------------------------------
	static int lastbut = -1;
	
	uint16 but=0, x=0, y=0, xpx=0, ypx=0, z1=0, z2=0;

	but = REG_KEYXY;

	if (!( (but ^ lastbut) & (1<<6))) {
 
		tempPos = touchReadXY();

		x = tempPos.x;
		y = tempPos.y;
		xpx = tempPos.px;
		ypx = tempPos.py;
		z1 = tempPos.z1;
		z2 = tempPos.z2;
		
	} else {
		lastbut = but;
		but |= (1 <<6);
	}

	if ( vcount == 80 ) {
		first = tempPos;
	} else {
		if (	abs( xpx - first.px) > 10 || abs( ypx - first.py) > 10 ||
				(but & ( 1<<6)) ) {

			but |= (1 <<6);
			lastbut = but;

		} else {
			IPC->mailBusy = 1;
			IPC->touchX			= x;
			IPC->touchY			= y;
			IPC->touchXpx		= xpx;
			IPC->touchYpx		= ypx;
			IPC->touchZ1		= z1;
			IPC->touchZ2		= z2;
			IPC->mailBusy = 0;
		}
	}
	IPC->buttons		= but;
	vcount ^= (80 ^ 130);
	SetYtrigger(vcount);

}

void VblankHandler()
{
	uint16 but=0, x=0, y=0, xpx=0, ypx=0, z1=0, z2=0, batt=0, aux=0;
	
	// Read the X/Y buttons and the /PENIRQ line
	
	but = REG_KEYXY;
	if (!(but & 0x40)) {
		// Read the touch screen
		touchPosition tempPos = touchReadXY();

		x = tempPos.x;
		y = tempPos.y;
		xpx = tempPos.px;
		ypx = tempPos.py;
	}
	
	// Update the IPC struct
	
	IPC->buttons   = but;
	IPC->touchX    = x;
	IPC->touchY    = y;
	IPC->touchXpx  = xpx;
	IPC->touchYpx  = ypx;
	IPC->touchZ1   = z1;
	IPC->touchZ2   = z2;
	IPC->battery   = batt;
	IPC->aux       = aux;
	
	Wifi_Update(); // update wireless in vblank
}

// callback to allow wifi library to notify arm9
void arm7_synctoarm9() { // send fifo message
   REG_IPC_FIFO_TX = 0x87654321;
}
// interrupt handler to allow incoming notifications from arm9
void arm7_fifo() { // check incoming fifo messages
   u32 msg = REG_IPC_FIFO_RX;
   if(msg==0x87654321) Wifi_Sync();
}

//////////////////////////////////////////////////////////////////////


int main(int argc, char **argv)
{
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR; // enable & prepare fifo asap
	
	// Reset the clock if needed
	rtcReset();

	// Turn on Sound
	powerON(POWER_SOUND);
	
	// Set up sound defaults.
	SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);
	IPC->soundData = 0;
	
	// Set up the interrupt handler
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	swiWaitForVBlank();

	SetYtrigger(80);
	vcount = 80;
	irqSet(IRQ_VCOUNT, VcountHandler);
	irqEnable(IRQ_VCOUNT);
	
	{ // sync with arm9 and init wifi
		irqSet(IRQ_WIFI, Wifi_Interrupt); // set up wifi interrupt
		irqEnable(IRQ_WIFI);

		u32 fifo_temp;
	
		  while(1) { // wait for magic number
			while(REG_IPC_FIFO_CR&IPC_FIFO_RECV_EMPTY) swiWaitForVBlank();
		  fifo_temp=REG_IPC_FIFO_RX;
		  if(fifo_temp==0x12345678) break;
		}
		while(REG_IPC_FIFO_CR&IPC_FIFO_RECV_EMPTY) swiWaitForVBlank();
		fifo_temp=REG_IPC_FIFO_RX; // give next value to wifi_init
		Wifi_Init(fifo_temp);
		
		irqSet(IRQ_FIFO_NOT_EMPTY,arm7_fifo); // set up fifo irq
		irqEnable(IRQ_FIFO_NOT_EMPTY);
		REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ;
	
		Wifi_SetSyncHandler(arm7_synctoarm9); // allow wifi lib to notify arm9
  	} // arm7 wifi init complete

	// Keep the ARM7 out of main RAM
	while (1)
	{
		VblankHandler();
		swiWaitForVBlank();
	}
}
