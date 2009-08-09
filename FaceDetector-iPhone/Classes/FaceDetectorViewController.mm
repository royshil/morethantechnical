//
//  FaceDetectorViewController.m
//  FaceDetector1
//

#import "FaceDetectorViewController.h"

#import "CameraTestAppDelegate.h"
#import "FaceAugmentingView.h"

extern "C" void detect_and_draw( IplImage* img, CvRect* found_face );
extern "C" void init_detection(char* cascade_location);
extern "C" void release_detection();


@implementation FaceDetectorViewController


// The designated initializer. Override to perform setup that is required before the view is loaded.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
        // Initialization code
		ctad = [[CameraTestAppDelegate alloc] init];
		[ctad doInit];
		
		NSString* myImage = [[NSBundle mainBundle] pathForResource:@"haarcascade_frontalface_alt" ofType:@"xml"];
		char* chars = (char*)malloc(512); 
		[myImage getCString:chars maxLength:512 encoding:NSUTF8StringEncoding];
		init_detection(chars);
		
		fav = [[FaceAugmentingView alloc] initWithFrame:[ctad getPreviewView].frame];
		fav.backgroundColor = [UIColor clearColor];
		//		fav.face = CGRectMake(100.0, 100.0, 50.0, 50.0);
		[self.view addSubview:fav];
		//		[fav setNeedsDisplay];
		
		[self.view setBackgroundColor:[UIColor whiteColor]];
		
		
		[self.view addSubview:[ctad getPreviewView]];
		[self.view sendSubviewToBack:[ctad getPreviewView]];
		
		repeatingTimer = [NSTimer scheduledTimerWithTimeInterval:0.0909 target:self selector:@selector(doDetection:) userInfo:nil repeats:YES];
		
//		UIToolbar* tb = [[UIToolbar alloc] init];
//		int h = self.frame.size.height;
//		int tbh = tb.frame.size.height;
//		tb.frame.origin = CGPointMake(0,h - tbh);
//		[self addSubview:tb];
//		[tb release];
    }
    return self;
}

-(void)doDetection:(NSTimer*) timer {
	if([ctad getPixelData]) {
		if(!im) {
			im = cvCreateImageHeader(cvSize([ctad getVideoSize].width,[ctad getVideoSize].height), 8, 4);
		}
		cvSetData(im, [ctad getPixelData],[ctad getBytesPerRow]);
		CvRect r;
		detect_and_draw(im,&r);
		if(r.width > 0 && r.height > 0) {
			fav.face = CGRectMake(r.x, r.y, r.width, r.height);
			
			[fav setNeedsDisplay];
		}
	}
}

-(IBAction)plusClicked:(id)sender {
	[fav toggleShowRect];
}

-(IBAction)hatClicked:(id)sender {
	fav.whatToPut = HAT;
}

-(IBAction)tieClicked:(id)sender {
	fav.whatToPut = TIE;
}

-(IBAction)bandanaClicked:(id)sender {
	fav.whatToPut = BANDANA;
}


/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
}
*/

/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/

/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}


- (void)dealloc {
	[repeatingTimer invalidate];
	release_detection();
	[ctad release];
	[fav release];
	
    [super dealloc];
}


@end
