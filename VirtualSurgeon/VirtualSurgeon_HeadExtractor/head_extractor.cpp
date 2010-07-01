#include "head_extractor.h"

#include "../tclap-1.2.0/include/tclap/CmdLine.h"
using namespace TCLAP;

#include <vector>
#include <fstream>

using namespace std;

#include "GCoptimization.h"

#include "matting.h"

#include "../BeTheModel/util.h"

#define _PI 3.14159265
#define TWO_PI 6.2831853

#define RELABLE_HIST_MAX 0
#define RELABLE_GRAPHCUT 1

namespace VirtualSurgeon {

void HeadExtractor::takeBiggestCC(Mat& mask, Mat& bias) {
	if(bias.rows == 0 || bias.cols == 0) bias = Mat::ones(mask.size(),CV_64FC1);

	vector<vector<Point> > contours;
	contours.clear();
	cv::findContours(mask,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);

	//compute areas
	vector<double> areas(contours.size());
	for(int ai=0;ai<contours.size();ai++) {
		Mat _pts(contours[ai]);
		Scalar mp = mean(_pts);

		//bias score according to distance from center face
		areas[ai] = contourArea(Mat(contours[ai])) * bias.at<double>(mp.val[1],mp.val[0]);
	}

	//find largest connected component
	double max; Point maxLoc;
	minMaxLoc(Mat(areas),0,&max,0,&maxLoc);

	//draw back on mask
	mask.setTo(Scalar(0)); //clear...
	drawContours(mask,contours,maxLoc.y,Scalar(255),CV_FILLED);
}

//void checkArgF(const char* arg, const char* c, int l, double* val) {
//	if(strncmp(arg,c,l)==0) {
//		char __s[10] = {0};
//		strncpy_s(__s,10,arg+l+1,10);
//		*val = atof(__s);
//	}
//}
//
//void checkArgI(const char* arg, const char* c, int l, int* val) {
//	if(strncmp(arg,c,l)==0) {
//		char __s[10] = {0};
//		strncpy_s(__s,10,arg+l+1,10);
//		*val = atoi(__s);
//	}
//}

/*
filter - the output filter
sigma - is the sigma of the Gaussian envelope
n_stds - number of sigmas in Gaussian
freq - represents the wavelength of the cosine factor, 
theta - represents the orientation of the normal to the parallel stripes of a Gabor function, 
phase - is the phase offset, 
gamma - is the spatial aspect ratio, and specifies the ellipticity of the support of the Gabor function.
*/
Mat HeadExtractor::gabor_fn(double sigma, int n_stds, double theta, double freq, double phase, double gamma) {
	double sigma_x = sigma;
	double sigma_y = sigma / gamma;

	//int sz_x=(int)floor((double)n_stds * sigma_x + 1.0);
	//int sz_y=(int)floor((double)n_stds * sigma_y + 1.0);
	//sz_x = MAX(sz_x,sz_y);
	//sz_y = MAX(sz_x,sz_y);

	//int sz_x_2 = (int)floor((double)sz_x / 2.0);
	//int sz_y_2 = (int)floor((double)sz_y / 2.0);

	double d_sz_x_2 = MAX(abs(n_stds*sigma_x*cos(theta)),abs(n_stds*sigma_y*sin(theta)));
	int sz_x_2 = (int)ceil(MAX(1.0,d_sz_x_2));
	double d_sz_y_2 = MAX(abs(n_stds*sigma_x*sin(theta)),abs(n_stds*sigma_y*cos(theta)));
	int sz_y_2 = (int)ceil(MAX(1.0,d_sz_y_2));

	int sz_x = sz_x_2 * 2 + 1;
	int sz_y = sz_y_2 * 2 + 1;

	//cerr << sz_x_2 << "->" << sz_x << "," << sz_y_2 << "->" << sz_y << endl;

	//int totSize = sz_x * sz_y;
	//double* gb_d = new double[totSize];
	//memset(gb_d,0,totSize*sizeof(double));

	Mat filter(sz_y,sz_x,CV_32FC1);
#ifdef _PI
	cerr << "Mat("<<sz_y<<","<<sz_x<<",CV_32FC1)"<<endl;
#endif

	double  x_theta;
	double  y_theta;
	double  x_theta_sq;
	double  y_theta_sq;
	double  sigma_x_sq = sigma_x * sigma_x;
	double  sigma_y_sq = sigma_y * sigma_y;

	int step = sz_x;
	int count = 0;
	float* _f_ptr = filter.ptr<float>();
	for(int y=-sz_y_2;y<=sz_y_2;y++) {
		for(int x=-sz_x_2;x<=sz_x_2;x++) {
			// Rotation 
			x_theta=x*cos(theta)+y*sin(theta);
			y_theta=-x*sin(theta)+y*cos(theta);
			x_theta_sq = x_theta * x_theta;
			y_theta_sq = y_theta * y_theta;

			//((float*)(filter.data))[(y+sz_y_2) * step + (x+sz_x_2)] = 
			_f_ptr[count++] = 
				(float)(exp(-.5*(x_theta_sq/sigma_x_sq+y_theta_sq/sigma_y_sq))*cos(TWO_PI*freq*x_theta + phase));
		}
	}

	//memcpy(filter.data,gb_d,totSize * sizeof(double));

	//delete[] gb_d;
	return filter;
}

void HeadExtractor::make_gabor_bank(vector<Mat>& filter_bank, int bank_size, double sigma, int n_stds, double freq, double phase, double gamma) {
	cerr << "make gabor bank.. ";

	//int bank_size = filter_bank.size();
	cerr << "size: "<<bank_size;
	vector<double> angs;
	for(int i=0;i<bank_size;i++) angs.push_back((double)(i) * _PI / bank_size);

	//namedWindow("gabor");
	for(int i=0;i<bank_size;i++) {
		cerr << ".";
		filter_bank.push_back(gabor_fn(sigma,n_stds,angs[i],freq,phase,gamma));
		//imshow("gabor",(filter_bank[i] + 1.0)/2.0);
		//waitKey();
	}

	cerr << " done"<<endl;
}

//im - the image to calc hist for
//mask - the area in the image to calc over
// range - the highest value for the bins
void HeadExtractor::calcHistogramWithMask(vector<MatND>& hist, Mat &im, vector<Mat>& mask, float _max, int win_size, int histCompareMethod, vector<Mat>& backProj, vector<Mat>& hists) {
    int bins = (int)_max;
    int histSize[1] = {bins};
    float range[2] = { 0, _max };
    float* ranges[] = { range };
    int channels[1] = {0};

	if(backProj.size() != hist.size()) {
		backProj = vector<Mat>(hist.size());
	}
	if(hists.size() != hist.size()) {
		hists = vector<Mat>(hist.size());
	}

	for(unsigned int i=0;i<hist.size();i++) {

		calcHist((const Mat*)( &(im) ), 1, channels, mask[i],
			hist[i], 1, histSize, (const float**)(ranges),
			true, // the histogram is uniform
			false );

		double maxVal=0;
		minMaxLoc(hist[i], 0, &maxVal, 0, 0);

#ifdef _PI
		if(!params.no_gui) {
			int scale = 10;
			Mat histImg = Mat::zeros(200, bins*scale, CV_8UC1);

			for( int s = 0; s < bins; s++ )
			{
				float binVal = hist[i].at<float>(s);
				if(binVal <= 0) continue;
				//int intensity = cvRound(binVal*255/maxValue);
				rectangle( histImg, Point(s*scale, 199),
							 Point( (s+1)*scale - 1, 200 - (int)floor((double)binVal*200.0/maxVal)),
							 Scalar::all(255),
							 CV_FILLED );
			}

			imshow("tmp",histImg);
		}
#endif

	}

	//Mat backProject;
	//calcBackProject((const Mat*)(&im),1,channels,hist,backProject,(const float**)ranges);
	//imshow("gabor",backProject);

	Mat paddedIm(im.rows+win_size,im.cols+win_size,im.type());
	paddedIm.setTo(Scalar(0));//on the borders there will be bias...
	im.copyTo(paddedIm(Rect(win_size/2,win_size/2,im.cols,im.rows)));

	for(int i=0;i<hist.size();i++) {
		normalize(hist[i],hist[i]);

		backProj[i] = Mat(paddedIm.size(),CV_64FC1,Scalar(0));
		hists[i] = Mat(countNonZero(mask[i]),bins,CV_32FC1);
	}

	vector<int> cnt(backProj.size(),0);
	for(int y=0;y<paddedIm.rows-win_size;y++) {
		//double* _bp_ptr = backProj.ptr<double>(y);

		for(int x=0;x<paddedIm.cols-win_size;x++) {
			Mat window = paddedIm(Rect(x,y,win_size,win_size));
			MatND winhist;
			calcHist((const Mat*)( &(window) ), 1, channels, Mat(),
				winhist, 1, histSize, (const float**)(ranges),
				true, // the histogram is uniform
				false );
			normalize(winhist,winhist);

			for(int i=0;i<backProj.size();i++) {
				double score = compareHist(winhist,hist[i],histCompareMethod);
				backProj[i].at<double>(y,x) = score;
				if(mask[i].at<uchar>(y,x) > 0) {
					memcpy(hists[i].ptr<float>(cnt[i]),winhist.data,sizeof(float)*bins);
					cnt[i]++;
				}
			}
		}
	}

	for(int i=0;i<backProj.size();i++) {
		backProj[i] = backProj[i](Rect(win_size/2,win_size/2,im.cols,im.rows));
#ifdef _PI
		if(!params.no_gui) {
			double maxV,minV;
			minMaxLoc(backProj[i],&minV,&maxV);
			imshow("tmp1",(backProj[i] - minV) / (maxV - minV));

			waitKey(BTM_WAIT_TIME);
		}
#endif
	}
}

void HeadExtractor::getSobels(Mat& gray, Mat& grayInt, Mat& grayInt1) {
	Mat _tmp,_tmp1,gray32f;
	
	gray.convertTo(gray32f,CV_32FC1,1.0/255.0);

	GaussianBlur(gray32f,gray32f,Size(15,15),.75);

	Sobel(gray32f,_tmp,-1,2,0,3);	//sobel for dx
	Sobel(gray32f,_tmp1,-1,2,0,3,-1.0);	//sobel for -dx
	//Canny(gray,_tmp,50.0,150.0);
	_tmp = abs(_tmp) + abs(_tmp1);

	if(!params.no_gui) {
		imshow("tmp1",_tmp1);
		imshow("tmp",_tmp); 
		waitKey(params.wait_time);
	}

	double maxVal,minVal;
	minMaxLoc(_tmp,&minVal,&maxVal);
	cv::log((_tmp - minVal) / (maxVal - minVal),_tmp);
	_tmp = -_tmp * 0.17;

	_tmp.convertTo(grayInt1,CV_32SC1);
	
	//grayInt = grayInt * 5;

	//filter2D(gray,_tmp,CV_32F,gk.t());
	Sobel(gray32f,_tmp,-1,0,2,3);	//sobel for dy
	Sobel(gray32f,_tmp1,-1,0,2,3,-1.0);	//sobel for -dy
	//Canny(gray,_tmp,50.0,150.0);
	_tmp = abs(_tmp) + abs(_tmp1);

	if(!params.no_gui) {
		imshow("tmp",_tmp); waitKey(params.wait_time);
	}

	minMaxLoc(_tmp,&minVal,&maxVal);
	cv::log((_tmp - minVal) / (maxVal - minVal),_tmp);
	_tmp = -_tmp * 0.17;
	_tmp.convertTo(grayInt,CV_32SC1);

}

void HeadExtractor::getEdgesForGC(Mat& gray, Mat& horiz, Mat& vert) {
	double sigma = 3.0; double two_sigma_sq = 2*sigma*sigma;
	uchar* gray_ptr_p1;
	//float* horiz_ptr_p1;
	double* vert_ptr_p1;
	uchar* gray_ptr_m1;
	//float* horiz_ptr_m1;
	double* vert_ptr_m1;

	horiz = Mat::zeros(gray.size(),CV_64FC1);
	vert = Mat::zeros(gray.size(),CV_64FC1);

	double lambda = 1.0f;

	for(int y=0;y<gray.rows;y++) {
		uchar* gray_ptr = gray.ptr<uchar>(y);
		double* horiz_ptr = horiz.ptr<double>(y);
		double* vert_ptr = vert.ptr<double>(y);

		if(y<gray.rows-1) {
			gray_ptr_p1 = gray.ptr<uchar>(y+1);
			//horiz_ptr_p1 = horiz.ptr<int>(y+1);
			vert_ptr_p1 = vert.ptr<double>(y+1);
		}
		if(y>0) {
			gray_ptr_m1 = gray.ptr<uchar>(y-1);
			//horiz_ptr_m1 = horiz.ptr<int>(y-1);
			vert_ptr_m1 = vert.ptr<double>(y-1);
		}


		for(int x=0;x<gray.cols;x++) {
			if(x>0) {
				double d = (double)gray_ptr[x] - (double)gray_ptr[x-1];
				horiz_ptr[x-1] += lambda*(exp(-(d*d)/two_sigma_sq));
			}
			//if(x<gray.cols-1) {
			//	double d = (double)gray_ptr[x] - (double)gray_ptr[x+1];
			//	horiz_ptr[x+1] += lambda*(exp(-(d*d)/two_sigma_sq));
			//}
			if(y>0) {
				double d = (double)gray_ptr[x] - (double)gray_ptr_m1[x];
				vert_ptr_m1[x] += lambda*(exp(-(d*d)/two_sigma_sq));
			}
			//if(y<gray.rows-1) {
			//	double d = (double)gray_ptr[x] - (double)gray_ptr_p1[x];
			//	vert_ptr_p1[x] += lambda*(exp(-(d*d)/two_sigma_sq));
			//}
		}
	}

	if(!params.no_gui) {
		double minv,maxv;
		minMaxLoc(horiz,&minv,&maxv);
		imshow("tmp",(horiz - minv) / (maxv - minv));
		minMaxLoc(vert,&minv,&maxv);
		imshow("tmp1",(vert - minv) / (maxv - minv));
		waitKey(params.wait_time);
	}
}

void HeadExtractor::create2DGaussian(Mat& im, double sigma_x, double sigma_y, Point mean) {
	double sig_x_sq = sigma_x*sigma_x;
	double sig_y_sq = sigma_y*sigma_y;
	for(int y=0;y<im.rows;y++) {
		double* y_ptr = im.ptr<double>(y);
		for(int x=0;x<im.cols;x++) {
			//g(x,y) = exp(-((x-x0)^2)/(sigma_x^2) + ((y-y0)^2)/(sigma_x^2)))

			y_ptr[x] = exp(-((x-mean.x)*(x-mean.x)/(sig_x_sq) + (y-mean.y)*(y-mean.y)/(sig_y_sq)));
		}
	}
}

//void matting(Mat& mask, Mat& im) {
//	Mat inner;
//	erode(mask,inner,Mat::ones(10,10,CV_8UC1));
//	Mat outer;
//	dilate(mask,outer,Mat::ones(10,10,CV_8UC1));
//
//	//vector
//}

void HeadExtractor::NaiveRelabeling(Size s, vector<Mat>& backP, vector<Mat>& maskA) {
	//"naive" re-labeling using histograms backprojection maximum value
	for(int _x=0;_x<s.width;_x++) {
		for(int _y=0;_y<s.height;_y++) {
			double bp0 = backP[0].at<double>(_y,_x);
			double bp1 = backP[1].at<double>(_y,_x);
			double bp2 = backP[2].at<double>(_y,_x);

			if(fabs(bp0-bp1) < .2 && fabs(bp0-bp2) < .2 && fabs(bp1-bp2) < .2) continue;

			if(bp0 > bp1) {
				if(bp0 > bp2) {
					//0 is max
					maskA[0].ptr<uchar>(_y)[_x] = 255;
					maskA[1].ptr<uchar>(_y)[_x] = 0;
					maskA[2].ptr<uchar>(_y)[_x] = 0;
				} else {
					//2 is max
					maskA[0].ptr<uchar>(_y)[_x] = 0;
					maskA[1].ptr<uchar>(_y)[_x] = 0;
					maskA[2].ptr<uchar>(_y)[_x] = 255;
				}
			} else {
				if(bp1 > bp2) {
					//1 is max
					maskA[0].ptr<uchar>(_y)[_x] = 0;
					maskA[1].ptr<uchar>(_y)[_x] = 255;
					maskA[2].ptr<uchar>(_y)[_x] = 0;
				} else {
					//2 is max
					maskA[0].ptr<uchar>(_y)[_x] = 0;
					maskA[1].ptr<uchar>(_y)[_x] = 0;
					maskA[2].ptr<uchar>(_y)[_x] = 255;
				}
			}
		}
	}
}

int HeadExtractor::head_extract_main(int argc, char** argv) {
	//VIRTUAL_SURGEON_PARAMS params;

	params.ParseParams(argc,argv);

	Mat im;

	if(params.filename.substr(0,6).compare("http://")) {
		params.FaceDotComDetection(im);
	} else {
		im = imread(params.filename);

		params.li = Point(253*0.4122,338*0.2624);
		params.ri = Point(253*0.5129,338*0.2603);
	}

	ExtractHead(im);

	return 0;
}

void HeadExtractor::calcSegmentsLikelihood(Mat& labled_im, 
										   vector<Mat> masks, 
										   int bins,
										   GCoptimizationGridGraph* gc,
										   Mat& vert_edge_score,
										   Mat& horiz_edge_score,
										   int* score_matrix) {

    int histSize[1] = {bins};
    float range[2] = { 0, (float)bins };
    float* ranges[] = { range };
    int channels[1] = {0};

	//get 1D lables frequencies from labled image
	MatND hist;
	vector<Mat> hists(masks.size());
	for(int i=0;i<masks.size();i++) {
		calcHist((const Mat*)(&labled_im),
			1,
			(const int*)channels,
			(const Mat&)(masks[i]),
			hist,
			1,
			(const int*)histSize,
			(const float**)ranges);

		Mat(hist).copyTo(hists[i]);

		double sum = cv::sum(hists[i])[0];
		hists[i] = hists[i] / sum;

		if(!params.no_gui) {
			Mat histP(100,bins*10,CV_8UC1,Scalar(0));
			for(int j=0;j<bins;j++) {
				cv::rectangle(histP,
					Point(j*10,100.0-((float*)hists[i].data)[j]*100.0),
					Point(j*10+10,100),
					Scalar(255),CV_FILLED);
			}
			imshow("tmp",histP);
			waitKey(params.wait_time);
		}

		//normalize(hists[i],hists[i]);
	}

	//4-connectivity frequencies
	vector<Mat> segment_neighbour_probs(masks.size());
	for(int i=0;i<masks.size();i++) { 
		segment_neighbour_probs[i] = Mat::zeros(bins,bins,CV_32SC4); 
	}

#define UP_PXL 0
#define RG_PXL 1
#define BT_PXL 2
#define LF_PXL 3

	//get frequencies from labled image
	for(int y=0;y<labled_im.rows;y++) {
		uchar* row_ptr = labled_im.ptr<uchar>(y);
		uchar* row_m1_ptr = NULL; if(y>0) row_m1_ptr = labled_im.ptr<uchar>(y-1);
		uchar* row_p1_ptr = NULL; if(y<labled_im.rows-1) row_p1_ptr = labled_im.ptr<uchar>(y+1);

		for(int x=0;x<labled_im.cols;x++)  {
			//scan for each segment
			for(int i=0;i<masks.size();i++) {
				if(masks[i].at<uchar>(y,x) > 0) {
					uchar pxl = row_ptr[x];
					//left pixel
					if(x>0) {
						uchar pxl_t = row_ptr[x-1];
						segment_neighbour_probs[i].at<Vec4i>(pxl,pxl_t)[LF_PXL]++;
					}
					//right pixel
					if(x<labled_im.cols-1) {
						uchar pxl_t = row_ptr[x+1];
						segment_neighbour_probs[i].at<Vec4i>(pxl,pxl_t)[RG_PXL]++;
					}
					//up pixel
					if(y>0) {
						uchar pxl_t = row_m1_ptr[x];
						segment_neighbour_probs[i].at<Vec4i>(pxl,pxl_t)[UP_PXL]++;
					}
					//bottom pixel
					if(y<labled_im.rows-1) {
						uchar pxl_t = row_p1_ptr[x];
						segment_neighbour_probs[i].at<Vec4i>(pxl,pxl_t)[BT_PXL]++;
					}
				}
			}
		}
	}

	vector<Scalar> sums(masks.size());
	for(int i=0;i<masks.size();i++) { 
		//vector<Mat> split(4);
		//cv::split(segment_neighbour_probs[i],split);
		//for(int j=0;j<4;j++)
		//	sums[i][j] = cv::sum(split[j])[0];

		//Mat _tmp32f; segment_neighbour_probs[i].convertTo(_tmp32f,CV_32F);
		sums[i] = cv::sum(segment_neighbour_probs[i]);
		//segment_neighbour_probs[i] = _tmp32f.mul(; 
	}

	//fill smoothness and data cost in graph
	for(int y=0;y<labled_im.rows;y++) {
		uchar* row_ptr = labled_im.ptr<uchar>(y);
		uchar* row_m1_ptr = NULL; if(y>0) row_m1_ptr = labled_im.ptr<uchar>(y-1);
		uchar* row_p1_ptr = NULL; if(y<labled_im.rows-1) row_p1_ptr = labled_im.ptr<uchar>(y+1);

		for(int x=0;x<labled_im.cols;x++)  {
			int pos = y * labled_im.cols + x;

			for(int i=0;i<masks.size();i++) {
				//data cost: P(label = i | pixel value, frequencies)
				//just get the normalized i'th-histogram value for this pixel value
				uchar pxl = row_ptr[x];
				double p = (double)(((float*)(hists[i].data))[pxl]);

				if(params.consider_pixel_neighbourhood) {
					//smoothness cost
					Mat pxl_row = segment_neighbour_probs[i](Range(pxl,pxl+1),Range(0,bins));
					Scalar summ = cv::sum(pxl_row);

					//to left
					if(x>0) {
						uchar pxl_t = row_ptr[x-1];
						double h_val = (double)(pxl_row.at<Vec4i>(0,pxl_t)[LF_PXL]);
						p *= h_val / summ[LF_PXL];
					}
					//right pixel
					if(x<labled_im.cols-1) {
						uchar pxl_t = row_ptr[x+1];
						double h_val = (double)(pxl_row.at<Vec4i>(0,pxl_t)[RG_PXL]);
						p *= h_val / summ[RG_PXL];
					}
					//up pixel
					if(y>0) {
						uchar pxl_t = row_m1_ptr[x];
						double h_val = (double)(pxl_row.at<Vec4i>(0,pxl_t)[UP_PXL]);
						p *= h_val / summ[UP_PXL];
					}
					//bottom pixel
					if(y<labled_im.rows-1) {
						uchar pxl_t = row_p1_ptr[x];
						double h_val = (double)(pxl_row.at<Vec4i>(0,pxl_t)[BT_PXL]);
						p *= h_val / summ[BT_PXL];
					}
				}

				double log_l;
				if(p <= 0.0 || p > 1.0 || p != p) log_l = 10000.0f;
				else log_l = -log(p);
				gc->setDataCost(pos,i,log_l);
			}
		}
	}
}

Mat HeadExtractor::ExtractHead(Mat& im) {
	btm_wait_time = params.wait_time;

#ifdef _PI
	if(!params.no_gui) {
		namedWindow("tmp");
		namedWindow("tmp1");
	}
#endif

	//Point li(102,111),ri(144,111);
	//Point li(102,109),ri(147,109);//angeline
	
	Point midp = (params.li + params.ri) * 0.5; //((int)(((double)params.li.x+(double)params.ri.x)/2.0),(int)(((double)params.li.y+(double)params.ri.y)/2.0));
	//vector<double> v(2); 
	//v[0] = ri.x-li.x; 
	//v[1] = ri.y-li.y;
	//double v_norm = norm(Mat(v));
	//Mat vm(v);
	//vm = vm / v_norm;
	//double vN[2] = {-v[1],v[0]};
	//double phi = acos(v[0]);

	Mat maskFace = Mat::zeros(im.rows,im.cols,CV_8UC1);
	//double li_ri = (double)abs(params.li.x - params.ri.x) / (double)(im.cols);
	double li_ri = -1.0;
	{
		int xdf = (params.li.x - params.ri.x);
		int ydf = (params.li.y - params.ri.y);
		li_ri = sqrt((double)(xdf*xdf) + (double)(ydf*ydf)) / (double)(im.cols);
	}

	//inner face estimate (ellipse)
	Point2d faceEllipse(
			((double)(params.li.x+params.ri.x))/2.0 - li_ri * params.yaw * 1.75,
			((double)(params.li.y+params.ri.y))/2.0 + (int)(li_ri * (double)im.cols / 2.5)
		);
	ellipse(maskFace,
		faceEllipse,
		//midp,
		Size((int)floor((double)(im.cols) * li_ri * 1/* - li_ri * params.yaw * 2.0*/),(int)floor(((double)im.cols) * li_ri * 1.68)),
		-params.roll,	//angle
		0.0,	//start angle
		360.0,	//end angle
		Scalar(255),CV_FILLED);

	if(params.gc_iter > 0)
		params.face_grab_cut(im,maskFace,1,170.0*li_ri);

	//------------ hair segmentation --------------
	{
	Mat hairMask = Mat::zeros(im.rows,im.cols,CV_8UC1);
	Point hairEllipse(
			(int)floor((double)midp.x - li_ri * 6.0 * params.yaw),
			(int)floor((double)midp.y - li_ri * 5.0 * params.pitch)
			);
	ellipse(hairMask,
		//Point((li.x+ri.x)/2.0,(li.y+ri.y)/2.0 + (int)(li_ri * (double)im.rows / 3.0)),
		hairEllipse,
		Size((int)floor((double)im.cols * li_ri * 1.38 * params.hair_ellipse_size_mult),(int)floor((double)im.cols * li_ri * 1.9 * params.hair_ellipse_size_mult)),
		-params.roll,	//angle
		0.0,	//start angle
		360.0,	//end angle
		Scalar(255),CV_FILLED);
	hairMask = hairMask - maskFace;

	if(!params.no_gui) {
		imshow("tmp",hairMask);
		{
			vector<Mat> ims; split(im,ims);
			ims[0] = ims[0] & hairMask;
			ims[1] = ims[1] & hairMask;
			ims[2] = ims[2] & hairMask;
			Mat _im; cv::merge(ims,_im);
			imshow("tmp1",_im);
		}
		waitKey(params.wait_time);
	
		namedWindow("gabor");
	}

	//make gabors
	vector<Mat> filter_bank;
	make_gabor_bank(filter_bank,params.gb_size,params.gb_sig,params.gb_nstds,params.gb_freq,params.gb_phase,params.gb_gamma);

	Mat im_small;
	resize(im,im_small,Size((int)floor((double)im.cols/params.im_scale_by),
							(int)floor((double)im.rows/params.im_scale_by)));

	Mat gray;
	cvtColor(im_small,gray,CV_RGB2GRAY);

	if(!params.no_gui) {
		Mat _col;
		cvtColor(gray,_col,CV_GRAY2RGB);
		circle(_col,Point((int)((double)params.li.x/params.im_scale_by),(int)((double)params.li.y/params.im_scale_by)),2,Scalar(255,0,0),CV_FILLED);
		circle(_col,Point((int)((double)params.ri.x/params.im_scale_by),(int)((double)params.ri.y/params.im_scale_by)),2,Scalar(255,0,0),CV_FILLED);
		imshow("tmp",_col);
		waitKey(params.wait_time);
	}

	Mat tmp;

	//samples matrix
	int featVecLength = im_small.rows*im_small.cols;

	//size of f.vec = <gb_size> gabor responses + 3 RGB val + x,y location
	Mat featureVec(featVecLength,params.gb_size + 3 + ((params.doPositionInKM)?2:0),CV_32FC1); 

	int im_ch = im_small.channels();
	for(int i=0;i<featVecLength;i++) {
		//rgb
		for(int ch = 0; ch<3; ch++) {
			((float*)(featureVec.data + i*featureVec.step))[ch] = (float)((im_small.data + i * im_ch)[ch]) / 255.0f;
		}
		if(params.doPositionInKM) {
			//x,y position
			((float*)(featureVec.data + i*featureVec.step))[3] = (float)ceil(((float)(i % im_small.cols) / (float)im_small.cols) * (float)params.km_numc) / (float)params.km_numc;
			((float*)(featureVec.data + i*featureVec.step))[4] = (float)ceil(((float)(i / im_small.cols) / (float)im_small.rows) * (float)params.km_numc) / (float)params.km_numc;
		}
	}

	int _off = 3 + ((params.doPositionInKM)?2:0);
	for(int i=0;i<params.gb_size;i++) {
		tmp.setTo(Scalar(0));
		Mat filter8bit; //(filter_bank[i].rows,filter_bank[i].cols,CV_8UC1);
		Mat _f = ((filter_bank[i] + 1.0)/2.0);
		_f.convertTo(filter8bit,CV_8UC1,255.0);
//#ifdef _PI
//		imshow("gabor",filter8bit);
//#endif
		//filter2D(gray,tmp,-1,filter_bank[i]);
		matchTemplate(gray,/*filter_bank[i]*/filter8bit,tmp,CV_TM_CCOEFF_NORMED);

		for(int ii=0;ii<featVecLength;ii++) {
			((float*)(featureVec.data + ii*featureVec.step))[i+_off] = (float)((gray.data + ii)[0]) / 255.0f;
		}
//#ifdef _PI
//		imshow("tmp",tmp);
//		waitKey(30);
//#endif
	}

	Mat featureLables;
	Mat centers; //(35,35,CV_32FC1);
	printf("Start K-Means...");
	kmeans(featureVec,
		params.km_numc,
		featureLables,
		TermCriteria(TermCriteria::MAX_ITER+TermCriteria::EPS,250,0.0001),
		params.km_numt,
		KMEANS_PP_CENTERS,
		&centers);
	printf("end\n");

	{
	Mat __tmp = featureLables.reshape(1,im_small.rows);
#ifdef _PI
	Mat __tmp1 = __tmp * 255.0 / (double)params.km_numc;
	__tmp1.convertTo(tmp,CV_8UC1);

	if(!params.no_gui) {
		imshow("tmp",tmp);
		int c = waitKey(params.wait_time);
		if(c=='q') return Mat();
	}
#endif
	__tmp.convertTo(tmp,CV_8UC1);
	}

	//calc histogram for face area
	Mat _maskFace,_faceBackp;
	resize(maskFace,_maskFace,tmp.size(),0,0,INTER_NEAREST);
	//imshow("gabor",_maskFace);

	Mat _hairMask;
	resize(hairMask,_hairMask,tmp.size(),0,0,INTER_NEAREST);
	//imshow("gabor",_hairMask);
	//calcHistogramWithMask(tmp,_hairMask,km_numc);

	//calc histogram for background area
	Mat notHair = Mat(tmp.size(),CV_8UC1,Scalar(255)) - _maskFace - _hairMask;
	//imshow("gabor",notHair);

	//test image for area that is not face or background (== hair!)
	if(params.doInitStep) {
		MatND faceHist,backHist;
		Mat _backBackp;

		vector<MatND> hists;
		hists.push_back(faceHist);
		hists.push_back(backHist);

		vector<Mat> masks;
		masks.push_back(_maskFace);
		masks.push_back(notHair);

		vector<Mat> backPs;
		backPs.push_back(_faceBackp);
		backPs.push_back(_backBackp);

		calcHistogramWithMask(hists,tmp,masks,(float)params.km_numc,4,0,backPs);

		Mat backProjD(tmp.size(),CV_64FC1,Scalar(0));

		double maxV,minV;
		minMaxLoc(_faceBackp,&minV,&maxV);
		_faceBackp = (_faceBackp - minV) / (maxV - minV);
		minMaxLoc(_backBackp,&minV,&maxV);
		_backBackp = (_backBackp - minV) / (maxV - minV);

		switch(params.com_add_type) {
			case 0:
				cv::sqrt(_faceBackp.mul(_faceBackp) + _backBackp.mul(_backBackp),backProjD); //sqrt(score_w_back*score_w_back + score_w_face*score_w_face);
				break;
			case 1:
				backProjD = max(_faceBackp,_backBackp); //_bpd_ptr[x] = MAX(score_w_back,score_w_face);
				break;
			case 2:
				backProjD = max(max(_faceBackp,_backBackp),_faceBackp+_backBackp); //_bpd_ptr[x] = MAX(MAX(score_w_back,score_w_face),score_w_face + score_w_back);
				break;
			case 3:
				backProjD = _faceBackp + _backBackp; //_bpd_ptr[x] = score_w_face + score_w_back;
		}

		minMaxLoc(backProjD,&minV,&maxV);
		backProjD = (backProjD - minV) / (maxV - minV);

		if(!params.no_gui) {
			imshow("gabor",backProjD);
		}
		
		Mat backProj = (backProjD < params.com_thresh);

		if(!params.no_gui) {
			imshow("tmp",backProj);
			waitKey(params.wait_time);
		}
		resize(backProj,hairMask,hairMask.size());

		hairMask = hairMask & ~maskFace;

		params.face_grab_cut(im,hairMask,params.gc_iter,18);

		if(!params.no_gui) {
			imshow("tmp",hairMask);
			waitKey(params.wait_time);
		}
	}

	Mat backMask = Mat(hairMask.size(),CV_8UC1,255) - hairMask - maskFace;

	if(!params.no_gui) {
		imshow("tmp",maskFace);
		imshow("gabor",hairMask);
		imshow("tmp1",backMask);
		waitKey(params.wait_time);
	}

	//iterative process
	{
	Mat _it_hairMask,_it_faceMask,_it_backMask;
	resize(maskFace,_it_faceMask,tmp.size(),0,0,INTER_NEAREST);
	resize(hairMask,_it_hairMask,tmp.size(),0,0,INTER_NEAREST);
	resize(backMask,_it_backMask,tmp.size(),0,0,INTER_NEAREST);

	MatND _it_hairHist,_it_faceHist,_it_backHist;
	
	vector<MatND> histA;
	histA.push_back(_it_hairHist); histA.push_back(_it_faceHist); histA.push_back(_it_backHist);
	vector<Mat> maskA;
	maskA.push_back(_it_hairMask); maskA.push_back(_it_faceMask); maskA.push_back(_it_backMask);

	Mat grayInt,grayInt1;
	//getSobels(gray,grayInt,grayInt1);
	getEdgesForGC(gray,grayInt,grayInt1);

	int num_lables = 3;
	GCoptimizationGridGraph gc(im_small.cols,im_small.rows,num_lables);

	double score[9] = {0.0f, 35.0f, 35.0f,
					35.0f, 0.0f, 35.0f,
					35.0f,35.0f, 0.0f};
	//Mat Sc = 15 * (Mat::ones(num_lables,num_lables,CV_32SC1) - Mat::eye(num_lables,num_lables,CV_32SC1));
	gc.setSmoothCostVH((double*)(score),(double*)grayInt.data,(double*)grayInt1.data);
	//gc.setSmoothCost((int*)(score));

	Mat bias(im_small.rows,im_small.cols,CV_64FC1),bias1;
	create2DGaussian(bias,li_ri*im_small.cols*2.3,li_ri*im_small.cols*2.7,
		Point(hairEllipse.x/params.im_scale_by,hairEllipse.y/params.im_scale_by));
	//bias.copyTo(bias1);
	bias = max(bias,Mat(bias.rows,bias.cols,CV_64FC1,Scalar(0.5)));
	bias1 = -bias + 1.0;

	for(int _i=0;_i<params.num_cut_backp_iters;_i++) {
		if(params.relable_type == RELABLE_HIST_MAX) {
			vector<Mat> backP(3),_tmp(3);

			//calc histogram from mask
			calcHistogramWithMask(histA,tmp,maskA,(float)params.km_numc,params.com_winsize,CV_COMP_INTERSECT,backP);

			backP[0] = backP[0].mul(bias);
			backP[1] = backP[1].mul(bias);
			backP[2] = backP[2].mul(bias1);

			if(!params.no_gui) {
				imshow("tmp",backP[0]);
				imshow("tmp1",backP[1]);
				imshow("gabor",backP[2]);
				waitKey(params.wait_time);
			}

			NaiveRelabeling(tmp.size(),backP,maskA);
		} else {
			//re-labeling using graphcut

			//if(_i > 0) {
			//	//Leap of faith - try to conquer more area
			//	dilate(maskA[0],maskA[0],Mat::ones(10,10,CV_8UC1));
			//	//dilate(maskA[1],maskA[1],Mat::ones(10,10,CV_8UC1));
			//}

			calcSegmentsLikelihood(tmp,maskA,params.km_numc,&gc,Mat(),Mat(),NULL);

			//double _a = log(1.3);

			//double maxv0; minMaxLoc(backP[0],0,&maxv0);
			//double maxv1; minMaxLoc(backP[1],0,&maxv1);
			//double maxv = MAX(maxv0,maxv1);

			////int __C = 0;

			//for(int _y=0;_y<tmp.rows;_y++) {
			//	for(int _x=0;_x<tmp.cols;_x++) {
			//		double bp0 = backP[0].at<double>(_y,_x);
			//		double bp1 = backP[1].at<double>(_y,_x);
			//		double bp2 = 0.2; //backP[2].at<double>(_y,_x);

			//		double sum = bp0 + bp1 + bp2;
			//		double P_bp0 = (bp0+EPSILON)/sum;
			//		double P_bp1 = (bp1+EPSILON)/sum;
			//		//double _bp2 = (bp2+EPSILON)/sum;

			//		bp0 = P_bp0*(1.0-P_bp1); //*(1.0-_bp2);
			//		bp1 = P_bp1*(1.0-P_bp0); //*(1.0-_bp2);
			//		bp2 = /*_bp2*/(1.0-P_bp1)*(1.0-P_bp0);

			//		//if((__C++) % 100 == 0) {
			//		//	cout << "P_bp0 = " << P_bp0 << endl;
			//		//	cout << "P_bp1 = " << P_bp1 << endl;
			//		//	cout << "bp0 = " << bp0 << endl;
			//		//	cout << "bp1 = " << bp1 << endl;
			//		//	cout << "bp2 = " << bp2 << endl;
			//		//}

			//		bp0 = -log(bp0)/_a;
			//		bp1 = -log(bp1)/_a;
			//		bp2 = -log(bp2)/_a;

			//		int pos = _y * im_small.cols + _x;
			//		gc.setDataCost(pos,0,(int)bp0);
			//		gc.setDataCost(pos,1,(int)bp1);
			//		gc.setDataCost(pos,2,(int)bp2);
			//	}
			//}

			printf("\nBefore optimization energy is %f",gc.compute_energy());
			//gc.expansion();// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
			//printf("\nAfter exp optimization energy is %d",gc.compute_energy());
			//printf("\nBefore optimization energy is %d",gc.compute_energy());
			gc.swap();// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
			printf("\nAfter swap optimization energy is %f",gc.compute_energy());

			Mat lables(im_small.cols*im_small.rows,1,CV_32SC1);
			for ( int  i = 0; i < lables.rows; i++ )
				((int*)(lables.data + lables.step * i))[0] = gc.whatLabel(i);

			if(!params.no_gui) {
				Mat _tmp = lables.reshape(1,im_small.rows);
				Mat _tmpUC;
				_tmp.convertTo(_tmpUC,CV_8UC1,255.0/(double)num_lables);
				vector<Mat> chns; split(im,chns);
				for(unsigned int ch=0;ch<chns.size();ch++) 
				{
					chns[ch] = /*chns[ch] + */(_tmp == ch)/**0.5*/;
				}
				cv::merge(chns,_tmpUC);
				imshow("tmp", _tmpUC);
				int c = waitKey(params.wait_time);
				if(c=='q') break;
			}

			//get masks out of lables
			for(int _ilb=0;_ilb<3;_ilb++) {
				Mat __labels = (lables == _ilb);
				__labels = __labels.reshape(1,tmp.rows);
				__labels.copyTo(maskA[_ilb]);
			}

			//find connected components in hair and face masks
			//for(int itr=0;itr<2;itr++) {
			//	takeBiggestCC(maskA[itr],bias);
			//}

			//TODO: optimize this biggest component choosing
			{
			Mat _combinedHairAndFaceMask = maskA[0] | maskA[1];
			takeBiggestCC(_combinedHairAndFaceMask,bias);
			maskA[0] = maskA[0] & _combinedHairAndFaceMask; //hair
			//maskA[1] = maskA[1] & _combinedHairAndFaceMask;
			takeBiggestCC(maskA[1],bias);	//face
			}

			//back mask is derived from hair and face
			//NOT(actually now it's always the whole image.)
			maskA[2] = Mat(maskA[2].rows,maskA[2].cols,CV_8UC1,Scalar(255)) - maskA[0] - maskA[1];

			if(!params.no_gui) {
				Mat _tmp;
				vector<Mat> v; for(int _ii=0;_ii<3;_ii++) v.push_back(maskA[_ii]);
				cv::merge(v,_tmp);
				imshow("tmp",_tmp);
				waitKey(params.wait_time);
			}

			//gc->setSmoothCost((GCoptimization::EnergyType*)0);
			//delete gc;
		}

		if(!params.no_gui) {
			imshow("tmp",_it_faceMask);
			imshow("gabor",_it_hairMask);
			imshow("tmp1",_it_backMask);
			waitKey(params.wait_time);
		}
	}

	resize(_it_faceMask,maskFace,maskFace.size(),0,0,INTER_NEAREST);
	resize(_it_hairMask,hairMask,hairMask.size(),0,0,INTER_NEAREST);
	//resize(_it_backMask,backMask,backMask.size(),0,0,INTER_NEAREST);

	} //end iterative proccess


	if(params.doScore) {
		Mat gt_im = imread(params.groundtruth,0);
		int groundTruth_count = countNonZero(gt_im);
		gt_im = (gt_im ^ hairMask);
#ifdef _PI
		if(!params.no_gui) {
			imshow("gabor",gt_im);
		}
#endif
		//double segscore = sum(gt_im > 0).val[0];
		int xor_count = countNonZero(gt_im);
		double segscore = (double)xor_count / (double)groundTruth_count;
		cout << "seg score: " << segscore << "(" << xor_count << "/" << groundTruth_count << ")" << endl;

	#ifdef _PI
		if(!params.no_gui) {
			waitKey(params.wait_time);
		}
	#endif
	}

	maskFace = maskFace | hairMask;

	takeBiggestCC(maskFace);

	if(params.gc_iter > 0) {
		params.face_grab_cut(im,maskFace,2,10);
	}

	//alpha matting
	if(params.do_alpha_matt) {
		Mat tmpMask(maskFace.rows,maskFace.cols,CV_8UC1,Scalar(0));
		int dilate_size = params.alpha_matt_dilate_size;

		//prepare trimap
		{
			Mat __tmp(maskFace.rows,maskFace.cols,CV_8UC1,Scalar(0));
			dilate(maskFace,__tmp,Mat::ones(dilate_size,dilate_size,CV_8UC1),Point(-1,-1),1,BORDER_REFLECT);	//probably background
			tmpMask.setTo(Scalar(128),__tmp);

			erode(maskFace,__tmp,Mat::ones(dilate_size*1.5,dilate_size*1.5,CV_8UC1),Point(-1,-1),1,BORDER_REFLECT); // foreground
			tmpMask.setTo(Scalar(255),__tmp);
		}

		Matting *matting = new BayesianMatting( &((IplImage)im), &((IplImage)tmpMask) );
		matting->Solve(!params.no_gui);

		Mat(matting->alphamap).copyTo(maskFace);

		//maskFace.convertTo(maskFace,CV_8UC1,255);

		delete matting;
	}

	}

//#ifdef _PI
	if(!params.no_gui) {
		imshow("tmp",maskFace);
	}
	
	Mat imMasked;
	{
		if(maskFace.type()!=CV_32FC1) 
			maskFace.convertTo(maskFace,CV_32FC1,1.0/255.0);

		Mat unMask = Mat::ones(maskFace.size(),CV_32FC1) - maskFace; //Mat(maskFace.size(),CV_8UC1,Scalar(255)) - maskFace;
		if(!params.no_gui) {
			imshow("gabor",unMask);
		}

		vector<Mat> v;
		im.convertTo(im,CV_32FC3,1.0/255.0);
		split(im,v);
		v[0] = v[0].mul(maskFace) + (unMask);
		v[1] = v[1].mul(maskFace);
		v[2] = v[2].mul(maskFace);
		cv::merge(v,imMasked);
	}
	if(!params.no_gui) {
		imshow("tmp1",imMasked);
		waitKey(params.wait_time);
	}
//#endif

	return maskFace;
}

}//ns

//int main(int argc, char** argv) {
//	VirtualSurgeon::VirtualSurgeonParams p;
//	p.InitializeDefault();
//	p.no_gui = false;
//	p.wait_time = 0;
//	p.gc_iter = 0;
//	p.km_numc = 10;
//	p.hair_ellipse_size_mult = 0.95;
//	//p.do_alpha_matt = true;
//
//	p.filename = std::string(argv[1]);
//	Mat im;
//	p.FaceDotComDetection(im);
//
//	VirtualSurgeon::HeadExtractor he(p);
//	he.ExtractHead(im);
//
//	waitKey();
//}