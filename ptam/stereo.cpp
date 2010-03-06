/**
 * License...
 *
 *	Stereo-related functions
 *
 *
 */
#include "stdafx.h"
#include "utils.h"

Mat camera_matrix,distortion_coefficients;

/**
 * triangulate 3D features from 2 sets of corresponding points from 2 views.
 * assume only 5cm translation on -x axis between views
 */
void triangulate(vector<Point2d>& points1, vector<Point2d>& points2, vector<uchar>& status) {

	//Convert points to 1-channel, 2-rows, double precision
	Mat pts1M(points1), pts2M(points2);
	Mat pts1Mt(pts1M.t());
	Mat pts2Mt(pts2M.t());
	
	double __d[1000] = {0};
	Mat ___tmp(2,pts1Mt.cols,CV_64FC1,__d);
	double * _p = pts1Mt.ptr<double>();
	for(int i=0;i<pts1Mt.cols;i++) {
		__d[i] = _p[i*2];
		__d[i + pts1Mt.cols] = _p[i*2+1];
	}

	double __d1[1000] = {0};
	Mat ___tmp1(2,pts2Mt.cols,CV_64FC1,__d1);
	double * _p1 = pts2Mt.ptr<double>();
	for(int i=0;i<pts2Mt.cols;i++) {
		__d1[i] = _p1[i*2];
		__d1[i + pts2Mt.cols] = _p1[i*2+1];
	}

	//CvMat __points1 = pts1Mt.reshape(1,2), __points2 = pts2Mt.reshape(1,2);
	CvMat __points1 = ___tmp, __points2 = ___tmp1;

	//projection matrices
	double P1d[12] = {	-1,0,0,0,
						0,1,0,0,
						0,0,1,0 };	//Identity, but looking into -z axis
	Mat P1m(3,4,CV_64FC1,P1d);
	CvMat* P1 = &(CvMat)P1m;
	double P2d[12] = {	-1,0,0,-5,
						0,1,0,0,
						0,0,1,0 };  //Identity rotation, 5cm -x translation, looking into -z axis
	Mat P2m(3,4,CV_64FC1,P2d);
	CvMat* P2 = &(CvMat)P2m;

	float _d[1000] = {0.0f};
	Mat outTM(4,points1.size(),CV_32FC1,_d);
	CvMat* out = &(CvMat)outTM;

	cvTriangulatePoints(P1,P2,&__points1,&__points2,out);

	vector<Point2d> projPoints[2] = {points1,points2};

	double point2D_dat[3] = {0};
	double point3D_dat[4] = {0};
	Mat twoD(3,1,CV_64FC1,point2D_dat); 
	Mat threeD(4,1,CV_64FC1,point3D_dat); 

	Mat P[2] = {Mat(P1),Mat(P2)};

	int oc = out->cols, oc2 = out->cols*2, oc3 = out->cols*3;

	status = vector<uchar>(oc);

	//scan all points, reproject 3D->2D, and keep only good ones
	for(int i=0;i<oc;i++) {	
		double W = out->data.fl[i+oc3];
        point3D_dat[0] = out->data.fl[i] / W;
        point3D_dat[1] = out->data.fl[i+oc] / W;
        point3D_dat[2] = out->data.fl[i+oc2] / W;
        point3D_dat[3] = 1;

		/*printf("%.3f %.3f %.3f (%.3f)\n",
				point3D_dat[0],
				point3D_dat[1],
				point3D_dat[2],
				W
				);*/
        
        bool push = true;
        /* !!! Project this point for each camera */
        for( int currCamera = 0; currCamera < 2; currCamera++ )
        {
            //cvmMul(projMatrs[currCamera], &point3D, &point2D);
			twoD = P[currCamera] * threeD;
            
            float x,y;
            float xr,yr,wr;
            //x = (float)cvmGet(projPoints[currCamera],0,i);
            //y = (float)cvmGet(projPoints[currCamera],1,i);
			x = (float)projPoints[currCamera][i].x;
			y = (float)projPoints[currCamera][i].y;
            
            wr = (float)point2D_dat[2];
            xr = (float)(point2D_dat[0]/wr);
            yr = (float)(point2D_dat[1]/wr);
            
            float deltaX,deltaY;
            deltaX = (float)fabs(x-xr);
            deltaY = (float)fabs(y-yr);

			//printf("error from cam %d (%.2f,%.2f): %.6f %.6f\n",currCamera,x,y,deltaX,deltaY);
			
			if(deltaX > 0.01 || deltaY > 0.01) {
				push = false;
			}
        }
		if(push) {
			// A good 3D reconstructed point, add to known world points

			double s = 7;
			Point3d p3d(point3D_dat[0]/s,point3D_dat[1]/s,point3D_dat[2]/s);
			//printf("%.3f %.3f %.3f\n",p3d.x,p3d.y,p3d.z);
			points1Proj.push_back(p3d);
			status[i] = 1;
		} else {
			status[i] = 0;
		}

	}
}

