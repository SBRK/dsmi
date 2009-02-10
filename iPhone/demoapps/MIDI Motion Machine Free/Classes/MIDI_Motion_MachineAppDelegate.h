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
#import <UIKit/UIKit.h>
#import "SquareMatrixView.h"
#import "libdsmi_iphone.h"
#define kAccelerometerFrequency     40
@interface MIDI_Motion_MachineAppDelegate : NSObject <UIAccelerometerDelegate> {
    UIWindow *window;
	libdsmi_iphone *libdsmi;
	SquareMatrixView *smv;
	UILabel *xLabel;
	UILabel *yLabel;
	UIButton *holdButton;
}
- (void)sendXCCAction:(id)sender;
- (void)sendYCCAction:(id)sender;
-(void)holdAction:(id)sender;
@property (nonatomic, retain) IBOutlet UIWindow *window;
@end

