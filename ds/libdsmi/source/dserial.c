/*
* DSerial Library
*
* Copyright (c) 2007, Alexei Karpenko
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
static volatile bool UartSending;
static uint8 Servo[NUM_SERVOS];

static void (*dseUartReceiveHandler)(char * data, unsigned int size);
static void (*dseUartSendHandler)(void);

/* Helpers */

/*-------------------------------------------------------------------------------*/
void dseIrqHandler() {
/*-------------------------------------------------------------------------------*/
	char buffer[MAX_DATA_SIZE];
	uint8 size;
	bool stopFlashing = false;

	/* read interrupt flags */
	size = dseReadBuffer(SELECT_INTERRUPT, buffer);

	if (buffer[0] & INTERRUPT_UART_SENT) {
		UartSending = false;
		if (dseUartSendHandler != NULL) {
			dseUartSendHandler();
		}
	}

	if (buffer[0] & INTERRUPT_UART_RECEIVED) {
		char data[MAX_DATA_SIZE];
		size = dseReadBuffer(SELECT_UART_BUFFER, data);

		if (dseUartReceiveHandler != NULL) {
			dseUartReceiveHandler(data, size);
		}
	}

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

	CARD_EEPDATA = SELECT_READ | selector;	/* send character */
	while(cardSpiBusy());	/* busy wait */
	CARD_EEPDATA = 0;		/* send character */
	while(cardSpiBusy());	/* busy wait */
	CARD_EEPDATA = 0;		/* send character */
	while(cardSpiBusy());	/* busy wait */
	size = CARD_EEPDATA;	/* read character */

	if (size != 0 && size <= MAX_DATA_SIZE) {

		for ( i = 0; i < size-1; i++ ) {
			CARD_EEPDATA = 0;		/* send character */
			while(cardSpiBusy());	/* busy wait */
			data[i] = CARD_EEPDATA;	/* receive character */
		}

		/* last character has to be transferred with chip select hold disabled */
		cardSpiStart(false);

		CARD_EEPDATA = 0;				/* send last character */
		while(cardSpiBusy());			/* busy wait */
		data[size-1] = CARD_EEPDATA;	/* receive character */

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

	CARD_EEPDATA = SELECT_READ | SELECT_FLASH;	/* send character */
	while(cardSpiBusy());			/* busy wait */
	CARD_EEPDATA = size;			/* send character */
	while(cardSpiBusy());			/* busy wait */
	CARD_EEPDATA = pos >> 8;		/* send character */
	while(cardSpiBusy());			/* busy wait */
	CARD_EEPDATA = pos & 0xFF;		/* send character */
	while(cardSpiBusy());			/* busy wait */
	CARD_EEPDATA = 0;				/* send character */
	while(cardSpiBusy());			/* busy wait */
	

	for ( i = 0; i < size-1; i++ ) {
		CARD_EEPDATA = 0;		/* send character */
		while(cardSpiBusy());	/* busy wait */
		data[i] = CARD_EEPDATA;	/* receive character */
	}

	/* last character has to be transferred with chip select hold disabled */
	cardSpiStart(false);

	CARD_EEPDATA = 0;				/* send last character */
	while(cardSpiBusy());			/* busy wait */
	data[size-1] = CARD_EEPDATA;	/* receive character */



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

	CARD_EEPDATA = SELECT_READ | SELECT_REGISTER;	/* send character */
	while(cardSpiBusy());			/* busy wait */
	CARD_EEPDATA = reg;				/* send character */
	while(cardSpiBusy());			/* busy wait */
	CARD_EEPDATA = 0;				/* send character */
	while(cardSpiBusy());			/* busy wait */

	/* last character has to be transferred with chip select hold disabled */
	cardSpiStart(false);
	
	CARD_EEPDATA = 0;				/* send character */
	while(cardSpiBusy());			/* busy wait */
	val = CARD_EEPDATA;

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
	UartSending = false;

	cardSpiInit(CLOCK_512KHZ);

	if (dseStatus() == DISCONNECTED) {
		return false;
	}

	cardSpiSetHandler(dseIrqHandler);

	dseUartReceiveHandler = dseUartDefaultReceiveHandler;
	dseUartSendHandler = NULL;

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
			dseWriteBuffer(SELECT_FLASH_ERASE, 2, temp);
			while(Flashing); /* busy wait */
		}

		buffer[0] = loc >> 8;
		buffer[1] = loc & 0xFF;
		memcpy(buffer+2, &(fwdata[pos]), size);
		Flashing = true;
		dseWriteBuffer(SELECT_FLASH, size+2, buffer);
		while(Flashing); /* busy wait */
	}

	return true;
}

