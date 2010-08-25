#include "cv.h"
//#include "cvaux.h"
#include "highgui.h"

using namespace cv;

#include <iostream>
#include <vector>
#include <limits>
using namespace std;

#include "../VirtualSurgeon_Utils/VirtualSurgeon_Utils.h"
#include "VirtualSurgeon_ICP.h"
namespace VirtualSurgeon {

void ICP::ShowPoints(Mat& im, Mat& X,Scalar c) {
	for(int i=0;i<X.rows;i++) {
		Point p;
		if(X.type() == CV_32SC1 || X.type() == CV_32SC2)
			p = X.at<Point>(i,0); 
		else if(X.type() == CV_32FC1 || X.type() == CV_32FC2) {
			Point2f _p = X.at<Point2f>(i,0); 
			p.x = _p.x; p.y = _p.y;
		}
		circle(im,p,2,c,CV_FILLED);
	}
}

void ICP::ShowLines(Mat& im, Mat& X, Mat& X_bar, Scalar c1, Scalar c2) {
	{
		//Mat im = Mat::zeros(300,500,CV_8UC3);
		for(int i=0;i<X.rows;i++) {
			Point p;
			if(X.type() == CV_32SC1 || X.type() == CV_32SC2)
				p = X.at<Point>(i,0); 
			else if(X.type() == CV_32FC1 || X.type() == CV_32FC2) {
				Point2f _p = X.at<Point2f>(i,0); 
				p.x = _p.x; p.y = _p.y;
			}
			circle(im,p,3,c1,CV_FILLED);
			//Point p1 = X_bar.at<Point>(i,0);
			//circle(im,p1,3,c2,CV_FILLED);

			//line(im,p,p1,Scalar(0,0,255),2);

			if(i>=1) {
				Point p_tag;
				if(X.type() == CV_32SC1 || X.type() == CV_32SC2)
					p_tag = X.at<Point>(i-1,0);
				else if(X.type() == CV_32FC1 || X.type() == CV_32FC2) {
					Point2f _p = X.at<Point2f>(i-1,0); 
					p_tag.x = _p.x; p_tag.y = _p.y;
				}
				
				line(im,p,p_tag,Scalar(255,150,0),2);
				//Point p1_tag = X_bar.at<Point>(i-1,0);
				//line(im,p1,p1_tag,Scalar(155,255,0),2);
			}
		}
	}
}

void ICP::ShowQuery(Mat& destinations, Mat& query, Mat& closest) {
	if(q_iter == 0) {
		q_im = Mat::zeros(300,500,CV_8UC3);
	}

	q_im = q_im * 0.85;

	ShowPoints(q_im,destinations);
	ShowLines(q_im,query,closest);
	imshow("tmp",q_im);
	waitKey();

	q_iter++;
}

/**
Taken from "Estimating 3-D rigid body transformations: a comparison of four major algorithms", Eggert et al 97'
Originally from Arun et al. (1987) and Umeyama (1991) and Kanatani (1994)
**/
void ICP::findBestReansformSVD(Mat& _m, Mat& _d) {
	Mat m; _m.convertTo(m,CV_32F);
	Mat d; _d.convertTo(d,CV_32F);

	Scalar d_bar = mean(d);
	Scalar m_bar = mean(m);
	Mat mc = m - m_bar;
	Mat dc = d - d_bar;

	mc = mc.reshape(1); dc = dc.reshape(1);
	
	Mat H(2,2,CV_32FC1);
	for(int i=0;i<mc.rows;i++) {
		Mat mci = mc(Range(i,i+1),Range(0,2));
		Mat dci = dc(Range(i,i+1),Range(0,2));
		H = H + mci.t() * dci;
	}

	cv::SVD svd(H);

	Mat R = svd.vt.t() * svd.u.t();
	double det_R = cv::determinant(R);
	if(abs(det_R + 1.0) < 0.0001) {
		float _tmp[4] = {1,0,0,cv::determinant(svd.vt*svd.u)};
		R = svd.u * Mat(2,2,CV_32FC1,_tmp) * svd.vt;
	}
#ifdef BTM_DEBUG
	//for some strange reason the debug version of OpenCV is flipping the matrix
	R = -R;
#endif
	float* _R = R.ptr<float>(0);
	Scalar T(d_bar[0] - (m_bar[0]*_R[0] + m_bar[1]*_R[1]),d_bar[1] - (m_bar[0]*_R[2] + m_bar[1]*_R[3]));

	mc = mc.reshape(1);
	mc = mc * R;
	mc = mc.reshape(2);
	mc = mc + T*0.5;// + m_bar;
	mc = mc + m_bar;
	mc.convertTo(_m,CV_32S);

	//{
	//	Mat im = Mat::zeros(300,500,CV_8UC3);
	//	ShowLines(im,_m,_d);
	//	imshow("tmp",im);
	//	waitKey(30);
	//}

}

void ICP::findBestTransform(Mat& X, Mat& X_bar) {
	if(!params.no_gui) {
		namedWindow("tmp");
		Mat im = Mat::zeros(300,500,CV_8UC3);
		ShowLines(im,X,X_bar);
		imshow("tmp",im);
		waitKey(30);
	}

	//Shift points to mean point
	Scalar xm = mean(X);

	X = X - xm;
	X_bar = X_bar - xm;
	
	Mat X32f; X.convertTo(X32f,CV_32F);
	Mat X_bar_32f; X_bar.convertTo(X_bar_32f,CV_32F);

	Mat A(2*X.rows,4,CV_32FC1);
	Mat b(2*X.rows,1,CV_32FC1);

	for(int i=0;i<X.rows;i++) {
		float* Ap = A.ptr<float>(2*i);
		Point2f xi = X32f.at<Point2f>(i,0);
		float _A[8] =	{xi.x,xi.y,1,0,
						 xi.y,-xi.x,0,1};
		memcpy(Ap,_A,sizeof(float)*8);

		float* bp = b.ptr<float>(2*i);
		Point2f xi_b = X_bar_32f.at<Point2f>(i,0);
		float _b[2] = {xi_b.x,xi_b.y};
		memcpy(bp,_b,sizeof(float)*2);
	}

	//Solve linear eq. system: Ax = b  ==>  x = inv(AtA) * (Atb)
	Mat sol = (A.t()*A).inv()*(A.t()*b);

	float* sd = (float*)sol.data;

	cout << "solution: ";
	for(int i=0;i<4;i++) cout << sd[i] << ",";
	cout << endl;

	//2D Rotation matrix 
	float _R[4] = {sd[0],sd[1],-sd[1],sd[0]};
	Mat R(2,2,CV_32FC1,_R);

	//Transform points
	X32f = X32f.reshape(1);
	Mat Xnew = (X32f) * R.inv();
	Xnew = Xnew.reshape(2);
	Xnew += Scalar(sd[2],sd[3]);

	//restore to original location
	Xnew = Xnew + xm;
	X_bar = X_bar + xm;

	if(!params.no_gui) {
		Mat im = Mat::zeros(300,500,CV_8UC3);
		ShowLines(im,Xnew,X_bar);
		imshow("tmp",im);
		waitKey(30);
	}

	Xnew = Xnew.reshape(2);
	Xnew.convertTo(X,CV_32SC1);
}

float ICP::flann_knn(Mat& m_destinations, Mat& m_object, vector<int>& ptpairs, vector<float>& dists) {
	// find nearest neighbors using FLANN
	cv::Mat m_indices(m_object.rows, 1, CV_32S);
	cv::Mat m_dists(m_object.rows, 1, CV_32F);

	Mat dest_32f; m_destinations.convertTo(dest_32f,CV_32FC2);
	assert(dest_32f.type() == CV_32FC2 || (dest_32f.type() == CV_32FC1 && dest_32f.cols == 2));

	Mat obj_32f; m_object.convertTo(obj_32f,CV_32FC2);

	cv::flann::Index flann_index(dest_32f, cv::flann::KDTreeIndexParams(2));  // using 4 randomized kdtrees
    flann_index.knnSearch(obj_32f, m_indices, m_dists, 1, cv::flann::SearchParams(64) ); // maximum number of leafs checked

    int* indices_ptr = m_indices.ptr<int>(0);
    //float* dists_ptr = m_dists.ptr<float>(0);
    for (int i=0;i<m_indices.rows;++i) {
   		ptpairs.push_back(indices_ptr[i]);
    }

	dists.resize(m_dists.rows);
	m_dists.copyTo(Mat(dists));

	return cv::sum(m_dists)[0];
}

void ICP::doICP(Mat& X, Mat& destination) {
	assert(X.rows > 0 && destination.rows > 0);

	vector<int> pair;
	double lastDist = std::numeric_limits<double>::max();
	Mat lastGood;

	bool re_reshape = false;
	if(X.channels() == 2) { X = X.reshape(1); re_reshape = true; }
	if(destination.channels() == 2) { destination = destination.reshape(1); re_reshape = true; }
	vector<float> dists;

	while(true) {
		pair.clear(); dists.clear();
		double dist = flann_knn(destination, X, pair, dists);

		if(lastDist <= dist) {
			X = lastGood;
			break;	//converged?
		}
		lastDist = dist;
		X.copyTo(lastGood);

		cout << "distance: " << dist << endl;

		Mat X_bar(X.size(),X.type());
		//vector<Point> X_barv;
		//vector<Point> X_v;
		for(int i=0;i<X.rows;i++) {
			Point p = destination.at<Point>(pair[i],0);
			X_bar.at<Point>(i,0) = p;
			//if(dists[i] < 100.0) {
			//	X_barv.push_back(p);
			//	X_v.push_back(X.at<Point>(i,0));
			//}
		}

		if(!params.no_gui) {
			ShowQuery(destination,X,X_bar);
		}

		//Mat X_bar(X_barv);
		//Mat subsetX(X_v);

		X = X.reshape(2);
		X_bar = X_bar.reshape(2);
		//findBestTransform(X,X_bar);
		findBestReansformSVD(X,X_bar);
		X = X.reshape(1); // back to 1-channel
	}

	lastGood.copyTo(X);

	if(re_reshape) {
		X = X.reshape(2);
		destination = destination.reshape(2);
	}

	cout << "converged" << endl;
}

int ICP::icp_main(int argc, char** argv) {
	//int _X[22] = {7, 198, 37, 155, 76, 138, 126, 123, 177, 112, 229, 114, 277, 110, 363, 136, 399, 151, 433, 196, 439, 222,};
	//Mat X(11,2,CV_32SC1,_X);
	
//	int _X_bar[22] = {6, 125, 67, 130, 97, 92, 150, 99, 188, 68, 208, 89, 285, 78, 305, 105, 369, 100, 390, 137, 432, 150,};
//	//int _X_bar[22] = {38, 232, 20, 184, 28, 127, 53, 69, 92, 33, 154, 12, 200, 12, 270, 13, 318, 26, 357, 46, 386, 76,};
////	int _X_bar[22] = {5, 217, 22, 168, 68, 141, 111, 125, 158, 115, 202, 111, 235, 106, 283, 111, 366, 136, 395, 151, 432, 188,};
//	//Mat X_bar(11,2,CV_32SC1,_X_bar); Mat X_bar_32f; X_bar.convertTo(X_bar_32f,CV_32F);
	double theta = -0.01*CV_PI;
	float _RotM[4] = { cos(theta), -sin(theta), sin(theta), cos(theta) };

	//RNG rng;
	//Mat X32f; X.convertTo(X32f,CV_32F);
	//Mat n(X32f.size(),X32f.type()); rng.fill(n,RNG::UNIFORM,Scalar(-35),Scalar(35));

	//Mat X_bar_32f = (X32f);
	////X_bar_32f(Range(0,X32f.rows),Range(1,2)) -= 45;
	//X_bar_32f = X_bar_32f * Mat(2,2,CV_32FC1,_RotM) + n;

	//Mat destinations; X_bar_32f.convertTo(destinations,CV_32S);

	RNG rng;
	Mat X(10,2,CV_32SC1);
	for(int i=0;i<10;i++) {
		X.at<Point>(i,0) = Point(100+sin(((double)i/10.0)*CV_PI)*50.0,100+cos(((double)i/10.0)*CV_PI)*50.0);
	}
	Mat destinations(50,2,X.type()); 
	rng.fill(destinations,RNG::NORMAL,Scalar(150.0),Scalar(125.0,50.0));

	//Mat X_1ch = X.reshape(2,X.rows);
	//Mat X_bar_1ch = destinations.reshape(2,X_bar.rows);

	//findBestTransform(X_1ch,X_bar_1ch);

	doICP(X,destinations);

	return 0;
}

}//ns