//
//  FaceAugmentingView.h
//  FaceDetector1
//

#import <UIKit/UIKit.h>

enum FACE_AUGMENT {
	HAT = 0,TIE = 1,BANDANA = 2
};

@interface FaceAugmentingView : UIView {
	int whatToPut;
	CGRect face;
	UIImage* bHatIm,*tieIm, *bandanaIm;	
	BOOL showRect;
}

@property (nonatomic,assign) CGRect face;
@property (nonatomic,assign) int whatToPut;

-(void)toggleShowRect;

@end
