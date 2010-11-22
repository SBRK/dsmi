/*
* DSerial Library
*
* Copyright (c) 2008, Alexei Karpenko
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the <organization> nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "dserial.h"
#include "card_spi.h"
#include "mcu.h"
#include <nds.h>
#include <stdio.h>
#include <string.h>

static volatile bool Flashing;
static volatile bool UartSending[2];

static uint8 Version;
static bool UartEnabled[2];

static uint8 NumAnalogPins;
static bool AnalogPin[16];
static uint8 AnalogMuxSequence[16];
#define ANALOG_INDEX(port, pin) (((port - 1) << 3) | pin)

static void (*dseUart0ReceiveHandler)(char * data, unsigned int size);
static void (*dseUart0SendHandler)(void);
static void (*dseUart1ReceiveHandler)(char * data, unsigned int size);
static void (*dseUart1SendHandler)(void);

/* Helpers */

/*-------------------------------------------------------------------------------*/
void dseIrqHandler() {
/*-------------------------------------------------------------------------------*/
	char buffer[MAX_DATA_SIZE];
	uint8 size;
	bool stopFlashing = false;

	/* read interrupt flags */
	size = dseReadBuffer(SELECT_INTERRUPT, buffer);

	/* UART0 interrupts */

	if (buffer[0] & INTERRUPT_UART0_TX) {
		UartSending[0] = false;
		if (dseUart0SendHandler != NULL) {
			dseUart0SendHandler();
		}
	}

	if (buffer[0] & INTERRUPT_UART0_RX) {
		char data[MAX_DATA_SIZE];
		size = dseReadBuffer(SELECT_UART0_BUFFER, data);

		if (dseUart0ReceiveHandler != NULL) {
			dseUart0ReceiveHandler(data, size);
		}
	}

	/* UART1 interrupts */

	if (buffer[0] & INTERRUPT_UART1_TX) {
		UartSending[1] = false;
		if (dseUart1SendHandler != NULL) {
			dseUart1SendHandler();
		}
	}

	if (buffer[0] & INTERRUPT_UART1_RX) {
		char data[MAX_DATA_SIZE];
		size = dseReadBuffer(SELECT_UART1_BUFFER, data);

		if (dseUart1ReceiveHandler != NULL) {
			dseUart1ReceiveHandler(data, size);
		}
	}

	/* other interrupts */

	if (buffer[0] & INTERRUPT_BOOTLOADER) {
		stopFlashing = true;
	}

	/* ack interupts */
	dseWriteBuffer(SELECT_INTERRUPT, 1, buffer);

	if(stopFlashing) {
		Flashing = false;
	}
}

/*-------------------------------------------------------------------------------*/
void dseWriteBuffer(uint8 selector, uint8 size, char * data) {
/*-------------------------------------------------------------------------------*/
	char buffer[MAX_DATA_SIZE+2];

	buffer[0] = SELECT_WRITE | selector;
	buffer[1] = size;
	memcpy(buffer + 2, data, size);

	cardSpiWriteBuffer(buffer, size+2);
}

/*-------------------------------------------------------------------------------*/
uint8 dseReadBuffer(uint8 selector, char * data) {
/*-------------------------------------------------------------------------------*/
	uint8 size, i;

	/* enable card SPI with CS hold */
	cardSpiStart(true);

	REG_AUXSPIDATA = SELECT_READ | selector;	/* send character */
	while(cardSpiBusy());	/* busy wait */
	REG_AUXSPIDATA = 0;		/* send character */
	while(cardSpiBusy());	/* busy wait */
	REG_AUXSPIDATA = 0;		/* send character */
	while(cardSpiBusy());	/* busy wait */
	size = REG_AUXSPIDATA;	/* read character */

	if (size != 0 && size <= MAX_DATA_SIZE) {

		for ( i = 0; i < size-1; i++ ) {
			REG_AUXSPIDATA = 0;		/* send character */
			while(cardSpiBusy());	/* busy wait */
			data[i] = REG_AUXSPIDATA;	/* receive character */
		}

		/* last character has to be transferred with chip select hold disabled */
		cardSpiStart(false);

		REG_AUXSPIDATA = 0;				/* send last character */
		while(cardSpiBusy());			/* busy wait */
		data[size-1] = REG_AUXSPIDATA;	/* receive character */

	}

	/* disable card SPI */
	cardSpiStop();

#ifdef ARM9
	swiDelay(24);
#else
	swiDelay(12);
#endif

	return size;
}

