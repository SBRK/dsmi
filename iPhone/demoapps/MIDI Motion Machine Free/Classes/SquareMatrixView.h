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
#import "libdsmi_iphone.h"
@interface SquareMatrixView : UIView {
	UIImageView *ringView;
}
-(void) drawAroundPoint:(CGPoint) point;
@end
