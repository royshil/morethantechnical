// VirtualSurgeon_NeckFinder.cpp : Defines the entry point for the console application.
//

#include "cv.h"
#include "highgui.h"
//#include "cxflann.h"

using namespace cv;
using namespace cv::flann;

#include "../VirtualSurgeon_Utils/VirtualSurgeon_Utils.h"

#include <string>
#include <vector>
#include <limits>
#include <iostream>
using namespace std;

Mat findBestTransform1(vector<Point>& somePoints, vector<Point>& maximalPoints) {
	RNG& rng = theRNG();
	Mat _tmpDstPts(somePoints.size(),1,CV_32FC2);
	Mat _tmpSrcPts; Mat(somePoints).convertTo(_tmpSrcPts,CV_32FC2);
	vector<int> chosen(somePoints.size());
	vector<Point3f> homPts;
	convertPointsHomogeneous(_tmpSrcPts,homPts);
	Mat homPtsM(homPts);
	Mat B = repeat(Mat(maximalPoints),1,somePoints.size());
	B.convertTo(B,CV_32F);

	Mat bestTrans;
	double bestTransDist = std::numeric_limits<double>::max();

	for(int i=0;i<10;i++) {	
		rng.fill(Mat(chosen),RNG::UNIFORM,Scalar(0),Scalar(maximalPoints.size()));
		for(int j=0;j<somePoints.size();j++) {
			Point p = maximalPoints[chosen[j]];
			Point2f& p2f = _tmpDstPts.at<Point2f>(j,0);
			p2f.x = p.x; p2f.y = p.y;
		}

		//Mat T = findHomography(_tmpSrcPts,_tmpDstPts,CV_RANSAC);
		//float _Tf[9]; Mat _T(3,3,CV_32FC1,_Tf);
		Mat T = estimateRigidTransform(_tmpSrcPts,_tmpDstPts,false);
		Mat _T; T.convertTo(_T,CV_32FC1);

		//estimate
		Mat pT = _tmpSrcPts.reshape(1) * _T(Range(0,2),Range(0,2));
		//vector<Point2f> transPts(somePoints.size());
		//for(int j=0;j<somePoints.size();j++) {
		//	Point p = somePoints[j];
		//	float x = p.x, y = p.y;
		//	float z = 1./(_Tf[6]*x + _Tf[7]*y + _Tf[8]);
		//	transPts[j].x = (_Tf[0]*x + _Tf[1]*y + _Tf[2])*z;
		//	transPts[j].y = (_Tf[3]*x + _Tf[4]*y + _Tf[5])*z;
		//}
		//convertPointsHomogeneous(pT,transPts);
		//{
		//	Mat _tmp; pT(Range(0,pT.rows),Range(0,2)).copyTo(_tmp);
		//	_tmp.reshape(2).copyTo(Mat(transPts));
		//}

		//Mat A = repeat(Mat(Mat(transPts).t()),maximalPoints.size(),1);

		//Mat D = A - B;
		//D = D.mul(D);
		//Scalar ds = sum(D);
		//double dd = ds[0]+ds[1];

		//if(dd < bestTransDist) {
		//	bestTrans = T;
		//	bestTransDist = dd;
		//}
		cout << ".";
	}
	cout << "DONE"<<endl;
	return bestTrans;
}

