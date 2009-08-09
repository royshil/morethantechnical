//
//  FaceDetectView.m
//  FaceDetector1
//
//  Created by Innovation Labs on 8/3/09.
//  Copyright 2009 Comverse LTD. All rights reserved.
//
//
//#import "FaceDetectView.h"
//
//
//@implementation FaceDetectView
//
//- (id)initWithFrame:(CGRect)frame {
//    if (self = [super initWithFrame:frame]) {
//	}
//    return self;
//}
//
//
//- (void)drawRect:(CGRect)rect {
//	;//nop
//}
//
//- (void)manipulateOpenCVImagePixelDataWithCGImage:(CGImageRef)inImage openCVimage:(IplImage *)openCVimage  
//{  
//    // Create the bitmap context  
//    CGContextRef cgctx = [self createARGBBitmapContext:inImage];  
//    if (cgctx == NULL)  
//    {  
//        // error creating context  
//        return;  
//    }  
//	
//    int height,width,step,channels;  
//    uchar *cvdata;  
//    //int i,j,k;  
//    int x,y;  
//	
//    height = openCVimage->height;  
//    width = openCVimage->width;  
//    step = openCVimage->widthStep;  
//    channels = openCVimage->nChannels;  
//    cvdata = (uchar *)openCVimage->imageData;  
//	
//    CGRect rect = {{0,0},{width,height}};  
//	
//    // Draw the image to the bitmap context. Once we draw, the memory  
//    // allocated for the context for rendering will then contain the  
//    // raw image data in the specified color space.  
//    CGContextDrawImage(cgctx, rect, inImage);  
//	
//    // Now we can get a pointer to the image data associated with the bitmap  
//    // context.  
//    unsigned char *data = (unsigned char*)CGBitmapContextGetData (cgctx);  
//	
//    if (data != NULL)  
//    {  
//        //int counter = 0;  
//        for( y = 0; y < height; ++y )  
//        {  
//            for( x = 0; x < width; ++x )  
//            {  
//                cvdata[y*step+x*channels+0] = data[(4*y*width)+(4*x)+3];  
//                cvdata[y*step+x*channels+1] = data[(4*y*width)+(4*x)+2];  
//                cvdata[y*step+x*channels+2] = data[(4*y*width)+(4*x)+1];  
//            }  
//        }  
//    }  
//	
//    // When finished, release the context  
//    CGContextRelease(cgctx);  
//    // Free image data memory for the context  
//    if (data)  
//    {  
//        free(data);  
//    }  
//	
//}  
//
//- (CGContextRef)createARGBBitmapContext:(CGImageRef)inImage  
//{  
//    CGContextRef context = NULL;  
//    CGColorSpaceRef colorSpace;  
//    void * bitmapData;  
//    int bitmapByteCount;  
//    int bitmapBytesPerRow;  
//	
//    // Get image width, height. Weíll use the entire image.  
//    size_t pixelsWide = CGImageGetWidth(inImage);  
//    size_t pixelsHigh = CGImageGetHeight(inImage);  
//	
//    // Declare the number of bytes per row. Each pixel in the bitmap in this  
//    // example is represented by 4 bytes; 8 bits each of red, green, blue, and  
//    // alpha.  
//    bitmapBytesPerRow = (pixelsWide * 4);  
//    bitmapByteCount = (bitmapBytesPerRow * pixelsHigh);  
//	
//    // Use the generic RGB color space.  
//    colorSpace = CGColorSpaceCreateDeviceRGB();  
//    if (colorSpace == NULL)  
//    {  
//        return NULL;  
//    }  
//	
//    // Allocate memory for image data. This is the destination in memory  
//    // where any drawing to the bitmap context will be rendered.  
//    bitmapData = malloc( bitmapByteCount );  
//    if (bitmapData == NULL)  
//    {  
//        CGColorSpaceRelease( colorSpace );  
//        return NULL;  
//    }  
//	
//    // Create the bitmap context. We want pre-multiplied ARGB, 8-bits  
//    // per component. Regardless of what the source image format is  
//    // (CMYK, Grayscale, and so on) it will be converted over to the format  
//    // specified here by CGBitmapContextCreate.  
//    context = CGBitmapContextCreate (bitmapData,  
//                                     pixelsWide,  
//                                     pixelsHigh,  
//                                     8, // bits per component  
//                                     bitmapBytesPerRow,  
//                                     colorSpace,  
//                                     kCGImageAlphaPremultipliedFirst);  
//    if (context == NULL)  
//    {  
//        free (bitmapData);  
//    }  
//	
//    // Make sure and release colorspace before returning  
//    CGColorSpaceRelease( colorSpace );  
//	
//    return context;  
//}  
//
//- (IplImage *)getCVImageFromCGImage:(CGImageRef)cgImage  
//{  
//    IplImage *newCVImage = cvCreateImage(cvSize(CGImageGetWidth(cgImage), CGImageGetHeight(cgImage)), 8, 3);  
//	
//    [self manipulateOpenCVImagePixelDataWithCGImage:cgImage openCVimage:newCVImage];  
//	
//    return newCVImage;  
//}  
//
//-(CGImageRef)getCGImageFromCVImage:(IplImage*)cvImage  
//{  
//    // define used variables  
//    int height, width, step, channels;  
//    uchar *data;  
//	
//    // get the with and height of the used cvImage  
//    height = cvImage->height;  
//    width = cvImage->width;  
//    step = cvImage->widthStep;  
//    channels = cvImage->nChannels;  
//	
//    // create the new image with the flipped colors (BGR to RGB)  
//    IplImage* imgForUI = 0;  
//    imgForUI = cvCreateImage(cvSize(width, height), 8, 3);  
//    cvConvertImage(cvImage, imgForUI, CV_CVTIMG_SWAP_RB);  
//	
//    // the data with the flipped colors  
//    data = (uchar *)imgForUI->imageData;  
//	
//    // create a CFDataRef  
//    CFDataRef imgData = CFDataCreate(NULL, data, imgForUI->imageSize);  
//	
//    // create a CGDataProvider with the CFDataRef  
//    CGDataProviderRef imgDataProvider = CGDataProviderCreateWithCFData (imgData);  
//	
//    // create a CGImageRef with the CGDataProvider  
//    CGImageRef cgImage = CGImageCreate(width,  
//                                       height,  
//                                       8,  
//                                       8*channels,  
//                                       step,  
//                                       CGColorSpaceCreateDeviceRGB(),  
//                                       kCGImageAlphaNone,  
//                                       imgDataProvider,  
//                                       NULL,  
//                                       NO,  
//                                       kCGRenderingIntentDefault);  
//	
//    // release the CGDataProvider  
//    CGDataProviderRelease(imgDataProvider);  
//	
//    // return the new CGImageRef  
//    return cgImage;  
//}  
//
//
//- (void)dealloc {
//    [super dealloc];
//}
//
//
//@end
