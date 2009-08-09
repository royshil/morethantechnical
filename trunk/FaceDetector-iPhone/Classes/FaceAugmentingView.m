//
//  FaceAugmentingView.m
//  FaceDetector1
//

#import "FaceAugmentingView.h"


@implementation FaceAugmentingView
@synthesize face,whatToPut;

- (id)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        // Initialization code
		bandanaIm = [UIImage imageNamed:@"bandana.png"];
		[bandanaIm retain];
		tieIm = [UIImage imageNamed:@"tie-w-alpha.png"];
		[tieIm retain];
		bHatIm = [UIImage imageNamed:@"bowler-hat-black-w-alpha.png"];
		[bHatIm retain];
		
		whatToPut = TIE;
    }
    return self;
}

-(void)toggleShowRect { showRect = !showRect;}

- (void)drawRect:(CGRect)rect {
 	if(face.origin.x > 0 && face.size.width > 0) {
//		NSLog(@"%.0f %.0f %.0f %.0f",face.origin.x,face.origin.y,face.size.width,face.size.height);
		CGContextRef ctx = UIGraphicsGetCurrentContext();
		CGContextTranslateCTM (ctx, self.frame.size.width, self.frame.size.height);
		CGContextRotateCTM (ctx, M_PI);
		CGRect r = face;
		r.origin.x = self.frame.size.width - r.origin.x - r.size.width;
		r.origin.y = self.frame.size.height - r.origin.y - r.size.height;
		if(whatToPut == HAT) {
			CGSize bhatdim = bHatIm.size;
			float hat_to_face = bhatdim.width / r.size.width;
			float hat_w_to_h = bhatdim.width / bhatdim.height;
			bhatdim.width = (bhatdim.width / hat_to_face) * 1.3;
			bhatdim.height = bhatdim.width / hat_w_to_h;
			CGRect todraw = CGRectMake(r.origin.x - bhatdim.width / 6, r.origin.y + bhatdim.height/2, bhatdim.width, bhatdim.height);
			CGContextDrawImage(ctx, todraw, bHatIm.CGImage);
		} else if (whatToPut == TIE) {
			CGSize dim = tieIm.size;
			float i_to_face = dim.width / r.size.width;
			float i_w_to_h = dim.width / dim.height;
			dim.width = (dim.width / i_to_face) * 1.1;
			dim.height = dim.width / i_w_to_h;
			CGRect todraw = CGRectMake(r.origin.x, r.origin.y - dim.height, dim.width, dim.height);
			CGContextDrawImage(ctx, todraw, tieIm.CGImage);
		} else if (whatToPut == BANDANA) {
			CGSize dim = bandanaIm.size;
			float i_to_face = dim.width / r.size.width;
			float i_w_to_h = dim.width / dim.height;
			dim.width = (dim.width / i_to_face);
			dim.height = dim.width / i_w_to_h;
			CGRect todraw = CGRectMake(r.origin.x, r.origin.y - dim.height, dim.width, dim.height);
			CGContextDrawImage(ctx, todraw, bandanaIm.CGImage);
		}
		if(showRect) {
			CGContextSetRGBStrokeColor(ctx, 1, 0, 0, 1);
			UIRectFrame(r);
		}
	}
}


- (void)dealloc {
	[tieIm release];
	[bandanaIm release];
	[bHatIm release];
    [super dealloc];
}


@end
