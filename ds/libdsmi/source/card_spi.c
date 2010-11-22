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

#include "card_spi.h"
#include <nds.h>
#include <stdio.h>

static CARD_SPI_SETTINGS card_spi_settings;

/*-------------------------------------------------------------------------------*/
bool cardSpiBusy() {
/*-------------------------------------------------------------------------------*/
	return REG_AUXSPICNT & CR1_BUSY;
}

void cardSpiStart(bool hold) {
	/* disable IRQ because it's start of a critical section */
	REG_AUXSPICNT = card_spi_settings.clock | (hold ? CR1_HOLD_CS : 0) | CR1_CE | /* card_spi_settings.enable_irq | */ CR1_ENABLE_SLOT;
}

void cardSpiStop() {
	/* re-enable IRQ if they should be re-enabled to end critical section */
	REG_AUXSPICNT = card_spi_settings.clock | card_spi_settings.enable_irq;
}

/*-------------------------------------------------------------------------------*/
void cardSpiInit(CARD_SPI_CLOCK clock) {
/*-------------------------------------------------------------------------------*/
	card_spi_settings.clock = clock;
	card_spi_settings.enable_irq = 0;
#ifdef ARM9
	sysSetCardOwner(BUS_OWNER_ARM9);
#else
	sysSetCardOwner(BUS_OWNER_ARM7);
#endif
	cardSpiSetHandler(NULL);
	cardSpiStop();
}

/*-------------------------------------------------------------------------------*/
void cardSpiSetHandler(void (*irqHandler)(void)) {
/*-------------------------------------------------------------------------------*/
	card_spi_settings.enable_irq = irqHandler == NULL ? 0 : CR1_ENABLE_IRQ;
	irqSet(IRQ_CARD_LINE, irqHandler);
	irqEnable(IRQ_CARD_LINE);
	cardSpiStop();					/* update CR1 register */
}

/*-------------------------------------------------------------------------------*/
char cardSpiTransfer(char c) {
/*-------------------------------------------------------------------------------*/
	cardSpiStart(false);			/* enable card SPI */

	REG_AUXSPIDATA = c;				/* send charactr */
	while(cardSpiBusy());			/* busy wait */

	cardSpiStop();					/* disable card SPI */

#ifdef ARM9
	swiDelay(24);
#else
	swiDelay(12);
#endif

	return REG_AUXSPIDATA;
}

/*-------------------------------------------------------------------------------*/
void cardSpiTransferBuffer(char *out, char *in, uint16 count) {
/*-------------------------------------------------------------------------------*/
	uint16 i;

	if (count == 0) {
		return;
	} else if (count == 1) {
		*in = cardSpiTransfer(*out);
		return;
	}

	/* enable card SPI with CS hold */
	cardSpiStart(true);

	for ( i = 0; i < count-1; i++ ) {
		REG_AUXSPIDATA = out[i];		/* send character */
		while(cardSpiBusy());		/* busy wait */
		in[i] = REG_AUXSPIDATA;		/* receive character */
	}

	/* last character has to be transferred with chip select hold disabled */
	cardSpiStart(false);

	REG_AUXSPIDATA = out[count-1];	/* send last character */
	while(cardSpiBusy());			/* busy wait */
	in[count-1] = REG_AUXSPIDATA;		/* receive character */

	/* disable card SPI */
	cardSpiStop();

#ifdef ARM9
	swiDelay(24);
#else
	swiDelay(12);
#endif
}

/*-------------------------------------------------------------------------------*/
void cardSpiWriteBuffer(char *out, uint16 count) {
/*-------------------------------------------------------------------------------*/
	uint16 i;

	if (count == 0) {
		return;
	} else if (count == 1) {
		cardSpiTransfer(*out);
		return;
	}

	/* enable card SPI with CS hold */
	cardSpiStart(true);

	for ( i = 0; i < count-1; i++ ) {
		REG_AUXSPIDATA = out[i];		/* send character */
		while(cardSpiBusy());		/* busy wait */
	}

	/* last character has to be transferred with chip select hold disabled */
	cardSpiStart(false);

	REG_AUXSPIDATA = out[count-1];	/* send last character */
	while(cardSpiBusy());			/* busy wait */

	/* disable card SPI */
	cardSpiStop();

#ifdef ARM9
	swiDelay(24);
#else
	swiDelay(12);
#endif
}
