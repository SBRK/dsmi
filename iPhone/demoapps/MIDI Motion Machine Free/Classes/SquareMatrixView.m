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
#import "SquareMatrixView.h"
#include <math.h>

@implementation SquareMatrixView


- (id)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
		ringView=[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"ring.png"]];
		[ringView setFrame:CGRectMake(0,0,5,5)];
		[self addSubview:ringView];
		[self animate:ringView];
    }
    return self;
}
-(void)finishedSelector:(NSString*)animationID finished:(NSNumber*)finished context:(void*)context{
	[self animate:ringView];
}	
- (void)animate:(UIImageView*)view{
	[UIView beginAnimations:nil context:NULL];
	[UIView setAnimationDuration:0.0];
	[UIView setAnimationDelegate:self];
	[ringView setAlpha:1];
	//[UIView setAnimationDidStopSelector:@selector(growAnimationDidStop:finished:context:)];
	CGAffineTransform transform =CGAffineTransformIdentity;
	view.transform = transform;
	[UIView commitAnimations];
	[view setAlpha:1];
	//CGAffineTransform transform =CGAffineTransformIdentity;
	[UIView beginAnimations:nil context:nil];
	[UIView setAnimationDuration:.60];
	[UIView setAnimationDelegate:self];
	[UIView setAnimationRepeatCount:1];
	[UIView setAnimationDidStopSelector:@selector(finishedSelector:finished:context:)];
	transform = CGAffineTransformMakeScale(50, 50);
	[view setAlpha:0];
	view.transform=transform;
	[UIView commitAnimations];
}
-(void) drawAroundPoint:(CGPoint) apoint
{
	ringView.center=apoint;	
}
- (void)drawRect:(CGRect)rect {
}

- (void)dealloc {
    [super dealloc];
}


@end
