/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//																			   //
//                                                                             //
// Version 1.0                                                                 //
// by (Collin Meyer)                                                           //
//                                                                             //
// Licensed under LGPL                                                         //
//                                                                             //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////
#import "MIDI_Motion_MachineAppDelegate.h"

@implementation MIDI_Motion_MachineAppDelegate

@synthesize window;


- (void)applicationDidFinishLaunching:(UIApplication *)application {    
	// ***  Initialize DSMI
	//**********************
	libdsmi=[[libdsmi_iphone alloc] init];
	// ***************
	
	[[UIAccelerometer sharedAccelerometer] setDelegate:self]; // initialize accelerometer and set this object as delegate
	
	smv=[[SquareMatrixView alloc] initWithFrame:CGRectMake(0,20,320,420)]; // Create acclerometer status display
	[window addSubview:smv];
	
	// Make some buttons
	UIButton *sendXCC=[[UIButton buttonWithType:UIButtonTypeRoundedRect] retain];
	[sendXCC setFrame:CGRectMake(0, 440, 70, 40)];
	[sendXCC setTitle:@"teach x" forState:UIControlStateNormal];
	[sendXCC setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
	[sendXCC setTitleShadowOffset:CGSizeMake(1,1)];
	[sendXCC addTarget:self action:@selector(sendXCCAction:) forControlEvents:UIControlEventTouchUpInside];
	UIButton *sendYCC=[[UIButton buttonWithType:UIButtonTypeRoundedRect] retain];
	[sendYCC setFrame:CGRectMake(70, 440, 70, 40)];
	[sendYCC setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
	[sendYCC setTitleShadowOffset:CGSizeMake(1,1)];
	[sendYCC setTitle:@"teach y" forState:UIControlStateNormal];
	[sendYCC addTarget:self action:@selector(sendYCCAction:) forControlEvents:UIControlEventTouchUpInside];
	holdButton=[[UIButton buttonWithType:UIButtonTypeRoundedRect] retain];
	[holdButton setFrame:CGRectMake(250, 440, 70, 40)];
	[holdButton setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
	[holdButton setTitleShadowOffset:CGSizeMake(1,1)];
	[holdButton setTitle:@"Hold" forState:UIControlStateNormal];
	[holdButton addTarget:self action:@selector(holdAction:) forControlEvents:UIControlEventTouchUpInside];
	[window addSubview:sendXCC];
	[window addSubview:sendYCC];
	[window addSubview:holdButton];
	
	[window setBackgroundColor:[UIColor grayColor]];
    [window makeKeyAndVisible];
}

- (void)sendXCCAction:(id)sender
{
	// *** Use DSMI to send a MIDI_CC message on MIDI Channel 0, CC#1, level 50
	[libdsmi writeMIDIMessage:MIDI_CC MIDIChannel:0 withData1:1 withData2:50];
	
}
-(void)sendYCCAction:(id)sender
{
	// *** Use DSMI to send a MIDI_CC message on MIDI Channel 0, CC#2, level 50
	[libdsmi writeMIDIMessage:MIDI_CC MIDIChannel:0 withData1:2 withData2:50];	
}
-(void)holdAction:(id)sender
{
	if([holdButton.currentTitle caseInsensitiveCompare:@"Hold"]==0)
	{
		// Stop the accelerometer
		[holdButton setTitle:@"UnHold" forState:UIControlStateNormal];
		[[UIAccelerometer sharedAccelerometer] setDelegate:nil];
		
	}
	else
	{
		// Start the accelerometer again
		[holdButton setTitle:@"Hold" forState:UIControlStateNormal];
		[[UIAccelerometer sharedAccelerometer] setDelegate:self];
	}
}
- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration 
{
	if([holdButton.currentTitle caseInsensitiveCompare:@"Hold"]==0)
	{
		// do some math to scale the accelerometer value (-1 to +1) to MIDI values (0 to 127)
		float xvalue=(acceleration.x+1)/.016;  // = 0-127
		float yvalue=(acceleration.y+1)/.016;  // = 0-127
		CGPoint drawPoint=CGPointMake((acceleration.x+1)/.00625,(2-(acceleration.y+1))/.004762);
		[smv drawAroundPoint:drawPoint]; // This draws our fun graphic at a point on the screen 
		
		// *** Use DSMI to send a MIDI_CC message on MIDI Channel 0, CC#1, Use the accelerometer value as the CC value
		[libdsmi writeMIDIMessage:MIDI_CC MIDIChannel:0 withData1:1 withData2:(unsigned char)xvalue];
		
		// *** Use DSMI to send a MIDI_CC message on MIDI Channel 0, CC#2, Use the accelerometer value as the CC value
		[libdsmi writeMIDIMessage:MIDI_CC MIDIChannel:0 withData1:2 withData2:(unsigned char)yvalue];	
	}
}
- (void)dealloc {
    [window release];
    [super dealloc];
}


@end
