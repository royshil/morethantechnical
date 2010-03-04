#include "util.h"

extern IplImage *small_img;
//extern CvHaarClassifierCascade* cascade;
//extern CvHaarClassifierCascade* nested_cascade;
extern CascadeClassifier cascade;
extern CascadeClassifier nestedCascade;

extern int use_nested_cascade;
extern double scale;

extern int btm_wait_time;

void GCCallback(const GrabCutNS::GrabCut* GC) {
#ifdef BTM_DEBUG
	const GrabCutNS::Image<float>* im = GC->getAlphaImage();
	IplImage* __GCtmp = cvCreateImage(cvSize(im->width(),im->height()),8,1);
	GrabCutNS::fromImageMaskToIplImage(im,__GCtmp);
	cvShowImage("tmp",__GCtmp);
	cvWaitKey(BTM_WAIT_TIME);
#endif
}

CvRect find_bounding_rect_of_mask(IplImage* mask) {
	//find bounding rect of mask
	CvPoint br = cvPoint(-1,-1),tl = cvPoint(mask->width+1,mask->height+1);
	for(int y=0;y<mask->height;y++) {
		for(int x=0;x<mask->width;x++) {
			if(cvGet2D(mask,y,x).val[0] > 0) {
				if(x < tl.x) tl.x = x;
				if(y < tl.y) tl.y = y;
				if(x > br.x) br.x = x;
				if(y > br.y) br.y = y;
			}
		}
	}
	return cvRect(tl.x,tl.y,br.x-tl.x,br.y-tl.y);
}

void get_center_connected_component(IplImage* mask, IplImage* image, CvScalar midPt) {
	extern cv::Ptr<CvMemStorage> storage;


	//Find the connected component that includes the face
	CvSeq* contours = 0;
    cvFindContours( mask, storage, &contours, sizeof(CvContour),
            CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );

	cvZero(image);
	//cvCopy(orig,image);
	cvDrawContours( image, contours, CV_RGB(0,255,255), CV_RGB(0,0,0), 0, 1, CV_AA, cvPoint(0,0) );
	while(contours->h_next) {
		contours = contours->h_next;
		cvDrawContours( image, contours, CV_RGB(0,255,255), CV_RGB(0,0,0), 0, 1, CV_AA, cvPoint(0,0) );
	}
	//Fill the face connected component
	IplImage* flood_mask = cvCreateImage(cvSize(image->width+2,image->height+2),8,1);
	cvSet(flood_mask,cvScalarAll(255));
	cvRectangle(flood_mask,cvPoint(3,3),cvPoint(flood_mask->width-3,flood_mask->height-3),CV_RGB(0,0,0),CV_FILLED);

	cvFloodFill(image,
		cvPoint(cvFloor(midPt.val[0]),
				cvFloor(midPt.val[1])),
		CV_RGB(255,0,0),
		cvScalarAll(0),
		cvScalarAll(0),
		0,
		8,
		flood_mask);

	cvReleaseImage(&flood_mask);

	IplImage* red = cloneIm(mask);
	IplImage* green = cloneIm(mask);
	IplImage* blue = cloneIm(mask);
	cvSplit(image,red,green,blue,NULL);

	cvCopy(blue,mask);

	cvReleaseImage(&red);
	cvReleaseImage(&green);
	cvReleaseImage(&blue);
}

void prune_points_set(int* num_points_ptr, CvPoint2D32f** points_ptr) {
	int num_points_on_path = *num_points_ptr;
	CvPoint2D32f* points = *points_ptr;
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
	*points_ptr = new_points;
	*num_points_ptr = good_pts_count;
}