/*-------------------------------------------------------------------------------*/
bool dseReadFlash(char * data, uint16 pos, uint8 size) {
/*-------------------------------------------------------------------------------*/
	uint8 i;

	if (size == 0) {
		return false;
	}

	/* enable card SPI with CS hold */
	cardSpiStart(true);

	REG_AUXSPIDATA = SELECT_READ | SELECT_FLASH;	/* send character */
	while(cardSpiBusy());			/* busy wait */
	REG_AUXSPIDATA = size;			/* send character */
	while(cardSpiBusy());			/* busy wait */
	REG_AUXSPIDATA = pos >> 8;		/* send character */
	while(cardSpiBusy());			/* busy wait */
	REG_AUXSPIDATA = pos & 0xFF;		/* send character */
	while(cardSpiBusy());			/* busy wait */
	REG_AUXSPIDATA = 0;				/* send character */
	while(cardSpiBusy());			/* busy wait */


	for ( i = 0; i < size-1; i++ ) {
		REG_AUXSPIDATA = 0;		/* send character */
		while(cardSpiBusy());	/* busy wait */
		data[i] = REG_AUXSPIDATA;	/* receive character */
	}

	/* last character has to be transferred with chip select hold disabled */
	cardSpiStart(false);

	REG_AUXSPIDATA = 0;				/* send last character */
	while(cardSpiBusy());			/* busy wait */
	data[size-1] = REG_AUXSPIDATA;	/* receive character */



	/* disable card SPI */
	cardSpiStop();

#ifdef ARM9
	swiDelay(24);
#else
	swiDelay(12);
#endif

	return true;
}

/*-------------------------------------------------------------------------------*/
void dseWriteRegister(uint8 reg, uint8 val) {
/*-------------------------------------------------------------------------------*/
	char buffer[2];
	buffer[0] = reg;
	buffer[1] = val;
	dseWriteBuffer(SELECT_REGISTER, 2, buffer);
}

/*-------------------------------------------------------------------------------*/
uint8 dseReadRegister(uint8 reg) {
/*-------------------------------------------------------------------------------*/
	uint8 val;

	/* enable card SPI with CS hold */
	cardSpiStart(true);

	REG_AUXSPIDATA = SELECT_READ | SELECT_REGISTER;	/* send character */
	while(cardSpiBusy());			/* busy wait */
	REG_AUXSPIDATA = reg;				/* send character */
	while(cardSpiBusy());			/* busy wait */
	REG_AUXSPIDATA = 0;				/* send character */
	while(cardSpiBusy());			/* busy wait */

	/* last character has to be transferred with chip select hold disabled */
	cardSpiStart(false);

	REG_AUXSPIDATA = 0;				/* send character */
	while(cardSpiBusy());			/* busy wait */
	val = REG_AUXSPIDATA;

	/* disable card SPI */
	cardSpiStop();

#ifdef ARM9
	swiDelay(24);
#else
	swiDelay(12);
#endif

	return val;
}

/* Configuration */

