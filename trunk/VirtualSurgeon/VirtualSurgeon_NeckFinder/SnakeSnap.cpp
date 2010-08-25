#include "cv.h"
//#include "cvaux.h"
#include "highgui.h"

using namespace cv;

#include <iostream>
#include <vector>
#include <fstream>
using namespace std;

#include "../VirtualSurgeon_Utils/VirtualSurgeon_Utils.h"

#include "../tnc/tnc.h"

//#include "../CompassOperator/ruzon/greycompass.h"

#include "VirtualSurgeon_NeckFinder.h"

#include "VirtualSurgeon_ICP.h"

namespace VirtualSurgeon {

Mat Hysteresis(Mat& strength, double low, double high, int rows, int cols, int radius);

typedef struct data_for_tnc {
	Mat Xorig;
	Mat Xedges;
	Mat Xvector;
	Mat Xlength;

	Mat im_dx;
	Mat im_dy;
	Mat im_dx_dy;
	Mat im;

	bool do_gui;

	double w_edge;
	double w_direction;
	double w_consistency;

	Mat im_dx_orig;
	Mat im_dy_orig;

	Mat laplacian;

	MatND faceHist;
} DATA_FOR_TNC;

//Calculate the sum of the square values of the 2-channel matrix
//static double TwoD_SqSum(Mat& diff) {
//	//Mat diffsq = diff.mul(diff);	//squared
//	//diffsq = diffsq.reshape(1);		//split to 2 columns
//	//Mat _diffsq;
//	//diffsq(Range(0,10),Range(0,1)).copyTo(_diffsq);
//	//add(_diffsq,diffsq(Range(0,10),Range(1,2)),_diffsq);	//add 1st coloumn with 2nd
//	//_diffsq = _diffsq.mul(p.w_opt);	//weighted
//	//Scalar s = cv::sum(_diffsq);	//summed
//	//return s[0];
//	return cv::norm(diff);
//}

//void DoTheCompass(Mat& im) {
//	int imgrows, imgcols, numsigmas = 1, numangles = 1, nwedges = 6, radius;
//	//int i, j, k, anglewedges, index;
//	enum imgtype type = RGBImg;
//	double sigma[1] = {1.0}, spacing[1] = {1.0}, dimensions[4];
//	double angles[1] = {180}; //, wedgeangle;
//	double maxsigma = -1.0; //, *q;
//	Matrix *strength, *orientation, *uncertainty = NULL, *abnormality = NULL;
//	//Matrix *NMS;
//	unsigned char *imgdata; //, *p;
//	//int dims[3];
//
//	double low = 0.5, high = 0.7;
////	it_image *input, *output;
////	FILE *FP = stdout;
////	int mode = MODEC | MODET;
//
////	ParseArguments(argc, argv, &input, &FP, sigma, &low, &high, &mode);
//
//	/* Collect meta-data about the input */
//	imgrows = im.rows;
//	imgcols = im.cols;
//
//	/* The it_image data structure represents images as an array of pointers
//	* to rows of pixel values.  Since the compass operator was originally
//	* developed in the context of MATLAB, we must convert this structure to
//	* a one-dimensional array in column-major order.
//	*/
//	//imgdata = (unsigned char *)calloc(imgrows*imgcols, sizeof(unsigned char));
//	//p = imgdata;
//	//for (i = 0; i < imgcols; i++)
//	//	for (j = 0; j < imgrows; j++)
//	//		*p++ = im_byte_value(input,i,j);
//	Mat im_8uc; im.convertTo(im_8uc,CV_8UC1);
//
//	radius = ceil(3 * sigma[0]);
//	if (radius * 2 > imgrows || radius * 2 > imgcols)
//		cout << "Image is too small for sigma chosen";
//
//	dimensions[0] = radius;
//	dimensions[1] = radius;
//	dimensions[2] = imgrows - radius;
//	dimensions[3] = imgcols - radius;
//
//	/* Allocate output arguments */
//	orientation = (Matrix *)malloc(sizeof(Matrix));
//	orientation->rows = dimensions[2] - dimensions[0] + 1;
//	orientation->cols = dimensions[3] - dimensions[1] + 1;
//	orientation->sheets = MAXRESPONSES;
//	orientation->ptr = (double *)calloc(orientation->rows*orientation->cols*MAXRESPONSES, sizeof(double));
//	strength = (Matrix *)malloc(sizeof(Matrix));
//	strength->rows = orientation->rows;
//	strength->sheets = 1;
//	strength->cols = orientation->cols;
//	strength->ptr = (double *)calloc(strength->rows*strength->cols,sizeof(double));
//
//	/* Execute the compass operator */
//	//GreyCompass((void *)imgdata, imgrows, imgcols, type, sigma, numsigmas, 
//	//	radius, spacing, dimensions, angles, numangles, nwedges, 
//	//	strength, abnormality, orientation, uncertainty);
//}

Mat laplacian_mtx(int N, bool closed_poly) {
	Mat A = Mat::zeros(N, N, CV_64FC1);
	Mat d = Mat::zeros(N, 1, CV_64FC1);
    
    //## endpoints
	//if(closed_poly) {
		A.at<double>(0,1) = 1;
		d.at<double>(0,0) = 1;

		A.at<double>(N-1,N-2) = 1;
		d.at<double>(N-1,0) = 1;
	//} else {
	//	A.at<double>(0,1) = 1;
	//	d.at<double>(0,0) = 1;
	//}
    
    //## interior points
	for(int i = 1; i <= N-2; i++) {
        A.at<double>(i, i-1) = 1;
        A.at<double>(i, i+1) = 1;
        
        d.at<double>(i,0) = 0.5;
	}
    
	Mat Dinv = Mat::diag( d );
    
	return Mat::eye(N,N,CV_64FC1) - Dinv * A;
}

void normalize_per_element(Mat& Xv) {
	for(int i=0;i<Xv.rows;i++) {
		Vec2d v = Xv.at<Vec2d>(i,0);
		double n = norm(v);
		Xv.at<Vec2d>(i,0) = Vec2d(v[0]/n,v[1]/n);
	}
}

void calc_laplacian(Mat& X, Mat& Xlap) {
	static Mat lapX = laplacian_mtx(X.rows,false);
	if(lapX.rows != X.rows) lapX = laplacian_mtx(X.rows,false);

	vector<Mat> v; split(X,v);
	v[0] = v[0].t() * lapX.t();
	v[1] = v[1].t() * lapX.t();
	cv::merge(v,Xlap);

	Xlap = Xlap.t();
}

static double calc_Energy(Mat& X, DATA_FOR_TNC& d) {
	double sum = 0;

	//E_edge
	Mat Xe(X.rows,1,CV_32FC2);
	for(int i=0;i<X.rows;i++) {
		Point2d p = X.at<Point2d>(i,0);
		Point _p(MAX(MIN(floor(p.x),d.im_dx.cols-1),0),
				 MAX(MIN(floor(p.y),d.im_dx.rows-1),0));
		
		//linear interpolate sub-pixel accuracy
		double x_d = p.x - _p.x; 
		double y_d = p.y - _p.y;

		Point _px(MAX(MIN(_p.x+1,d.im_dx.cols-1),0),_p.y);
		float _dx_p = d.im_dx.at<float>(_p);
		float _dx_px = d.im_dx.at<float>(_px);
		float _dx = (1.0 - x_d) * _dx_p + x_d * _dx_px;

		Point _py(_p.x,MAX(MIN(_p.y+1,d.im_dx.rows-1),0));
		float _dy = (1.0 - y_d) * d.im_dy.at<float>(_p) + 
							y_d * d.im_dy.at<float>(_py);

		//float _dx_dy = d.im_dx_dy.at<float>(p.y,p.x);
		Xe.at<Point2f>(i,0) = Point2f(_dx,_dy);
	}
	sum += d.w_edge * cv::norm(Xe,d.Xedges);

	//E direction
	Mat Xv = X(Range(0,X.rows-1),Range(0,1)) - X(Range(1,X.rows),Range(0,1));
	normalize_per_element(Xv);
	Mat x_m_orig = Xv - d.Xvector;
	sum += d.w_direction*norm(x_m_orig(Range(0,x_m_orig.rows-1),Range(0,1)),x_m_orig(Range(1,x_m_orig.rows),Range(0,1)));

	//E_tension_length
	//sum += cv::norm(Xl,d.Xlength);
	//Mat _a = Xl / d.Xlength;
	//sum += 5.0*cv::norm(_a(Range(0,_a.rows-1),Range(0,1)),_a(Range(1,_a.rows),Range(0,1)));

	//E model curvature (shape) consistency
	//Laplacian coordinates?
	Mat Xlap; calc_laplacian(X,Xlap);
	Mat Xorig_lap; calc_laplacian(d.Xorig,Xorig_lap);
	sum += d.w_consistency * norm(Xlap,Xorig_lap);

	return (sum != sum) ? DBL_MAX : sum;
}

static tnc_function my_f;

#define EPSILON 0.1

static int my_f(double x[], double *f, double g[], void *state) {
	DATA_FOR_TNC* d_ptr = (DATA_FOR_TNC*)state;

	Mat XM(d_ptr->Xorig.size(),d_ptr->Xorig.type(),x);

	*f = calc_Energy(XM, *d_ptr);

	{
		Mat im;
		vector<Point> _newX(d_ptr->Xorig.rows),_xo(d_ptr->Xorig.rows);
		XM.convertTo(Mat(_newX),CV_32SC2);
		d_ptr->Xorig.convertTo(Mat(_xo),CV_32SC2);

		//d_ptr->im.copyTo(im);

		//show the dx and dy edges as red and blue "haze"
		vector<Mat> v(3); //split(im,v);
		//v[2] += d_ptr->im_dx_dy;
		v[1] = Mat::zeros(d_ptr->im.size(),CV_8UC1);
		Mat _tmp; 
		d_ptr->im_dx_orig.convertTo(_tmp,CV_8UC1,255.0);
		_tmp.copyTo(v[2]);
		d_ptr->im_dy_orig.convertTo(_tmp,CV_8UC1,255.0);
		_tmp.copyTo(v[0]);
		cv::merge(v,im);

		if(d_ptr->do_gui) {
			for(int ti = 0; ti < d_ptr->Xorig.rows; ti++) {
				Point p1(MAX(MIN(_newX[ti].x,d_ptr->im_dx.cols-1),0),MAX(MIN(_newX[ti].y,d_ptr->im_dx.cols-1),0));
				if(ti > 0) {
					Point p2(MAX(MIN(_newX[ti-1].x,d_ptr->im_dx.cols-1),0),MAX(MIN(_newX[ti-1].y,d_ptr->im_dx.cols-1),0));
					line(im,p1,p2,Scalar(0,255),2);
				}
				circle(im,p1,3,Scalar(255),CV_FILLED);
				line(im,p1,_xo[ti],Scalar(0,0,255),2);

				p1 = Point(MAX(MIN(_xo[ti].x,d_ptr->im_dx.cols-1),0),MAX(MIN(_xo[ti].y,d_ptr->im_dx.cols-1),0));
				if(ti > 0) {
					Point p2(MAX(MIN(_xo[ti-1].x,d_ptr->im_dx.cols-1),0),MAX(MIN(_xo[ti-1].y,d_ptr->im_dx.cols-1),0));
					line(im,p1,p2,Scalar(0,255,255),2);
				}
				circle(im,p1,3,Scalar(255,255),CV_FILLED);
				//line(im,p1,_xo[ti],Scalar(0,0,255),2);

			}
			imshow("tmp",im);
			waitKey(30);
		}
	}

	//calc gradients
	{
		vector<Point2d> _v(XM.rows);
		Mat vM(_v);

		for(int i=0;i<XM.rows;i++) {
			XM.copyTo(vM);

			double orig_x = _v[i].x;
			_v[i].x = MAX(0.0,MIN(d_ptr->im.cols-1,_v[i].x + EPSILON));
			double E_epsilon = calc_Energy(vM,*d_ptr);
			g[i*2] = ((E_epsilon - *f) / EPSILON);

			_v[i].x = orig_x;
			_v[i].y = MAX(0.0,MIN(d_ptr->im.rows-1,_v[i].y + EPSILON));
			E_epsilon = calc_Energy(vM,*d_ptr);
			g[i*2+1] = ((E_epsilon - *f) / EPSILON);
		}
	}

	return 0;
}

void NeckFinder::smear(Mat& im_dx, int levels) {
	Mat sumDx = Mat::zeros(im_dx.size(),CV_32FC1);
	vector<int> sizes(levels); 
	sizes[0]=3; sizes[1]=5; sizes[2]=7; sizes[3]=11;
	for(int i=4;i<levels;i++) sizes[i] = sizes[i-1]+4;

	for(int i=0;i<levels;i++) {
		Mat _tmp;
		GaussianBlur(im_dx,_tmp,Size(sizes[i],sizes[i]),sizes[i]/2.0);

		//imshow("tmp",(_tmp + 1.0) / 2.0);
		//waitKey(p.wait_time);

		sumDx += _tmp * ((i+1)*(i+1));
	}
	sumDx += im_dx;	//add last level
	double minv,maxv; minMaxLoc(sumDx,&minv,&maxv);
	//Mat sumDx_8u; sumDx.convertTo(sumDx_8u,CV_8UC1,1.0/maxv);
	//adaptiveThreshold(sumDx_8u,im_dx,255.0,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY
	im_dx = sumDx / maxv;
	//imshow("tmp",im_dx);
	//waitKey();
	//im_dx = min(im_dx,);
	//im_dx = im_dx / (max+minv)/2.0;
}


int NeckFinder::snake_snap_main(int argc, char** argv) {
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

			SnakeSnap(argc,argv);
		}
	} else {
		SnakeSnap(argc,argv);
	}

	return 0;
}

