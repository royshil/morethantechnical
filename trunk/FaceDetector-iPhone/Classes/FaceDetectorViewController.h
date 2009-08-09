//
//  FaceDetectorViewController.h
//  FaceDetector1
//

#import <UIKit/UIKit.h>

#include "cv.h"
#include "highgui.h"

@class CameraTestAppDelegate, FaceAugmentingView;

@interface FaceDetectorViewController : UIViewController {
	CameraTestAppDelegate* ctad;
	IplImage* im;
	NSTimer* repeatingTimer;
	
	FaceAugmentingView* fav;

}

-(IBAction)hatClicked:(id)sender;
-(IBAction)tieClicked:(id)sender;
-(IBAction)bandanaClicked:(id)sender;
-(IBAction)plusClicked:(id)sender;
-(void)doDetection:(NSTimer*) timer;

@end