/*-------------------------------------------------------------------------------*/
bool dseInit() {
/*-------------------------------------------------------------------------------*/
	Flashing = false;
	UartSending[0] = UartSending[1] = false;

	cardSpiInit(CLOCK_512KHZ);

	/* send a couple of zeros just in case */
	char buffer[MAX_DATA_SIZE];
	memset(buffer, 0, MAX_DATA_SIZE);
	dseWriteBuffer(0, MAX_DATA_SIZE, buffer);

	if (dseStatus() == DISCONNECTED) {
		return false;
	}

	cardSpiSetHandler(dseIrqHandler);

	Version = dseVersion();

	UartEnabled[0] = true;
	UartEnabled[1] = Version > 0;

	NumAnalogPins = 0;
	memset(AnalogPin, 0, 16 * sizeof(bool));

	dseUart0ReceiveHandler = dseUartDefaultReceiveHandler;
	dseUart0SendHandler = NULL;
	dseUart1ReceiveHandler = dseUartDefaultReceiveHandler;
	dseUart1SendHandler = NULL;

	return true;
}

/*-------------------------------------------------------------------------------*/
DseStatus dseStatus() {
/*-------------------------------------------------------------------------------*/
	char buffer[MAX_DATA_SIZE];
	uint8 size;

	size = dseReadBuffer(SELECT_READ | SELECT_CHECK, buffer);
	if (size != 1) {
		return DISCONNECTED;
	} else if (buffer[0] == 0xAB) {
		return BOOTLOADER;
	} else if (buffer[0] == 0xAC) {
		return FIRMWARE;
	} else {
		return DISCONNECTED;
	}
}

/*-------------------------------------------------------------------------------*/
int dseVersion() {
/*-------------------------------------------------------------------------------*/
	char buffer[MAX_DATA_SIZE];
	uint8 size;
	buffer[0] = 0;
	size = dseReadBuffer(SELECT_READ | SELECT_VERSION, buffer);
	return size == 1 ? buffer[0] : 0;
}

/*-------------------------------------------------------------------------------*/
bool dseUploadFirmware(char * fwdata, unsigned int fwsize) {
/*-------------------------------------------------------------------------------*/
	const uint8 BLOCK_SIZE = 32; /* max 32 */
	uint16 pos, loc;
	uint8 size;
	char buffer[BLOCK_SIZE+2];

	for (pos = 0; pos < fwsize; pos += BLOCK_SIZE) {
		size = (pos + BLOCK_SIZE <= fwsize) ? BLOCK_SIZE : (fwsize - pos);
		//iprintf("<%i%%>\n", pos * 100 / fwsize); /* show percentage */

		loc = 0x0800 + pos;

		if (pos % 512 == 0) {
			char temp[2];
			temp[0] = loc >> 8;
			temp[1] = loc & 0xFF;
			Flashing = true;
			//iprintf("(");
			dseWriteBuffer(SELECT_FLASH_ERASE, 2, temp);
			while(Flashing); /* busy wait */
			//iprintf(")");
		}

		buffer[0] = loc >> 8;
		buffer[1] = loc & 0xFF;
		memcpy(buffer+2, &(fwdata[pos]), size);
		Flashing = true;
		//iprintf("<");
		dseWriteBuffer(SELECT_FLASH, size+2, buffer);
		while(Flashing); /* busy wait */
		//iprintf(">");
	}

	return true;
}

/*-------------------------------------------------------------------------------*/
bool dseMatchFirmware(char * data, unsigned int fwsize) {
/*-------------------------------------------------------------------------------*/
	const uint8 BLOCK_SIZE = 16; /* max 32 */
	uint16 i;
	char buffer[BLOCK_SIZE];

	iprintf("Firmware size is 0x%04X.\n", fwsize);
	for (i = 0; i < fwsize; i++) {
		uint16 pos = i % BLOCK_SIZE;
		uint8 size = (i + BLOCK_SIZE <= fwsize) ? BLOCK_SIZE : (fwsize - i);
		if (pos == 0) {
			dseReadFlash(buffer, 0x0800 + i, size);
		}
		if (data[i] != buffer[pos]) {
			iprintf("Difference at 0x%04X.\n", i);
			return false;	/* firmwares do not match */
		}
	}
	return true;			/* firmwares match */
}

