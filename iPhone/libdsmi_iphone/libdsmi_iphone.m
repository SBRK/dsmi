/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//                DSMI_iPhone - iPhone/iPod touch port of the DSMI library     //
//                                                                             //
// Version .9                                                                 //
// by 0xtob (Tobias Weyand) & TheRain (Collin Meyer)                           //
// OSC client by fishuyo                                                       //
// Licensed under LGPL                                                         //
//                                                                             //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

#import "libdsmi_iphone.h"

#define PC_PORT		9000
#define IPHONE_PORT		9001
#define IPHONE_SENDER_PORT	9002
// Message types must be sent with a MIDI Channel # (see MIDI spec for more details)
// Usage like so:  write_MIDI(NOTE_ON|0x01, 60, 127);
//    where the MIDI channel (0-15) is being OR'd with the note on message,
//    60 is the note number, and 127 is the keypress velocity.

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
@implementation libdsmi_iphone
- (id) init
{
	if (self = [super init])
	{
		int result;
		int on=1;
		sock = socket(AF_INET, SOCK_DGRAM, 0); // setup socket for DGRAM (UDP), returns with a socket handle
		in_size=sizeof(in);
		sockin = socket(AF_INET, SOCK_DGRAM, 0);
		bzero(&addr_in, sizeof(addr_in)); 
		addr_in.sin_family = AF_INET;
		addr_in.sin_port = htons(IPHONE_PORT);
		addr_in.sin_addr.s_addr = INADDR_ANY;
		bind(sockin, (struct sockaddr*)&addr_in, sizeof(addr_in));
		
		// Destination
		addr_out_to.sin_family = AF_INET;
		addr_out_to.sin_port = htons(PC_PORT);
		
		// Source
		addr_out_from.sin_family = AF_INET;
		addr_out_from.sin_port = htons(IPHONE_SENDER_PORT);
		addr_out_from.sin_addr.s_addr = htonl(INADDR_ANY);
		result=setsockopt(sock,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on));
		result=bind(sock, (struct sockaddr*)&addr_out_from, sizeof(addr_out_from));
		[self getBroadCastIp];
	}
	return self;
}
-(void) getBroadCastIp
{
	const char *ifn="en0";
	struct in_addr broadaddr;
	int return_val;
	int fd = -1;
	struct ifreq ifr;	
	fd = socket (PF_INET, SOCK_DGRAM, 0);
	
	strncpy (ifr.ifr_name, ifn, sizeof(ifr.ifr_name));
	
	// Fetch the broadcast address of this interface by calling ioctl()
	return_val = ioctl(fd,SIOCGIFBRDADDR, &ifr);
	
	if (return_val == 0 ) {
		if (ifr.ifr_broadaddr.sa_family == AF_INET) {
			broadaddr =((struct sockaddr_in *)&ifr.ifr_broadaddr)->sin_addr; //sin_ptr->sin_addr;
		}
		
	}
	
	//close(fd);
	//addr_in.sin_addr=broadaddr;
	addr_out_to.sin_addr=broadaddr;
}
- (void) writeMIDIMessage:(unsigned char) messageType MIDIChannel:(unsigned char)midichannel withData1:(unsigned char)data1 withData2:(unsigned char) data2
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	unsigned char message=messageType|midichannel;
	char sendbuf[3] = {message, data1, data2};
	int result=0;	
	result=sendto(sock, &sendbuf, 3, 0, (struct sockaddr*)&addr_out_to, sizeof(addr_out_to));
	[pool release];
}
- (void) listenerSelector
{
	while(1)
	{
		int res = recvfrom(sockin, recbuf, 3, 0, (struct sockaddr*)&in, &in_size);
		if(res <= 0){
		}
		else
		{
			unsigned char MessageType=recbuf[0]&0xF0;
			unsigned char MIDIChannel=recbuf[0]&0x0F;
			[MIDIListenerInvocation setArgument:(void*)&MessageType atIndex:2];
			[MIDIListenerInvocation setArgument:(void*)&MIDIChannel atIndex:3];
			[MIDIListenerInvocation setArgument:(void*)&recbuf[1] atIndex:4];
			[MIDIListenerInvocation setArgument:(void*)&recbuf[2] atIndex:5];
			[MIDIListenerInvocation retainArguments];	
			[MIDIListenerInvocation invoke];
		}
	}
}
- (void) startMIDIListener:(id)target withSelector:(SEL)selector
{
	[self writeMIDIMessage:NOTE_OFF MIDIChannel:0xFF withData1:0xFF withData2:0];
	NSMethodSignature * sig = nil;
	sig = [[target class] instanceMethodSignatureForSelector:selector];
	MIDIListenerInvocation = [[NSInvocation invocationWithMethodSignature:sig] retain];
	[MIDIListenerInvocation setTarget:target];
	[MIDIListenerInvocation setSelector:selector];
	MIDIListenerThread=[[NSThread alloc] initWithTarget:self selector:@selector(listenerSelector) object:nil];
	[MIDIListenerThread start];
}
-(void) stopMIDIListener
{
	[MIDIListenerThread cancel];
	[MIDIListenerThread dealloc];
	[MIDIListenerInvocation dealloc];
}
// ------------ OSC WRITE ------------ //

// Resets the OSC buffer and sets the destination open sound control address, returns 1 if ok, 0 if address string not valid
-(int) OSCNew:(NSString *) addr{
	osc_init( &osc_buffer);
	return osc_writeaddr( &osc_buffer,(char *)[addr cStringUsingEncoding: NSASCIIStringEncoding]);
}

// Adds arguments to the OSC packet
-(int) OSCAddIntArg:(long) arg{
	return osc_addintarg( &osc_buffer, arg);
}

-(int) OSCAddStringArg:(NSString *) addr{
	return osc_addstringarg( &osc_buffer,  (char *)[addr cStringUsingEncoding: NSASCIIStringEncoding]);
}

// Sends the OSC packet
-(int) OSCSend{
	char* msg = osc_getPacket( &osc_buffer);
	int size = osc_getPacketSize( &osc_buffer);
	inet_aton([ipAddress cStringUsingEncoding: NSASCIIStringEncoding],&saddr.sin_addr);
	addr_out_to.sin_addr=saddr.sin_addr;
	return sendto(sock, msg, size, 0, (struct sockaddr*)&addr_out_to, sizeof(addr_out_to));
}

@end