/**
 * Initialize world from stereo views (triangulate 3D points)
 */
void stereoInit() {
	FileStorage fs("cam_work.out",CV_STORAGE_READ);
	FileNode fn = fs["camera_matrix"];
	camera_matrix = Mat((CvMat*)fn.readObj(),true);
	double* _d = camera_matrix.ptr<double>();
	//_d[2] = 352.0/2.0;
	//_d[5] = 288.0/2.0;
	fn = fs["distortion_coefficients"];
	distortion_coefficients = Mat((CvMat*)fn.readObj(),true);
	//distortion_coefficients = Mat(1,4,CV_64FC1,Scalar(0));

	Mat frame1,frame2,image;

	//read_frame_and_points("frame1.points","frame1.png",points[0],frames[0]);
	//vector<Point2d> points1Orig = points1;
	//for(unsigned int i=0;i<points1.size();points1Orig.push_back(points1[i]),i++);

	//read_frame_and_points("frame2.points","frame2.png",points[0],frames[0]);
	//vector<Point2d> points2Orig = points2;
	//for(unsigned int i=0;i<points2.size();points2Orig.push_back(points2[i]),i++);

	//vector<uchar> _tmpStatus(points1.size(),1);
	//draw_frame_points_w_status("1st frame",frame1,points1,_tmpStatus);
	//draw_frame_points_w_status("2nd frame",frame2,points2,_tmpStatus);

	//frame1.copyTo(image);
	//namedWindow("tmp",1);
	//for(unsigned int i=0;i<points1.size();i++) {
	//	circle(image,points1[i],2,CV_RGB(0,255,0),CV_FILLED);
	//}
	//imshow("tmp",image);

	//Mat image(frame1.rows,frame1.cols+frame2.cols,CV_8UC3);

	_points[0] = points[0];
	_points[1] = points[1];
	Mat pts1M(_points[0]), pts2M(_points[1]);//,pts1Morig,pts2Morig;
	//pts1M.copyTo(pts1Morig); pts2M.copyTo(pts2Morig);

	//Undistort points
	Mat tmp,tmpOut;
	pts1M.convertTo(tmp,CV_32FC2);
	undistortPoints(tmp,tmpOut,camera_matrix,distortion_coefficients);
	tmpOut.convertTo(pts1M,CV_64FC2);

	pts2M.convertTo(tmp,CV_32FC2);
	undistortPoints(tmp,tmpOut,camera_matrix,distortion_coefficients);
	tmpOut.convertTo(pts2M,CV_64FC2);

	//Mat H = findHomography(pts1M,pts2M,CV_RANSAC,10.0);
	Mat H = findHomography(pts1M,pts2M,CV_LMEDS);
	double* ptH = (double*)H.ptr(0);
	printf("Homography:\n%.3f %.3f %.3f\n%.3f %.3f %.3f\n%.3f %.3f %.3f\n",
		ptH[0],ptH[1],ptH[2],ptH[3],ptH[4],ptH[5],ptH[6],ptH[7],ptH[8],ptH[9]);
	//warpPerspective(frame1,image,H,frame1.size(),INTER_LINEAR);

	u = Vec3d(3.0*(1.0-ptH[0]),3.0*ptH[1],3.0*ptH[2]);
	double s = sqrt(u.ddot(u));
	u[0] /= s;
	u[1] /= s;
	u[2] /= s;
	printf("normal %.3f %.3f %.3f\n",u[0],u[1],u[2]);

	v = Vec3d(0.0,-1.0,0.0);

	triangulate(_points[0],_points[1],tri_status);
	
	
	int nz = countNonZero(Mat(tri_status));
	points1ProjF = vector<Point3f>(nz);
	points1projMF = Mat(points1ProjF);

	Mat _tmp(points1Proj);
	_tmp.convertTo(points1projMF,CV_32FC3);
	
	/*
	vector<Point2d> points1Orig1,points2Orig1;
	for(unsigned int j=0;j<tri_status.size();j++) {
		if(tri_status[j] == 1) {
			points1Orig1.push_back(points1Orig[j]);
			points2Orig1.push_back(points2Orig[j]);
		}
	}
	findExtrinsics(points1Orig1);
	findExtrinsics(points2Orig1);
*/

#ifndef VIDEO_MODE
	vector<double> rv,tv;
	for(int i=0;i<5;i++) {
		std::stringstream st;
		st << "frame" << (i+1) << ".png";
		std::stringstream st1;
		st1 << "frame" << (i+1) << ".points";
		read_frame_and_points(st1.str().c_str(),st.str().c_str(),_points[i],frames[i]);
		pointsOrig[i] = _points[i];

		//undistort(frames[i],frames[i],camera_matrix,distortion_coefficients);

		/*stringstream _st;
		_st << "frame" << (i+1);
		draw_frame_points_w_status(_st.str(),frames[i],pointsOrig[i],tri_status);*/

		for(unsigned int j=0;j<_points[i].size();j++) {
			if(tri_status[j] == 1) {
				points[i].push_back(_points[i][j]);
			}
		}

		findExtrinsics(points[i],rv,tv);

		cams[i] = tv;
		rots[i] = rv;
	}

	cam[0] = cams[0][0]; cam[1] = cams[0][1]; cam[2] = cams[0][2];
	Mat _r(3,3,CV_64FC1,rot); 
	Rodrigues(rots[0],_r);

	frames[0].copyTo(backPxls);
	cvtColor(backPxls,backPxls,CV_BGR2RGB);
	cvFlip(&cvMat(288,352,CV_8UC3,backPxls.data),0,0);
#endif
}

