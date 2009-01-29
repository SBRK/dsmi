#include <nds.h>
#include <dswifi7.h>

#include "psg.h"
#include "../../generic/midimsg.h"

// Thanks to LiraNuna for this cool function
void PM_SetRegister(int reg, int control)
{
    SerialWaitBusy();
    REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER |SPI_BAUD_1MHz | SPI_CONTINUOUS;
    REG_SPIDATA = reg;
    SerialWaitBusy();
    REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER |SPI_BAUD_1MHz;
    REG_SPIDATA = control;
}

void VcountHandler() {
	inputGetAndSend();
}

void VblankHandler(void) {
	Wifi_Update();
	psg_update();
}

void midiHandler(int num_bytes, void * userdata) {
	MidiMsg midimsg;
	fifoGetDatamsg(FIFO_MIDI, num_bytes, (u8*)&midimsg);
	psg_midimsg(midimsg.msg, midimsg.data1, midimsg.data2);
}

int main() {
	irqInit();
	fifoInit();

	readUserSettings();

	// Start the RTC tracking IRQ
	initClockIRQ();

	SetYtrigger(80);

	installWifiFIFO();
	installSystemFIFO();
	fifoSetDatamsgHandler(FIFO_MIDI, midiHandler, 0);
	
	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);
	
	psg_init();
	
	irqEnable( IRQ_VBLANK | IRQ_VCOUNT | IRQ_NETWORK);   

    PM_SetRegister(0, 9);

	while (1) {swiWaitForVBlank(); }
}