void refine_face_path_with_LLSQ(int num_points_on_path, CvPoint2D32f* points, IplImage* paint_over) {
	/************
	Refine the outer border of the face
	************/
	//Get top points
	//--------------
	CvMat* _m = cvCreateMatHeader(num_points_on_path,1,CV_32FC2);
	cvSetData(_m,points,sizeof(CvPoint2D32f));
	CvScalar avg = cvAvg(_m);
	cvAddS(_m,cvScalar(-avg.val[0],-avg.val[1]),_m);
	cvConvertScale(_m,_m,1/avg.val[0]);

	//find top-left and bottom right (bounding box of face path)
	CvPoint2D32f topLeft = cvPoint2D32f(FLT_MAX,FLT_MAX), bottomRight = cvPoint2D32f(FLT_MIN,FLT_MIN);
	for(int i=0;i<num_points_on_path;i++) {
		CvPoint2D32f _pt = points[i];
		if(_pt.x < topLeft.x) topLeft.x = _pt.x;
		if(_pt.y < topLeft.y) topLeft.y = _pt.y;
		if(_pt.x > bottomRight.x) bottomRight.x = _pt.x;
		if(_pt.y > bottomRight.y) bottomRight.y = _pt.y;
	}
	//move bottom right to be half-way up the face path
	bottomRight.y -= ((bottomRight.y - topLeft.y)/5)*3;

	CvMat* ptsInRange = cvCreateMat(num_points_on_path,1,CV_8UC1);
	cvInRangeS(_m,cvScalar(topLeft.x,topLeft.y),cvScalar(bottomRight.x,bottomRight.y),ptsInRange);

	int count_pts_in_range = 0;
	for(int i=0;i<num_points_on_path;i++) {
		if(cvGet1D(ptsInRange,i).val[0] > 0) {
#ifdef BTM_DEBUG
			cvCircle(paint_over,
				cvPoint(cvFloor(points[i].x * avg.val[0] + avg.val[0]), 
						cvFloor(points[i].y * avg.val[0] + avg.val[1])),
					2,CV_RGB(255,0,255),2);
#endif
			count_pts_in_range++;
		}
	}

#define BTM_NUM_COEFFS 7

	//perform least-squares - get estimated polynom
	int num_coeffs = BTM_NUM_COEFFS;
	CvMat* _A = cvCreateMat(count_pts_in_range,num_coeffs,CV_32FC1);
	CvMat* _B = cvCreateMat(count_pts_in_range,1,CV_32FC1);
	float X[BTM_NUM_COEFFS] = {0.0f};
	CvMat* _X = cvCreateMatHeader(num_coeffs,1,CV_32FC1);
	cvSetData(_X,X,sizeof(float));
	for(int i=0,ii=0;i<num_points_on_path;i++) {
		if(cvGet1D(ptsInRange,i).val[0] > 0) {
			CvPoint2D32f pt = points[i];
			_A->data.fl[ii*num_coeffs] = 1;
			_A->data.fl[ii*num_coeffs+1] = pt.x;
			_A->data.fl[ii*num_coeffs+2] = pt.x*pt.x;
			_A->data.fl[ii*num_coeffs+3] = pt.x*pt.x*pt.x;
			_A->data.fl[ii*num_coeffs+4] = pt.x*pt.x*pt.x*pt.x;
			_A->data.fl[ii*num_coeffs+5] = pt.x*pt.x*pt.x*pt.x*pt.x;
			_A->data.fl[ii*num_coeffs+6] = pt.x*pt.x*pt.x*pt.x*pt.x*pt.x;
			_B->data.fl[ii] = pt.y;
			ii++;
		}
	}
	cvSolve(_A,_B,_X,CV_SVD);

#ifdef BTM_DEBUG
	printf("coeffs: %f\n \t%f\n \t%f\n \t%f\n \t%f\n \t%f\n \t%f\n \t%f\n",X[0],X[1],X[2],X[3],X[4],X[5],X[6]);
#endif

	//transform points to conform with polynom
	for(int i=0,ii=0;i<num_points_on_path;i++) {
		if(cvGet1D(ptsInRange,i).val[0] > 0) {
			float x = points[i].x;
			points[i].y = X[0] + 
						  X[1] * x + 
						  X[2] * x*x +
						  X[3] * x*x*x +
						  X[4] * x*x*x*x + 
						  X[5] * x*x*x*x*x + 
						  X[6] * x*x*x*x*x*x;
		}
	}

	cvConvertScale(_m,_m,avg.val[0]);
	cvAddS(_m,avg,_m);

#ifdef BTM_DEBUG
	for(int i=0;i<num_points_on_path;i++) {
		if(cvGet1D(ptsInRange,i).val[0] > 0) {
			cvCircle(paint_over,cvPointFrom32f(points[i]),2,CV_RGB(255,255,0),2);
		}
	}

	cvShowImage("result",paint_over);
	cvWaitKey(BTM_WAIT_TIME);
#endif

	cvReleaseMat(&_A);
	cvReleaseMat(&_B);
	cvReleaseMat(&_X);
	cvReleaseMat(&_m);

	cvReleaseMat(&ptsInRange);
}

void draw_points_poly_mask(CvPoint2D32f* points, int num_points_on_path, IplImage* mask, IplImage* paint_over) {
	CvPoint* pointsI = new CvPoint[num_points_on_path];
	for(int i=0;i<num_points_on_path;i++) {
		pointsI[i] = cvPointFrom32f(points[i]);
	}
	int polyC[] = {num_points_on_path};
	CvPoint* ptsPtr = ((CvPoint*)pointsI);
	if(paint_over!=0) {
		cvDrawPolyLine(paint_over,&ptsPtr,polyC,1,1,CV_RGB(255,100,0),2);
	}
	cvFillPoly(mask,&ptsPtr,polyC,1,CV_RGB(255,255,255));
	cvErode(mask,mask,0,2);

	delete[] pointsI;
}

void paste_face_on_clone(FaceInfo* faceI, CloneInfo* cloneI) {
	IplImage* orig = faceI->face;
	IplImage* mask = faceI->face_mask;
	CvRect bx = faceI->face_rect;

	double eyesD = distance(faceI->lEye,faceI->rEye);
	double scaleFromFaceToBack = (double)cloneI->eye_distance / eyesD;
	int m = (int)floor((double)bx.height * scaleFromFaceToBack);
	int n = (int)floor((double)bx.width * scaleFromFaceToBack);
	int mn = m * n;
	int x_off = cloneI->left_eye.x - (int)floor((double)(MIN(faceI->lEye.x,faceI->rEye.x) - bx.x) * scaleFromFaceToBack), 
		y_off = cloneI->left_eye.y - (int)floor((double)(faceI->lEye.y - bx.y) * scaleFromFaceToBack);

	IplImage* face = cvCreateImage(cvSize(n,m),8,3);
	IplImage* face_mask = cvCreateImage(cvSize(n,m),8,1);

	cvSetImageROI(orig,bx);
	cvSetImageROI(mask,bx);
	cvResize(orig,face);
	cvResize(mask,face_mask,CV_INTER_NN);
	cvErode(face_mask,face_mask,0,2);	//make sure no radical pixels are included in the mask

	IplImage* back = cloneI->back_for_poisson;
	IplImage* back_mask = cloneI->back_mask_for_poisson;

	cvSetImageROI(back_mask,cvRect(x_off,y_off,n,m));
	cvAnd(face_mask,back_mask,face_mask);

	std::vector<double > rgbVector[3];// = {std::vector<double >(mn)};
	for (int color = 0; color < 3; color++) {
		rgbVector[color] = std::vector<double >(mn);
	}
	
	uchar* maskData = (uchar*)face_mask->imageData;
	uchar* backData = (uchar*)back->imageData;
	uchar* faceData = (uchar*)face->imageData;

	for (int y = 0; y < m; y++) {
		for (int x = 0; x < n; x++) {
			for (int color = 0; color < 3; color++) {
				if(maskData[y * face_mask->widthStep + x] == 0) {
					rgbVector[color][y*n+x] = backData[(y+y_off) * back->widthStep + (x+x_off) * back->nChannels + color];
				} else {
					int clonedLaplaceForPixel = 0;
					int numNeighbours = 0;

					//right pixel
					if (x < n - 1) {
						clonedLaplaceForPixel += faceData[y*face->widthStep + (x+1)*face->nChannels + color];
						numNeighbours++;
					}

					//below pixel
					if (y < m - 1) {
						clonedLaplaceForPixel += faceData[(y+1)*face->widthStep + x*face->nChannels + color];
						numNeighbours++;
					}

					//left pixel
					if (x > 0) {
						clonedLaplaceForPixel += faceData[y*face->widthStep + (x-1)*face->nChannels + color];
						numNeighbours++;
					}

					//top pixel
					if (y > 0) {
						clonedLaplaceForPixel += faceData[(y-1)*face->widthStep + x*face->nChannels + color];
						numNeighbours++;
					}

					clonedLaplaceForPixel -= faceData[y*face->widthStep + x*face->nChannels + color] * numNeighbours;

					rgbVector[color][y * n + x] = clonedLaplaceForPixel;																								
				}
			}
		}
	}

	gmm::row_matrix< gmm::rsvector<double> > M(mn,mn);
	OpenCVImage maskImage(face_mask);
	ImageEditingUtils::matrixCreate(M, n, mn, maskImage);

	std::vector<double > solutionVectors[3];
	for (int color = 0; color < 3; color++) {
		solutionVectors[color] = std::vector<double >(mn);
	}

	IplImage* output = cvCreateImage(cvGetSize(face),8,3);
	
	for (int color = 0; color < 3; color++) {
		ImageEditingUtils::solveLinear(M,solutionVectors[color],rgbVector[color]);
#ifdef BTM_DEBUG
		printf("done with color %d\n",color);
#endif
	}
	for(int y=0;y<m;y++) {
		for(int x=0;x<n;x++)
		{
			uchar* pxl = (uchar*)(output->imageData + y*output->widthStep + x*output->nChannels);
			pxl[0] = (uchar)cvFloor(MAX(MIN(solutionVectors[0][y*n + x],255.0),0.0));
			pxl[1] = (uchar)cvFloor(MAX(MIN(solutionVectors[1][y*n + x],255.0),0.0));
			pxl[2] = (uchar)cvFloor(MAX(MIN(solutionVectors[2][y*n + x],255.0),0.0));
		}
	}

	cvSetImageROI(back,cvRect(x_off,y_off,output->width,output->height));
	cvCopy(output,back);
	cvResetImageROI(back);
}

