

#import "3DGraphicsView.h"
#import <UIKit/UIKit.h>
#import "NyARToolkitWrapper.h"

#import "CameraTestAppDelegate.h"

@interface NyARToolkitCrossCompileAppDelegate : NSObject <UIApplicationDelegate, _DGraphicsViewDelegate> {
    UIWindow *window;
	NyARToolkitWrapper* nyartwrapper;
	
	float ogl_camera_matrix[16];
	float ogl_projection_matrix[16];
	
	/* OpenGL name for the sprite texture */
    GLuint spriteTexture;
	
	CameraTestAppDelegate* ctad;
	
	bool debug_alreadyLoadedPic;
	
	NSTimer* repeatingTimer;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;

-(void)load2DTex:(NSTimer*) timer;
-(void)load2DTexWithBytes:(NSTimer*) timer;
-(void)load2DTexFromFile:(NSTimer*) timer;

@end