int NeckFinder::SnakeSnap(int argc, char** argv) {
	//VIRTUAL_SURGEON_PARAMS p;
	p.ParseParams(argc,argv);

	Mat _im;
	p.FaceDotComDetection(_im);

	p.PrintParams();

	namedWindow("tmp");

	return FindNeck(_im);
}

void NeckFinder::calcHistogramWMask(Mat& im, Mat& mask, MatND& hist) {
    int bins = 256;
    int histSize[3] = {bins,bins,bins};
    float range[2] = { 0, 256 };
    float* ranges[3] = { range,range,range };
    int channels[3] = {0,1,2};

	calcHist((const Mat*)(&(im)),1,(const int*)channels,mask,hist,3,histSize,(const float**)ranges);
}

int NeckFinder::FindNeck(Mat& _im) {
	Mat im_clean; _im.copyTo(im_clean);
	Mat _maskFace=Mat::zeros(_im.size(),CV_8UC1);

	MatND faceHist;

	Rect r; //interesting part of picture
	{
		int distBetweenEyes = abs(p.li.x -p.ri.x );
		int totalWidth = distBetweenEyes*p.snale_snap_total_width_coeff;
		r.x = MAX(0,MIN((int)(
			/* x start: */((double)(p.li.x +p.ri.x))/2.0 - ((double)totalWidth)/2.0 - ((abs(p.yaw)>25.0) ? (p.yaw/70.0)*(double)distBetweenEyes : 0)),
			_im.cols)); 
		r.y = MAX(0,MIN((int)(
			/* y start: */(double)p.li.y -40  /*+ ((double)distBetweenEyes)*0.25*/ - ((-abs(p.pitch))/30.0)*(double)distBetweenEyes),
			_im.rows));
		r.width = MIN((int)((double)totalWidth - ((abs(p.yaw)>25.0) ? abs(p.yaw/70.0)*(double)distBetweenEyes : 0)),_im.cols-r.x); 
		r.height = MIN((int)(((double)totalWidth)/2.5),_im.rows-r.y);

		//Rect ptch(p.li.x,p.li.y,p.ri.x-p.li.x,abs(p.ri.x-p.li.x)*2);
		//Mat ptchM = _im(ptch);
		//Scalar mean = cv::mean(ptchM);

		//Mat _tmp; _im.copyTo(_tmp);
		//rectangle(_tmp,ptch.tl(),ptch.br(),mean,CV_FILLED);
		//imshow("tmp",_tmp);
		//waitKey();

		
		double li_ri = -1.0;
		{
			int xdf = (p.li.x - p.ri.x);
			int ydf = (p.li.y - p.ri.y);
			li_ri = sqrt((double)(xdf*xdf) + (double)(ydf*ydf)) / (double)(_im.cols);
		}

		Point2d faceEllipse(
			((double)(p.li.x+p.ri.x))/2.0 - li_ri * p.yaw * 10.0,
			((double)(p.li.y+p.ri.y))/2.0 + (int)(li_ri * (double)_im.cols * 1.0)
		);
		ellipse(_maskFace,
			faceEllipse,
			//midp,
			Size((int)floor((double)(_im.cols) * li_ri * 1 + li_ri * p.yaw * 1),(int)floor(((double)_im.cols) * li_ri * 1.58)),
			-p.roll,	//angle
			0.0,	//start angle
			360.0,	//end angle
			Scalar(255),CV_FILLED);

		//imshow("tmp",_maskFace);
		//waitKey(p.wait_time);

		{
			double li_ri = norm(p.li - p.ri);// / (double)(faceMask.cols);
			Rect r(MIN(_im.cols,MAX(0,p.li.x - li_ri*3 * p.head_mask_size_mult)),
					MIN(_im.rows,MAX(0,p.li.y - li_ri*3 * p.head_mask_size_mult)),
					MIN(_im.cols-MAX(0,p.li.x - li_ri*3 * p.head_mask_size_mult),MAX(0,li_ri*6.5 * p.head_mask_size_mult)),
					MIN(_im.rows-MAX(0,p.li.y - li_ri*3 * p.head_mask_size_mult),MAX(0,li_ri*6.5 * p.head_mask_size_mult)));

			p.face_grab_cut(_im(r),_maskFace(r),1);//,200.0*li_ri);
		}

		calcHistogramWMask(_im,_maskFace,faceHist);

		Mat im_no_face(_im.size(),CV_8UC3); // _im.copyTo(___im,_maskFace);
		//imshow("tmp",___im);
		//waitKey(p.wait_time);

		im_no_face.setTo(Scalar(128,128,128));

		_im.copyTo(im_no_face,~_maskFace);
		if(!p.no_gui) {
			imshow("tmp",im_no_face);
			waitKey(p.wait_time);
		}

		im_no_face.copyTo(_im);
	}

	//Mat __im;
	//resize(_im,__im,Size((int)floor((double)_im.cols/p.im_scale_by),
	//						(int)floor((double)_im.rows/p.im_scale_by)));

	Mat neck = imread("C:/Users/Roy/Documents/VirtualSurgeon/neck_template1.png");
	//Mat neck = imread("C:/Users/Roy/Documents/VirtualSurgeon/neck_template2.png");
	Mat neckGray; cvtColor(neck,neckGray,CV_BGR2GRAY);
	Size neck_points_size = neckGray.size();


	//imshow("tmp",_im);
	//waitKey(p.wait_time);

	//{
	//	Mat _tmp;
	//	bilateralFilter(_im,_tmp,-1,50.0,5.0);
	//	_tmp.copyTo(_im);
	//	imshow("tmp",_im);
	//	waitKey(p.wait_time);
	//}

	Mat im; _im(r).copyTo(im);
	Mat maskFace; _maskFace(r).copyTo(maskFace);

	Mat neck_32f; neckGray.convertTo(neck_32f,CV_32F,1.0/255.0);
	double minv,maxv; 
	
	//Mat dx,dy; 
	//Sobel(neck_32f,dx,-1,1,0,3); 
	//minMaxLoc(dx,&minv,&maxv); 
	//dx = abs(dx / MAX(abs(minv),maxv));
	//{
	//	Mat _tmp;
	//	Canny(neckGray,_tmp,180.0,250.0);
	//	_tmp.convertTo(dx,CV_32FC1,1.0/255.0);
	//	_tmp.convertTo(dy,CV_32FC1,1.0/255.0);
	//}
	
	//smear(dx,15);

	////Mat dy; 
	//Sobel(neck_32f,dy,-1,0,1,3); 
	//minMaxLoc(dy,&minv,&maxv); 
	//dy = abs(dy / MAX(abs(minv),maxv));
	//smear(dy,15);

	//{
	//	Mat _tmp;
	//	vector<Mat> v(3);
	//	v[0] = Mat::ones(dy.size(),CV_32FC1);
	//	dy.copyTo(v[2]);
	//	//add(dx,dy,v[2]);
	//	//minMaxLoc(dy,&minv,&maxv); 
	//	//v[2] = abs(v[2] / MAX(abs(minv),maxv));
	//	v[0] = v[0] - v[2];
	//	v[1] = Mat::zeros(dy.size(),CV_32FC1);
	//	cv::merge(v,_tmp);
	//	imshow("tmp",_tmp); //(dx + 1.0) / 2.0);
	//	waitKey(p.wait_time);
	//}

	//Mat dy = Mat::zeros(neck_32f.size(),CV_32FC1);

	//Mat dx_dy; Sobel(neck_32f,dx_dy,-1,1,1,5); 
	//minMaxLoc(dx_dy,&minv,&maxv); 
	//dx_dy = dx_dy / MAX(abs(minv),maxv);
	//smear(dx_dy,13);

	//imshow("tmp",dx);
	//waitKey(p.wait_time);
	//imshow("tmp",dy);
	//waitKey(p.wait_time);

	/*16: int points[] = {6, 198,26, 161,73, 134,120, 119,167, 108,176, 72,185, 123,198, 158,247, 159,258, 123, 268, 74, 276, 106,323, 120,374, 137,422, 164,440, 203};
	*///39: int points[] = {8, 190, 16, 172, 37, 154, 50, 144, 67, 140, 95, 132, 113, 124, 126, 120, 150, 113, 166, 109, 175, 97, 177, 70, 175, 53, 182, 80, 183, 103, 182, 120, 187, 137, 197, 158, 207, 159, 227, 157, 241, 156, 250, 155, 256, 142, 260, 122, 268, 95, 270, 73, 269, 59, 273, 80, 273, 98, 278, 107, 297, 111, 316, 117, 339, 123, 354, 128, 371, 137, 396, 145, 413, 153, 423, 163, 436, 181,};
	//28: int points[] = {20, 171, 30, 156, 54, 146, 75, 136, 100, 130, 121, 123, 144, 115, 158, 111, 179, 102, 186, 116, 188, 133, 196, 152, 206, 157, 226, 159, 243, 157, 253, 151, 261, 131, 265, 110, 267, 96, 277, 101, 284, 109, 311, 113, 327, 119, 357, 128, 375, 136, 402, 147, 417, 157, 425, 169,};
	//int points[] = {23, 127, 49, 110, 63, 94, 95, 72, 114, 61, 145, 36, 165, 25, 165, 10, 161, 42, 151, 64, 160, 90, 186, 102, 213, 104, 242, 90, 256, 70, 269, 49, 288, 26, 278, 44, 290, 50, 309, 58, 328, 76, 347, 87, 370, 108, 385, 130,};
	//int points[22] = {16, 175, 69, 135, 136, 119, 177, 82, 188, 123, 224, 136, 261, 116, 270, 78, 302, 114, 368, 135, 428, 169,};
	//int points[54] = {15, 172, 40, 151, 62, 141, 84, 133, 106, 125, 129, 118, 149, 114, 167, 107, 175, 83, 179, 105, 185, 120, 199, 129, 215, 134, 234, 130, 248, 124, 264, 114, 266, 94, 270, 72, 276, 106, 291, 112, 315, 119, 329, 122, 350, 128, 368, 133, 387, 141, 408, 152, 426, 166,};
	//int points[26] = {20, 170, 66, 139, 111, 124, 159, 112, 177, 80, 183, 115, 217, 135, 258, 119, 270, 78, 279, 110, 327, 121, 378, 138, 421, 161,};
	//int points[32] = {16, 176, 52, 145, 89, 131, 134, 118, 168, 109, 176, 80, 184, 119, 205, 135, 238, 133, 260, 118, 270, 80, 278, 108, 316, 118, 361, 132, 397, 145, 429, 172,};
	//int points[18] = {164, 110, 178, 82, 180, 106, 189, 123, 212, 136, 245, 128, 265, 110, 270, 79, 278, 107,};
	//int points[24] = {77, 135, 122, 121, 170, 108, 176, 77, 188, 123, 210, 133, 230, 133, 257, 120, 269, 78, 279, 107, 321, 119, 365, 132,};
	//int points[16] = {176, 84, 182, 108, 191, 125, 214, 136, 231, 132, 261, 118, 265, 98, 268, 80,};
	int points[24] = {16, 176, 134, 118, 168, 109, 176, 80, 184, 119, 205, 135, 238, 133, 260, 118, 270, 80, 278, 108, 316, 118,429, 172,};
	Mat Xorig(12,1,CV_32SC2,points);
	
	Mat Xedges(Xorig.rows,1,CV_32FC2);
	for(int i=0;i<Xorig.rows;i++) {
		Point p = Xorig.at<Point>(i,0);
		Xedges.at<Point2f>(i,0) = Point2f(1.0,1.0); //Point2f(dx.at<float>(p),dy.at<float>(p));
	}

	Mat Xorig_64f; Xorig.convertTo(Xorig_64f,CV_64F);
	Xorig_64f = Xorig_64f.mul(Mat(Xorig.rows,1,CV_64FC2,
							Scalar(	((double)r.width/(double)neckGray.cols),
									((double)r.height/(double)neckGray.rows))));
					//+ Scalar(r.x,r.y);

	Mat im_gray; cvtColor(im,im_gray,CV_BGR2GRAY); 
	Mat im_32f; im_gray.convertTo(im_32f,CV_32F,1.0/255.0);

	//DoTheCompass(im_gray);

	Mat im_dx; 
	//Sobel(im_32f,im_dx,-1,1,0,21);
	//{
		//Mat _tmp;Canny(maskFace,_tmp,185.0,250.0,3,true);// 
		//_tmp.convertTo(im_dx,CV_32F,1.0/255.0);
		//Sobel(im_32f,im_dx,-1,1,0,3); 
		Mat _tmp;
		Canny(im_gray,_tmp,60.0,250.0,3);
		_tmp.convertTo(im_dx,CV_32FC1,1.0/255.0);

		//minMaxLoc(im_dx,&minv,&maxv); 
		//im_dx = abs(im_dx / MAX(abs(minv),maxv));
		Mat im_dx_orig; 

		//try to keep only long edges..
		{
			vector<vector<Point>> contours;
			vector<Vec4i> hirarchy;
			Mat im_dx_8u; im_dx.convertTo(im_dx_8u,CV_8UC1);
			findContours(im_dx_8u,contours,hirarchy,CV_RETR_LIST,CV_CHAIN_APPROX_NONE);

			Mat contours_im = Mat::zeros(im_dx.size(),CV_8UC1);
			for(int i=0;i<contours.size();i++) {
				if(contours[i].size() > p.snake_snap_edge_len_thresh) {
					//double strength = MIN(1.0,(double)(contours[i].size())/200.0);
					double strength = 1.0;
					drawContours(contours_im,contours,i,Scalar(255.0 * strength));
				}
			}
			if(!p.no_gui) {
				imshow("tmp",contours_im);
				waitKey(p.wait_time);
			}

			contours_im.convertTo(im_dx_orig,CV_32F,1.0/255.0);

			//contours_im.convertTo(im_dx,CV_32FC1,1.0/255.0);
			distanceTransform(~contours_im,im_dx,CV_DIST_WELSCH,CV_DIST_MASK_PRECISE);

			minMaxLoc(im_dx,&minv,&maxv); 
			//Mat _tmp; im_dx.copyTo(_tmp);
			//_tmp.copyTo(im_dx,_tmp>(maxv/2.0));
			//im_dx = abs(im_dx / MAX(abs(minv),maxv));
			im_dx.setTo(maxv/5.0,im_dx>maxv/5.0);

			minMaxLoc(im_dx,&minv,&maxv); 
			im_dx = -((im_dx - minv) / (maxv - minv)) + 1.0;

			//imshow("tmp",im_dx);
			//waitKey(p.wait_time);
		}

		

		//Mat hyst = Hysteresis(im_dx_orig,0.1,0.85,im_dx_orig.rows,im_dx_orig.cols,0);
		//imshow("tmp",hyst);
		//waitKey();

	//imshow("tmp",im_dx>0.5);
	//waitKey(p.wait_time);


		//smear(im_dx,15);
		//imshow("tmp",im_dx); //(dx + 1.0) / 2.0);
		//waitKey(p.wait_time);
	//}


	Mat im_dy;
	//Sobel(im_32f,im_dy,-1,0,1,3);
	//Mat im_dy = Mat::zeros(im_dx.size(),CV_32FC1);
	//Mat im_dx_dy; Sobel(im_32f,im_dx_dy,-1,1,1,7);
	
	//minMaxLoc(im_dx,&minv,&maxv);
	//im_dx = abs(im_dx / MAX(abs(minv),maxv));
	//minMaxLoc(im_dy,&minv,&maxv);
	//im_dy = abs(im_dy / MAX(abs(minv),maxv));

	//_tmp.convertTo(im_dy,CV_32FC1,1.0/255.0);
	im_dx.copyTo(im_dy);

	Mat im_dy_orig; im_dx.copyTo(im_dy_orig);

	//imshow("tmp",im_dx);
	//waitKey(p.wait_time);

	//imshow("tmp",im_dy);
	//waitKey(p.wait_time);

	//minMaxLoc(im_dx_dy,&minv,&maxv);
	//im_dx_dy = im_dx_dy / MAX(abs(minv),maxv);

	//smear(im_dx,6);
	//smear(im_dy,15);
	//smear(im_dx_dy,13);
	//
	//imshow("tmp",im_dx);
	//waitKey(p.wait_time);
	//imshow("tmp",im_dy);
	//waitKey(p.wait_time);

	if(!p.no_gui) {
		Mat _tmp; im_clean.copyTo(_tmp);
		//vector<Mat> v(3); split(_tmp,v);
		//v[2](r) += maskFace;
		//cv::merge(v,_tmp);

		for(int i=0;i<Xorig.rows;i++) {
			Point2d p = Xorig_64f.at<Point2d>(i,0);
			
			circle(_tmp,Point(p.x+r.x,p.y+r.y),3,Scalar(0,255),CV_FILLED);

			if(i>0) {
				Point2d p1 = Xorig_64f.at<Point2d>(i-1,0);
				line(_tmp,Point(p.x+r.x,p.y+r.y),Point(p1.x+r.x,p1.y+r.y),Scalar(255),2);
			}
		}
		imshow("tmp",_tmp);
		waitKey(p.wait_time);
	}

	//for(int i=0;i<Xorig.rows;i++){
	//	Mat _tmp(im.size(),CV_32FC1);
	//	Point2f e = Xedges.at<Point2f>(i,0);
	//	for(int y=0;y<im.rows;y++) {
	//		for(int x=0;x<im.cols;x++) {
	//			float _dx = im_dx.at<float>(y,x);
	//			float _dy = im_dy.at<float>(y,x);
	//			//float dist = sqrt((e.x-_dx)*(e.x-_dx) + (e.y-_dy)*(e.y-_dy));
	//			float dist = abs(e.x-_dx)+abs(e.y-_dy);
	//			_tmp.at<float>(y,x) = dist;
	//		}
	//	}
	//	double minv,maxv; minMaxLoc(_tmp,&minv,&maxv);
	//	Mat toShow = (_tmp - minv) / (maxv - minv);
	//	Point2d _p = Xorig_64f.at<Point2d>(i,0);
	//	rectangle(toShow,Point(_p.x-4-r.x,_p.y-4-r.y),Point(_p.x+4-r.x,_p.y+4-r.y),Scalar(1.0));
	//	imshow("tmp",toShow);
	//	waitKey(p.wait_time);
	//}

	//ICP
	{
		vector<Point> vDest;
		for(int y=0;y<im_dx_orig.rows;y++) {
			float* d_ptr = im_dx_orig.ptr<float>(y);
			for(int x=0;x<im_dx_orig.cols;x++) {
				if(d_ptr[x] > 0.0f) {
					vDest.push_back(Point(x,y));
				}
			}
		}
		vector<Point> vP(Xorig_64f.rows);
		Mat X32s(vP); Xorig_64f.convertTo(X32s,CV_32S);

		ICP icp(p);
		icp.doICP(X32s,Mat(vDest));

		X32s.convertTo(Xorig_64f,CV_64F);
	}

	Mat Xvector = Xorig_64f(Range(1,Xorig_64f.rows),Range(0,1)) - Xorig_64f(Range(0,Xorig_64f.rows-1),Range(0,1));
	Mat Xlength(Xorig_64f.rows-1,1,CV_64FC1);
	for(int i=0;i<Xorig_64f.rows-1;i++) {
		//normalize direction vector
		Vec2d v = Xvector.at<Vec2d>(i,0);
		double n = norm(v);
		Xvector.at<Vec2d>(i,0) = Vec2d(v[0]/n,v[1]/n);

		Xlength.at<double>(i,0) = n; //keep length of vector
	}


	//Energy Optimization
	{
		DATA_FOR_TNC d;
		d.im_dx = im_dx;
		d.im_dy = im_dy;
		//d.im_dx_dy = im_dx_dy;
		d.Xedges = Xedges;
		d.Xorig = Xorig_64f;
		d.Xvector = Xvector;
		d.Xlength = Xlength;
		d.im = im;
		d.im_dx_dy = im_dy;
		d.im_dx_orig = im_dx_orig;
		d.im_dy_orig = im_dy_orig;

		d.faceHist = faceHist;

		d.do_gui = !p.no_gui;

		d.w_edge = p.snake_snap_weight_edge;
		d.w_direction = p.snake_snap_weight_direction;
		d.w_consistency = p.snake_snap_weight_consistency;

		cout << "init energy: " << calc_Energy(Xorig_64f,d) << endl;

		Mat Grad(Xorig_64f.rows*2,1,CV_64FC1);
		double f;

		Mat X; Xorig_64f.copyTo(X);
		
		simple_tnc(Xorig_64f.rows*2,(double*)X.data,&f,(double*)Grad.data,my_f,(void*)&d,im.cols-3,im.rows-3);

		if(!p.no_gui) {
			Mat _tmp; im_clean.copyTo(_tmp);

			for(int i=0;i<X.rows;i++) {
				Point2d p = X.at<Point2d>(i,0);
				
				Point p2i(p.x+r.x,p.y+r.y);
				circle(_tmp,p2i,3,Scalar(255),CV_FILLED);

				this->neck.push_back(p2i);

				if(i>0) {
					Point2d p1 = X.at<Point2d>(i-1,0);
					line(_tmp,Point(p.x+r.x,p.y+r.y),Point(p1.x+r.x,p1.y+r.y),Scalar(0,255),2);
				}
			}

			Point m1 = X.at<Point2d>(floor(X.rows/2.0)-1,0);
			Point m2 = X.at<Point2d>(floor(X.rows/2.0),0);
			circle(_tmp,Point2d(r.x,r.y) + Point2d((m1+m2)*0.5),5,Scalar(0,0,255),CV_FILLED);

			imshow("tmp",_tmp);
			waitKey(p.wait_time);
		} else {
			for(int i=0;i<X.rows;i++) {
				Point2d p = X.at<Point2d>(i,0);
				this->neck.push_back(Point(p.x+r.x,p.y+r.y));
			}
		}
	}

	return 0;	
}

}//ns

#ifdef NECKFINDER_MAIN
int main(int argc, char** argv) {
	VirtualSurgeon::VirtualSurgeonParams p;
	p.InitializeDefault();
	p.im_scale_by = 1;
	p.no_gui = false;
	p.wait_time = 1;
	p.gc_iter = 0;
	p.km_numc = 50;
	p.hair_ellipse_size_mult = 1.1;
	p.do_alpha_matt = false;
	p.consider_pixel_neighbourhood = false;
	p.do_two_segments = false;
	p.do_kmeans = false;

	p.filename = std::string(argv[1]);
	Mat _tmp,im;
	p.FaceDotComDetection(_tmp);
	p.PrintParams();

	VirtualSurgeon::NeckFinder nf(p);
	nf.FindNeck(_tmp);

	waitKey();
}
#endif