void find_face_path_homography(IplImage* edge_8bit, CvPoint2D32f* points, int numPathPts, IplImage* paint_over) {
	Mat m_object(numPathPts, 1, CV_32FC2, points);
	int numEdgePt = cvCountNonZero(edge_8bit);
	Mat m_image(numEdgePt, 1, CV_32FC2);

	int counter = 0;
	float* img_ptr = m_image.ptr<float>(0);
	for(int x=0;x<edge_8bit->width;x++) {
		for(int y=0;y<edge_8bit->height;y++) {
			if(cvGet2D(edge_8bit,y,x).val[0] > 0) {
				img_ptr[counter*2] = (float)x;
				img_ptr[counter*2+1] = (float)y;
				counter++;
			}
		}
	}

	//Mat status;
	Mat H = findHomography(m_object,m_image,CV_RANSAC,10.0);

	//getAffineTransform(

	perspectiveTransform(m_object,m_object,H);
}

void find_face_path(CvPoint eye1, CvPoint eye2, IplImage* edge_8bit, CvPoint2D32f* points, int numPathPts, IplImage* paint_over) {
#ifdef BTM_DEBUG
	for(int i=0;i<numPathPts;i++) {
		cvCircle(paint_over,cvPointFrom32f(points[i]),2,CV_RGB(255,0,255),2);
	}
#endif

	//TODO: gotta use FLANN in openCV 2.0 ...

    Mat m_object(numPathPts, 2, CV_32F, points);

	//Use KD-Tree to get exact face boundary
	//insert all edge points into kd-tree
	int numEdgePt = cvCountNonZero(edge_8bit);

	Mat m_image(numEdgePt, 2, CV_32F);

	int counter = 0;
	float* img_ptr = m_image.ptr<float>(0);
	for(int x=0;x<edge_8bit->width;x++) {
		for(int y=0;y<edge_8bit->height;y++) {
			if(cvGet2D(edge_8bit,y,x).val[0] > 0) {
				img_ptr[counter*2] = (float)x;
				img_ptr[counter*2+1] = (float)y;
				counter++;
			}
		}
	}

	// find nearest neighbors using FLANN
	Mat m_indices(numPathPts, 2, CV_32S);
    Mat m_dists(numPathPts, 2, CV_32F);
	cv::flann::Index flann_index(m_image, cv::flann::KDTreeIndexParams(4));  // using 4 randomized kdtrees
    flann_index.knnSearch(m_object, m_indices, m_dists, 1, cv::flann::SearchParams(64) ); // maximum number of leafs checked


	double thresh = distance(eye1,eye2) / 2;
	for(int i=0;i<numPathPts;i++) {
		int idx = m_indices.at<int>(i,0);
		if(idx >= 0) { //found a match for the i'th point in the path
			float distV = m_dists.at<float>(i,0);
			if(distV < thresh) {
				CvPoint2D32f match = cvPoint2D32f(
					m_image.at<float>(idx,0),
					m_image.at<float>(idx,1)
					);
				points[i] = match;
			}
		}
	}

	//Mat H = findHomography(m_object,m_image,CV_RANSAC,10.0);

	////getAffineTransform(

	//perspectiveTransform(m_object,m_object,H);

#ifdef BTM_DEBUG
	for(int i=0;i<numPathPts;i++) {
		cvCircle(paint_over,cvPointFrom32f(points[i]),2,CV_RGB(255,255,0),2);
	}
#endif
}	

