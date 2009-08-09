//
//  CameraTestAppDelegate.h
//  CameraTest
//

#import <UIKit/UIKit.h>

@interface CameraTestAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
	id cameraController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) id cameraController;

- (IBAction)capturePhoto:(id)sender;
//- (IBAction)capturePreviewWithInstalledHook:(id)sender;
- (CGImageRef)capturePreviewWithInstalledHook;
//- (IBAction)capturePreviewsFromCoreSurfaces:(id)sender;
- (CGImageRef)capturePreviewsFromCoreSurfaces;

-(void)doInit;
-(void*)getPixelData;
-(CGSize)getVideoSize;
-(int)getPixelDataLength;
-(int)getBytesPerRow;
-(UIView*)getPreviewView;

@end

