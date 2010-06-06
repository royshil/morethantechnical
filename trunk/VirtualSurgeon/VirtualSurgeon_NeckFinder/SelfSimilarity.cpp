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

#include "../tnc/tnc.h"

#include "MySelfSimilarity.h"

int ExtractNeck(int argc, char** argv);

int self_similarity_main(int argc, char** argv) {
	//VIRTUAL_SURGEON_PARAMS p;
	//ParseParams(p,argc,argv);

	int _find = (int)(strstr(argv[1],"txt") - argv[1]);
	if(_find > 0 && _find <= strlen(argv[1])) {
		//this is a text file, probably with many pictures to run on
		ifstream f(argv[1]);
		string line;
		while(!f.eof()) {
			getline(f,line);
			if(line.length() == 0) continue;
			char buf[512] = {0};
			strncpy(buf,line.c_str(),line.length());
			argv[1] = buf;

			ExtractNeck(argc,argv);
		}
	} else {
		ExtractNeck(argc,argv);
	}

	return 0;
}

typedef struct points_path_for_tnc {
	Mat X;
	Mat Xorig;
	Mat Xopt;
	Mat im;
	Mat w_opt;
	Mat w_smooth;
} POINTS_PATH_FOR_TNC;

static tnc_function my_f;

#define EPSILON 0.00001

static double calc_Energy(Mat& X, POINTS_PATH_FOR_TNC& p) {
	double sum = 0;
	{
		//translation term
		//xi_xj = (x[i] - xopt[i]);
		Mat diff = X - p.Xopt;			//difference
		Mat diffsq = diff.mul(diff);	//squared
		diffsq = diffsq.reshape(1);
		Mat _diffsq;
		diffsq(Range(0,10),Range(0,1)).copyTo(_diffsq);
		add(_diffsq,diffsq(Range(0,10),Range(1,2)),_diffsq);
		_diffsq = _diffsq.mul(p.w_opt);	//weighted
		Scalar s = cv::sum(_diffsq);
		sum += (s[0] + s[1]);			//summed

		//smoothness term
		{
			diff = X - p.Xorig;
			Mat diff_m1 = diff(Range(1,diff.rows),Range(0,1)) - diff(Range(0,diff.rows-1),Range(0,1));
			//diff_m1_sq = diff_m1.mul(diff_m1);
			vector<Point2d> _tmpp(diff.rows);
			Mat diff_tot(_tmpp);
			//add(diff_tot(Range(1,diff_tot.rows),Range(0,1)),diff_m1,diff_tot(Range(1,diff_tot.rows),Range(0,1)));
			diff_m1.copyTo(diff_tot(Range(1,diff_tot.rows),Range(0,1)));
			add(diff_tot(Range(0,diff_tot.rows-1),Range(0,1)),diff_m1,diff_tot(Range(0,diff_tot.rows-1),Range(0,1)));
			diffsq = diff_tot.mul(diff_tot);
			diffsq = diffsq.reshape(1);
			diffsq(Range(0,10),Range(0,1)).copyTo(_diffsq);
			add(_diffsq,diffsq(Range(0,10),Range(1,2)),_diffsq);
			_diffsq = _diffsq.mul(p.w_smooth);	//weighted
			Scalar s = cv::sum(diffsq);
			sum += (s[0] + s[1]);
		}
	}
	return sum;
}