void detect_and_draw(IplImage* gray, IplImage* paint_over, CvPoint& eye1, CvPoint& eye2, float& output_phi) {
	extern cv::Ptr<CvMemStorage> storage;


    cvResize( gray, small_img, CV_INTER_LINEAR );
	IplImage* small_orig = cvCreateImage(cvGetSize(small_img),small_img->depth,small_img->nChannels);
	cvCopy(small_img,small_orig);
    cvEqualizeHist( small_img, small_img );
    //cvClearMemStorage( storage );

    double t = (double)cvGetTickCount();
	vector<Rect> faces;
	cascade.detectMultiScale(
		Mat(small_img),
		faces,
		1.1,
		2,
		0 | CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_DO_CANNY_PRUNING,
		Size(30,30));

    //CvSeq* faces = cvHaarDetectObjects( small_img, cascade, storage,
    //                                    1.1, 2, 0
    //                                    |CV_HAAR_FIND_BIGGEST_OBJECT
    //                                    //|CV_HAAR_DO_ROUGH_SEARCH
    //                                    |CV_HAAR_DO_CANNY_PRUNING
    //                                    //|CV_HAAR_SCALE_IMAGE
    //                                    ,
    //                                    cvSize(30, 30) );
    t = (double)cvGetTickCount() - t;
#ifdef BTM_DEBUG
    printf( "detection time = %gms\n", t/((double)cvGetTickFrequency()*1000.) );
#endif
	if(!(faces.size() > 0)) {
		//No fce recognized!!! take left eye as (4/10,1/3) and right eyes as (6/10,1/3)
		// assuming the photographer tried to put the face in the middle. sorta
		CvSize s = cvGetSize(gray);
		eye1 = cvPoint((int)((double)s.width * 4.0 / 10.0),(int)((double)s.height / 3.0)); 
		eye2 = cvPoint((int)((double)s.width * 6.0 / 10.0),(int)((double)s.height / 3.0)); 
		return;
	}
	for(unsigned int i = 0; i < faces.size(); i++ )
    {
        //CvRect* r = (CvRect*)cvGetSeqElem( faces, i );
		Rect* r = &(faces[i]);

#ifdef BTM_DEBUG
		cvDrawRect(paint_over,cvPoint(r->x,r->y),cvPoint(r->x+r->width,r->y+r->height),CV_RGB(255,0,0),2);
#endif

		//CvSeq* nested_objects;
		vector<Rect> nested_objects;
		CvMat small_img_roi;
        cvGetSubRect( small_orig, &small_img_roi, *r );
		//cvEqualizeHist(&small_img_roi,&small_img_roi);
		
		nestedCascade.detectMultiScale(Mat(&small_img_roi),nested_objects,1.1,1,0,Size(r->width/10,r->height/10));

        //nested_objects = cvHaarDetectObjects( &small_img_roi, nested_cascade, storage,
        //                            1.1, 1, 0
        //                            //|CV_HAAR_FIND_BIGGEST_OBJECT
        //                            //|CV_HAAR_DO_ROUGH_SEARCH
        //                            //|CV_HAAR_DO_CANNY_PRUNING
        //                            //|CV_HAAR_SCALE_IMAGE
        //                            ,
								//	cvSize(r->width/10, r->height/10) );
									//cvSize(0,0));
		if(nested_objects.size() >= 2) {
			//CvRect b = cvRect(-1,-1,0,0), nb = cvRect(-1,-1,0,0);
			//int num_n = -1, num_n_nb = -1;
			unsigned int nested_objects_count = nested_objects.size();
			double* sizes = new double[nested_objects_count];
			CvPoint* points = new CvPoint[nested_objects_count];

			for(unsigned int j = 0; j < nested_objects_count; j++ )
			{
				//CvAvgComp* ac = (CvAvgComp*)cvGetSeqElem( nested_objects, j );
				Rect ac = nested_objects[j];
				double sizeMul = ac.height * ac.width;
				sizes[j] = sizeMul;
				//CvRect b = ac->rect;
				CvPoint center;
				center.x = cvRound((r->x + ac.x + ac.width*0.5)*scale);
				center.y = cvRound((r->y + ac.y + ac.height*0.5)*scale);
				points[j] = center;
#ifdef BTM_DEBUG
				cvDrawRect(paint_over,cvPoint(r->x+ac.x,r->y+ac.y),cvPoint(r->x+ac.x+ac.width,r->y+ac.y+ac.height),CV_RGB(0,255,255),2);
#endif
			}

			//Find nearest neighbour
			int* pairs = new int[nested_objects_count];
			double* phis = new double[nested_objects_count];
			for(unsigned int ii=0;ii<nested_objects_count;pairs[ii] = -1,phis[i] = 0.0f,i++);

			double* pairsSize = new double[nested_objects_count];
			for(unsigned int k = 0; k < nested_objects_count; k++ )
			{
				int idx = -1;
				double min = DBL_MAX;
				double mins_phi = 0.0;
				CvPoint mins_left = cvPoint(-1,-1),mins_right = cvPoint(-1,-1);
				for(unsigned int j = k+1; j < nested_objects_count; j++ )
				{
					if(k==j || k == pairs[j]) continue;
					double sizeD = abs(sizes[k]-sizes[j]); //don't allow 0s
					if(sizeD == 0.0) sizeD = 1.0;

					//Make sure calculating angle from left eye to right eye
					CvPoint left,right;
					if(points[k].x < points[j].x) {
						left = points[k]; right = points[j];
					} else {
						left = points[j]; right = points[k];
					}
					double _phi = calc_angle_with_x_axis(cvPointTo32f(left),cvPointTo32f(right));
					double phi = MIN(_phi,abs(CV_PI-_phi)) / CV_PI; //for eyes choosing take the value after normalizing to 0 or PI
					if(phi == 0.0) phi = EPSILON;

					double szMul = sqrt(sizes[k] * sizes[j]);
					double d = distance(points[k],points[j]);
					int eye_est = r->y + r->height/3;
					double d_frm_est = abs(points[k].y - eye_est) * abs(points[j].y - eye_est);

					double left_d_frm_est = 0.0, right_d_frm_est = 0.0;
					if(points[k].x<points[j].x) {
						left_d_frm_est = points[k].x - r->x + r->width/3;
						right_d_frm_est = points[j].x - r->x + r->width*2/3;
					} else {
						left_d_frm_est = points[j].x - r->x + r->width/3;
						right_d_frm_est = points[k].x - r->x + r->width*2/3;
					}
					if(left_d_frm_est == 0.0) left_d_frm_est = 1.0;
					if(right_d_frm_est == 0.0) right_d_frm_est = 1.0;
					double lrdfe = left_d_frm_est*right_d_frm_est;

					double score =	100*sizeD + 
									10000*log(phi) + 
									2000*d_frm_est + 
									lrdfe - 
									(/*szMul + */800*d);
#ifdef BTM_DEBUG
					printf("%d (%d,%d) - %d (%d,%d) score: %.3f (sizeD %.0f) (size: %.0f) (phi: %.3f) (d: %.3f) (dfe: %.0f) (lrdfe: %.0f)\n",
						k,points[k].x,points[k].y,
						j,points[j].x,points[j].y,
						score,sizeD,szMul,log(phi),d,d_frm_est,lrdfe);
#endif

					if(score < min) {
						idx = j;
						min = score;
						mins_phi = _phi;
						mins_left = left;
						mins_right = right;
					}
				}
				//if(pairs[idx] == k) continue; //already a pair from the other side...
				pairs[k] = idx;
				phis[k] = (mins_right.y > mins_left.y) ? -mins_phi : mins_phi;

				pairsSize[k] = min;
#ifdef BTM_DEBUG
				char str[10] = {0}; sprintf_s(str,10,"%d",k);
				CvFont f;
				cvInitFont(&f,CV_FONT_HERSHEY_PLAIN,1.0,1.0);
				cvPutText(paint_over,str,cvPoint(points[k].x-20,points[k].y-20),&f,CV_RGB(255,0,0));
#endif
			}

			int idxA = -1, idxB = -1;
			double min = DBL_MAX;
			for(unsigned int k = 0; k < nested_objects_count; k++ )
			{
				if(pairs[k] < 0) continue; //some don't have pairs
				if(pairsSize[k] < min) {
					min = pairsSize[k];
					idxA = k;
					idxB = pairs[k];
				}
			}
#ifdef BTM_DEBUG
			cvCircle( paint_over, points[idxA], 2, CV_RGB(0,255,0), 3, 8, 0 );
			cvCircle( paint_over, points[idxB], 2, CV_RGB(0,255,0), 3, 8, 0 );
			printf("chosen eyes: %d %d (phi %.3f)\n",idxA,idxB,phis[idxA]);
#endif

			if(points[idxA].x<points[idxB].x) {
				eye1 = points[idxA];
				eye2 = points[idxB];
			} else {
				eye2 = points[idxA];
				eye1 = points[idxB];
			}
			output_phi = (float)(phis[idxA]);

			delete sizes,points,pairs,pairsSize,phis;
		} //eyes exist if
		else {
			//no eyes detected... estimate!
			eye1 = cvPoint(r->x+(int)((double)r->width / 3.0),r->y+(int)((double)r->height / 3.0)); 
			eye2 = cvPoint(r->x+(int)(2.0*(double)r->width / 3.0),r->y+(int)((double)r->height / 3.0)); 
		}
	} // faces for
}