/*-------------------------------------------------------------------------------*/
bool dseBoot() {
/*-------------------------------------------------------------------------------*/
	cardSpiTransfer(SELECT_BOOT);

	return true;
}

/*-------------------------------------------------------------------------------*/
void dseSetModes(unsigned char modes) {
/*-------------------------------------------------------------------------------*/
	dseWriteBuffer(SELECT_ENABLE, 1, (char *) &modes);
}

/* UART */

/*-------------------------------------------------------------------------------*/
bool dseUartSendBuffer(DseUart uart, char * data, unsigned int size, bool blocking) {
/*-------------------------------------------------------------------------------*/
	if (size > MAX_DATA_SIZE) {
		return false;
	}

	if (blocking) {
		UartSending[uart] = true;
	}
	
	if (uart == UART0) {
		dseWriteBuffer(SELECT_UART0_BUFFER, size, data);
	} else {
		dseWriteBuffer(SELECT_UART1_BUFFER, size, data);
	}

	if (blocking) {
		while(UartSending[uart]); /* busy wait */
	}

	return true;
}

/*-------------------------------------------------------------------------------*/
void dseUartSetReceiveHandler(DseUart uart, void (*receiveHandler)(char * data, unsigned int size)) {
/*-------------------------------------------------------------------------------*/
	if(uart == UART0) {
		dseUart0ReceiveHandler = receiveHandler;
	} else if(uart == UART1) {
		dseUart1ReceiveHandler = receiveHandler;
	}
}
/*-------------------------------------------------------------------------------*/
void dseUartSetSendHandler(DseUart uart, void (*sendHandler)(void)) {
/*-------------------------------------------------------------------------------*/
	if(uart == UART0) {
		dseUart0SendHandler = sendHandler;
	} else if(uart == UART1) {
		dseUart1SendHandler = sendHandler;
	}
}

/*-------------------------------------------------------------------------------*/
bool dseUartSetBaudrate(DseUart uart, unsigned int baudrate) {
/*-------------------------------------------------------------------------------*/
	/*
		To change baudrate, we need to change Timer1 reload value and clock source

		UART clock can be SYSCLK, SYSCLK / 4, SYSCLK / 12, SYSCLK / 48
		(see UART and CKCON documentation for C8051F320)
	*/

	if (uart == UART0) {
		unsigned int counter = ((Version == 0 ? 24000000 : 48000000) / 2 / baudrate);
		unsigned char ckcon, th1, temp;
		if (counter < 256) {
			// clock = SYSCLK
			ckcon = 0x08;
		} else {
			counter /= 4;
			if (counter < 256) {
				// clock = SYSCLK / 4
				ckcon = 0x01;
			} else {
				counter /= 3;
				if (counter < 256) {
					// clock = SYSCLK / 12
					ckcon = 0x00;
				} else {
					// clock = SYSCLK / 48
					counter /= 4;
					ckcon = 0x02;
				}
			}
		}

		th1 = 256 - counter;

		/* TODO: Blocking */
		temp = dseReadRegister(MCU_CKCON);
		dseWriteRegister(MCU_CKCON, (temp & ~0x0B) | ckcon);
		dseWriteRegister(MCU_TH1, th1);
	} else if (uart == UART1) {
		uint16 counter = 65536 - (48000000 / baudrate / 2);
		dseWriteRegister(MCU_SBRLH1, counter >> 8);
		dseWriteRegister(MCU_SBRLL1, counter & 0xFF);
	}

	return true;
}

/*-------------------------------------------------------------------------------*/
void dseUartDefaultReceiveHandler(char * data, unsigned int size) {
/*-------------------------------------------------------------------------------*/
/*  This runs in interrupt mode, so make sure you understand what this entails!  */
	char buffer[MAX_DATA_SIZE+1];

	memcpy(buffer, data, size);
	buffer[size] = '\0';
	iprintf(buffer);
}