static int my_f(double x[], double *f, double g[], void *state) {
	POINTS_PATH_FOR_TNC* points_ptr = (POINTS_PATH_FOR_TNC*)state;

	Mat XM(points_ptr->Xopt.size(),points_ptr->Xopt.type(),x);
	*f = calc_Energy(XM, *points_ptr);

	{
		Mat im;
		vector<Point> _newX(points_ptr->Xopt.rows),_xopt(points_ptr->Xopt.rows);
		XM.convertTo(Mat(_newX),CV_32SC2);
		points_ptr->Xopt.convertTo(Mat(_xopt),CV_32SC2);
		points_ptr->im.copyTo(im);
		for(int ti = 0; ti < points_ptr->Xopt.rows; ti++) {
			Point p1(_newX[ti].x,_newX[ti].y);
			if(ti > 0) {
				Point p2(_newX[ti-1].x, _newX[ti-1].y);
				line(im,p1,p2,Scalar(0,255),2);
			}
			circle(im,p1,3,Scalar(255),CV_FILLED);
			line(im,p1,_xopt[ti],Scalar(0,0,255),2);
		}
		imshow("tmp",im);
		waitKey(30);
	}

	//calc gradients
	{
		//POINTS_PATH_FOR_TNC _pointss;
		//_pointss.Xopt = points_ptr->Xopt;
		vector<Point2d> _v(XM.rows);
		Mat vM(_v);
		//_pointss.X = vM;

		for(int i=0;i<XM.rows;i++) {
			//points_ptr->X.copyTo(vM);
			XM.copyTo(vM);

			_v[i].x += EPSILON;
			double E_epsilon = calc_Energy(vM,*points_ptr);
			g[i*2] = (E_epsilon - *f) / EPSILON;

			_v[i].x -= EPSILON;
			_v[i].y += EPSILON;
			E_epsilon = calc_Energy(vM,*points_ptr);
			g[i*2+1] = (E_epsilon - *f) / EPSILON;
		}
	}

	return 0;
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
			/* x start: */(p.li.x/p.im_scale_by+p.ri.x/p.im_scale_by)/2 - totalWidth/2 - ((abs(p.yaw)>25.0) ? (p.yaw/70.0)*distBetweenEyes : 0),
			__im.cols)); 
		r.y = MAX(0,MIN(
			/* y start: */p.li.y/p.im_scale_by + distBetweenEyes*1.25 - (p.pitch/30.0)*distBetweenEyes,
			__im.rows));
		r.width = MIN(totalWidth - ((abs(p.yaw)>25.0) ? abs(p.yaw/70.0)*distBetweenEyes : 0),__im.cols-r.x); 
		r.height = MIN(totalWidth/3.5,__im.rows-r.y);
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
			Point pt((float)neck_points[i].x * ((float)_tmp_r.cols/(float)neck_points_size.width),
					 (float)neck_points[i].y * ((float)_tmp_r.rows/(float)neck_points_size.height));

			circle(_tmp_r,pt, 
						2, Scalar(0,255),CV_FILLED);
			char s[10]; sprintf(s,"%d",i);
			putText(_tmp_r,string(s),pt,FONT_HERSHEY_PLAIN,1.0,Scalar(0,255),2);
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
	vector<Point> destinations(numTemplateDescs);
	for(int ti = 0; ti < numTemplateDescs; ti++) {
		Point p1(im_desc_loc[indexAndSSD[ti].first].x + r.x,im_desc_loc[indexAndSSD[ti].first].y + r.y);
		if(ti > 0) {
			Point p2(im_desc_loc[indexAndSSD[ti-1].first].x + r.x, im_desc_loc[indexAndSSD[ti-1].first].y + r.y);
			line(im,p1,p2,Scalar(0,255),2);
		}
		circle(im,p1,3,Scalar(255),CV_FILLED);
		destinations[ti] = p1;
	}
	imshow("tmp",im);
	waitKey();

	//////////////// Enery minimization ////////////////
	{
		//tnc stuff: minimize the energy for the destinations and original locations
		Mat Grad(numTemplateDescs*2,1,CV_64FC1);
		double f;
		POINTS_PATH_FOR_TNC ps;

		//where the points should go
		Mat(destinations).convertTo(ps.Xopt,CV_64F);

		//where the points come from
		Mat(neck_points).convertTo(ps.X,CV_64F);
		ps.X = ps.X.mul(Mat(numTemplateDescs,1,CV_64FC2,
										Scalar(	((double)r.width/(double)neck_points_size.width),
												((double)r.height/(double)neck_points_size.height))))
					+ Scalar(r.x,r.y);

		ps.X.copyTo(ps.Xorig);

		double w_opt[] = {2.0,1.5,1.0,1.0,1.0,1.0,1.0,1.0,1.5,2.0};
		ps.w_opt = Mat(numTemplateDescs,1,CV_64FC1,w_opt);
		double w_smooth[] = {1.0,1.0,2.0,2.0,2.0,2.0,2.0,2.0,1.0,1.0};
		ps.w_smooth = Mat(numTemplateDescs,1,CV_64FC1,w_smooth);

		ps.im = __im;

		//minimize energy
		//simple_tnc(numTemplateDescs*2,(double*)ps.X.data,&f,(double*)Grad.data,my_f,(void*)&ps);

		//draw result
		{
			vector<Point> _newX(numTemplateDescs);
			ps.X.convertTo(Mat(_newX),CV_32SC2);
			__im.copyTo(im);
			for(int ti = 0; ti < numTemplateDescs; ti++) {
				Point p1(_newX[ti].x,_newX[ti].y);
				if(ti > 0) {
					Point p2(_newX[ti-1].x, _newX[ti-1].y);
					line(im,p1,p2,Scalar(0,255),2);
				}
				circle(im,p1,3,Scalar(255),CV_FILLED);
			}
			imshow("tmp",im);
			waitKey();
		}
	}

	cout <<" file: "<<p.filename<<endl;

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