double calc_angle_with_x_axis(CvPoint2D32f l, CvPoint2D32f r) {
	double vX = r.x - l.x;
	double vY = r.y - l.y;
	//normllize
	double norm = sqrt(vX*vX + vY*vY);
	vX /= norm;
	//vY /= norm;
	//cllc dot product with x lxis
	double dProd = vX*1; // + vY*0...
	//cllc lngle
	return acos(dProd);
}

float calc_angle(CvPoint2D32f a, CvPoint2D32f b) {
	//normalize
	float aNorm = sqrt(a.x*a.x + a.y*a.y);
	a.x /= aNorm;
	a.y /= aNorm;
	float bNorm = sqrt(b.x*b.x + b.y*b.y);
	b.x /= bNorm;
	b.y /= bNorm;
	//calc dot product with x axis
	float dProd = a.x*b.x + a.y*b.y;
	//calc angle
	return acos(dProd);
}

void create_ellipse_path(int numPathPts,CvPoint2D32f* points, CvPoint eye1, CvPoint eye2) {
	//Create an ellipse path

	double eyeD = distance(eye1,eye2);

	//TODO: maybe use a path that looks more like a face than an ellipse
	CvPoint Xcn = cvPoint((eye1.x+eye2.x)/2,(int)((double)(eye1.y+eye2.y) / 2.0 + eyeD * 0.53));
	double t, phi = 0.0, a = eyeD * 1.07, b = a * 1.61;
	for(int i=0;i<numPathPts;i++) {
		t = (2.0 * CV_PI * i / (double)numPathPts) - CV_PI;
		points[i] = cvPoint2D32f(
			Xcn.x + a * cos(t) * cos(phi) - b * sin(t) * sin(phi),
			Xcn.y + a * cos(t) * sin(phi) + b * sin(t) * cos(phi)
			);
	}
}

double distance(CvPoint a, CvPoint b) {
	double eyeDX = a.x-b.x;
	double eyeDY = a.y-b.y;
	return sqrt(eyeDX*eyeDX + eyeDY*eyeDY);
}

/**
File structure:
<img for paste>
<img for poisson>
<mask for paste>
<mask for poisson>
<eye_dist>
<left_eye_x> <left_eye_y>
<right_eye_x> <right_eye_y>
<hue_measure.x> <hue_measure.y> <hue_measure.width> <hue_measure.height>
<grayscale = 1 [, color = 0]>
*/
void readCloneInfoFile(char* filename, CloneInfo& info) {
	std::ifstream i(filename);
	if(!i.is_open()) {
		fprintf(stderr,"Cannot open clone data file: %s", filename);
		return;
	}
	char str[256] = {0};
	i >> str;
	info.back = cvLoadImage(str);
	i >> str;
	info.back_for_poisson = cvLoadImage(str);
	i >> str;
	info.back_mask = cvLoadImage(str,0);
	i >> str;
	info.back_mask_for_poisson = cvLoadImage(str,0);
	i >> info.eye_distance >> info.left_eye.x >> info.left_eye.y;
	if(!i.eof()) {
		i >> info.right_eye.x >> info.right_eye.y;
	}
	if(!i.eof()) {
		i >> info.hue_measure.x >> info.hue_measure.y >> info.hue_measure.width >> info.hue_measure.height;
	}
	info.grayscale = 0;
	if(!i.eof()) {
		i >> info.grayscale;
	}
	i.close();
}