/* Helpers */

/*-------------------------------------------------------------------------------*/
void dseTimerStart(uint16 delay_us) {
/*-------------------------------------------------------------------------------*/
	uint16 reload;

	dseWriteRegister(MCU_TMR2CN, 0x00);			/* disable Timer2 */

	/* select SYSCLK */
	/*
	unsigned char ckcon = dseReadRegister(MCU_CKCON);
	dseWriteRegister(MCU_CKCON, ckcon | 0x10);
	*/

	/* select SYSCLK / 12 */
	unsigned char ckcon = dseReadRegister(MCU_CKCON);
	dseWriteRegister(MCU_CKCON, ckcon & ~0x10);
	if (Version > 0) {
		reload = 0xFFFF - 4 * delay_us;
	} else {
		reload = 0xFFFF - 2 * delay_us;
	}

	dseWriteRegister(MCU_TMR2RLH, reload >> 8);
	dseWriteRegister(MCU_TMR2RLL, reload & 0xFF);

	dseWriteRegister(MCU_TMR2CN, 0x04);			/* enable Timer2 */
}

/*-------------------------------------------------------------------------------*/
void dseTimerStop() {
/*-------------------------------------------------------------------------------*/
	dseWriteRegister(MCU_TMR2CN, 0x00);			/* disable Timer2 */
}

/*-------------------------------------------------------------------------------*/
void dseAnalogPinsUpdate() {
/*-------------------------------------------------------------------------------*/
	dseTimerStop();

	if(NumAnalogPins > 0) {
		int i;
		int index = 0;
		for(i = 0; i < 16; i++) {
			if(AnalogPin[i]) {
				AnalogMuxSequence[index++] = i;
			} else {
				AnalogMuxSequence[index] = 0xFF; /* list terminator */
			}
		}
		dseWriteBuffer(SELECT_ADC_SEQUENCE, 16, (char *) AnalogMuxSequence);
		dseTimerStart(10); /* start converting ADC each 10 us */
	}
}

/* GPIO */

const uint8 pin_p[] = {MCU_P0, MCU_P1, MCU_P2, MCU_P3};
const uint8 pin_mdin[] = {MCU_P0MDIN, MCU_P1MDIN, MCU_P2MDIN, MCU_P3MDIN};
const uint8 pin_mdout[] = {MCU_P0MDOUT, MCU_P1MDOUT, MCU_P2MDOUT, MCU_P3MDOUT};
const uint8 pin_skip[] = {MCU_P0SKIP, MCU_P1SKIP, MCU_P2SKIP, MCU_P3SKIP};
const uint8 pin_mask[][4] = {{0xC0, 0xFF, 0xFF, 0x00},		/* GPIO pins when UART1 disabled */
							{0xC0, 0xFF, 0x3F, 0x00}};		/* GPIO pins when UART1 enabled */
const uint8 pin_adc_mask[][4] = {{0x00, 0xFF, 0xFF, 0x00},	/* ADC pins when UART1 disabled */
								{0x00, 0xFF, 0x3F, 0x00}};	/* ADC pins when UART1 enabled */