/**
 * find extrinsic parameters of this camera according to the detected points
 */
void findExtrinsics(vector<Point2d>& points, vector<double>& rv, vector<double>& tv) {
	//estimate extrinsics for these points

	Mat rvec(rv),tvec(tv);

	if(rv.size()!=3) {
		rv = vector<double>(3); 
		rvec = Mat(rv);
		double _d[9] = {1,0,0,
						0,-1,0,
						0,0,-1};
		Rodrigues(Mat(3,3,CV_64FC1,_d),rvec);
	}
	if(tv.size()!=3) {
		tv = vector<double>(3);
		tv[0]=0;tv[1]=0;tv[2]=0;
		tvec = Mat(tv);
	}

	//create a float rep  of points
	vector<Point2f> v2(points.size());
	Mat tmpOut(v2);
	Mat _tmpOut(points);
	_tmpOut.convertTo(tmpOut,CV_32FC2);

#ifdef TRIANGULATE_TOY_EXAMPLE
	//toy example for triangulation
	Mat __tmp = tmp.reshape(1,3); 
	Mat __tmpOut = tmpOut.reshape(1,2);

	vector<Point3f> v3;
	v3.push_back(Point3f(0.0f,0.0f,0.0f));
	v3.push_back(Point3f(0.0f,1.0f,0.0f));
	v3.push_back(Point3f(1.0f,0.0f,0.0f));
	v3.push_back(Point3f(0.0f,-1.0f,0.0f));
	Mat __tmp(v3);

	vector<Point2f> v2;
	v2.push_back(Point2f(202.0f,107.0f));
	v2.push_back(Point2f(202.0f,117.0f));
	v2.push_back(Point2f(212.0f,107.0f));
	v2.push_back(Point2f(202.0f,97.0f));
	Mat __tmpOut(v2);
#endif

	solvePnP(points1projMF,tmpOut,camera_matrix,distortion_coefficients,rvec,tvec,true);

	//printf("frame extrinsic:\nrvec: %.3f %.3f %.3f\ntvec: %.3f %.3f %.3f\n",rv[0],rv[1],rv[2],tv[0],tv[1],tv[2]);

	Mat rotM(3,3,CV_64FC1); ///,_r);
	Rodrigues(rvec,rotM);
	double* _r = rotM.ptr<double>();
	printf("rotation mat: \n %.3f %.3f %.3f\n%.3f %.3f %.3f\n%.3f %.3f %.3f\n",
		_r[0],_r[1],_r[2],_r[3],_r[4],_r[5],_r[6],_r[7],_r[8]);

#ifdef CHECK_ESTIMATED_POSE
	//try to reproject points to image plane to find outliers

	vector<Point2d> _tmpPts = points, _tmpUndistorted(points.size());
	Mat _tmpM(_tmpPts);
	Mat _tmp,_tmpOut;
	Mat _tmpUdistM(_tmpUndistorted);

	_tmpM.convertTo(_tmp,CV_32FC2);
	undistortPoints(_tmp,_tmpOut,camera_matrix,distortion_coefficients);
	tmpOut.convertTo(_tmpUdistM,CV_64FC2);

	for(int i=0;i<points.size();i++) {
		//reproject point to image plane

		//check distance to observed
	}
#endif
}