void rotate_around_point(IplImage* im, CvPoint pt, double ang, CvPoint* pt1, CvPoint* pt2) {
	int WH = im->width*im->height;
	CvMat* im_map = cvCreateMat(WH + 2,1,CV_32FC2);
	for(int y=0;y<im->height;y++) {
		int yW = y*im->width;
		for(int x=0;x<im->width;x++) {
			int ptr = (yW + x) * 2;
			im_map->data.fl[ptr] = (float)x;
			im_map->data.fl[ptr + 1] = (float)y;
		}
	}
	if(pt1) {
		im_map->data.fl[WH * 2] = (float)(pt1->x);
		im_map->data.fl[WH * 2 + 1] = (float)(pt1->y);
	}
	if(pt2) {
		im_map->data.fl[(WH + 1) * 2] = (float)(pt2->x);
		im_map->data.fl[(WH + 1) * 2 + 1] = (float)(pt2->y);
	}

	CvMat* transmat = cvCreateMatHeader(3,3,CV_32FC1);
	float theta = (float)ang;
	float cosT = cos(theta),sinT = sin(theta);
	float mat[] = {	cosT,	sinT,	0.0f,
					-sinT,	cosT,	0.0f,
					0.0f,	0.0f,	1.0f };
	cvSetData(transmat,mat,sizeof(float)*3);

	//Rotate around point: move to point, rotate, move back
	cvAddS(im_map,cvScalar(-pt.x,-pt.y),im_map);
	cvPerspectiveTransform(im_map,im_map,transmat);
	cvAddS(im_map,cvScalar(pt.x,pt.y),im_map);

	IplImage* transformed = cvCreateImage(cvGetSize(im),im->depth,im->nChannels);
	cvZero(transformed);
	for(int y=1;y<im->height-1;y++) {
		int yW = y*im->width;
		for(int x=1;x<im->width-1;x++) {
			int ptr = (yW + x) * 2;
			int ptr1 = (yW + im->width + x) * 2;
			int ptrm1 = (yW - im->width + x) * 2;

			int xT = cvFloor(im_map->data.fl[ptr]);
			int xT1 = cvFloor(im_map->data.fl[ptr + 2]);
			int xTm1 = cvFloor(im_map->data.fl[ptr - 2]);

			int yT = cvFloor(im_map->data.fl[ptr + 1]);
			int yT1 = cvFloor(im_map->data.fl[ptr1 + 1]);
			int yTm1 = cvFloor(im_map->data.fl[ptrm1 + 1]);

			if(xT > 0 && xT < im->width - 1 && yT > 0 && yT < im->height - 1) {
				CvScalar p1 = cvGet2D(im,yTm1,xTm1);
				CvScalar p2 = cvGet2D(im,yTm1,xT);
				CvScalar p3 = cvGet2D(im,yTm1,xT1);
				CvScalar p4 = cvGet2D(im,yT,xTm1);
				CvScalar p0 = cvGet2D(im,yT,xT);
				CvScalar p5 = cvGet2D(im,yT,xT1);
				CvScalar p6 = cvGet2D(im,yT1,xTm1);
				CvScalar p7 = cvGet2D(im,yT1,xT);
				CvScalar p8 = cvGet2D(im,yT1,xT1);
				CvScalar res = cvScalar(p0.val[0] * .111 + p1.val[0] * .111 + p2.val[0] * .111 + p3.val[0] * .111 + p4.val[0] * .111 + p5.val[0] * .111 + p6.val[0] * .111 + p7.val[0] * .111 + p8.val[0] * .111,
							p0.val[1] * .111 + p1.val[1] * .111 + p2.val[1] * .111 + p3.val[1] * .111 + p4.val[1] * .111 + p5.val[1] * .111 + p6.val[1] * .111 + p7.val[1] * .111 + p8.val[1] * .111,
							p0.val[2] * .111 + p1.val[2] * .111 + p2.val[2] * .111 + p3.val[2] * .111 + p4.val[2] * .111 + p5.val[2] * .111 + p6.val[2] * .111 + p7.val[2] * .111 + p8.val[2] * .111);
				cvSet2D(transformed,y,x,res);
			} else {
				cvSet2D(transformed,y,x,cvScalarAll(0));
			}
		}
	}
	cvCopy(transformed,im);

	cvReleaseImage(&transformed);

	if(pt1) {
		pt1->x = (int)(im_map->data.fl[WH * 2]);
		pt1->y = (int)(im_map->data.fl[WH * 2 + 1]);
	}
	if(pt2) {
		pt2->x = (int)(im_map->data.fl[(WH + 1) * 2]);
		pt2->y = (int)(im_map->data.fl[(WH + 1) * 2 + 1]);
	}
	cvReleaseMat(&im_map);
}

void refine_pupil_point(IplImage* im, CvPoint& pupil, int window_size) {
	int h_win_size = window_size/2;
	cvSetImageROI(im,cvRect(pupil.x - h_win_size,pupil.y - h_win_size, window_size, window_size));
	IplImage* bw = cvCreateImage(cvSize(window_size,window_size),8,1);
	cvCvtColor(im,bw,CV_RGB2GRAY);

	IplImage* tmp = cvCreateImage(cvSize(window_size,window_size),8,3);
	

	cvEqualizeHist(bw,bw);
	cvThreshold(bw,bw,30.0,255.0,CV_THRESH_BINARY_INV);
	cvCvtColor(bw,tmp,CV_GRAY2RGB);

	//CvMemStorage* memst = cvCreateMemStorage();
	//CvContour* fc = 0;
	//cvFindContours(bw,memst,(CvSeq**)&fc);

	//cvDrawRect(tmp,cvPoint(fc->rect.x,fc->rect.y),cvPoint(fc->rect.x+fc->rect.width,fc->rect.y+fc->rect.height),CV_RGB(255,0,0));
	//while(fc->h_next != NULL) {
	//	fc = (CvContour*)(fc->h_next);
	//	cvDrawRect(tmp,cvPoint(fc->rect.x,fc->rect.y),cvPoint(fc->rect.x+fc->rect.width,fc->rect.y+fc->rect.height),CV_RGB(255,0,0));
	//}
	
	CvScalar avgPt = cvScalarAll(0);
	for(int y=0;y<window_size;y++) {
		for(int x=0;x<window_size;x++) {
			if(cvGet2D(bw,y,x).val[0] > 0) {
				avgPt.val[0] += x;
				avgPt.val[1] += y;
				avgPt.val[2] += 1.0;
			}
		}
	}

	CvPoint pt = cvPoint((int)(avgPt.val[0]/avgPt.val[2]),(int)(avgPt.val[1]/avgPt.val[2]));

#ifdef BTM_DEBUG
	cvCircle(tmp,pt,2,CV_RGB(255,0,0));
	cvNamedWindow("refine_pupil_point");
	cvShowImage("refine_pupil_point",tmp);
	cvWaitKey(BTM_WAIT_TIME);
	cvDestroyWindow("refine_pupil_point");
#endif
	cvResetImageROI(im);
	cvReleaseImage(&tmp);
	cvReleaseImage(&bw);
	//cvReleaseMemStorage(&memst);

	pupil.x = pupil.x - h_win_size + pt.x;
	pupil.y = pupil.y - h_win_size + pt.y + h_win_size/2;
}