Mat findBestTransform(vector<Point>& src, vector<Point>& dst, Mat& im) {
	cout << "Find best transform... ";
	bool end = false;
	RNG& rng = theRNG();
	Point2f sp[3],dp[3];

	//{
		Scalar halfSize(im.cols/2,im.rows/2);
		Mat _srcM(src); _srcM = _srcM - halfSize;
		Mat _dstM(dst); _dstM = _dstM - halfSize;
	//}

	Mat dataset; 
	{
		//vector<Point2f> _dst(dst.size());
		Mat _dst;
		Mat(dst).convertTo(_dst,CV_32FC2);
		_dst.reshape(1,_dst.rows).copyTo(dataset);
	}
	
	cv::flann::Index flann_index(dataset, cv::flann::KDTreeIndexParams());  // using 4 randomized kdtrees
	
	Mat indices(src.size(),1,CV_32SC1);
	Mat dists(src.size(),1,CV_32FC1);
	Mat srcM;
	{
		//vector<Point2f> _p(src.size());
		Mat _p;
		Mat(src).convertTo(_p,CV_32FC2);
		_p.reshape(1,_p.rows).copyTo(srcM);
		//srcM.convertTo(srcM,CV_64FC1);
	}
	
	Mat bestTransform;
	float bestSSD = std::numeric_limits<float>::max();

	Mat srcMT,srcMT2D;

	int count = 0;
	while(bestSSD > 180000.0f && !end) {//TODO: should not be a numeric limit, some cases will not be able to achieve the goal
		//randomize 3 points from each
		for(int i=0;i<4;i++) {
			int randidx = rng.uniform(0,src.size());
			sp[i] = src[randidx];
		}

		
		Mat queryI(3,2,CV_32SC1,sp);
		Mat queryF; queryI.convertTo(queryF,CV_32FC1);
		Mat _indices(3,1,CV_32SC1),_dists(3,1,CV_32FC1);
		//flann_index.radiusSearch(queryF,_indices,_dists,50.0f,SearchParams(64));
		//flann_index.knnSearch(queryF,_indices,_dists,1,SearchParams(64));
		//for(int i=0;i<3;i++) {
		//	int found_index = _indices.ptr<int>(i)[0];
		//	dp[i] = dst[found_index];
		//}
		for(int i=0;i<4;i++) {
			//double bestD = std::numeric_limits<double>::max(); int bestIdx = -1;
			//for (int j=0;j<dst.size();j++) {
			//	double _dx = sp[i].x - dst[j].x;
			//	double _dy = sp[i].y - dst[j].y;
			//	double _d = (_dx*_dx + _dy*_dy);
			//	if(_d < bestD) {
			//	}
			//}
			Mat _tmp = Mat(dst) - Scalar(sp[i].x,sp[i].y);	//x,y distances
			_tmp = _tmp.mul(_tmp);							//squared
			_tmp = _tmp.reshape(1,_tmp.rows);				//summed
			_tmp = _tmp(Range(0,_tmp.rows),Range(0,1)) + _tmp(Range(0,_tmp.rows),Range(1,2));
			double minV; Point minL;
			minMaxLoc(_tmp,&minV,0,&minL);					//minimal distance
			dp[i] = dst[minL.y];
		}

		//get transform
		//Mat T = getAffineTransform(sp,dp);
		Mat T = getPerspectiveTransform(sp,dp);
		T.convertTo(T,CV_32FC1);

		//apply transform to src points
		//vector<Point3f> tp3d(srcM.rows);
		srcMT = srcM * T;
		//srcMT.reshape(3,srcMT.rows).copyTo(Mat(tp3d));

		//{
		//	_srcM = _srcM + halfSize;
		//	_dstM = _dstM + halfSize;
		//	Mat im_small; im.copyTo(im_small);

		//	for(int i=0;i<dst.size();i++) {
		//		circle(im_small,dst[i],2,Scalar(255,0,0),CV_FILLED);
		//	}

		//	for(int i=0;i<src.size();i++) {
		//		circle(im_small,src[i],2,Scalar(0,0,255),CV_FILLED);
		//	}

		//	for(int i=0;i<3;i++) {
		//		Point correctsp_i(sp[i].x+halfSize[0],sp[i].y+halfSize[1]);
		//		Point correctdp_i(dp[i].x+halfSize[0],dp[i].y+halfSize[1]);
		//		circle(im_small,correctsp_i,5,Scalar(0,255,255),CV_FILLED);
		//		circle(im_small,correctdp_i,5,Scalar(255,0,255),CV_FILLED);
		//		if(i>0) {
		//			Point correctsp_i_1(sp[i-1].x+halfSize[0],sp[i-1].y+halfSize[1]);
		//			Point correctdp_i_1(dp[i-1].x+halfSize[0],dp[i-1].y+halfSize[1]);
		//			line(im_small,correctsp_i_1,correctsp_i,Scalar(0,255,255),3);
		//			line(im_small,correctdp_i_1,correctdp_i,Scalar(255,0,255),3);
		//		}
		//	}

		//	vector<Point2f> transPoints(src.size());
		//	Mat tPt(transPoints);
		//	//tPt = tPt.reshape(2,tPt.rows);
		//	
		//	//{
		//		//Mat srcM;
		//		//vector<Point2f> _p(somePoints.size());
		//		//Mat(somePoints).convertTo(Mat(_p),CV_32FC2);
		//		//Mat(_p).reshape(1,_p.size()).copyTo(srcM);
		//		//Mat sT = srcM*T;
		//		srcMT(Range(0,srcMT.rows),Range(0,2)).copyTo(tPt.reshape(1,tPt.rows));

		//		tPt = tPt + halfSize;

		//		//convertPointsHomogeneous(srcMT,transPoints);
		//	//}

		//	for(int i=0;i<transPoints.size();i++) {
		//		circle(im_small,transPoints[i],2,Scalar(0,255,0),CV_FILLED);
		//	}

		//	imshow("tmp",im_small);
		//	waitKey();

		//	_srcM = _srcM - halfSize;
		//	_dstM = _dstM - halfSize;
		//}

		//calculate error
		//get closest points
		//{
			srcMT(Range(0,srcMT.rows),Range(0,2)).copyTo(srcMT2D);
			//flann_index.knnSearch(srcMT2D,indices,dists,/*knn=*/1,SearchParams());
		//}

		for(int i=0;i<srcMT2D.rows;i++) {
			Point2f p = srcMT2D.at<Point2f>(i,0);
			Mat _tmp = Mat(dst) - Scalar(p.x,p.y);	
			_tmp = _tmp.mul(_tmp);							//squared
			_tmp = _tmp.reshape(1,_tmp.rows);				//summed
			_tmp = _tmp(Range(0,_tmp.rows),Range(0,1)) + _tmp(Range(0,_tmp.rows),Range(1,2));
			double minV; Point minL;
			minMaxLoc(_tmp,&minV,0,&minL);					//minimal distance
			dp[i] = dst[minL.y];
		}

		//ssd for measure
		{
			Mat dsq = dists.t() * dists;
			float ssd = dsq.ptr<float>(0)[0];

			if(ssd < bestSSD) {
				bestTransform = T;
				bestSSD = ssd;
			}
		}
		if(count%100 == 0)
			cout << ".";
		end = (++count >= 1000);
	}

	{
		srcMT = srcM * bestTransform;
		Mat _tmp = _srcM.reshape(1,_srcM.rows);
		srcMT(Range(0,srcMT.rows),Range(0,2)).convertTo(_tmp,CV_32SC1);
	}

	_srcM = _srcM + halfSize;
	_dstM = _dstM + halfSize;

	cout << "DONE";
	return bestTransform;
}

