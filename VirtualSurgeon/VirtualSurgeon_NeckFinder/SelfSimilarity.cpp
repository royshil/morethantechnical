#include "cv.h"
#include "highgui.h"
#include "cvaux.h"

using namespace std;

#include "../VirtualSurgeon_Utils/VirtualSurgeon_Utils.h"

#include <string>
#include <vector>
#include <limits>
#include <iostream>
using namespace std;

int main(int argc, char** argv) {
	VIRTUAL_SURGEON_PARAMS p;
	ParseParams(p,argc,argv);

	Mat _im;
	FaceDotComDetection(p,_im);

	Mat __im;
	resize(_im,__im,Size((int)floor((double)_im.cols/p.im_scale_by),
							(int)floor((double)_im.rows/p.im_scale_by)));

	namedWindow("tmp");
	imshow("tmp",__im);
	waitKey();

	Mat im; __im.copyTo(im);

	SelfSimDescriptor imdescs;
	vector<float> im_descriptors;
	cout << "compute self-sim descs for imgae...";
	imdescs.compute(im,im_descriptors,Size(5,5));
	cout << "DONE " <<im_descriptors.size() << endl;

	Mat neck = imread("H:/Face Replace Thesis/BeTheModel/neck_template.png");
	SelfSimDescriptor templateDescs;
	vector<float> neck_descriptors;
	vector<Point> neck_points;
	neck_points.push_back(Point(7,133));
	neck_points.push_back(Point(27,89));
	neck_points.push_back(Point(72,54));
	neck_points.push_back(Point(127,34));
	neck_points.push_back(Point(184,81));
	neck_points.push_back(Point(227,81));
	neck_points.push_back(Point(274,29));
	neck_points.push_back(Point(329,50));
	neck_points.push_back(Point(377,84));
	neck_points.push_back(Point(400,131));

	Mat neckGray; cvtColor(neck,neckGray,CV_BGR2GRAY);
	templateDescs.compute(neckGray,neck_descriptors,Size(),neck_points);

	//Mat im(__im.size(),CV_64FC3);
	//cvtColor(__im,im,CV_BGR2Lab);

	//double var_noise = 1000;

	//Mat y_minus_1 = im(Range(1,im.rows),Range(0,im.cols)) - im(Range(0,im.rows-1),Range(0,im.cols));
	//Mat x_minus_1 = im(Range(0,im.rows),Range(1,im.cols)) - im(Range(0,im.rows),Range(0,im.cols-1));

	//imshow("tmp",y_minus_1);
	//waitKey();
	//imshow("tmp",x_minus_1);
	//waitKey();

	//namedWindow("tmp1");
	//for(int px = 21; px < im.cols - 25; px+=3) {
	//	for(int py = 21; py < im.rows - 25; py+=2) {
	//		Mat patch = im(Range(py-3,py+2),Range(px-3,px+2));
	//		Mat Sq(40,40,CV_64FC1);

	//		//Compare this patch:
	//		//vs. top patch
	//		Mat p1 = im(Range(py-4,py+1),Range(px-3,px+2));
	//		Mat A = p1 - patch;
	//		//Mat A = y_minus_1(Range(py-4,py+1),Range(px-3,px+2));
	//		Scalar _sum = sum(A.mul(A));
	//		double SSD1 = _sum[0] + _sum[1] + _sum[2];

	//		//vs. bottom patch
	//		p1 = im(Range(py-2,py+3),Range(px-3,px+2));
	//		A = p1 - patch;
	//		//A = y_minus_1(Range(py-3,py+2),Range(px-3,px+2));
	//		_sum = sum(A.mul(A));
	//		double SSD2 = _sum[0] + _sum[1] + _sum[2];

	//		//vs. right patch
	//		p1 = im(Range(py-3,py+2),Range(px-2,px+3));
	//		A = p1 - patch;
	//		//A = x_minus_1(Range(py-3,py+2),Range(px-3,px+2));
	//		_sum = sum(A.mul(A));
	//		double SSD3 = _sum[0] + _sum[1] + _sum[2];

	//		p1 = im(Range(py-3,py+2),Range(px-4,px+1));
	//		A = p1 - patch;
	//		//vs. left patch
	//		//A = x_minus_1(Range(py-3,py+2),Range(px-4,px+1));
	//		_sum = sum(A.mul(A));
	//		double SSD4 = _sum[0] + _sum[1] + _sum[2];

	//		double meanSSD = (SSD1+SSD2+SSD3+SSD4)/4.0;
	//		SSD1 -= meanSSD;// SSD1 = SSD1*SSD1;
	//		SSD2 -= meanSSD;// SSD2 = SSD2*SSD2;
	//		SSD3 -= meanSSD;// SSD3 = SSD3*SSD3;
	//		SSD4 -= meanSSD;// SSD4 = SSD4*SSD4;
	//		double var = max(max(max(SSD1,SSD2),SSD3),SSD4);

	//		for(int x=px-20;x<px+20;x++) {
	//			for(int y=py-20;y<py+20;y++) {
	//				Mat checkPatch = im(Range(y,y+5),Range(x,x+5));
	//				A = patch-checkPatch;
	//				double _A[25]; Mat _Am(5,5,CV_64FC1,_A); A.convertTo(_Am,CV_64FC1);
	//				_sum = sum(A.mul(A));
	//				double SSD = _sum[0] + _sum[1] + _sum[2];
	//				Sq.at<double>(y-py+20,x-px+20) = SSD;

	//				//double S = exp(-SSD/max(var_noise,var));

	//				//Sq.at<double>(y-py+20,x-px+20) = S;
	//			}
	//		}

	//		cv::exp(-Sq / max(var_noise,var),Sq);
	//		
	//		{
	//			Mat copy; im.copyTo(copy);
	//			/*circle(copy,Point(px,py),2,Scalar(255),CV_FILLED);*/
	//			rectangle(copy,Point(px-2,py-2),Point(px+2,py+2),Scalar(255));
	//			rectangle(copy,Point(px-20,py-20),Point(px+20,py+20),Scalar(255));
	//			imshow("tmp",copy);
	//		}

	//		double maxv;
	//		minMaxLoc(Sq,0,&maxv);
	//		Mat SqLP(Sq.size(),CV_64FC1);
	//		cvLogPolar(&((CvMat)Sq),&((CvMat)SqLP),cvPoint2D32f(20.0,20.0),40,INTER_NEAREST+CV_WARP_FILL_OUTLIERS);
	//		imshow("tmp1",SqLP);
	//		waitKey(1);
	//	}
	//}
}