/*-------------------------------------------------------------------------------*/
bool dseMatchFirmware(char * data, unsigned int fwsize) {
/*-------------------------------------------------------------------------------*/
	const uint8 BLOCK_SIZE = 16; /* max 32 */
	uint16 i;
	char buffer[BLOCK_SIZE];
	
	for (i = 0; i < fwsize; i++) {
		uint16 pos = i % BLOCK_SIZE;
		uint8 size = (i + BLOCK_SIZE <= fwsize) ? BLOCK_SIZE : (fwsize - i);
		if (pos == 0) {
			dseReadFlash(buffer, 0x0800 + i, size);
		}
		if (data[i] != buffer[pos]) {
			iprintf("Difference at 0x%02X.\n", i);
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
bool dseUartSendBuffer(char * data, unsigned int size, bool blocking) {
/*-------------------------------------------------------------------------------*/
	if (size > MAX_DATA_SIZE) {
		return false;
	}
	
	if (blocking) {
		UartSending = true;
	}

	dseWriteBuffer(SELECT_UART_BUFFER, size, data);

	if (blocking) {
		while(UartSending); /* busy wait */
	}

	return true;
}

/*-------------------------------------------------------------------------------*/
void dseUartSetReceiveHandler(void (*receiveHandler)(char * data, unsigned int size)) {
/*-------------------------------------------------------------------------------*/
	dseUartReceiveHandler = receiveHandler;
}
/*-------------------------------------------------------------------------------*/
void dseUartSetSendHandler(void (*sendHandler)(void)) {
/*-------------------------------------------------------------------------------*/
	dseUartSendHandler = sendHandler;
}

/*-------------------------------------------------------------------------------*/
bool dseUartSetBaudrate(unsigned int baudrate) {
/*-------------------------------------------------------------------------------*/
	/* 
		To change baudrate, we need to change Timer1 reload value and clock source

		UART clock can be SYSCLK, SYSCLK / 4, SYSCLK / 12, SYSCLK / 48 
		(see UART and CKCON documentation for C8051F320)
	*/

	unsigned int counter = (12000000 / baudrate);
	unsigned char ckcon, th1, temp;
	if (counter < 256) {
		// clock = SYSCLK
		ckcon = 0x80;
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

/* ADC */

/*-------------------------------------------------------------------------------*/
void dseAdcStart(int16 delay_us) {
/*-------------------------------------------------------------------------------*/
	uint16 reload;

	if (delay_us < 0) {
		delay_us = 0;
	}

	dseWriteRegister(MCU_TMR2CN, 0x00);			/* disable Timer2 */
	
	/* select SYSCLK == 24MHz */
	/*
	unsigned char ckcon = dseReadRegister(MCU_CKCON);
	dseWriteRegister(MCU_CKCON, ckcon | 0x10);		
	reload = 0xFFFF - 24 * delay_us;
	*/

	/* select SYSCLK / 12 == 2MHz */
	unsigned char ckcon = dseReadRegister(MCU_CKCON);
	dseWriteRegister(MCU_CKCON, ckcon & ~0x10);
	reload = 0xFFFF - 2 * delay_us;


	dseWriteRegister(MCU_TMR2RLH, reload >> 8);
	dseWriteRegister(MCU_TMR2RLL, reload & 0xFF);

	dseWriteRegister(MCU_TMR2CN, 0x04);			/* enable Timer2 */
}

/*-------------------------------------------------------------------------------*/
uint8 dseAdcRead(char * data) {
/*-------------------------------------------------------------------------------*/
	return dseReadBuffer(SELECT_ADC, data);
}

/* Servo */

/*-------------------------------------------------------------------------------*/
void dseServoSetAll(uint8 * positions) {
/*-------------------------------------------------------------------------------*/
	int i;
	for (i = 0; i < 10; i++) {
		memcpy(Servo, positions, NUM_SERVOS);
		dseWriteBuffer(SELECT_SERVO, NUM_SERVOS, (char *) Servo);
		swiDelay(999);
	}
}

/*-------------------------------------------------------------------------------*/
void dseServoSet(uint16 servo, uint8 position) {
/*-------------------------------------------------------------------------------*/
	if (servo < NUM_SERVOS) {
		Servo[servo] = position;
		dseWriteBuffer(SELECT_SERVO, NUM_SERVOS, (char *) Servo);
	}
}
