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

#ifndef DSERIAL_H
#define DSERIAL_H

#include <nds.h>
#include "card_spi.h"

#define MAX_DATA_SIZE			32
#define NUM_SERVOS				5

#define SELECT_READ				0x00
#define SELECT_WRITE			0x80
#define SELECT_INTERRUPT		0x01
#define SELECT_CHECK			0x9F

#define SELECT_FLASH			0x02
#define SELECT_FLASH_ERASE		0x03
#define SELECT_BOOT				0x04
#define SELECT_REGISTER			0x05
#define SELECT_ENABLE			0x06

#define SELECT_UART_BUFFER		0x11

#define SELECT_ADC				0x20

#define SELECT_SERVO			0x30

/* Enable Flags */
#define ENABLE_RS232			0x01
#define ENABLE_CMOS				0x00
#define ENABLE_SERVO			0x02

/* Interrupt Flags */
#define INTERRUPT_BOOTLOADER	0x01
#define INTERRUPT_UART_SENT		0x02
#define INTERRUPT_UART_RECEIVED	0x04
#define INTERRUPT_IO			0x08
#define INTERRUPT_ADC			0x10
#define INTERRUPT_PWM			0x20
#define INTERRUPT_USB			0x40

#ifdef __cplusplus
extern "C" {
#endif
	/* Type Defines */
	typedef enum { DISCONNECTED, BOOTLOADER, FIRMWARE } DseStatus;

	/* Helpers */
	void dseWriteBuffer(uint8 selector, uint8 size, char * data);
	uint8 dseReadBuffer(uint8 selector, char * data);
	bool dseReadFlash(char * data, uint16 pos, uint8 size);
	void dseWriteRegister(uint8 reg, uint8 val);
	uint8 dseReadRegister(uint8 reg);

	/* Configuration */
	bool dseInit();
	DseStatus dseStatus();
	bool dseUploadFirmware(char * data, unsigned int fwsize);
	bool dseMatchFirmware(char * data, unsigned int fwsize);
	bool dseBoot();
	void dseSetModes(unsigned char modes);

	/* UART */
	bool dseUartSendBuffer(char * data, unsigned int size, bool blocking);
	bool dseUartSetBaudrate(unsigned int baudrate);
	void dseUartSetReceiveHandler(void (*receiveHandler)(char * data, unsigned int size));
	void dseUartSetSendHandler(void (*sendHandler)(void));

	/* ADC */
	uint8 dseAdcRead(char * buffer);

	/* Servo */
	void dseServoSetAll(uint8 * positions);
	void dseServoSet(uint16 servo, uint8 position);

	/* Misc */
	void dseUartDefaultReceiveHandler(char * data, unsigned int size);

#ifdef __cplusplus
};
#endif

#endif // CARD_SPI_H
