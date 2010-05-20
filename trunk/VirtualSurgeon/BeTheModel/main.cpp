//#include <omp.h>

/*

*******************************************************************************
THIS FILE IS DEPRECATED, ALL NEW WORK IS DONE IN new_main.cpp FILE
*******************************************************************************









#include "ImageEditingUtils.h"

#include "cv.h"
#include "highgui.h"

using namespace cv;

#include "../tclap-1.2.0/include/tclap/CmdLine.h"
using namespace TCLAP;

#include "OpenCVImageWrapper.h"

//extern void histMatchRGB(Mat& src, const Mat& src_mask, const Mat& dst, const Mat& dst_mask);
//extern void histMatchHS(Mat& src, const Mat& src_mask, const Mat& dst, const Mat& dst_mask);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>

#include <fstream>
#include <iostream>
#include <cstdio>

#ifdef _EiC
#define WIN32
#endif

cv::Ptr<CvMemStorage> storage = 0;
//CvHaarClassifierCascade* cascade = 0;
//CvHaarClassifierCascade* nested_cascade = 0;

CascadeClassifier cascade, nestedCascade;

int use_nested_cascade = 0;
IplImage *small_img = 0;

int btm_wait_time = 0;

String cascade_name = "D:\\opencv-latest\\data\\haarcascades\\haarcascade_frontalface_alt2.xml";
String nested_cascade_name = "D:/opencv-latest/data/haarcascades/haarcascade_eye.xml";

double scale = 1;

#include "util.h"

#include "face_path.h"

#include "../GrabCut/GrabCut.h"

#include "../VirtualSurgeon_HeadExtractor/head_extractor.h"

int _main( int argc, char** argv )
{
	
 //   cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );
	//nested_cascade = (CvHaarClassifierCascade*)cvLoad( nested_cascade_name, 0, 0, 0 );
	//cascade_name.assign("D:\\OpenCV2.0\\data\\haarcascades\\haarcascade_frontalface_alt.xml");

	//bool use_ellipse = false;
	//bool do_poisson = false;
	//bool use_hist_backproj = false;
	bool use_hist_match_hs = false;
	bool use_hist_match_rgb = false;
	//bool use_grab_cut_opencv = false;
	//bool use_grab_cut_JT = false;
	bool use_overlay = false;
	bool use_warp_rigid = false;
	bool use_warp_affine = false;
	bool use_double_warp = false;
	//const char* bwt_c = "--btm-wait-time";
	//int bwt_l = strlen(bwt_c);

	//char face_path_path[256] = {0};
	//const char* face_path_str = "--face-path";
	//int face_path_strlen = strlen(face_path_str);

	//for(int i=0;i<argc;i++) {
	//	do_poisson = do_poisson ||					(strcmp(argv[i],"--do-poisson") == 0);
	//	use_ellipse = use_ellipse ||				(strcmp(argv[i],"--use-ellipse") == 0);
	//	use_hist_backproj = use_hist_backproj ||	(strcmp(argv[i],"--use-hist-backproj") == 0);
	//	use_hist_match_hs = use_hist_match_hs ||	(strcmp(argv[i],"--use-hist-match-hs") == 0);
	//	use_hist_match_rgb = use_hist_match_rgb ||	(strcmp(argv[i],"--use-hist-match-rgb") == 0);
	//	use_grab_cut_opencv = use_grab_cut_opencv ||(strcmp(argv[i],"--use-grabcut-opencv") == 0);
	//	use_grab_cut_JT = use_grab_cut_JT ||		(strcmp(argv[i],"--use-grabcut-JT") == 0);
	//	use_overlay = use_overlay ||				(strcmp(argv[i],"--use-overlay") == 0);
	//	use_warp_rigid = use_warp_rigid ||			(strcmp(argv[i],"--use-warp-rigid") == 0);
	//	use_warp_affine = use_warp_affine ||		(strcmp(argv[i],"--use-warp-affine") == 0);
	//	use_double_warp = use_double_warp ||		(strcmp(argv[i],"--use-double-warp") == 0);
	//	// --btm-wait-time=...
	//	if(strncmp(argv[i],bwt_c,bwt_l)==0) {
	//		char __s[10] = {0};
	//		strncpy_s(__s,10,argv[i]+bwt_l+1,10);
	//		btm_wait_time = atoi(__s);
	//	}
	//	// --face-path=...
	//	if(strncmp(argv[i],face_path_str,face_path_strlen)==0) {
	//		strncpy_s(face_path_path,strlen(argv[i])-face_path_strlen,argv[i]+face_path_strlen+1,256);
	//	}
	//}

	try {  

	// Define the command line object.
	CmdLine cmd("Virtual Surgeon Head Replacer", ' ', "0.2");

	SwitchArg use_hist_match_hs_arg(string(),"use-hist-match-hs","Use histogram matching over HS space for recoloring?",cmd,false);
	SwitchArg use_hist_match_rgb_arg(string(),"use-hist-match-rgb","Use histogram matching over RGB space for recoloring?",cmd,false);
	SwitchArg use_overlay_arg(string(),"use-overlay","Use overlay for recoloring?",cmd,false);
	SwitchArg use_warp_rigid_arg(string(),"use-warp-rigid","Use rigid warp for neck warping?",cmd,false);
	SwitchArg use_warp_affine_arg(string(),"use-warp-affine","Use affine warp for neck warping?",cmd,false);
	SwitchArg use_double_warp_arg(string(),"use-double-warp","Use 2-way warping?",cmd,false);

	// Parse the args.
	cmd.parse( argc, argv );

	use_hist_match_hs = use_hist_match_hs_arg.getValue();
	use_hist_match_rgb = use_hist_match_rgb_arg.getValue();
	use_overlay = use_overlay_arg.getValue();
	use_warp_affine = use_warp_affine_arg.getValue();
	use_warp_rigid = use_warp_rigid_arg.getValue();
	use_double_warp = use_double_warp_arg.getValue();

	}catch (ArgException &e)  // catch any exceptions
	{ cerr << "error: " << e.error() << " for arg " << e.argId() << endl; scanf("press any key...\n"); }


	{
		cout << cascade_name << endl;

		if(!cascade.load(cascade_name)) {
			fprintf(stderr,"cannot load cascade: %s",cascade_name); 
			return -1;
		}
		if(!nestedCascade.load(nested_cascade_name)) {fprintf(stderr,"cannot load cascade: %s",cascade_name); return -1;}

		storage = cvCreateMemStorage();
	}

	IplImage* image = cvLoadImage( argv[1], 1 );

	if(!image) {
		fprintf(stderr,"ERROR: cannot load image \"%s\"\n",argv[1]);
		goto end_cascades_and_storage;
	}

	if(image->width*image->height > 300000) {
		//big pic - make it smaller
		CvSize s = cvSize(550,cvFloor(550.0*((double)(image->height)/(double)(image->width))));
		IplImage* tmp_i = cvCreateImage(s,image->depth,image->nChannels);
		cvResize(image,tmp_i,CV_INTER_CUBIC);
		cvReleaseImage(&image);
		image = cvCreateImage(cvGetSize(tmp_i),tmp_i->depth,tmp_i->nChannels);
		cvCopy(tmp_i,image);
		cvReleaseImage(&tmp_i);

#ifdef BTM_DEBUG
		printf("resized original to (%d,%d)\n",image->width,image->height);
#endif
	}


	IplImage /** frame,* *edge, *gray = 0,*edge_8bit,*orig/*,*orig_32f*;

	//CvCapture* cap = cvCaptureFromCAM(CV_CAP_ANY);
	//frame = cvQueryFrame(cap);
	//IplImage* image = cvCreateImage(cvGetSize(frame),frame->depth,frame->nChannels);

	orig = cvCreateImage(cvGetSize(image),image->depth,image->nChannels);
	gray = cvCreateImage( cvGetSize(image), 8, 1 );
	edge = cvCreateImage(cvGetSize(image),16,1);
	edge_8bit = cvCreateImage(cvGetSize(edge),8,1);
	small_img = cvCreateImage( cvSize( cvRound(image->width/scale), cvRound(image->height/scale)), 8, 1 );

	IplImage* mask = cvCreateImage( cvGetSize(image), 8, 1 );
	IplImage* mask_inner = cvCreateImage( cvGetSize(image), 8, 1 );
	cvZero(mask);
	cvZero(mask_inner);

#ifdef BTM_DEBUG
    cvNamedWindow( "result", 1 );
#endif
	//---------------------------------------- Detection and Alignment -------------------------------------

	{
		/************
		Detect eyes
		************

		cvCopy(image,orig);

		cvSmooth(image,image,CV_GAUSSIAN,5);

		cvCvtColor( image, gray, CV_BGR2GRAY );

		CvPoint eye1 = cvPoint(-1,-1),eye2 = cvPoint(-1,-1);
		float phi = 0;
		detect_and_draw(gray,image,eye1,eye2,phi);

		if(eye1.x == -1 || eye1.y == -1 || eye2.x == -1 || eye2.y == -1) {
			fprintf(stderr,"No eyes detected in image\n");
			goto end;
		}

#ifdef BTM_DEBUG
		cvShowImage("result",image);
		cvWaitKey(BTM_WAIT_TIME);
#endif

		/************
		Refine eyes detection to mark pupils
		************

		int refine_win_size = cvFloor(distance(eye1,eye2) / 4.5);
		refine_pupil_point(orig,eye1,refine_win_size);
		refine_pupil_point(orig,eye2,refine_win_size);

#ifdef BTM_DEBUG
		cvCopy(orig,image);
		cvCircle( image, eye1, 2, CV_RGB(0,255,0), 3, 8, 0 );
		cvCircle( image, eye2, 2, CV_RGB(0,255,0), 3, 8, 0 );

		cvShowImage("result",image);
		cvWaitKey(BTM_WAIT_TIME);
#endif
		//cvDrawLine(image,eye1,eye2,CV_RGB(0,0,255));

		/************
		Rotate the face to align eyes line with x axis
		************

		phi = (float)-calc_angle_with_x_axis(cvPointTo32f(eye1),cvPointTo32f(eye2));
		if(eye2.y < eye1.y) phi = -phi;
		rotate_around_point(orig,eye1,phi,NULL,NULL);

		//Transform eyes location as well
		float theta = -phi;
		float cosT = cos(theta),sinT = sin(theta);
		CvPoint eye2Tag;
		eye2Tag.x = (int)floor((float)(eye2.x-eye1.x) * cosT + (float)(eye2.y-eye1.y) * sinT) + eye1.x;
		eye2Tag.y = (int)floor((float)(eye2.x-eye1.x) * -sinT + (float)(eye2.y-eye1.y) * cosT) + eye1.y;
		
		eye2 = eye2Tag;
		
#ifdef BTM_DEBUG
		cvCopy(orig,image);
		cvCircle( image, eye1, 2, CV_RGB(0,255,0), 3, 8, 0 );
		cvCircle( image, eye2, 2, CV_RGB(0,255,0), 3, 8, 0 );

		cvShowImage("result",image);
		cvWaitKey(BTM_WAIT_TIME);
#endif

		/*---------------------------------------- Masking --------------------------------------

		/************
		Find the outer border of the face
		************

		cvCvtColor( orig, gray, CV_BGR2GRAY );

		cvSmooth(gray,gray,CV_GAUSSIAN,5);

		cvCanny(gray,edge_8bit,950.0,100.0,5);

		cvCvtColor(edge_8bit,image,CV_GRAY2RGB);
		cvAdd(image,orig,image);

		int num_points_on_path = -1;
		CvPoint2D32f *points = NULL;
		if(use_ellipse) {
			num_points_on_path = 100;
			points = new CvPoint2D32f[num_points_on_path];

			create_ellipse_path(num_points_on_path,points,eye1,eye2);
		} else {
			//FacePath face_path;
			//read_path_from_file(face_path_path,face_path);
			//points = face_path.pts;
			//num_points_on_path = face_path.num;

			//CvMat* m = cvCreateMatHeader(face_path.num,1,CV_32FC2);
			//cvSetData(m,face_path.pts,sizeof(CvPoint2D32f));

			CvMat* m = &((CvMat)face_path_M);

			//scale to match eyes distance
			double scale_f = distance(eye1,eye2)/distance(cvPoint((int)face_path_leye[0],(int)face_path_leye[1]),cvPoint((int)face_path_reye[0],(int)face_path_reye[1]));
							//distance(face_path.left_eye,face_path.right_eye);
			cvConvertScale(m,m,scale_f);

			//move to align with left eye
			//cvAddS(m,cvScalar(eye1.x-face_path.left_eye.x*scale_f,eye1.y-face_path.left_eye.y*scale_f),m);
			cvAddS(m,cvScalar(eye1.x-face_path_leye[0]*scale_f,eye1.y-face_path_leye[1]*scale_f),m);
			//cvReleaseMatHeader(&m);

			points = (CvPoint2D32f*)face_path;
			num_points_on_path = face_path_num_points;
		}

		find_face_path(eye1,eye2,edge_8bit,points,num_points_on_path,image);
		//find_face_path_homography(edge_8bit,points,num_points_on_path,image);

		prune_points_set(&num_points_on_path,&points);

		/************ unify close points ************
		for(int i=0;i<num_points_on_path;i++) {
			if(!(points[i].x >= 0.0 && points[i].y >= 0.0)) continue;
			CvPoint2D32f pt = points[i];
			for(int j=i+1;j<num_points_on_path;j++) {
				if(!(points[j].x >= 0.0 && points[j].y >= 0.0)) continue;
				if(abs(points[j].x - pt.x) < 0.001 && abs(points[j].y - pt.y) < 0.001) {
					points[j] = cvPoint2D32f(-1.0,-1.0);
				}
			}
		}
		int good_pts_count = 0;
		for(int i=0;i<num_points_on_path;i++) {
			if(points[i].x >= 0.0 && points[i].y >= 0.0) good_pts_count++;
		}
		CvPoint2D32f* new_points = new CvPoint2D32f[good_pts_count];
		for(int i=0,j=0;i<num_points_on_path;i++) {
			if(points[i].x >= 0.0 && points[i].y >= 0.0) {
				new_points[j++] = points[i];
			}
		}
#ifdef BTM_DEBUG
		printf("original %d points, after pruning %d points\n",num_points_on_path,good_pts_count);
#endif
		delete[] points;
		points = new_points;
		num_points_on_path = good_pts_count;
		/*********************************************

#ifdef BTM_DEBUG
		cvShowImage("result",image);
		cvWaitKey(BTM_WAIT_TIME);
#endif

		/************
		Find the INNER border of the face
		************

		int num_points_on_path_inner = 100;
		CvPoint2D32f *points_inner = new CvPoint2D32f[num_points_on_path_inner];

		create_ellipse_path(num_points_on_path_inner,points_inner,eye1,eye2);

#ifdef BTM_DEBUG
		cvCopy(orig,image);
#endif
		find_face_path(eye1,eye2,edge_8bit,points_inner,num_points_on_path_inner,image);
		prune_points_set(&num_points_on_path_inner,&points_inner);
#ifdef BTM_DEBUG
		cvShowImage("result",image);
		cvWaitKey(BTM_WAIT_TIME);
#endif

		if(do_poisson) {
			FaceInfo finfo;
			finfo.face = orig;

			draw_points_poly_mask(points_inner, num_points_on_path_inner, mask_inner);
			finfo.face_mask = mask_inner;
			finfo.face_rect = find_bounding_rect_of_mask(mask_inner);

			finfo.lEye = eye1;
			finfo.rEye = eye2;

			CloneInfo cinfo;
			readCloneInfoFile(argv[2],cinfo);

			if(cinfo.grayscale == 1) {	//model should be grayscale
				IplImage* __tmp = cvCreateImage(cvGetSize(orig),8,1);
				cvCvtColor(orig,__tmp,CV_RGB2GRAY);
				cvCvtColor(__tmp,orig,CV_GRAY2RGB);
				cvReleaseImage(&__tmp);
			}

			paste_face_on_clone(&finfo,&cinfo);

#ifdef BTM_DEBUG
			cvShowImage("result",cinfo.back_for_poisson);
			cvWaitKey(BTM_WAIT_TIME);
#else
			fprintf(stderr,"Output file: %s\n",argv[3]);
			cvSaveImage(argv[3],cinfo.back_for_poisson);
#endif
			goto end;
		}

//#ifdef BTM_DEBUG
//		cvCopy(orig,image);
//#endif

		//refine_face_path_with_LLSQ(num_points_on_path,points,image);

		CvMat* ptsM = cvCreateMatHeader(num_points_on_path,1,CV_32FC2);
		cvSetData(ptsM,points,sizeof(CvPoint2D32f));
		CvScalar midPt = cvAvg(ptsM);
		cvReleaseMatHeader(&ptsM);

		/************
		Marching pixels refining
		************
		//find normal for each point on path
		//------------------------------------
		//interpolate between normals between neighbouring points
		CvPoint2D32f* normals = new CvPoint2D32f[num_points_on_path];
		CvMat* ptsM = cvCreateMatHeader(num_points_on_path,1,CV_32FC2);
		cvSetData(ptsM,points,sizeof(CvPoint2D32f));
		CvScalar mid = cvAvg(ptsM);
		//int* hasNormal = new int[num_points_on_path];
		//memset(hasNormal,1,sizeof(int)*num_points_on_path);
		for(int i = 0; i < num_points_on_path; i++) {
			CvPoint2D32f A = points[i];
			//CvPoint2D32f B = points[i-1];	//prev pt
			//CvPoint2D32f C = points[i+1];	//next pt
			//CvPoint2D32f nAB = cvPoint2D32f(-(A.y-B.y),(A.x-B.x));
			//CvPoint2D32f nAC = cvPoint2D32f(-(C.y-A.y),(C.x-A.x));

			//CvPoint2D32f _n;
			//if((nAB.x != 0 || nAB.y != 0) && (nAC.x != 0 || nAC.y != 0)) {
			//	_n = cvPoint2D32f((nAB.x+nAC.x)/2.0,(nAB.y+nAC.y)/2.0);
			//} else if(nAB.x != 0 || nAB.y != 0) {
			//	_n = nAB;
			//} else if(nAC.x != 0 || nAC.y != 0) {
			//	_n = nAC;
			//} else {
			//	hasNormal[i] = 0;
			//}

			//if(hasNormal[i] != 0) {
			//	normalize2D(_n);

				CvPoint2D32f point_to_center = cvPoint2D32f(A.x-mid.val[0],A.y-mid.val[1]);
				normalize2D(point_to_center);
				//float dp = dotp(_n,point_to_center);
				//if(dp < 0) {
				//	_n = cvPoint2D32f(-_n.x,-_n.y);
				//}
				
				normals[i] = point_to_center;
			//}
		}

		cvReleaseMatHeader(&ptsM);

#ifdef BTM_DEBUG
		cvCopy(orig,image);
		for(int i = 0; i < num_points_on_path; i++) {
			//if(hasNormal[i] > 0) {
				cvCircle(image,cvPointFrom32f(points[i]),2,CV_RGB(255,0,255),1);
				drawArrow(cvPointFrom32f(points[i]),
					cvPoint(points[i].x+normals[i].x*10.0,points[i].y+normals[i].y*10.0),
					image);
			//}
		}
		cvShowImage("result",image);
		cvWaitKey(BTM_WAIT_TIME);
#endif
		*

//---------------------------------------------- Segmenting ---------------------------------------------

		//Use the face outer and inner points path to get an initial mask
		draw_points_poly_mask(points_inner, num_points_on_path_inner, mask_inner);
		draw_points_poly_mask(points, num_points_on_path, mask); //initial mask


		if( use_hist_backproj ) {
		//------------------ pyramid histogram backprojection ------------------------
			pyramid_histogram_backprojection_RGB(orig,image,mask,mask_inner,midPt,false);
		}

		/*
		int itr = 0, max_itr = 50;
		while(itr++ < max_itr)
		{
			//IplImage* win_mask = cvCreateImage(cvGetSize(orig),8,1);
			CvSize win_size = cvSize(10,10);
			cvZero(pt_scores);
			//iteratively test each point to see if it needs to move forward or backwards
			for (int i=0;i<num_points_on_path;i++) {
				CvPoint2D32f pt = points[i];
				//if(pt.x < 0 || pt.x > orig->width || pt.y < 0 || pt.y > orig->height) {
				//	points[i] = points[(i+1)%num_points_on_path];
				//	continue;
				//}
				CvRect win_roi = cvRect(pt.x-win_size.width/2,
										pt.y-win_size.height/2,
										win_size.width,
										win_size.height);
				if(win_roi.x<0) { win_roi.width += win_roi.x; win_roi.x = 0;}
				if(win_roi.y<0) { win_roi.height += win_roi.y; win_roi.y = 0;}
				if(win_roi.x+win_size.width>orig->width) { 
					win_roi.width += orig->width-win_roi.x-win_size.width;
				}
				if(win_roi.y+win_size.height>orig->height) { 
					win_roi.height += orig->height-win_roi.y-win_size.height;
				}
				if(win_roi.width<=0 || win_roi.height<=0) {
					pt_scores->data.db[i] = 0.0;
					continue;
				}

				IplImage* win_red = cvCreateImage(cvSize(win_roi.width,win_roi.height),8,1);
				IplImage* win_green = cvCreateImage(cvSize(win_roi.width,win_roi.height),8,1);
				IplImage* win_blue = cvCreateImage(cvSize(win_roi.width,win_roi.height),8,1);
				IplImage* win_planes[] = {win_red,win_green,win_blue};

				cvSetImageROI(red,win_roi); cvCopy(red,win_red);
				cvSetImageROI(green,win_roi); cvCopy(red,win_red);
				cvSetImageROI(blue,win_roi); cvCopy(red,win_red);
				cvResetImageROI(red); cvResetImageROI(green); cvResetImageROI(blue);

				//cvSetImageROI(orig,win_roi);
				//cvSplit(orig,win_red,win_green,win_blue,NULL);
				//cvZero(win_mask);
				//cvRectangle(win_mask,
				//	cvPoint(pt.x-win_size.width/2,pt.y-win_size.height/2),
				//	cvPoint(pt.x+win_size.width/2,pt.y+win_size.height/2),
				//	CV_RGB(255,255,255),CV_FILLED);

				cvClearHist(window_hst);
				cvCalcHist(win_planes,window_hst,0,0);
				cvNormalizeHist(window_hst,1.0);

				double score = cvCompareHist(window_hst,face_hst,CV_COMP_CORREL);
				pt_scores->data.db[i] = score;

				//CvPoint2D32f n = normals[i];

				//move along the normal 2 units
				//pt = cvPoint2D32f(pt.x + n.x * 2.0f,pt.y + n.y * 2.0f);

				cvReleaseImage(&win_red);
				cvReleaseImage(&win_green);
				cvReleaseImage(&win_blue);
			}
		

			cvAbs(pt_scores,pt_scores);
			cvNormalize(pt_scores,pt_scores,1.0,0.0,CV_MINMAX);

			float normal_mult_factor = 3.0f;

			cvCopy(orig,image);
			for (int i=0;i<num_points_on_path;i++) {
				CvPoint2D32f pt = points[i];
				//printf("%d,%d: score %.3f - ",(int)pt.x,(int)pt.y,pt_scores->data.db[i]);

				double a = 255.0 * pt_scores->data.db[i];
				cvCircle(image,cvPointFrom32f(pt),2,CV_RGB(a,255-a,0),CV_FILLED);

				CvPoint2D32f n = normals[i];
				if(pt_scores->data.db[i] > 0.2) {
					//move along the normal 2 units
					points[i] = cvPoint2D32f(pt.x + n.x * normal_mult_factor,pt.y + n.y * normal_mult_factor);
					//printf("move out \n");
				} else if(pt_scores->data.db[i] < 0.005) {
					//move backwards along the normal 2 units
					points[i] = cvPoint2D32f(pt.x - n.x * normal_mult_factor,pt.y - n.y * normal_mult_factor);
					//printf("move in \n");
				} else {
					//printf("stay\n");
				}
			}
			cvShowImage("result",image);
			cvWaitKey(5);
		}
		*/

		//CvMat* _ptsM = cvCreateMatHeader(10,1,CV_32FC2);
		//cvSetData(_ptsM,points_inner,sizeof(CvPoint2D32f));
		//CvScalar _avg = cvAvg(_ptsM);
		//cvAddS(_ptsM,cvScalar(-_avg.val[0],-_avg.val[1]),_ptsM);

		//float* b = new float[10 * 3];
		//float* p = new float[20 * 3];
		//for(int i=0;i<10;i++) {
		//	b[i] = points_inner[i].x;
		//	b[i+1] = points_inner[i].y;
		//	b[i+2] = 1;
		//}
		//bezier(num_points_on_path_inner,b,20,p);

		/************
		Create the inner and outer masks of the face area
		************

		//draw_points_poly_mask(points, num_points_on_path, mask);

		//smooth outer mask to eliminate jagged edges

		//draw_points_poly_mask(points_inner, num_points_on_path_inner, mask_inner,image);

		delete[] points;
		delete[] points_inner;

#ifdef BTM_DEBUG
		cvShowImage("result",image);
		cvShowImage("tmp",mask);
		cvWaitKey(BTM_WAIT_TIME);
#endif
		/**********************************
		//	GrabCut for refining the mask
		***********************************
		if(use_grab_cut_JT) {
			//Justin Talbuts GrabCut
			GrabCutNS::Image<GrabCutNS::Color>* imageGC = GrabCutNS::loadIplImage(orig);
			GrabCutNS::Image<GrabCutNS::Color>* maskGC = GrabCutNS::loadIplImage(mask);

			GrabCutNS::GrabCut *grabCut = new GrabCutNS::GrabCut( imageGC );
			//grabCut->setCallbak(GCCallback);
			grabCut->initializeWithMask(maskGC);
			grabCut->fitGMMs();
			//grabCut->refineOnce();
			grabCut->refine();

			IplImage* __GCtmp = cvCreateImage(cvSize(orig->width,orig->height),8,1);
			GrabCutNS::fromImageMaskToIplImage(grabCut->getAlphaImage(),__GCtmp);
			cvCopy(__GCtmp,mask);
			cvReleaseImage(&__GCtmp);

			cvShowImage("tmp",mask);
		}
		
		if(use_grab_cut_opencv) {
			//OpenCV internal GrabCut
			face_grab_cut(Mat(orig),Mat(mask));
		}

		if(use_grab_cut_JT || use_grab_cut_opencv) {
			//Smooth jaggy edges...
			cvSmooth(mask,mask,CV_GAUSSIAN,15);
			cvThreshold(mask,mask,128.0,255.0,CV_THRESH_BINARY);

		}//end grabcut

		CvRect bx = find_bounding_rect_of_mask(mask);

		bx.x = bx.x - 1;
		bx.y = bx.y - 1;
		bx.width = bx.width + 1;
		bx.height = bx.height + 1;

#ifdef BTM_DEBUG
		cvCopy(orig,image);
		cvDrawRect(image,cvPoint(bx.x,bx.y),cvPoint(bx.x+bx.width,bx.y+bx.height),CV_RGB(255,0,0),2);

		cvCircle(image,eye1,2,CV_RGB(0,255,0),CV_FILLED);
		cvCircle(image,eye2,2,CV_RGB(0,255,0),CV_FILLED);

		cvShowImage("result",image);
		cvWaitKey(BTM_WAIT_TIME);
#endif

		/************
		Paste face on the output image
		************

		CloneInfo cInfo;
		readCloneInfoFile(argv[2],cInfo);

//--------------------------------------- warping ------------------------------------------

		vector<Point2d> face_warp_points1,face_warp_points2;
		std::string inputFile(argv[1]);
		
		if(use_warp_affine || use_warp_rigid)
		{
		face_warp_read_points(cInfo.neck_path_file,Size(cInfo.back->width,cInfo.back->height),face_warp_points1);
		std::stringstream ss;
		
		//try to open neck path for input image
		ss << inputFile.substr(0,inputFile.find_last_of('.')) << ".txt";
		
		FILE* _f;
		int _fopenResponse = fopen_s(&_f,ss.str().c_str(),"r");
		if(_fopenResponse != 0) {
			//neck path file could not be opened, try to estimate... but how?

//			Mat edges; //(orig->height,orig->width,CV_8UC1);
//			cvtColor(Mat(orig),edges,CV_RGB2GRAY);
//			Canny(edges,edges,100,150);
//			vector<Point2f> edgesPoints;
//			for(int _y=0;_y<edges.rows;_y++) {
//				for(int _x=0;_x<edges.cols;_x++) {
//					if(*(edges.data + _y*edges.step + _x) == 255) {
//						edgesPoints.push_back(Point(_x,_y));
//					}
//				}
//			}
//#ifdef BTM_DEBUG
//			Mat _tmp;
//			cvtColor(edges,_tmp,CV_GRAY2RGB);
//			for(unsigned int _i=0;_i<edgesPoints.size();_i++) {
//				circle(_tmp,edgesPoints[_i],2,Scalar(0,0,255));
//			}
//			imshow("tmp",_tmp);
//			waitKey(BTM_WAIT_TIME);
//#endif
//
//			Mat _pts1,_pts2(edgesPoints);
//			Mat(face_warp_points1).convertTo(_pts1,CV_32FC2);
//			vector<uchar> _maskV;
//			Mat _h = findHomography(_pts1,_pts2,_maskV);
//
//			face_warp_points2 = vector<Point2d>(face_warp_points1.size());
//			Mat newpts2(face_warp_points2);
//			newpts2 = _pts1 * _h;
//#ifdef BTM_DEBUG
//			Mat(orig).copyTo(_tmp);
//			for(unsigned int _i=0;_i<face_warp_points2.size();_i++) {
//				circle(_tmp,face_warp_points2[_i],2,Scalar(0,0,255));
//			}
//			imshow("tmp",_tmp);
//			waitKey(BTM_WAIT_TIME);
//#endif
		} else {
			face_warp_read_points(ss.str(),Size(orig->width,orig->height),face_warp_points2);
		}

		//warp
		{
			double scaleFromFaceToBack = (double)cInfo.eye_distance / distance(eye1,eye2);
			Mat facePts(face_warp_points2);
			facePts = facePts * scaleFromFaceToBack;

			vector<Mat* > imgs;
			Mat backM(cInfo.back),maskM(cInfo.back_mask);
			imgs.push_back(&(backM));
			imgs.push_back(&(maskM));

			//warp back into face
			face_warp(imgs,
				face_warp_points1,
				face_warp_points2,
				Size(orig->width,orig->height),
				(use_warp_affine)?FACE_WARP_AFFINE:FACE_WARP_RIGID,
				(use_double_warp)?0.75:1.0);

			if(use_double_warp) {
				//warp face into back...
				imgs.clear();
				backM = Mat(orig);
				maskM = Mat(mask);
				imgs.push_back(&(backM));
				imgs.push_back(&(maskM));
				face_warp(imgs,
					face_warp_points2,
					face_warp_points1,
					Size(cInfo.back->width,cInfo.back->height),
					(use_warp_affine)?FACE_WARP_AFFINE:FACE_WARP_RIGID,
					0.25);
			}

			GaussianBlur(maskM,maskM,Size(7,7),1);
			//blur(maskM,maskM,Size(7,7));

			facePts = Mat(face_warp_points2);
			facePts = facePts / scaleFromFaceToBack;
		}

		}
//------------------------------------- end warping -------------------------------------


		if(cInfo.grayscale == 1) {	//model should be grayscale
			IplImage* __tmp = cvCreateImage(cvGetSize(orig),8,1);
			cvCvtColor(orig,__tmp,CV_RGB2GRAY);
			cvCvtColor(__tmp,orig,CV_GRAY2RGB);
			cvReleaseImage(&__tmp);
		}

		FaceInfo fInfo;
		fInfo.face = orig;
		fInfo.face_mask = mask;
		fInfo.lEye = eye1;
		fInfo.rEye = eye2;
		fInfo.face_rect = bx;

		//CvRect bound = cvRect(cInfo.back_mask->width,cInfo.back_mask->height,0,0);
		//for(int y=0;y<cInfo.back_mask->height;y++) {
		//	char* linePtr = cInfo.back_mask->imageData + cInfo.back_mask->widthStep * y;
		//	for(int x=0;x<cInfo.back_mask->width;x++) {
		//		char* ptr = linePtr + x * cInfo.back_mask->nChannels;
		//		uchar val = (uchar)(*ptr);
		//		if(val>0) {
		//			if(y<bound.y) bound.y = y;
		//			if(x<bound.x) bound.x = x;
		//			if(y-bound.y > bound.height) bound.height = y - bound.y;
		//			if(x-bound.x > bound.width) bound.width = x - bound.x;
		//		}
		//	}
		//}

		//cvDrawRect(cInfo.back,cvPoint(bound.x,bound.y),cvPoint(bound.x+bound.width,bound.y+bound.height),CV_RGB(255,0,0));

		double eyesD = distance(fInfo.lEye,fInfo.rEye);
		double scaleFromFaceToBack = (double)cInfo.eye_distance / eyesD;
		int m = (int)floor((double)bx.height * scaleFromFaceToBack);
		int n = (int)floor((double)bx.width * scaleFromFaceToBack);
		int mn = m * n;
		int x_off = cInfo.left_eye.x - (int)floor((double)(MIN(fInfo.lEye.x,fInfo.rEye.x) - bx.x) * scaleFromFaceToBack), 
			y_off = cInfo.left_eye.y - (int)floor((double)(fInfo.lEye.y - bx.y) * scaleFromFaceToBack);

		if(use_warp_affine || use_warp_rigid) {
			Scalar m1 = mean(Mat(face_warp_points1));
			Scalar m2 = mean(Mat(face_warp_points2));
			x_off = m1[0] - (int)floor((m2[0] - bx.x) * scaleFromFaceToBack);
			y_off = m1[1] - (int)floor((m2[1] - bx.y) * scaleFromFaceToBack);
		}

		IplImage* face = cvCreateImage(cvSize(n,m),8,3);
		IplImage* face_mask = cvCreateImage(cvSize(n,m),8,1);
		IplImage* face_mask_inner = cvCreateImage(cvSize(n,m),8,1);

		cvSetImageROI(orig,bx);
		cvSetImageROI(mask,bx);
		cvSetImageROI(mask_inner,bx);
		cvResize(orig,face);
		cvResize(mask,face_mask,CV_INTER_NN);
		cvResize(mask_inner,face_mask_inner,CV_INTER_NN);

		cvSetImageROI(cInfo.back,cInfo.hue_measure);
		IplImage* im = cvCreateImage(cvSize(cInfo.hue_measure.width,cInfo.hue_measure.height),8,3);
		cvCopy(cInfo.back,im);
		CvScalar I = cvAvg(im);
		cvResetImageROI(cInfo.back);
		cvReleaseImage(&im);

//---------------------------------------------- Recoloring ---------------------------------------------

		/******************************************/
		/**			 histogram matching			 **
		if(use_hist_match_hs || use_hist_match_rgb) {

		Mat src(face);
		Mat src1;
		src.copyTo(src1);

		Mat src_mask(face_mask);
		Mat dst(cInfo.back_for_poisson);
		Mat dst_mask = Mat::zeros(dst.size(),CV_8UC1);
		rectangle(dst_mask,Point(cInfo.hue_measure.x,cInfo.hue_measure.y),
			Point(cInfo.hue_measure.x+cInfo.hue_measure.width,cInfo.hue_measure.y+cInfo.hue_measure.height),
			CV_RGB(255,255,255),CV_FILLED);

		//if(use_hist_match_hs) {
			histMatchHS(src,src_mask,dst,dst_mask);
		//} else {
			histMatchRGB(src1,src_mask,dst,dst_mask);
		//}
		src = (src * .75) + (src1 * .25);
#ifdef BTM_DEBUG
		cvNamedWindow("recoloring",CV_WINDOW_AUTOSIZE);
		imshow("recoloring",src);

		waitKey(BTM_WAIT_TIME);

		cvDestroyWindow("recoloring");
#endif
		}
		/**************************************/
		/** use poisson to set the face color /
		CloneInfo tmpCI;
		tmpCI.back = cvCreateImage(cvSize(n+2,m+2),8,3);
		cvSet(tmpCI.back,cvScalar(I.val[0]*.5,I.val[1]*.5,I.val[2]*.5));
		tmpCI.back_mask = cvCreateImage(cvGetSize(tmpCI.back),8,1);
		cvSet(tmpCI.back_mask,cvScalarAll(255));
		tmpCI.eye_distance = eyesD * scaleFromFaceToBack;
		tmpCI.left_eye = cvPoint(fInfo.lEye.x*scaleFromFaceToBack+1,fInfo.lEye.y*scaleFromFaceToBack+1);
		tmpCI.right_eye = cvPoint(fInfo.rEye.x*scaleFromFaceToBack+1,fInfo.rEye.y*scaleFromFaceToBack+1);

		FaceInfo tmpFI = fInfo;
		tmpFI.face = face;
		tmpFI.face_rect = cvRect(0,0,n,m);
		tmpFI.lEye.x *= scaleFromFaceToBack;
		tmpFI.lEye.y *= scaleFromFaceToBack;
		tmpFI.rEye.x *= scaleFromFaceToBack;
		tmpFI.rEye.y *= scaleFromFaceToBack;
		tmpFI.face_mask = face_mask;

		paste_face_on_clone(&tmpFI,&tmpCI);

		cvSetImageROI(tmpCI.back,cvRect(1,1,tmpFI.face_rect.width,tmpFI.face_rect.height));
		cvCopy(tmpCI.back,face);//,face_mask);
		cvResetImageROI(tmpCI.back);

		cvReleaseImage(&tmpCI.back);
		cvReleaseImage(&tmpCI.back_mask);
		/**************************************/



		/*************************************************/
		/**			 "Overlay" alg from GIMP			**
		if(use_overlay) {
		for(int x=0;x<face->width;x++) {
			for (int y=0;y<face->height;y++) {
				double alpha = cvGet2D(face_mask,y,x).val[0];
				if(alpha > 0) {
					CvScalar M = cvGet2D(face,y,x);
					CvScalar res = cvScalar(
						doOverlay(M.val[0],I.val[0]),
						doOverlay(M.val[1],I.val[1]),
						doOverlay(M.val[2],I.val[2]));
					cvSet2D(face,y,x,res);
				}
			}
		}
		}
		/**************************************************/



		//IplImage* red = cvCreateImage(cvGetSize(face),8,1);
		//IplImage* green = cvCreateImage(cvGetSize(face),8,1);
		//IplImage* blue = cvCreateImage(cvGetSize(face),8,1);
		//cvSplit(face,red,green,blue,NULL);
		//cvEqualizeHist(red,red);
		//cvEqualizeHist(green,green);
		//cvEqualizeHist(blue,blue);
		//cvMerge(red,green,blue,NULL,face);
		//cvReleaseImage(&red);
		//cvReleaseImage(&green);
		//cvReleaseImage(&blue);

		/********************** HSV TRIES *****************************
		cvCvtColor(face,face,CV_BGR2HSV);
		//CvScalar face_hsv_avg = cvAvg(face,face_mask);

		//cvCvtColor(cInfo.back,cInfo.back,CV_BGR2HSV);
		//cvSetImageROI(cInfo.back,cvRect(110,136,5,5));
		////cvSetImageCOI(cInfo.back,1);
		//IplImage* im = cvCreateImage(cvSize(5,5),8,3);
		//cvCopy(cInfo.back,im);
		//CvScalar bs = cvAvg(im);
		//cvResetImageROI(cInfo.back);
		////cvSetImageCOI(cInfo.back,0);
		//cvReleaseImage(&im);
		//cvCvtColor(cInfo.back,cInfo.back,CV_HSV2BGR);

		IplImage* value = cvCreateImage(cvSize(n,m),8,1);
		IplImage* saturation = cvCreateImage(cvSize(n,m),8,1);
		IplImage* hue = cvCreateImage(cvSize(n,m),8,1);
		//IplImage* signedIm = cvCreateImage(cvSize(n,m),32,1);
		cvSplit(face,hue,saturation,value,NULL);
//		IplImage* _value = cvCreateImage(cvSize(n,m),8,1);
//		cvZero(_value);
//		//cvAddS(_value,cvScalarAll(face_hsv_avg.val[2]),_value);
//		cvCopy(value,_value,face_mask_inner);
//
#ifdef BTM_DEBUG
		cvShowImage("result",saturation);
		cvWaitKey(0);
#endif
//
		//cvEqualizeHist(value,value);
//#ifdef BTM_DEBUG
//		cvShowImage("result",_value);
//		cvWaitKey(0);
//#endif
		//cvCopy(_value,value,face_mask_inner);
		//cvEqualizeHist(saturation,saturation);

		//cvConvertScale(hue,signedIm,1.0/255.0);
		//CvScalar s = cvAvg(hue_signed,face_mask_inner);
		//cvAddS(hue_signed,cvScalarAll(bs.val[0]/255.0 - s.val[0]),hue_signed,face_mask_inner);

		//cvSet(hue,cvScalarAll(bs.val[0]),face_mask_inner);
		//cvSet(saturation,cvScalarAll(bs.val[1]),face_mask_inner);
		//cvScaleAdd(signedIm,cvScalarAll(bs.val[0]),NULL,signedIm);
		//cvConvertScale(signedIm,hue,255.0);
		//cvAddS(hue,cvScalarAll(bs.val[0]),hue,face_mask_inner);

		//double M = bs.val[0];
		//for(int x=0;x<hue->width;x++) {
		//	for (int y=0;y<hue->height;y++) {
		//		if(cvGet2D(face_mask_inner,y,x).val[0] > 0) {
 	//				double I = cvGet2D(hue,y,x).val[0];
		//			double val = (I / 255.0) * (I + 2.0 * M / 255.0 /** (255.0 - I)/);
		//			//val = val*.2+bs.val[0]*.8;
		//			//val = 255.0 - ((255.0-bs.val[0])*(255.0-val)/255);
		//			//double rs = (255.0-((255.0-i)*(255.0-M)))/256.0;
		//			//double val = ((255.0-i)*M*i + (i*rs))/255.0;
		//			cvSet2D(hue,y,x,cvScalarAll(M));
		//			//cvSet2D(hue,y,x,cvScalarAll(bs.val[0]));

		//			val = cvGet2D(saturation,y,x).val[0];
		//			//val = (val / 255.0) * (val + bs.val[1] * 2.0 / 255.0 * (255.0 - val));
		//			//val = val*.2+bs.val[1]*.8;
		//			//val = 255.0 - ((255.0-bs.val[1])*(255.0-val)/255);
		//			cvSet2D(saturation,y,x,cvScalarAll(val));
		//			////cvSet2D(saturation,y,x,cvScalarAll(bs.val[1]));

		//			//val = cvGet2D(value,y,x).val[0];
		//			////val = (val / 255.0) * (val + bs.val[2] * 2.0 / 255.0 * (255.0 - val));
		//			//val = val*.2+bs.val[2]*.8;
		//			//cvSet2D(value,y,x,cvScalarAll(val));
		//		}
		//	}
		//}

		cvMerge(hue,saturation,value,NULL,face);

		cvCvtColor(face,face,CV_HSV2BGR);
		/**************************** END HSV TRIES ******************************


//---------------------------------------- Matting (pasting) -----------------------------------------

		{
		cvSetImageROI(cInfo.back,cvRect(x_off,y_off,n,m));
		cvSetImageROI(cInfo.back_mask,cvRect(x_off,y_off,n,m));

		cvSmooth(face_mask,face_mask);

		Mat __tmpMask(cInfo.back->height,cInfo.back->width,CV_8UC1,Scalar(0));

		for(int y=0;y<cInfo.back->roi->height;y++) {
			uchar* __tmpMaskPtr = __tmpMask.ptr(y);
			for(int x=0;x<cInfo.back->roi->width;x++) {
				double alpha_b_m = cvGet2D(cInfo.back_mask,y,x).val[0];
				double alpha_f_m = cvGet2D(face_mask,y,x).val[0];
				if(alpha_b_m * alpha_f_m > 0.0) {
					alpha_b_m = (alpha_b_m * alpha_f_m) / 255;
					CvScalar backRGB = cvGet2D(cInfo.back,y,x);
					CvScalar faceRGB = cvGet2D(face,y,x);
					CvScalar RGB;
					__tmpMaskPtr[x] = (uchar)alpha_b_m;
					if(alpha_b_m == 255.0) {
						RGB = faceRGB;
					} else if(alpha_b_m == 0.0) {
						RGB = backRGB;
					} else {
						double alpha = alpha_b_m / 255.0;
						RGB = cvScalar(
							faceRGB.val[0] * alpha + backRGB.val[0] * (1.0 - alpha),
							faceRGB.val[1] * alpha + backRGB.val[1] * (1.0 - alpha),
							faceRGB.val[2] * alpha + backRGB.val[2] * (1.0 - alpha)
							);
					}
					cvSet2D(cInfo.back,y,x,RGB);
				}
			}
		}

		//cvAnd(face_mask,cInfo.back_mask,face_mask);
		//cvCopy(face,cInfo.back,face_mask);
		cvResetImageROI(cInfo.back);
		

//showresult:
#ifdef BTM_DEBUG
		imshow("tmp",__tmpMask);
		cvShowImage("result",cInfo.back);
		cvWaitKey(0);
#endif
		}	// end pasting

#ifndef BTM_SERVER
		{
			std::stringstream ss;
			ss << "result_" << inputFile.substr(0,inputFile.find_last_of('.'));
			//if(use_ellipse) ss << "_ellipse";
			//if(do_poisson) ss << "_poisson";
			//if(use_hist_backproj) ss << "_histBP";
			if(use_hist_match_hs) ss << "_histmatchHS";
			if(use_hist_match_rgb) ss << "_histmatchRGB";
			//if(use_grab_cut_opencv) ss << "_grabcutOpCV";
			//if(use_grab_cut_JT) ss << "_grabcutJT";
			if(use_overlay) ss << "_overlay";
			if(use_double_warp) ss << "_dbl";
			if(use_warp_rigid) ss << "_warpRigid";
			if(use_warp_affine) ss << "_warpAffine";

			ss << ".png";
			fprintf(stderr,"Output file: %s\n",ss.str().c_str());
			cvSaveImage(ss.str().c_str(),cInfo.back);
		}
#else
		{
			//argv[3] contains the output filename
			cvSaveImage(argv[3],cInfo.back);
		}
#endif
		
		cvReleaseImage( &(cInfo.back) );
		cvReleaseImage( &(cInfo.back_mask) );

		//cvReleaseImage( &value);
		//cvReleaseImage( &saturation);
		//cvReleaseImage( &hue);
		//cvReleaseImage( &signedIm);
		cvReleaseImage( &face);
		cvReleaseImage( &face_mask);
		cvReleaseImage( &face_mask_inner);
		
	}
	//	if(c=='q') break;
	//}

	//IplImage* small = cvCreateImage(cvSize(image->width/2,image->height/2),image->depth,image->nChannels);
	//cvResize(image,small);
	//cvShowImage("result",small);
 //   cvWaitKey(0);;

    //cvReleaseImage( &small );
	//cvReleaseCapture(&cap);

#ifdef BTM_DEBUG
	cvDestroyAllWindows();
#endif

end:

	cvReleaseImage( &image );
	cvReleaseImage( &gray );
    cvReleaseImage( &small_img );
	cvReleaseImage( &orig );
	cvReleaseImage( &edge );
	cvReleaseImage( &edge_8bit );
	cvReleaseImage( &mask );
	cvReleaseImage( &mask_inner );

end_cascades_and_storage:
	//cvReleaseMemStorage(&storage);

	//cvReleaseHaarClassifierCascade(&cascade);
	//cvReleaseHaarClassifierCascade(&nested_cascade);

	printf("END\n");
	return 0;
}
**/