int _main(int argc, char** argv)
{
	VirtualSurgeon::VIRTUAL_SURGEON_PARAMS p;
	p.ParseParams(argc,argv);

	Mat im;
	p.FaceDotComDetection(im);

	Mat im_small;
	resize(im,im_small,Size((int)floor((double)im.cols/p.im_scale_by),
							(int)floor((double)im.rows/p.im_scale_by)));

	namedWindow("tmp");
	imshow("tmp",im_small);

	Mat gray; cvtColor(im_small,gray,CV_BGR2GRAY); GaussianBlur(gray,gray,Size(11,11),7.0);
	Mat edges; 
	//Keep strong edges
	{
		Mat _edges;
		Sobel(gray,_edges,-1,1,1);
		_edges.copyTo(edges,_edges > 0.75);
		namedWindow("tmp1");
		imshow("tmp1",edges);
		waitKey(p.wait_time);
	}

	vector<Point> maximalPoints;

	//Local maxima supression
	{
		Mat edges_d;
		dilate(edges,edges_d,Mat::ones(15,15,CV_8UC1));
		Mat points = (edges_d == edges & edges_d > 0);
		{
		Mat _im; im_small.copyTo(_im);
		_im.setTo(Scalar(255),points);
		imshow("tmp",_im);
		}
		waitKey(p.wait_time);

		for(int i=0;i<points.rows*points.cols;i++) {
			if(points.data[i] > 0) {
				int x = i % points.cols;
				int y = (int)floor((double)i / (double)points.cols);
				maximalPoints.push_back(Point(x,y));
			}
		}
	}

	vector<Point> somePoints(50);
	RNG& rng = theRNG();
	rng.fill(Mat(somePoints),RNG::NORMAL,Scalar(im_small.cols / 2,im_small.rows / 2),Scalar(30,30));
	//randn(Mat(somePoints),Scalar(im_small.cols / 2,im_small.rows / 2),Scalar(30,30));

	{
		Mat _tmp; im_small.copyTo(_tmp);
		for(int i=0;i<maximalPoints.size();i++) {
			circle(_tmp,maximalPoints[i],2,Scalar(255,0,0),CV_FILLED);
		}

		for(int i=0;i<somePoints.size();i++) {
			circle(_tmp,somePoints[i],2,Scalar(0,0,255),CV_FILLED);
		}
		imshow("tmp",_tmp);
		waitKey();
	}

	//Alek RANSAC
	//Mat T = findBestTransform(somePoints,maximalPoints,im_small);
	Mat T = findBestTransform1(somePoints,maximalPoints);


	for(int i=0;i<maximalPoints.size();i++) {
		circle(im_small,maximalPoints[i],2,Scalar(255,0,0),CV_FILLED);
	}

	for(int i=0;i<somePoints.size();i++) {
		circle(im_small,somePoints[i],2,Scalar(0,0,255),CV_FILLED);
	}

	vector<Point2f> transPoints(50);
	Mat tPt(transPoints);
	//tPt = tPt.reshape(2,tPt.rows);
	
	{
		Mat srcM;
		vector<Point2f> _p(somePoints.size());
		Mat(somePoints).convertTo(Mat(_p),CV_32FC2);
		Mat(_p).reshape(1,_p.size()).copyTo(srcM);
		Mat sT = srcM*T;
		sT(Range(0,sT.rows),Range(0,2)).copyTo(tPt.reshape(1,tPt.rows));
	}

	for(int i=0;i<transPoints.size();i++) {
		circle(im_small,transPoints[i],2,Scalar(0,255,0),CV_FILLED);
	}

	imshow("tmp",im_small);
	waitKey();

	return 0;
}

