/* Demo of modified Lucas-Kanade optical flow algorithm.
   See the printf below */

//#ifdef _CH_
//#pragma package <opencv>
//#endif
//
//#define CV_NO_BACKWARD_COMPATIBILITY
//
//#ifndef _EiC
//#include "cv.h"
//#include "highgui.h"
//#include "cxcore.h"
//#include <stdio.h>
//#include <ctype.h>
//#endif

#include "stdafx.h"

//IplImage *image = 0, *grey = 0, *prev_grey = 0, *pyramid = 0, *prev_pyramid = 0, *swap_temp;

int win_size = 10;
const int MAX_COUNT = 150;
CvPoint2D32f* points[2] = {0,0}, *swap_points;
char* status = 0;
int _count = 0;
int need_to_init = 0;
int night_mode = 0;
int flags = 0;
int add_remove_pt = 0;
CvPoint pt;
uchar* swap_temp;

using namespace cv;
using namespace std;

TermCriteria tc = TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 30, 0.01);

//void on_mouse( int event, int x, int y, int flags, void* param )
//{
//    if( !image )
//        return;
//
//    if( image->origin )
//        y = image->height - y;
//
//    if( event == CV_EVENT_LBUTTONDOWN )
//    {
//        pt = cvPoint(x,y);
//        add_remove_pt = 1;
//    }
//}