/**
 * failed attempt to use stereoCalibrat() to find rotation and translation
 */
void findExtrinsics(vector<Point2f>& points1,
					vector<Point2f>& points2,
					vector<double>& rv, vector<double>& tv) {
	Mat R,T,E,F;

	vector< vector<Point3f>> _vp(1,points1ProjF);
	vector< vector<Point2f>> _vpp1(1,points1);
	vector< vector<Point2f>> _vpp2(1,points2);

	stereoCalibrate(_vp,
		_vpp1,
		_vpp2,
		camera_matrix,distortion_coefficients,
		camera_matrix,distortion_coefficients,
		frames[0].size(),
		R,T,Mat(),Mat(),TermCriteria(TermCriteria::COUNT+TermCriteria::EPS,30,0.000001),
		CALIB_FIX_INTRINSIC);

	double* _r = R.ptr<double>();
	printf("rotation mat: \n %.3f %.3f %.3f\n%.3f %.3f %.3f\n%.3f %.3f %.3f\n",
		_r[0],_r[1],_r[2],_r[3],_r[4],_r[5],_r[6],_r[7],_r[8]);

	double* _t = T.ptr<double>();
	printf("translation: %.3f %.3f %.3f\n",_t[0],_t[1],_t[2]);
}

void drawReprojectedOnImage(Mat& image, vector<double>& rv, vector<double>& tv, vector<Point2d>& tracked, vector<uchar>& status) {
	vector<Point2f> imagePoints(points1Proj.size());
	projectPoints(points1projMF,Mat(rv),Mat(tv),camera_matrix,distortion_coefficients,imagePoints);

	for(unsigned int i=0;i<imagePoints.size();i++) {
		circle(image,imagePoints[i],2,
			((status[i] == 1) ? Scalar(0,255,255) : Scalar(0,0,255)),
			CV_FILLED);
		line(image,imagePoints[i],tracked[i],Scalar(255,0,0));
	}
}

/**
 * check which of the tracked points align with thier reprojected 3D feature
 * mark in the status which points are good and which aren't
 * return the SSD between tracked and reprojected
 */
