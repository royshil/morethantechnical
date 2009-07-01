

#import <Foundation/Foundation.h>

@interface NyARToolkitWrapper : NSObject {
	CGContextRef cgctx;
	int ctxWidth;
	int ctxHeight;
	CGContextRef texCtx;
	
	bool wasInit;
}

@property (nonatomic,readonly) CGContextRef cgctx;
@property (nonatomic,readonly) CGContextRef texCtx;
@property (nonatomic,readonly) int ctxWidth;
@property (nonatomic,readonly) int ctxHeight;
@property (nonatomic,assign) bool wasInit;

unsigned char* load_file_image(const char* i_filename);
int _wrapper_main(int argc,char* argv[]);

CGContextRef CreateARGBBitmapContextWithWidthAndHeight(int pixelsWide, int pixelsHigh);
CGContextRef CreateARGBBitmapContext (CGImageRef inImage);

-(void)doMain;
-(void)initNyARTwithWidth:(int)width andHeight:(int)height;
-(bool)detectMarker:(float[])resultMat;
-(void)setNyARTWidth:(int)width andHeight:(int)height;
-(void)setNyARTBuffer:(Byte*)buf;
-(void)detectMarkerWithIamge:(CGImageRef)inImage intoMatrix:(float[])m;
-(void)getProjectionMatrix:(float[])m;

@end