void read_path_from_file(char* filename, FacePath& fp) {
	std::ifstream is(filename);

	if(!is.is_open()) {
		fprintf(stderr,"ERROR: can't read face path file \"%s\"\n",filename);
		return;
	}

	is >> fp.num >> fp.left_eye.x >> fp.left_eye.y >> fp.right_eye.x >> fp.right_eye.y;
	fp.pts = new CvPoint2D32f[fp.num];
	std::vector<CvPoint2D32f> pts(fp.num);
	for(int i=0;i<fp.num;i++) {
		int x,y;
		is >> x >> y;
		pts[i] = cvPoint2D32f(x,y);
	}
	is.close();
	std::vector<CvPoint2D32f> ordered(fp.num);
	ordered[0] = pts[0];
	pts[0] = cvPoint2D32f(-1.0,-1.0);
	for(int j=1;j<fp.num;j++) {
		double min = DBL_MAX;
		int idx = -1;
		for(int i=0;i<fp.num;i++) {
			double d = distance(cvPointFrom32f(pts[i]),cvPointFrom32f(ordered[j-1]));
			if(d < min) {
				min = d;
				idx = i;
			}
		}
		ordered[j] = pts[idx];
		pts[idx] = cvPoint2D32f(-1.0,-1.0);
	}
	for(int i=0;i<fp.num;fp.pts[i] = ordered[i],i++);
}

void drawArrow(CvPoint p, CvPoint q, IplImage* frame1) {
	int px_minus_qx = p.x - q.x, py_minus_qy = p.y - q.y;

	double angle;		angle = atan2( (double) py_minus_qy, (double) px_minus_qx );
	double hypotenuse;	hypotenuse = sqrt((double)( px_minus_qx*px_minus_qx + py_minus_qy*py_minus_qy ));

	/* Here we lengthen the arrow by a factor of three. */
	q.x = (int) (p.x - 3 * hypotenuse * cos(angle));
	q.y = (int) (p.y - 3 * hypotenuse * sin(angle));


	/* Now we draw the main line of the arrow. */
	/* "frame1" is the frame to draw on.
	 * "p" is the point where the line begins.
	 * "q" is the point where the line stops.
	 * "CV_AA" means antialiased drawing.
	 * "0" means no fractional bits in the center cooridinate or radius.
	 */

	cvLine( frame1, p, q, CV_RGB(255,0,0), 1, CV_AA, 0 );

	/* Now draw the tips of the arrow.  I do some scaling so that the
	 * tips look proportional to the main line of the arrow.
	 */			
	p.x = (int) (q.x + 9.0 * cos(angle + M_PI / 4.0));
	p.y = (int) (q.y + 9.0 * sin(angle + M_PI / 4.0));
	cvLine( frame1, p, q, CV_RGB(255,0,0), 1, CV_AA, 0 );
	p.x = (int) (q.x + 9.0 * cos(angle - M_PI / 4.0));
	p.y = (int) (q.y + 9.0 * sin(angle - M_PI / 4.0));
	cvLine( frame1, p, q, CV_RGB(255,0,0), 1, CV_AA, 0 );
}

