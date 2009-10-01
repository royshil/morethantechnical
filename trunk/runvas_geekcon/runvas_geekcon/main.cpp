
#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <ctype.h>

IplImage *image = 0, *grey = 0, *prev_grey = 0, *pyramid = 0, *prev_pyramid = 0, *swap_temp, *orig = 0;

int win_size = 7;
const int MAX_COUNT = 250;
CvPoint2D32f* points[2] = {0,0}, *swap_points;
char* status = 0;
int count = 0;
int need_to_init = 0;
int night_mode = 0;
int flags = 0;
int add_remove_pt = 0;
CvPoint pt;


void on_mouse( int event, int x, int y, int flags, void* param )
{
    if( !image )
        return;

    if( image->origin )
        y = image->height - y;

    if( event == CV_EVENT_LBUTTONDOWN )
    {
        pt = cvPoint(x,y);
        add_remove_pt = 1;
    }
}


int main( int argc, char** argv )
{
    CvCapture* capture = 0;
    
	IplImage* canvas = 0;

    if( argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
        capture = cvCaptureFromCAM( argc == 2 ? argv[1][0] - '0' : 0 );
    else if( argc == 2 )
        capture = cvCaptureFromAVI( argv[1] );

    if( !capture )
    {
        fprintf(stderr,"Could not initialize capturing...\n");
        return -1;
    }

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

    cvNamedWindow( "LkDemo", 0 );
	cvMoveWindow("LkDemo",200,200);
    cvSetMouseCallback( "LkDemo", on_mouse, 0 );
	cvNamedWindow( "LkDemo1", 0 );
	cvMoveWindow("LkDemo1",520,200);
	

	night_mode = need_to_init = 1;

	int frame_count = 0;

    for(;;)
    {
        IplImage* frame = 0;
        int i, k, c;

        frame = cvQueryFrame( capture );
        if( !frame )
            break;

        if( !image )
        {
            /* allocate all the buffers */
            image = cvCreateImage( cvGetSize(frame), 8, 3 );
            image->origin = frame->origin;
            orig = cvCreateImage( cvGetSize(frame), 8, 3 );
            orig->origin = frame->origin;
            grey = cvCreateImage( cvGetSize(frame), 8, 1 );
            prev_grey = cvCreateImage( cvGetSize(frame), 8, 1 );
            pyramid = cvCreateImage( cvGetSize(frame), 8, 1 );
            prev_pyramid = cvCreateImage( cvGetSize(frame), 8, 1 );
            points[0] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
            points[1] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
            status = (char*)cvAlloc(MAX_COUNT);
            flags = 0;

			canvas = cvCreateImage(cvGetSize(frame),8,3);
			cvSet(canvas,CV_RGB(255,255,255));
        }

        cvCopy( frame, image, 0 );
        cvCopy( frame, orig, 0 );
        cvCvtColor( image, grey, CV_BGR2GRAY );

		cvConvertScale(canvas,canvas,1,2);

		if( night_mode ) {
			cvZero( image );
		}
        
        if( need_to_init )
        {
            /* automatic initialization */
            IplImage* eig = cvCreateImage( cvGetSize(grey), 32, 1 );
            IplImage* temp = cvCreateImage( cvGetSize(grey), 32, 1 );
            double quality = 0.01;
            double min_distance = 10;

            count = MAX_COUNT;
            cvGoodFeaturesToTrack( grey, eig, temp, points[1], &count,
                                   quality, min_distance, 0, 3, 0, 0.04 );
            cvFindCornerSubPix( grey, points[1], count,
                cvSize(win_size,win_size), cvSize(-1,-1),
                cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
            cvReleaseImage( &eig );
            cvReleaseImage( &temp );

            add_remove_pt = 0;
        }
        else if( count > 0 )
        {
            cvCalcOpticalFlowPyrLK( prev_grey, grey, prev_pyramid, pyramid,
                points[0], points[1], count, cvSize(win_size,win_size), 2, status, 0,
                cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), flags );
            flags |= CV_LKFLOW_PYR_A_READY;
            for( i = k = 0; i < count; i++ )
            {
                if( add_remove_pt )
                {
                    double dx = pt.x - points[1][i].x;
                    double dy = pt.y - points[1][i].y;

                    if( dx*dx + dy*dy <= 25 )
                    {
                        add_remove_pt = 0;
                        continue;
                    }
                }
                
                if( !status[i] )
                    continue;
                
                points[1][k++] = points[1][i];
				CvPoint ptx = cvPointFrom32f(points[1][i]);
				CvPoint pt1 = cvPointFrom32f(points[0][i]);

                double dx = pt1.x - ptx.x;
                double dy = pt1.y - ptx.y;
				double dist = dx*dx + dy*dy;

				if(dist > 5.0) {
					CvScalar clr = cvGet2D(orig,MIN(MAX(points[0][i].y,0.0),orig->height-1),MIN(MAX(points[0][i].x,0.0),orig->width-1));
					double a = MIN(dist / 15.0,1.0);
					a = 1.0 / a;
					clr.val[0] = clr.val[0] * a;
					clr.val[1] = clr.val[1] * a;
					clr.val[2] = clr.val[2] * a;
					cvCircle( canvas, ptx, 3, clr, -1, 8,0);
				}
            }
            count = k;
        }

        if( add_remove_pt && count < MAX_COUNT )
        {
            points[1][count++] = cvPointTo32f(pt);
            cvFindCornerSubPix( grey, points[1] + count - 1, 1,
                cvSize(win_size,win_size), cvSize(-1,-1),
                cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
            add_remove_pt = 0;
        }

        CV_SWAP( prev_grey, grey, swap_temp );
        CV_SWAP( prev_pyramid, pyramid, swap_temp );
        CV_SWAP( points[0], points[1], swap_points );
        need_to_init = 0;
        cvShowImage( "LkDemo", canvas );
		cvShowImage("LkDemo1",orig);

        c = cvWaitKey(10);
        if( (char)c == 27 )
            break;
        switch( (char) c )
        {
        case 'r':
            need_to_init = 1;
            break;
        case 'c':
            count = 0;
            break;
        case 'n':
            night_mode ^= 1;
            break;
        default:
            ;
        }

		frame_count++;
		if(frame_count % 30 == 0) {
			need_to_init = 1;
			frame_count = 0;
		}
    }

    cvReleaseCapture( &capture );
    cvDestroyAllWindows();

    return 0;
}

