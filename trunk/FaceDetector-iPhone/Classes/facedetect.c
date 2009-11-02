#define CV_NO_BACKWARD_COMPATIBILITY

#include "cv.h"
#include "highgui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>


static CvMemStorage* storage = 0;
static CvHaarClassifierCascade* cascade = 0;
static CvHaarClassifierCascade* nested_cascade = 0;
int use_nested_cascade = 0;

extern CvSeq* mycvHaarDetectObjects( const CvArr* _img,
					  CvHaarClassifierCascade* cascade,
					  CvMemStorage* storage, double scale_factor,
					  int min_neighbors, int flags, CvSize min_size );

void detect_and_draw( IplImage* img, CvRect* found_face );

double scale = 1.5;

void init_detection(char* cascade_location) {
	cascade = (CvHaarClassifierCascade*)cvLoad( cascade_location, 0, 0, 0 );
	storage = cvCreateMemStorage(0);
}

static IplImage *gray = 0, *small_img = 0;

void release_detection() {
	if (storage)
    {
        cvReleaseMemStorage(&storage);
    }
    if (cascade)
    {
        cvReleaseHaarClassifierCascade(&cascade);
    }
	cvReleaseImage(&gray);
	cvReleaseImage(&small_img);
}

#define PAD_FACE 40
#define PAD_FACE_2 80

void detect_and_draw( IplImage* img, CvRect* found_face )
{
	static CvRect prev;
	
	if(!gray) {
		gray = cvCreateImage( cvSize(img->width,img->height), 8, 1 );
		small_img = cvCreateImage( cvSize( cvRound (img->width/scale),
							 cvRound (img->height/scale)), 8, 1 );
	}

	if(prev.width > 0 && prev.height > 0) {
		cvSetImageROI(small_img, prev);

		CvRect tPrev = cvRect(prev.x * scale, prev.y * scale, prev.width * scale, prev.height * scale);
		cvSetImageROI(img, tPrev);
		cvSetImageROI(gray, tPrev);
	} else {
		cvResetImageROI(img);
		cvResetImageROI(small_img);
		cvResetImageROI(gray);
	}
	
    cvCvtColor( img, gray, CV_BGR2GRAY );
    cvResize( gray, small_img, CV_INTER_LINEAR );
    cvEqualizeHist( small_img, small_img );
    cvClearMemStorage( storage );

//	for(i=0;i<10;i++) {
//		double t = (double)cvGetTickCount();
		CvSeq* faces = mycvHaarDetectObjects( small_img, cascade, storage,
										   1.2, 0, 0
										   |CV_HAAR_FIND_BIGGEST_OBJECT
										   |CV_HAAR_DO_ROUGH_SEARCH
										   //|CV_HAAR_DO_CANNY_PRUNING
										   //|CV_HAAR_SCALE_IMAGE
										   ,
										   cvSize(30, 30) );
//		t = (double)cvGetTickCount() - t;
//		printf( "detection time = %gms, faces: %d\n", t/((double)cvGetTickFrequency()*1000.), faces->total );
		
	if(faces->total>0) {
		CvRect* r = (CvRect*)cvGetSeqElem( faces, 0 );
		int startX,startY;
		if(prev.width > 0 && prev.height > 0) {
			r->x += prev.x;
			r->y += prev.y;
		}
		startX = MAX(r->x - PAD_FACE,0);
		startY = MAX(r->y - PAD_FACE,0);
		int w = small_img->width - startX - r->width - PAD_FACE_2;
		int h = small_img->height - startY - r->height - PAD_FACE_2;
		int sw = r->x - PAD_FACE, sh = r->y - PAD_FACE;
		prev = cvRect(startX, startY, 
					  r->width + PAD_FACE_2 + ((w < 0) ? w : 0) + ((sw < 0) ? sw : 0),
					  r->height + PAD_FACE_2 + ((h < 0) ? h : 0) + ((sh < 0) ? sh : 0));
//		printf("found face (%d,%d,%d,%d) setting ROI to (%d,%d,%d,%d)\n",r->x,r->y,r->width,r->height,prev.x,prev.y,prev.width,prev.height);
		found_face->x = (int)((double)r->x * scale);
		found_face->y = (int)((double)r->y * scale);
		found_face->width = (int)((double)r->width * scale);
		found_face->height = (int)((double)r->height * scale);
	} else {
		prev.width = prev.height = found_face->width = found_face->height = 0;
	}
}

