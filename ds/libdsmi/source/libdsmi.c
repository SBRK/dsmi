#include <nds.h>

#include <dswifi9.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "libdsmi.h"
#include "dserial.h"
#include "firmware_bin.h"
#include "osc_client.h"

#define PC_PORT		9000
#define DS_PORT		9001
#define DS_SENDER_PORT	9002

int sock, sockin;
struct sockaddr_in addr_out_from, addr_out_to, addr_in;

OSCbuf osc_buffer;
OSCbuf osc_recv_buff;

char recbuf[3];

int in_size;
struct sockaddr_in in;

int default_interface = -1;

int wifi_enabled = 0;
int dserial_enabled = 0;

extern void wifiValue32Handler(u32 value, void* data);
extern void arm9_synctoarm7();

// ------------ PRIVATE ------------ //

void dsmi_uart_recv(char * data, unsigned int size)
{
	// TODO
}

// ------------ SETUP ------------ //

// If a DSerial is inserted, this sets up the connection to the DSerial.
// Else, it connects to the default access point stored in Nintendo WFC
// memory (use an official game, e.g. mario kart to set this up)
// The initialized interface is set as the default interface.
//
// Returns 1 if connected, and 0 if failed.
extern int dsmi_connect(void)
{
	if(dsmi_connect_dserial()) {
		return 1;
	}
	
	if(dsmi_connect_wifi()) {
		return 1;
	}
	
	return 0;
}


// Using these you can force a wifi connection even if a DSerial is
// inserted or set up both connections for forwarding.
extern int dsmi_connect_dserial(void)
{
	if(!dseInit())
		return 0;
	
	int version = dseVersion();
	//if(version < 2) {
	//	printf("Version: DSerial1/2\n");
	//} else if(version == 2) {
	//	printf("Version: DSerial Edge\n");
	//}
	
	// Upload firmware if necessary
	if (!dseMatchFirmware((char*)firmware_bin, firmware_bin_end - firmware_bin))
	{
		dseUploadFirmware((char *) firmware_bin, firmware_bin_end - firmware_bin);
	}
	
	dseBoot();
	
	swiDelay(9999); // Wait for the FW to boot
	if (dseStatus() != FIRMWARE)
		return 0;
	
	dseSetModes(ENABLE_CMOS);
	
	dseUartSetBaudrate(UART0, 31250); // MIDI baud rate
	
	dseUartSetReceiveHandler(UART0, dsmi_uart_recv);
	
	default_interface = DSMI_SERIAL;

	dserial_enabled = 1;
	
	return 1;
}

void dsmi_timer_50ms(void) {
    Wifi_Timer(50);

    if(wifi_enabled == 1 && default_interface == DSMI_WIFI)
    {
        // Send a keepalive beacon every 3 seconds
        static u8 counter = 0;
        counter++;
        if(counter == 60)
        {
            counter = 0;
            dsmi_write(0, 0, 0);
        }
    }
}

// Modified version of dswifi's init function that uses a custom timer handler
// In addition to calling Wifi_Timer, new new handler also sends the DSMI keepalive
// beacon.
bool dsmi_wifi_init(void) {
    fifoSetValue32Handler(FIFO_DSWIFI,  wifiValue32Handler, 0);

    u32 wifi_pass = Wifi_Init(WIFIINIT_OPTION_USELED);

    if(!wifi_pass) return false;

    irqSet(IRQ_TIMER3, dsmi_timer_50ms); // setup timer IRQ
    irqEnable(IRQ_TIMER3);

    Wifi_SetSyncHandler(arm9_synctoarm7); // tell wifi lib to use our handler to notify arm7

    // set timer3
    TIMER3_DATA = -6553; // 6553.1 * 256 cycles = ~50ms;
    TIMER3_CR = 0x00C2; // enable, irq, 1/256 clock

    fifoSendAddress(FIFO_DSWIFI, (void *)wifi_pass);

    while(Wifi_CheckInit()==0) {
        swiWaitForVBlank();
    }

    int wifiStatus = ASSOCSTATUS_DISCONNECTED;

    Wifi_AutoConnect(); // request connect

    while(wifiStatus != ASSOCSTATUS_ASSOCIATED) {
        wifiStatus = Wifi_AssocStatus(); // check status

        if(wifiStatus == ASSOCSTATUS_CANNOTCONNECT) return false;
        swiWaitForVBlank();

    }

    return true;    
}