void pyramid_histogram_backprojection_RGB(IplImage* orig,IplImage* image,IplImage* mask,IplImage* mask_inner,CvScalar midPt,bool recalc_face_hist) {
	//Calculate histogram of face area
	IplImage* red = cvCreateImage(cvGetSize(orig),8,1);
	IplImage* green = cvCreateImage(cvGetSize(orig),8,1);
	IplImage* blue = cvCreateImage(cvGetSize(orig),8,1);
	IplImage* planes[] = {red,green,blue};
	cvSmooth(orig,image);

	cvCvtColor(image,image,CV_BGR2HSV);

	cvSplit(image,red,green,blue,NULL);
	int hist_size[] = {32,32,32};
	float range_0[]={0.0f,256.0f};
	float range_1[]={0.0f,181.0f};
	float* ranges[] = { range_1,range_0,range_0 };

	CvHistogram* face_hst, *window_hst;
	face_hst = cvCreateHist(3, hist_size, CV_HIST_ARRAY, ranges, 1);
	window_hst = cvCreateHist(3, hist_size, CV_HIST_ARRAY, ranges, 1);


	IplImage* hist_scan_mask = cloneIm(mask);
	cvSet(hist_scan_mask,CV_RGB(255,255,255));	//initialize histogram scan mask to be all white - scan all image at first

	float d_scl_factor = 8.0f; //initial down-scale factor

	cvClearHist(face_hst);
	cvCalcHist(planes,face_hst,0,mask);
	cvNormalizeHist(face_hst,1.0);
	//cvThreshHist(face_hst,0.5);

#ifdef BTM_DEBUG
	cvNamedWindow("tmp");
	cvShowImage("tmp",mask);
	IplImage* tmpRed = cloneIm(red);
	IplImage* tmpImage = cloneIm(image);
	cvCopy(red,tmpRed);
	cvAddWeighted(tmpRed,0.75,mask,0.25,0.0,tmpRed);
	cvMerge(tmpRed,green,blue,NULL,tmpImage);
	cvShowImage("result",tmpImage);
	cvWaitKey(BTM_WAIT_TIME);
	cvReleaseImage(&tmpRed);
	cvReleaseImage(&tmpImage);
#endif

	for(int i=0;i<4;i++)
	{
	//Calculate response (histogram correlation) of whole picture to face area histogram
	CvSize win_size = cvSize(10,10);
	int win_size_w_2 = cvFloor(win_size.width / 2.0);
	int win_size_h_2 = cvFloor(win_size.height / 2.0);

	CvMat* scores_mat = cvCreateMat(cvFloor(image->height / d_scl_factor),cvFloor(image->width / d_scl_factor),CV_64FC1);
	cvZero(scores_mat);
	IplImage* dscld_red = cvCreateImage(cvGetSize(scores_mat),8,1); cvResize(red,dscld_red);
	IplImage* dscld_green = cvCreateImage(cvGetSize(scores_mat),8,1); cvResize(green,dscld_green);
	IplImage* dscld_blue = cvCreateImage(cvGetSize(scores_mat),8,1); cvResize(blue,dscld_blue);
	IplImage* win_red = cvCreateImage(cvSize(win_size.width,win_size.height),8,1);
	IplImage* win_green = cvCreateImage(cvSize(win_size.width,win_size.height),8,1);
	IplImage* win_blue = cvCreateImage(cvSize(win_size.width,win_size.height),8,1);
	IplImage* win_planes[] = {win_red,win_green,win_blue};
	IplImage* dscld_scan_mask = cloneIm(dscld_red); cvResize(hist_scan_mask,dscld_scan_mask);

#ifdef BTM_DEBUG
	printf("Scanning (%d): ", (dscld_red->height - win_size.height)*(dscld_red->width - win_size.width));
#endif
	for(int y = win_size_h_2; y < dscld_red->height - win_size_h_2; y++) {
		for(int x = win_size_w_2; x < dscld_red->width - win_size_w_2; x++) {
			double mask_val = cvGet2D(dscld_scan_mask,y,x).val[0];
			if(mask_val == 0) { 
				continue;	//scan only according to scan mask
			}

			CvRect win_roi = cvRect(x - win_size_w_2,
									y - win_size_h_2,
									win_size.width,
									win_size.height);

			cvSetImageROI(dscld_red,win_roi);	cvCopy(dscld_red,win_red);
			cvSetImageROI(dscld_green,win_roi); cvCopy(dscld_green,win_green);
			cvSetImageROI(dscld_blue,win_roi);	cvCopy(dscld_blue,win_blue);
			//cvResetImageROI(dscld_red); cvResetImageROI(dscld_green); cvResetImageROI(dscld_blue);

			cvClearHist(window_hst);
			cvCalcHist(win_planes,window_hst,0,0);
			cvNormalizeHist(window_hst,1.0);

			//double score = cvCompareHist(window_hst,face_hst,CV_COMP_INTERSECT);
			double score = cvCompareHist(window_hst,face_hst,CV_COMP_CORREL);
			cvSet2D(scores_mat,y,x,cvScalar(score));
		}
#ifdef BTM_DEBUG
		printf(".");
#endif
	}
#ifdef BTM_DEBUG
	printf("\n");
#endif
	cvReleaseImage(&win_red);
	cvReleaseImage(&win_green);
	cvReleaseImage(&win_blue);
	cvReleaseImage(&dscld_red);
	cvReleaseImage(&dscld_green);
	cvReleaseImage(&dscld_blue);
	cvReleaseImage(&dscld_scan_mask);

	cvNormalize(scores_mat,scores_mat,1.0,0.0,CV_MINMAX);

	IplImage* scores_gray = cvCreateImage(cvGetSize(scores_mat),8,1);
	cvConvertScale(scores_mat,scores_gray,255.0);
	cvReleaseMat(&scores_mat);

#ifdef BTM_DEBUG
	cvShowImage("result",scores_gray);
	cvWaitKey(BTM_WAIT_TIME);
#endif
	//Threshold to get the mask of the highest response to the histogram matching
	cvThreshold(scores_gray,scores_gray,50.0,255.0,CV_THRESH_BINARY);
#ifdef BTM_DEBUG
	cvShowImage("tmp",scores_gray);
	cvWaitKey(BTM_WAIT_TIME);
#endif


	IplImage* scaled_up_scores = cvCreateImage(cvGetSize(mask),8,1);
	cvResize(scores_gray,scaled_up_scores);
	cvReleaseImage(&scores_gray);

	//cvOr(scaled_up_scores,mask,mask);
	cvCopy(scaled_up_scores,mask);

	cvReleaseImage(&scaled_up_scores);

	cvOr(mask,mask_inner,mask);

	get_center_connected_component(mask,image,midPt);

	if(recalc_face_hist) {
		//recalculate face histogram
		cvClearHist(face_hst);
		cvCalcHist(planes,face_hst,0,mask);
		cvNormalizeHist(face_hst,1.0);
	}

	//IplImage* dilated = cloneIm(mask);
	//IplImage* eroded = cloneIm(mask);
	//cvDilate(mask,dilated,0,10);
	//cvErode(mask,eroded,0,30);
	//cvSub(dilated,eroded,hist_scan_mask);

	cvDilate(mask,hist_scan_mask,0,20);

	//cvReleaseImage(&dilated);
	//cvReleaseImage(&eroded);

	d_scl_factor /= 1.5f;	//scale up by 2 for next iteration

#ifdef BTM_DEBUG
	cvShowImage("tmp",hist_scan_mask);
	cvShowImage("result",mask);
	cvWaitKey(BTM_WAIT_TIME);
#endif
	}/***************end levels for***************/

	cvReleaseImage(&red);
	cvReleaseImage(&green);
	cvReleaseImage(&blue);
	cvReleaseImage(&hist_scan_mask);
	cvReleaseHist(&face_hst);
	cvReleaseHist(&window_hst);
}