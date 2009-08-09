//
//  FaceDetector1AppDelegate.h
//  FaceDetector1
//

#import <UIKit/UIKit.h>

@class FaceDetectorViewController;

@interface FaceDetector1AppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
	FaceDetectorViewController* c;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;

@end