extern int dsmi_connect_wifi(void)
{
    Wifi_EnableWifi();

	if(!dsmi_wifi_init()) {
        Wifi_DisableWifi();
		return 0;
	}
	
	int i = Wifi_AssocStatus();
	if(i == ASSOCSTATUS_CANNOTCONNECT) {
		return 0;
	} else if(i == ASSOCSTATUS_ASSOCIATED) {
		sock = socket(AF_INET, SOCK_DGRAM, 0); // setup socket for DGRAM (UDP), returns with a socket handle
		sockin = socket(AF_INET, SOCK_DGRAM, 0);
		
		// Source
		addr_out_from.sin_family = AF_INET;
		addr_out_from.sin_port = htons(DS_SENDER_PORT);
		addr_out_from.sin_addr.s_addr = INADDR_ANY;
		
		// Destination
		addr_out_to.sin_family = AF_INET;
		addr_out_to.sin_port = htons(PC_PORT);
		
		struct in_addr gateway, snmask, dns1, dns2;
		Wifi_GetIPInfo(&gateway, &snmask, &dns1, &dns2);

		unsigned long my_ip = Wifi_GetIP(); // Set IP to broadcast IP
		unsigned long bcast_ip = my_ip | ~snmask.s_addr;
		
		addr_out_to.sin_addr.s_addr = bcast_ip;
		
		// Receiver
		addr_in.sin_family = AF_INET;
		addr_in.sin_port = htons(DS_PORT);
		addr_in.sin_addr.s_addr = INADDR_ANY;
		
		bind(sock, (struct sockaddr*)&addr_out_from, sizeof(addr_out_from));
		bind(sockin, (struct sockaddr*)&addr_in, sizeof(addr_in));
		
		u8 val = 1;
		ioctl(sockin, FIONBIO, (char*)&val);  // Enable non-blocking I/O
		
		default_interface = DSMI_WIFI;
		wifi_enabled = 1;
		
		return 1;
	} else {
		return 0;
	}
}



// ------------ WRITE ------------ //

// Send a MIDI message over the default interface, see MIDI spec for more details
extern void dsmi_write(u8 message,u8 data1, u8 data2)
{
	if(default_interface == DSMI_WIFI)
		dsmi_write_wifi(message, data1, data2);
	else if(default_interface == DSMI_SERIAL)
		dsmi_write_dserial(message, data1, data2);
}


// Force a MIDI message to be sent over DSerial
extern void dsmi_write_dserial(u8 message,u8 data1, u8 data2)
{
	u8 sendbuf[3] = {message, data1, data2};
	dseUartSendBuffer(UART0, (char*)sendbuf, 3, true);
}


// Force a MIDI message to be sent over Wifi
extern void dsmi_write_wifi(u8 message,u8 data1, u8 data2)
{
	char sendbuf[3] = {message, data1, data2};
	sendto(sock, &sendbuf, 3, 0, (struct sockaddr*)&addr_out_to, sizeof(addr_out_to));
}


// ------------ OSC WRITE ------------ //

// Resets the OSC buffer and sets the destination open sound control address, returns 1 if ok, 0 if address string not valid
extern int dsmi_osc_new( char* addr){

  osc_init( &osc_buffer);
  return osc_writeaddr( &osc_buffer, addr);
  
}

// Adds arguments to the OSC packet
extern int dsmi_osc_addintarg( long arg){

  return osc_addintarg( &osc_buffer, arg);

}
extern int dsmi_osc_addstringarg( char* arg){

  return osc_addstringarg( &osc_buffer, arg);
}

extern int dsmi_osc_addfloatarg( float arg){

  return osc_addfloatarg( &osc_buffer, arg);

}

// Sends the OSC packet
extern int dsmi_osc_send(void){

  char* msg = osc_getPacket( &osc_buffer);
  int size = osc_getPacketSize( &osc_buffer);
  
  return sendto(sock, msg, size, 0, (struct sockaddr*)&addr_out_to, sizeof(addr_out_to));

}

// ------------ READ ------------ //

// Checks if a new message arrived at the default interface and returns it by
// filling the given pointers
//
// Returns 1, if a message was received, 0 if not
extern int dsmi_read(u8* message, u8* data1, u8* data2)
{
	if(default_interface == DSMI_WIFI)
		return dsmi_read_wifi(message, data1, data2);
	//else if(default_interface == DSMI_SERIAL)
	//	dsmi_write_serial(message, data1, data2);
	else
		return 0;
}


// Force receiving over DSerial / Wifi
// extern int dsmi_read_dserial(u8* message, u8* data1, u8* data2); // TODO


// Force receiving over DSerial / Wifi
extern int dsmi_read_wifi(u8* message, u8* data1, u8* data2)
{
	int res = recvfrom(sockin, recbuf, 3, 0, (struct sockaddr*)&in, &in_size);
	
	if(res <= 0)
		return 0;
	
	*message = recbuf[0];
	*data1 = recbuf[1];
	*data2 = recbuf[2];
	
	return 1;
}

// ------------ OSC READ-------- //
extern int dsmi_osc_read(){

	int res = recvfrom(sockin, osc_recv_buff.buffer, OSC_MAX_SIZE, 0, (struct sockaddr*)&in, &in_size);
	
	if(res <= 0)
		return 0;

	return osc_decodePacket( &osc_recv_buff);
}
extern const char* dsmi_osc_getaddr(){

	return osc_getaddr( &osc_recv_buff);

}
extern int dsmi_osc_getnextarg( void* data, size_t* size, char* type ){

	return osc_getnextarg( &osc_recv_buff, data, size, type);

}

// ------------ MISC ------------ //

// Returns the default interface (DSMI_SERIAL or DSMI_WIFI)
extern int dsmi_get_default_interface(void)
{
	return default_interface;
}
