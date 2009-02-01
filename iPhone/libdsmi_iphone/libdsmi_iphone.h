/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//                DSMI_iPhone - iPhone/iPod touch port of the DSMI library     //
//                                                                             //
// Version 01-23-2009                                                          //
// by 0xtob (Tobias Weyand) & TheRain (Collin Meyer)                           //
// OSC client by fishuyo                                                       //
// Licensed under LGPL                                                         //
//                                                                             //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

#import <Foundation/Foundation.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include "osc_client.h"
@interface libdsmi_iphone : NSObject {
	int sock; 
	int sockin;
	struct sockaddr_in addr_out_from;
	struct sockaddr_in addr_out_to;
	struct sockaddr_in addr_in;
	struct sockaddr_in saddr;
	NSString *ipAddress;
	char recbuf[3];
	
	int in_size;
	struct sockaddr_in in;
	OSCbuf osc_buffer;
}
- (id) init;
- (void) setIPString:(NSString *) ip;
- (void) writeMIDIMessage:(unsigned char) message withData1:(unsigned char)data1 withData2:(unsigned char) data2;
// ------------ OSC WRITE ------------ //
// OSC messages are sent only over wifi and do not require the dsmidiwifi server application
//   To send and OSC message:
//     1. call dsmi_osc_new with an OSC address string ( i.e. anything starting with '/' )
//     2. add up to 31 arguments (an arbitrary limit I set) given that none of the arguments are gigantic strings
//     3. call dsmi_osc_send()
//     4. repeat!

// Resets the OSC buffer and sets the destination open sound control address, returns 1 if ok, 0 if address string not valid
-(int) OSCNew:(NSString *) addr;

// Adds arguments to the OSC packet
-(int) OSCAddIntArg:(long) arg;
-(int) OSCAddStringArg:(NSString *) addr;

// Sends the OSC packet
-(int) OSCSend;
@end