double keepGood2D3DMatch(vector<Point2d>& trackedPoints, vector<double>& rv, vector<double>& tv, vector<uchar>& status) {
	int totalPoints = points1Proj.size();
	vector<Point2f> imagePoints(totalPoints);
	//vector<Point2d> tmpTrackedPoints;
	//vector<Point3d> new3DPoints;

	//reproject 3D points to image plane
	projectPoints(points1projMF,Mat(rv),Mat(tv),camera_matrix,distortion_coefficients,imagePoints);

	//compute SSD to see if the result is bad enough to drop points 
	//vector<Point2f> trackedF(trackedPoints.size());
	//Mat m(imagePoints), m1(trackedF);
	//Mat(trackedPoints).convertTo(m1,CV_32FC2);
	//Mat m2 = m - m1;
	//Point2f* _dp = (Point2f*)m2.ptr<Point2f>();
	//for(int i=0;i<m2.rows;i++) {
	//	printf("%.3f %.3f\n",_dp[i].x,_dp[i].y);
	//}
	//Mat m3 = m2 * m2.t();
	//Mat d = m3.diag();
	//_dp = (Point2f*)d.ptr<Point2f>();
	//printf("\nDiag:\n");
	//for(int i=0;i<d.rows;i++) {
	//	printf("%.3f %.3f\n",_dp[i].x,_dp[i].y);
	//}

	//vector<Mat > d_s(2);
	//split(d,d_s);
	//Mat d_x_plus_y = d_s[0] + d_s[1];
	//vector<float> tmpV(d_x_plus_y.rows);
	//d = Mat(tmpV);
	//cv::sqrt(d_x_plus_y,d);
	//Scalar totalDelta = cv::sum(d);

	//printf("keepGood2D3DMatch: total delta %.3f\n",totalDelta[0]);
	//
	//if(fabs(totalDelta[0]) < 1.0) return;	//overall change is not big enough

	double totalSum = 0;

	for(int i=0;i<totalPoints;i++) {
		double dx = imagePoints[i].x - trackedPoints[i].x;
		double dy = imagePoints[i].y - trackedPoints[i].y;
		double sqdiff = sqrt(dx*dx + dy*dy);
		totalSum += sqdiff;
		/*if(sqdiff > 3.0 && sqdiff < 10.0) {
			trackedPoints[i].x = (double)(imagePoints[i].x);
			trackedPoints[i].y = (double)(imagePoints[i].y);
		} else*/ if(sqdiff > 10.0) {
			status[i] = 0;
			//tmpTrackedPoints.push_back(trackedPoints[i]);
			//new3DPoints.push_back(points1Proj[i]);
		}
	}

	//trackedPoints = tmpTrackedPoints;
	//points1Proj = new3DPoints;

	//points1ProjF = vector<Point3f>(new3DPoints.size());
	//points1projMF = Mat(points1ProjF);

	//Mat _tmp(points1Proj);
	//_tmp.convertTo(points1projMF,CV_32FC3);

	////estimate new rot and trans vectors, according to better points matched
	//findExtrinsics(trackedPoints,rv,tv);

	return totalSum;
}

bool reprojectInivibles(vector<Point2f>& trackedPoints, vector<double> rv, vector<double> tv) {
	int totalPoints = points1Proj_invisible.size();
	if(totalPoints > 0) { // are there any "invisible" points?
		vector<Point2f> imagePoints(totalPoints);
		Mat invisMF(totalPoints,1,CV_32FC2);
		Mat(points1Proj_invisible).convertTo(invisMF,CV_32FC2);
		projectPoints(invisMF,Mat(rv),Mat(tv),camera_matrix,distortion_coefficients,imagePoints);
		
		Rect r(Point(0,0),frames[0].size());
		vector<uchar> _status(totalPoints);
		bool anyPointsToRemove = false;
		for(int i=0;i<totalPoints;i++) {
			if(imagePoints[i].inside(r)) {
				//only use points that are inside the frame...
				trackedPoints.push_back(imagePoints[i]);			//2d for tracking
				points1Proj.push_back(points1Proj_invisible[i]);	//3d for pose estim

				//no longer invisible...
				_status[i] = 1;

				anyPointsToRemove = true;
			} else {
				_status[i] = 0;
			}
		}
	
		if(anyPointsToRemove) {
			//int _nz = countNonZero(Mat(_status));
			vector<Point3d> newInvisible;
			for(int i=0;i<totalPoints;i++) {
				if(_status[i] == 0) {
					newInvisible.push_back(points1Proj_invisible[i]);
				}
			}
			points1Proj_invisible = newInvisible;
			return true;
		}
	}
	return false;
}