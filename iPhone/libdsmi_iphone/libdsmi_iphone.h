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

// note on is 0x9n where n is the MIDI Channel
// data1 is note number, data2 is key velocity 
#define NOTE_ON 0x90

// note off is 0x8n where n is the MIDI channel
// data1 is note number, data2 is release velocity
#define NOTE_OFF 0x80

// midi CC (control change) is 0xBn where n is the MIDI channel
// data1 is the control number, data2 is the control value 0-127
#define MIDI_CC 0xB0

// midi PC (pitch change) is 0xEn where n is the MIDI channel
// the message is a 14 bit value, where
// data1 is the least significant 7 bits and data2 is the most significant 7 bits
#define MIDI_PC	0xE0
@interface libdsmi_iphone : NSObject {
	int sock; 
	int sockin;
	struct sockaddr_in addr_out_from;
	struct sockaddr_in addr_out_to;
	struct sockaddr_in addr_in;
	struct sockaddr_in saddr;
	NSString *ipAddress;
	char recbuf[3];
	socklen_t in_size;
	struct sockaddr_in in;
	OSCbuf osc_buffer;
	NSThread *MIDIListenerThread;
	NSInvocation *MIDIListenerInvocation;
}
- (id) init;
- (void) writeMIDIMessage:(unsigned char) messageType MIDIChannel:(unsigned char)midichannel withData1:(unsigned char)data1 withData2:(unsigned char) data2;
- (void) startMIDIListener:(id)target withSelector:(SEL)selector;
- (void) stopMIDIListener;
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