/*-------------------------------------------------------------------------------*/
void dsePinMode(DsePort port, uint8 pin, DsePinMode mode) {
/*-------------------------------------------------------------------------------*/
	uint8 u = UartEnabled[1] ? 1 : 0;
	if(pin > 7 || port > 3
		|| !(pin_mask[u][port] & (1 << pin))
		|| (mode == ANALOG_INPUT && !(pin_adc_mask[u][port] & (1 << pin)))) {
		return;
	}

	uint8 skip = dseReadRegister(pin_skip[port]);
	skip |= (1 << pin);			/* crossbar should skip pin */
	dseWriteRegister(pin_skip[port], skip);

	uint8 mdin = dseReadRegister(pin_mdin[port]);

	if(mode == ANALOG_INPUT) {		/* analog pin */
		if(!AnalogPin[ANALOG_INDEX(port, pin)]) {
			AnalogPin[ANALOG_INDEX(port, pin)] = true;
			NumAnalogPins++;
			dseAnalogPinsUpdate();
		}
		mdin &= ~(1 << pin);		/* analog input */
	} else {						/* digital pin */
		if(AnalogPin[ANALOG_INDEX(port, pin)]) {
			AnalogPin[ANALOG_INDEX(port, pin)] = false;
			NumAnalogPins--;
			dseAnalogPinsUpdate();
		}

		uint8 mdout = dseReadRegister(pin_mdout[port]);
		mdin |= (1 << pin);			/* not analog input */
		
		if(mode == INPUT) {
			mdout &= ~(1 << pin);	/* open-drain */
		} else if(mode == OUTPUT) {
			mdout |= 1 << pin;		/* push-pull */
		}
		
		dseWriteRegister(pin_mdout[port], mdout);
	}
	dseWriteRegister(pin_mdin[port], mdin);
}

/*-------------------------------------------------------------------------------*/
bool dsePinRead(DsePort port, uint8 pin) {
/*-------------------------------------------------------------------------------*/
	uint8 u = UartEnabled[1] ? 1 : 0;
	if(pin > 7 || port > 3 || !(pin_mask[u][port] & (1 << pin))) {
		return false;
	}
	uint8 p = dseReadRegister(pin_p[port]);
	return p & (1 << pin);
}

/*-------------------------------------------------------------------------------*/
void dsePinWrite(DsePort port, uint8 pin, bool state) {
/*-------------------------------------------------------------------------------*/
	uint8 u = UartEnabled[1] ? 1 : 0;
	if(pin > 7 || port > 3 || !(pin_mask[u][port] & (1 << pin))) {
		return;
	}
	uint8 p = dseReadRegister(pin_p[port]);
	if(state) {
		p |= 1 << pin;
	} else {
		p &= ~(1 << pin);
	}
	dseWriteRegister(pin_p[port], p);
}

/*-------------------------------------------------------------------------------*/
uint16 dsePinReadAnalog(uint8 port, uint8 pin) {
/*-------------------------------------------------------------------------------*/
	uint8 u = UartEnabled[1] ? 1 : 0;
	uint8 mux = ANALOG_INDEX(port, pin);

	if(pin > 7 || port > 3 || !(pin_adc_mask[u][port] & (1 << pin)) || !AnalogPin[mux]) {
		return 0;
	}

	uint16 val;
	uint8 index;

	/* find sequence index */
	for(index = 0; index < NumAnalogPins; index++) {
		if(AnalogMuxSequence[index] == mux) {
			break;
		}
	}
	if(index == NumAnalogPins) {
		return 0; /* given pin not in sequence... */
	}

	/* enable card SPI with CS hold */
	cardSpiStart(true);

	REG_AUXSPIDATA = SELECT_READ | SELECT_ADC;	/* send character */
	while(cardSpiBusy());			/* busy wait */
	REG_AUXSPIDATA = index;			/* send character */
	while(cardSpiBusy());			/* busy wait */
	REG_AUXSPIDATA = 0;				/* send character */
	while(cardSpiBusy());			/* busy wait */
	REG_AUXSPIDATA = 0;				/* send character */
	while(cardSpiBusy());			/* busy wait */
	val = ((uint16) REG_AUXSPIDATA) << 8;

	/* last character has to be transferred with chip select hold disabled */
	cardSpiStart(false);

	REG_AUXSPIDATA = 0;				/* send character */
	while(cardSpiBusy());			/* busy wait */
	val |= REG_AUXSPIDATA & 0xFF;

	/* disable card SPI */
	cardSpiStop();

#ifdef ARM9
	swiDelay(24);
#else
	swiDelay(12);
#endif

	return val;
}