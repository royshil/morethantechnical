#include "cv.h"
#include "highgui.h"
#include "cvaux.h"

using namespace cv;

#include "../VirtualSurgeon_Utils/VirtualSurgeon_Utils.h"

#include <string>
#include <vector>
#include <limits>
#include <iostream>
#include <fstream>
using namespace std;

#include "MySelfSimilarity.h"

int ExtractNeck(int argc, char** argv);

int main(int argc, char** argv) {
	//VIRTUAL_SURGEON_PARAMS p;
	//ParseParams(p,argc,argv);

	int _find = (int)(strstr(argv[1],"txt") - argv[1]);
	if(_find > 0 && _find <= strlen(argv[1])) {
		//this is a text file, probably with many pictures to run on
		ifstream f(argv[1]);
		string line;
		while(!f.eof()) {
			getline(f,line);
			char buf[512] = {0};
			strncpy(buf,line.c_str(),line.length());
			argv[1] = buf;

			ExtractNeck(argc,argv);
		}
	} else {
		ExtractNeck(argc,argv);
	}
}

int ExtractNeck(int argc, char** argv) {
	VIRTUAL_SURGEON_PARAMS p;
	ParseParams(p,argc,argv);

	Mat _im;
	FaceDotComDetection(p,_im);

	PrintParams(p);

	Mat __im;
	resize(_im,__im,Size((int)floor((double)_im.cols/p.im_scale_by),
							(int)floor((double)_im.rows/p.im_scale_by)));

	Mat neck = imread("C:/Users/Roy/Documents/VirtualSurgeon/neck_template1.png");
	Mat neckGray; cvtColor(neck,neckGray,CV_BGR2GRAY);
	Size neck_points_size = neckGray.size();

	namedWindow("tmp");
	imshow("tmp",__im);
	waitKey(p.wait_time);

	//bilateral filter for edge-preserving smoothing
	{
		Mat _tmp;
		bilateralFilter(__im,_tmp,30,80.0,5.0,BORDER_REPLICATE);
		_tmp.copyTo(__im);
	}
	//imshow("tmp",__im);
	//waitKey();
	

	Rect r; //interesting part of picture
	{
		int distBetweenEyes = abs(p.li.x/p.im_scale_by-p.ri.x/p.im_scale_by);
		int totalWidth = distBetweenEyes*10;
		r.x = MAX(0,MIN(
			/* x start: */(p.li.x/p.im_scale_by+p.ri.x/p.im_scale_by)/2 - totalWidth/3 - ((abs(p.yaw)>25.0) ? (p.yaw/60.0)*distBetweenEyes : 0),
			__im.cols)); 
		r.y = MAX(0,MIN(
			/* y start: */p.li.y/p.im_scale_by + distBetweenEyes/* - (p.pitch/40.0)*distBetweenEyes*2*/,
			__im.rows));
		r.width = MIN(totalWidth - ((abs(p.yaw)>25.0) ? abs(p.yaw/40.0)*distBetweenEyes*2 : 0),__im.cols-r.x); 
		r.height = MIN(totalWidth/3,__im.rows-r.y);
	}

	vector<Point> neck_points;
	//neck_points.push_back(Point(7,133));
	//neck_points.push_back(Point(27,89));
	//neck_points.push_back(Point(72,54));
	//neck_points.push_back(Point(127,34));
	//neck_points.push_back(Point(184,81));
	//neck_points.push_back(Point(227,81));
	//neck_points.push_back(Point(274,29));
	//neck_points.push_back(Point(329,50));
	//neck_points.push_back(Point(377,84));
	//neck_points.push_back(Point(400,131));

	neck_points.push_back(Point(22,166));
	neck_points.push_back(Point(71,136));
	neck_points.push_back(Point(167,108));
	neck_points.push_back(Point(174,44));
	neck_points.push_back(Point(196,159));
	neck_points.push_back(Point(248,159));
	//neck_points.push_back(Point(329,120));
	neck_points.push_back(Point(268,44));
	neck_points.push_back(Point(277,108));
	neck_points.push_back(Point(377,138));
	neck_points.push_back(Point(422,166));

	{
		Mat _tmp; __im.copyTo(_tmp);
		rectangle(_tmp,r,Scalar(255),2);
		Mat _tmp_r = _tmp(r);
		for(int i=0;i<neck_points.size();i++) {
			Point pt((float)neck_points[i].x * ((float)_tmp.cols/(float)neck_points_size.width),
					 (float)neck_points[i].y * ((float)_tmp.rows/(float)neck_points_size.height));

			circle(_tmp_r,pt, 
						2, Scalar(0,255),CV_FILLED);
			char s[10]; sprintf(s,"%d",i);
			putText(_tmp,string(s),pt,FONT_HERSHEY_PLAIN,1.0,Scalar(0,255),2);
		}
		imshow("tmp",_tmp);
		waitKey(p.wait_time);
	}

	Mat im; __im(r).copyTo(im);

	//Mat bias(r.size(),CV_64FC1);
	//{
	//	Mat small_temp; resize(neckGray,small_temp,r.size(),0,0,INTER_NEAREST);
	//	//imshow("tmp",small_temp);
	//	//waitKey();
	//	GaussianBlur(small_temp,small_temp,Size(21,21),5.0);
	//	//imshow("tmp",small_temp);
	//	//waitKey();
	//	small_temp.convertTo(bias,CV_64FC1,1/255.0);
	//	//imshow("tmp",bias);
	//	//waitKey();
	//	//bias = -bias + 1.0;
	//	//bias = bias * 4;
	//	imshow("tmp",bias);
	//	waitKey();
	//}

	cout << "compute self-sim descs for imgae...";

	myselfsim::SelfSimDescriptor imdescs;
	imdescs.largeSize = 41;
	imdescs.smallSize = 5;
	vector<float> im_descriptors;
	vector<Point> im_desc_loc;

	Mat im_gray; cvtColor(im,im_gray,CV_BGR2GRAY);
	imshow("tmp",im_gray);
	waitKey(p.wait_time);

	imdescs.compute(im_gray,im_descriptors,Size(5,5),im_desc_loc,true);

	cout << "DONE " <<im_descriptors.size() << endl;

	myselfsim::SelfSimDescriptor templateDescs;
	templateDescs.largeSize = 51;
	templateDescs.smallSize = 5;
	vector<float> neck_descriptors;
	

	templateDescs.compute(neckGray,neck_descriptors,Size(-1,-1),neck_points);

	//ofstream f("neck_descs.txt");
	//for(int i=0;i<neck_descriptors.size();i++) {
	//	f << neck_descriptors[i] << "," << endl;
	//}	
	//f.close();

	int featureSize = templateDescs.getDescriptorSize();
	int numTemplateDescs = neck_descriptors.size() / featureSize;
	int numImageDescs = im_descriptors.size() / featureSize;

	vector<std::pair<int,double> > indexAndSSD(numTemplateDescs);
	for(int i=0;i<numTemplateDescs;i++) indexAndSSD[i] = std::pair<int,double>(-1,DBL_MAX);

	vector<double> template_angles(numTemplateDescs,0.0);
	for(int i=1;i<numTemplateDescs;i++) {
		Point a = neck_points[i-1],b  = neck_points[i];
		double ang = b.ddot(a) / (sqrt(a.ddot(a))*sqrt(b.ddot(b)));
		template_angles[i] = ang;
	}

	for(int ti = 0; ti < numTemplateDescs; ti++) {
		float *tfeat = &(neck_descriptors[ti * featureSize]);

		vector<double> ssds(numImageDescs);

		for(int ii = 0; ii < numImageDescs; ii++) {
			float* ifeat = &(im_descriptors[ii * featureSize]);

			Mat D = Mat(1,80,CV_32FC1,tfeat) - Mat(1,80,CV_32FC1,ifeat);
			//Mat SSD = D * D.t();			//L2...
			//double ssd = SSD.at<float>(0,0);

			double ssd = sum(abs(D))[0];	//L1...
			if(ssd < 0 || ssd > 100) ssd = 100;

			double geometric_bias = 1.0;
			//if(ti > 0 && (im_desc_loc[ii].x > 0 || im_desc_loc[ii].y > 0)) {
			//	//angle to the prev 
			//	Point a = neck_points[ti-1];
			//	double img_ang = im_desc_loc[ii].ddot(a) / (sqrt(a.ddot(a))*sqrt(im_desc_loc[ii].ddot(im_desc_loc[ii])));

			//	//L2 dist
			//	geometric_bias = 1.0- abs(template_angles[ti] - img_ang);
			//}
				//if(ti > 1) {
				//	geometric_bias += (template_angles[ti-1] - img_ang);
				//}
				//geometric_bias = geometric_bias * geometric_bias;

				Point2f template_pt((float)neck_points[ti].x / neck_points_size.width,
									(float)neck_points[ti].y / neck_points_size.height);
				Point2f img_pt((float)im_desc_loc[ii].x / im.cols,
								(float)im_desc_loc[ii].y / im.rows);
				Point2f d_pt(template_pt.x-img_pt.x,template_pt.y-img_pt.y);

				geometric_bias = MAX(geometric_bias*sqrt(d_pt.ddot(d_pt)),0.15);
			//}

			//sigmoid on the L1
			ssds[ii] = 1.0/((1.0 + exp(-(ssd * geometric_bias))));

			//ssds[ii] = MIN(ssd * (0.002/geometric_bias),200.0);
		}

		double minv,maxv; Point minl;
		Mat ssdsM(ssds);
		minMaxLoc(ssdsM,&minv,&maxv,&minl,0);
		//Scalar meanSSD = mean(Mat(ssds));
		//double twomean =  meanSSD[0]*2.0;

		ssdsM = (ssdsM - minv) / (maxv - minv);

		for(int ii=0;ii<numImageDescs;ii++) {
			double ssdsii = ssds[ii];
			int v = 255 * (1 - ssdsii); // / twomean);
			circle(im,im_desc_loc[ii],5,Scalar(v,v,v),CV_FILLED);
		}

		indexAndSSD[ti].first = minl.y;
		indexAndSSD[ti].second = minv;

		circle(im,im_desc_loc[indexAndSSD[ti].first],3,Scalar(0,0,255),CV_FILLED);

		imshow("tmp",im);
		waitKey(p.wait_time);

		//"clear" out feature, so it wont repeat
		Mat(1,80,CV_32FC1,&(im_descriptors[indexAndSSD[ti].first * featureSize])).setTo(Scalar(FLT_MAX));
	}

	__im.copyTo(im);
	for(int ti = 0; ti < numTemplateDescs; ti++) {
		Point p1(im_desc_loc[indexAndSSD[ti].first].x + r.x,im_desc_loc[indexAndSSD[ti].first].y + r.y);
		if(ti > 0) {
			Point p2(im_desc_loc[indexAndSSD[ti-1].first].x + r.x, im_desc_loc[indexAndSSD[ti-1].first].y + r.y);
			line(im,p1,p2,Scalar(0,255),2);
		}
		circle(im,p1,3,Scalar(255),CV_FILLED);
	}

	cout <<" file: "<<p.filename<<endl;
	imshow("tmp",im);
	waitKey();

	return 0;

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