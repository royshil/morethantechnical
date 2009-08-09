//
//  FaceDetector1AppDelegate.m
//  FaceDetector1
//

#import "FaceDetector1AppDelegate.h"

#import "FaceDetectorViewController.h"

@implementation FaceDetector1AppDelegate

@synthesize window;


- (void)applicationDidFinishLaunching:(UIApplication *)application {   
	c = [[FaceDetectorViewController alloc] initWithNibName:@"FaceDetectorView" bundle:[NSBundle mainBundle]];
	[window addSubview:c.view];

    // Override point for customization after application launch
    [window makeKeyAndVisible];
}


- (void)dealloc {
	[c release];
    [window release];
    [super dealloc];
}


@end