int main( int argc, char** argv )
{
    //CvCapture* capture = 0;

    //if( argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
    //    capture = cvCaptureFromCAM( argc == 2 ? argv[1][0] - '0' : 0 );
    //else if( argc == 2 )
    //    capture = cvCaptureFromAVI( argv[1] );

	VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        return -1;

    //if( !capture )
    //{
    //    fprintf(stderr,"Could not initialize capturing...\n");
    //    return -1;
    //}

    /* print a welcome message, and the OpenCV version */
    printf ("Welcome to lkdemo, using OpenCV version %s (%d.%d.%d)\n",
	    CV_VERSION,
	    CV_MAJOR_VERSION, CV_MINOR_VERSION, CV_SUBMINOR_VERSION);

    printf( "Hot keys: \n"
            "\tESC - quit the program\n"
            "\tr - auto-initialize tracking\n"
            "\tc - delete all the points\n"
            "\tn - switch the \"night\" mode on/off\n"
            "To add/remove a feature point click it\n" );

    namedWindow( "LkDemo", 0 );
    //cvSetMouseCallback( "LkDemo", on_mouse, 0 );

	Mat image, grey, prev_grey;
	vector<Point2f> _prevPts;
	vector<Point2f> _nextPts;
	vector<Point2f>* prevPts = &_prevPts;
	vector<Point2f>* nextPts = &_nextPts;
	vector<Point2f>* tempPts;
	vector<uchar> status;
	vector<float> err;

	vector<Point2f> first_snap;
	vector<Point2f> second_snap;
	int snapNum = 0;

    for(;;)
    {
        Mat frame;
        cap >> frame; // get a new frame from camera

		//IplImage* frame = 0;
        int i, k, c;

        //frame = cvQueryFrame( capture );
        //if( !frame )
        //    break;

        //if( !image )
        //{
        //    /* allocate all the buffers */
        //    image = cvCreateImage( cvGetSize(frame), 8, 3 );
        //    image->origin = frame->origin;
        //    grey = cvCreateImage( cvGetSize(frame), 8, 1 );
        //    prev_grey = cvCreateImage( cvGetSize(frame), 8, 1 );
        //    pyramid = cvCreateImage( cvGetSize(frame), 8, 1 );
        //    prev_pyramid = cvCreateImage( cvGetSize(frame), 8, 1 );
        //    points[0] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
        //    points[1] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
        //    status = (char*)cvAlloc(MAX_COUNT);
        //    flags = 0;
        //}

        //cvCopy( frame, image, 0 );
		frame.copyTo(image);
        cvtColor( image, grey, CV_BGR2GRAY );

        //if( night_mode )
        //    cvZero( image );

        if( need_to_init )
        {
            /* automatic initialization */
			grey.copyTo(prev_grey);

            //IplImage* eig = cvCreateImage( cvGetSize(grey), 32, 1 );
            //IplImage* temp = cvCreateImage( cvGetSize(grey), 32, 1 );
            double quality = 0.01;
            double min_distance = 10;

            _count = MAX_COUNT;
			goodFeaturesToTrack( grey, *nextPts, MAX_COUNT,
                                   quality, min_distance);
            cornerSubPix( grey, *nextPts, cvSize(win_size,win_size), cvSize(-1,-1),
				TermCriteria(TermCriteria::COUNT+TermCriteria::EPS,20,0.03));
            //cvReleaseImage( &eig );
            //cvReleaseImage( &temp );

            //add_remove_pt = 0;
        }
        else if( _count > 0 )
        {
			calcOpticalFlowPyrLK(prev_grey, grey, *prevPts, *nextPts, 
				status, err, Size(win_size, win_size),3,
				tc,	0.5, flags);

            //cvCalcOpticalFlowPyrLK( prev_grey, grey, prev_pyramid, pyramid,
            //    points[0], points[1], count, cvSize(win_size,win_size), 3, status, 0,
            //    cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), flags );
            //flags = OPTFLOW_USE_INITIAL_FLOW;
			for( i = k = 0; i < nextPts->size(); i++ )
            {
                //if( add_remove_pt )
                //{
                //    double dx = pt.x - points[1][i].x;
                //    double dy = pt.y - points[1][i].y;

                //    if( dx*dx + dy*dy <= 25 )
                //    {
                //        add_remove_pt = 0;
                //        continue;
                //    }
                //}

                if( !status[i] )
                    continue;

				//"compact" points to start of vector
                //(*nextPts)[k++] = (*nextPts)[i];

				//draw point
				circle( image, (*nextPts)[i], 3, CV_RGB(0,255,0), -1, 8,0);
            }
            //_count = k;
			nextPts->resize(_count);
        }

   //     if( add_remove_pt && _count < MAX_COUNT )
   //     {
			//nextPts->resize(_count);
   //         //points[1][count++] = cvPointTo32f(pt);
   //         //cornerSubPix( grey, points[1] + count - 1, 1,
   //         //    cvSize(win_size,win_size), cvSize(-1,-1),
   //         //    cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
   //         add_remove_pt = 0;
   //     }

        CV_SWAP( prev_grey.data, grey.data, swap_temp );
        //CV_SWAP( prev_pyramid, pyramid, swap_temp );
		CV_SWAP( prevPts, nextPts, tempPts );
		
        need_to_init = 0;
        imshow( "LkDemo", image );

        c = cvWaitKey(10);
        if( (char)c == 27 )
            break;
        switch( (char) c )
        {
        case 'r':
            need_to_init = 1;
            break;
        case 'c':
            _count = 0;
            break;
        case 'n':
            night_mode ^= 1;
            break;
		case ' ':
			if(snapNum == 0) {
				//save snapshot of points
				first_snap = (*prevPts);
				snapNum = 1;
			} else {
				//save snap for 2nd frame
				second_snap = (*prevPts);

				//find camera matrices..
				Mat F = findFundamentalMat(first_snap,second_snap);

//[U,S,V] = svd(F);
//e = U(:,3);
//P = [-vgg_contreps(e)*F e];

				SVD svd(F);
				Mat U = svd.u;
				Mat e = U.col(3);
//Y = [0		X(3)	-X(2)
//    -X(3)	0		X(1)
//    X(2)	-X(1)	0];
				double* ep = e.ptr<double>();
				double m[3][3] = {{0, ep[2], -ep[1]}, 
								  {-ep[2], 0, ep[0]}, 
								  {ep[1], -ep[0], 0}};
				Mat Y = Mat(3, 3, CV_64F, m);
				Mat l = -Y*F;
			}
        default:
            ;
        }
    }

    //cvReleaseCapture( &capture );
    //cvDestroyWindow("LkDemo");

    return 0;
}

#ifdef _EiC
main(1,"lkdemo.c");
#endif
