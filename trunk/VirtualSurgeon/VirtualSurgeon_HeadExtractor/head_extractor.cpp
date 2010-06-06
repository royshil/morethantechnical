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

//#define BTM_WAIT_TIME btm_wait_time
//
//int btm_wait_time = 1;


#define RELABLE_HIST_MAX 0
#define RELABLE_GRAPHCUT 1

void takeBiggestCC(Mat& mask, Mat& bias = Mat()) {
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

void checkArgF(const char* arg, const char* c, int l, double* val) {
	if(strncmp(arg,c,l)==0) {
		char __s[10] = {0};
		strncpy_s(__s,10,arg+l+1,10);
		*val = atof(__s);
	}
}

void checkArgI(const char* arg, const char* c, int l, int* val) {
	if(strncmp(arg,c,l)==0) {
		char __s[10] = {0};
		strncpy_s(__s,10,arg+l+1,10);
		*val = atoi(__s);
	}
}

/*
filter - the output filter
sigma - is the sigma of the Gaussian envelope
n_stds - number of sigmas in Gaussian
freq - represents the wavelength of the cosine factor, 
theta - represents the orientation of the normal to the parallel stripes of a Gabor function, 
phase - is the phase offset, 
gamma - is the spatial aspect ratio, and specifies the ellipticity of the support of the Gabor function.
*/
Mat gabor_fn(double sigma, int n_stds, double theta, double freq, double phase, double gamma) {
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
#ifdef BTM_DEBUG
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

void make_gabor_bank(vector<Mat>& filter_bank, int bank_size, double sigma, int n_stds, double freq, double phase, double gamma) {
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
void calcHistogramWithMask(vector<MatND>& hist, Mat &im, vector<Mat>& mask, float _max, int win_size = 10, int histCompareMethod = CV_COMP_CORREL, vector<Mat>& backProj = vector<Mat>(), vector<Mat>& hists = vector<Mat>()) {
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

	for(int i=0;i<hist.size();i++) {

		calcHist((const Mat*)( &(im) ), 1, channels, mask[i],
			hist[i], 1, histSize, (const float**)(ranges),
			true, // the histogram is uniform
			false );

		double maxVal=0;
		minMaxLoc(hist[i], 0, &maxVal, 0, 0);

#ifdef BTM_DEBUG
		{
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
#ifdef BTM_DEBUG
		double maxV,minV;
		minMaxLoc(backProj[i],&minV,&maxV);
		imshow("tmp1",(backProj[i] - minV) / (maxV - minV));

		waitKey(BTM_WAIT_TIME);
#endif
	}
}

void getSobels(Mat& gray, Mat& grayInt, Mat& grayInt1) {
	Mat _tmp,_tmp1,gray32f;
	
	gray.convertTo(gray32f,CV_32FC1,1.0/255.0);

	GaussianBlur(gray32f,gray32f,Size(15,15),0.75);

	Sobel(gray32f,_tmp,-1,2,0,3);	//sobel for dx
	Sobel(gray32f,_tmp1,-1,2,0,3,-1.0);	//sobel for -dx
	//Canny(gray,_tmp,50.0,150.0);
	_tmp = abs(_tmp) + abs(_tmp1);
#ifdef BTM_DEBUG
	imshow("tmp1",_tmp1);
	imshow("tmp",_tmp); waitKey(BTM_WAIT_TIME);
#endif
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
#ifdef BTM_DEBUG
	imshow("tmp",_tmp); waitKey(BTM_WAIT_TIME);
#endif
	minMaxLoc(_tmp,&minVal,&maxVal);
	cv::log((_tmp - minVal) / (maxVal - minVal),_tmp);
	_tmp = -_tmp * 0.17;
	_tmp.convertTo(grayInt,CV_32SC1);

}

void create2DGaussian(Mat& im, double sigma_x, double sigma_y, Point mean) {
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

void matting(Mat& mask, Mat& im) {
	Mat inner;
	erode(mask,inner,Mat::ones(10,10,CV_8UC1));
	Mat outer;
	dilate(mask,outer,Mat::ones(10,10,CV_8UC1));

	//vector
}

void NaiveRelabeling(Size s, vector<Mat>& backP, vector<Mat>& maskA) {
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

/*
void ParseHeadExtractParams(HEAD_EXTRACTOR_PARAMS& params) {
	params.gb_sig = 1.0;
	params.gb_freq = 0.15;
	params.gb_phase = _PI/2.0;
	params.gb_gamma = 1.0;
	params.gb_nstds = 3;
	params.gb_size = 32;
	params.km_numc = 10;
	params.com_winsize = 5;
	params.com_thresh = 0.25;
	params.com_add_type = 0;
	params.com_calc_type = 1;
	params.im_scale_by = 2.0;
	params.gc_iter = 1;
	params.km_numt = 2;
	params.doScore = false;
	params.relable_type = 0;
	params.doPositionInKM = false;
	params.doInitStep = false;
	params.num_cut_backp_iters = 1;
	params.do_alpha_matt = false;
	params.alpha_matt_dilate_size = 10;
	params.use_hist_match_hs = false;
	params.use_hist_match_rgb = false;
	params.use_overlay = false;
	params.use_warp_rigid = false;
	params.use_warp_affine = false;
	params.use_double_warp = false;

	try {  

	// Define the command line object.
	CmdLine cmd("Command description message", ' ', "0.9");

	UnlabeledValueArg<string> filename_arg("filename","file to work on",true,"","string",cmd);
	UnlabeledValueArg<string> groundtruth_arg("groundtruth","ground truth file",false,"","string",cmd);
	ValueArg<double> gb_freq_arg("f","gabor-freq","Gabor func frequency",false,0.15,"float",cmd);
	ValueArg<double> gb_sig_arg("s","gabor-sigma","Gabor func sigma",false,1.0,"float",cmd);
	ValueArg<double> gb_phase_arg("p","gabor-phase","Gabor func phase",false,_PI/2.0,"float",cmd);
	ValueArg<double> gb_gamma_arg("g","gabor-gamma","Gabor func gamma",false,1.0,"float",cmd);
	ValueArg<int> gb_nstds_arg("n","gabor-nstds","Gabor func number of std devs",false,3,"int",cmd);
	ValueArg<int> gb_size_arg("z","gabor-size","Gabor filter bank size",false,32,"int",cmd);
	ValueArg<int> km_numc_arg("c","kmeans-num-centers","K-Means number of clusters",false,10,"int",cmd);
	ValueArg<int> km_numt_arg("m","kmeans-num-tries","K-Means number of tries",false,2,"int",cmd);
	ValueArg<int> com_winsize_arg("w","combine-win-size","Hist combine window size",false,5,"int",cmd);
	ValueArg<double> com_thresh_arg("t","combine-threshold","Hist combine threshold",false,0.15,"float",cmd);
	ValueArg<int> com_add_type_arg("y","combine-add-type","Hist combine scores add type (0=L2, 1=MAX, 2=MAX+, 3=just +)",false,2,"int [0-3]",cmd);
	ValueArg<int> com_calc_type_arg("l","combine-type","Hist combine scores calc type (0=COREL, 1=CHISQR, 2=INTERSECT, 3=BAHAT.)",false,0,"int [0-3]",cmd);
	ValueArg<double> im_scale_by_arg("b","image-scale-by","Scale down image by factor",false,2.0,"float",cmd);
	ValueArg<int> gc_iter_arg("r","grabcut-iterations","Number of grabcut iterations",false,1,"int",cmd);
	ValueArg<int> relable_type_arg("e","relable-type","Type of relabeling in iterative procecess (0 = hist max, 1 = graph cut)",false,0,"int",cmd);
	SwitchArg position_in_km_arg("q","position-in-km","Include position in K-Means?",cmd,false);
	SwitchArg initialization_step_arg("a","initialization_step","Do initialization step?",cmd,false);
	//SwitchArg doScore("a","compute-score","Should compute score according to ground truth");
	ValueArg<int> num_iters_arg("x","num-iters","Number of cut-backp iterations",false,1,"int",cmd);
	ValueArg<int> btm_wait_time_arg("d","wait-time","Time in msec to wait on debug pauses",false,1,"int",cmd);
	SwitchArg do_alphamatt_arg("u","do-alpha-matting","Do alpha matting?",cmd,false);
	ValueArg<int> alpha_matt_dilate_arg("i","alpha-matt-dilate-size","Size in pixels to dilate mask for alpha matting",false,10,"int",cmd);
	SwitchArg use_hist_match_hs_arg(string(),"use-hist-match-hs","Use histogram matching over HS space for recoloring?",cmd,false);
	SwitchArg use_hist_match_rgb_arg(string(),"use-hist-match-rgb","Use histogram matching over RGB space for recoloring?",cmd,false);
	SwitchArg use_overlay_arg(string(),"use-overlay","Use overlay for recoloring?",cmd,false);
	SwitchArg use_warp_rigid_arg(string(),"use-warp-rigid","Use rigid warp for neck warping?",cmd,false);
	SwitchArg use_warp_affine_arg(string(),"use-warp-affine","Use affine warp for neck warping?",cmd,false);
	SwitchArg use_double_warp_arg(string(),"use-double-warp","Use 2-way warping?",cmd,false);

	// Parse the args.
	cmd.parse( argc, argv );

	// Get the value parsed by each arg. 
	params.filename = filename_arg.getValue();
	params.groundtruth = groundtruth_arg.getValue();
	params.gb_sig = gb_sig_arg.getValue();
	params.gb_freq = gb_freq_arg.getValue();
	params.gb_phase = gb_phase_arg.getValue();
	params.gb_gamma = gb_gamma_arg.getValue();
	params.gb_nstds = gb_nstds_arg.getValue();
	params.gb_size = gb_size_arg.getValue();
	params.km_numc = km_numc_arg.getValue();
	params.km_numt = km_numt_arg.getValue();
	params.com_thresh = com_thresh_arg.getValue();
	params.com_winsize = com_winsize_arg.getValue();
	params.com_add_type = com_add_type_arg.getValue();
	params.com_calc_type = com_calc_type_arg.getValue();
	params.im_scale_by = im_scale_by_arg.getValue();
	params.gc_iter = gc_iter_arg.getValue();
	params.doScore = groundtruth_arg.isSet();
	params.relable_type = relable_type_arg.getValue();
	params.doPositionInKM = position_in_km_arg.getValue();
	params.doInitStep = initialization_step_arg.getValue();
	params.num_cut_backp_iters = num_iters_arg.getValue();
	btm_wait_time = btm_wait_time_arg.getValue();
	params.do_alpha_matt = do_alphamatt_arg.getValue();
	params.alpha_matt_dilate_size = alpha_matt_dilate_arg.getValue();
	params.use_hist_match_hs = use_hist_match_hs_arg.getValue();
	params.use_hist_match_rgb = use_hist_match_rgb_arg.getValue();
	params.use_overlay = use_overlay_arg.getValue();
	params.use_warp_affine = use_warp_affine_arg.getValue();
	params.use_warp_rigid = use_warp_rigid_arg.getValue();
	params.use_double_warp = use_double_warp_arg.getValue();

	}catch (ArgException &e)  // catch any exceptions
	{ cerr << "error: " << e.error() << " for arg " << e.argId() << endl; scanf("press any key...\n"); }
}
*/

int head_extract_main(int argc, char** argv) {
	VIRTUAL_SURGEON_PARAMS params;

	ParseParams(params,argc,argv);

	Mat im;

	if(params.filename.substr(0,6).compare("http://")) {
		FaceDotComDetection(params,im);
	} else {
		im = imread(params.filename);

		params.li = Point(253*0.4122,338*0.2624);
		params.ri = Point(253*0.5129,338*0.2603);
	}

	return ExtractHead(im,params);
}

int ExtractHead(Mat& im, VIRTUAL_SURGEON_PARAMS& params) {
	btm_wait_time = params.wait_time;

#ifdef BTM_DEBUG
	namedWindow("tmp");
	namedWindow("tmp1");
#endif

	//Point li(102,111),ri(144,111);
	//Point li(102,109),ri(147,109);//angeline
	
	Point midp((int)(((double)params.li.x+(double)params.ri.x)/2.0),(int)(((double)params.li.y+(double)params.ri.y)/2.0));
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
			((double)(params.li.x+params.ri.x))/2.0 - li_ri * params.yaw * 2.0,
			((double)(params.li.y+params.ri.y))/2.0 + (int)(li_ri * (double)im.cols / 3.0)
		);
	ellipse(maskFace,
		faceEllipse,
		//midp,
		Size((int)floor((double)(im.cols) * li_ri * 1 - li_ri * params.yaw * 2.0),(int)floor(((double)im.cols) * li_ri * 1.38)),
		-params.roll,	//angle
		0.0,	//start angle
		360.0,	//end angle
		Scalar(255),CV_FILLED);

	face_grab_cut(im,maskFace,1,170.0*li_ri);

	//------------ hair segmentation --------------
	{
	Mat hairMask = Mat::zeros(im.rows,im.cols,CV_8UC1);
	Point hairEllipse(
			(int)floor((double)midp.x - li_ri * 6.0 * params.yaw),
			(int)floor((double)midp.y - li_ri * 17.0)
			);
	ellipse(hairMask,
		//Point((li.x+ri.x)/2.0,(li.y+ri.y)/2.0 + (int)(li_ri * (double)im.rows / 3.0)),
		hairEllipse,
		Size((int)floor((double)im.cols * li_ri * 1.38),(int)floor((double)im.cols * li_ri * 1.9)),
		-params.roll,	//angle
		0.0,	//start angle
		360.0,	//end angle
		Scalar(255),CV_FILLED);
	hairMask = hairMask - maskFace;

#ifdef BTM_DEBUG
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
#endif

	//make gabors
#ifdef BTM_DEBUG
	namedWindow("gabor");
#endif
	vector<Mat> filter_bank;
	make_gabor_bank(filter_bank,params.gb_size,params.gb_sig,params.gb_nstds,params.gb_freq,params.gb_phase,params.gb_gamma);

	Mat im_small;
	resize(im,im_small,Size((int)floor((double)im.cols/params.im_scale_by),
							(int)floor((double)im.rows/params.im_scale_by)));

	Mat gray;
	cvtColor(im_small,gray,CV_RGB2GRAY);
#ifdef BTM_DEBUG
	{
		Mat _col;
		cvtColor(gray,_col,CV_GRAY2RGB);
		circle(_col,Point(params.li.x/params.im_scale_by,params.li.y/params.im_scale_by),2,Scalar(255,0,0),CV_FILLED);
		circle(_col,Point(params.ri.x/params.im_scale_by,params.ri.y/params.im_scale_by),2,Scalar(255,0,0),CV_FILLED);
		imshow("tmp",_col);
		waitKey(params.wait_time);
	}
#endif
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
//#ifdef BTM_DEBUG
//		imshow("gabor",filter8bit);
//#endif
		//filter2D(gray,tmp,-1,filter_bank[i]);
		matchTemplate(gray,/*filter_bank[i]*/filter8bit,tmp,CV_TM_CCOEFF_NORMED);

		for(int ii=0;ii<featVecLength;ii++) {
			((float*)(featureVec.data + ii*featureVec.step))[i+_off] = (float)((gray.data + ii)[0]) / 255.0f;
		}
//#ifdef BTM_DEBUG
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
#ifdef BTM_DEBUG
	Mat __tmp1 = __tmp * 255.0 / (double)params.km_numc;
	__tmp1.convertTo(tmp,CV_8UC1);

	imshow("tmp",tmp);
	int c = waitKey(params.wait_time);
	if(c=='q') return 0;
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

#ifdef BTM_DEBUG
		imshow("gabor",backProjD);
#endif
		
		Mat backProj = (backProjD < params.com_thresh);

#ifdef BTM_DEBUG
		imshow("tmp",backProj);
		waitKey(params.wait_time);
#endif
		resize(backProj,hairMask,hairMask.size());

		hairMask = hairMask & ~maskFace;

		face_grab_cut(im,hairMask,params.gc_iter,18);
#ifdef BTM_DEBUG
		imshow("tmp",hairMask);
		waitKey(params.wait_time);
#endif
	}

	Mat backMask = Mat(hairMask.size(),CV_8UC1,255); // - hairMask - maskFace;

#ifdef BTM_DEBUG
	imshow("tmp",maskFace);
	imshow("gabor",hairMask);
	imshow("tmp1",backMask);
	waitKey(params.wait_time);
#endif

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
	getSobels(gray,grayInt,grayInt1);
	int num_lables = 3;
	GCoptimizationGridGraph gc(im_small.cols,im_small.rows,num_lables);

	int score[9] = {0, 35, 35,
					35, 0, 35,
					35,35, 0};
	//Mat Sc = 15 * (Mat::ones(num_lables,num_lables,CV_32SC1) - Mat::eye(num_lables,num_lables,CV_32SC1));
	gc.setSmoothCostVH((int*)(/*Sc.data*/score),(int*)grayInt.data,(int*)grayInt1.data);
	//gc.setSmoothCost((int*)(Sc.data));

	Mat bias(im_small.rows,im_small.cols,CV_64FC1),bias1;
	create2DGaussian(bias,li_ri*im_small.cols*2.3,li_ri*im_small.cols*2.7,
		Point(hairEllipse.x/params.im_scale_by,hairEllipse.y/params.im_scale_by));
	//bias.copyTo(bias1);
	bias = max(bias,Mat(bias.rows,bias.cols,CV_64FC1,Scalar(0.5)));
	bias1 = -bias + 1.0;

	for(int _i=0;_i<params.num_cut_backp_iters;_i++) {
		vector<Mat> backP(3),_tmp(3);

		//calc histogram from mask
		calcHistogramWithMask(histA,tmp,maskA,(float)params.km_numc,params.com_winsize,CV_COMP_INTERSECT,backP);

		//for(int m=0;m<3;m++) {
		//	Mat cov,mean;

		//	//normalize backprojection
		//	double minv = -1.0,maxv = 1.0;
		//	minMaxLoc(backP[m],&minv,&maxv);
		//	backP[m] = (backP[m] - minv) / (maxv - minv);

		//	//minMaxLoc(_tmp[m],&minv,&maxv);
		//	//_tmp[m] = (_tmp[m] - minv) / (maxv - minv);

		//	//calcCovarMatrix(_tmp[m],cov,mean,CV_COVAR_NORMAL+CV_COVAR_ROWS);
		//	//for(int __i=0;__i<mean.cols;__i++) cout << mean.at<double>(0,__i) << ",\t";
		//	//cout << endl;
		//	//for(int __x=0;__x<cov.cols;__x++) {
		//	//	cout << endl;
		//	//	for(int __y=0;__y<cov.rows;__y++)
		//	//		cout << cov.at<double>(__y,__x) << ",\t";
		//	//}
		//	//waitKey();
		//}
		backP[0] = backP[0].mul(bias);
		backP[1] = backP[1].mul(bias);
		backP[2] = backP[2].mul(bias1);
#ifdef BTM_DEBUG
		imshow("tmp",backP[0]);
		imshow("tmp1",backP[1]);
		imshow("gabor",backP[2]);
		waitKey(params.wait_time);
#endif

		if(params.relable_type == RELABLE_HIST_MAX) {
			NaiveRelabeling(tmp.size(),backP,maskA);
		} else {
			//re-labeling using graphcut
			double _a = log(1.3);
			for(int _x=0;_x<tmp.cols;_x++) {
				for(int _y=0;_y<tmp.rows;_y++) {
					double bp0 = backP[0].at<double>(_y,_x);
					double bp1 = backP[1].at<double>(_y,_x);
					double bp2 = backP[2].at<double>(_y,_x);

					double sum = bp0+bp1+bp2;
					double _bp0 = (bp0+EPSILON)/sum;
					double _bp1 = (bp1+EPSILON)/sum;
					double _bp2 = (bp2+EPSILON)/sum;

					bp0 = _bp0*(1-_bp1)*(1-_bp2);
					bp1 = _bp1*(1-_bp0)*(1-_bp2);
					bp2 = _bp2*(1-_bp1)*(1-_bp0);

					bp0 = -log(bp0)/_a;
					bp1 = -log(bp1)/_a;
					bp2 = -log(bp2)/_a;

					gc.setDataCost(_y * im_small.cols + _x,0,(int)bp0);
					gc.setDataCost(_y * im_small.cols + _x,1,(int)bp1);
					gc.setDataCost(_y * im_small.cols + _x,2,(int)bp2);
				}
			}

			printf("\nBefore optimization energy is %d",gc.compute_energy());
			gc.expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
			printf("\nAfter optimization energy is %d",gc.compute_energy());

			Mat lables(im_small.cols*im_small.rows,1,CV_32SC1);
			for ( int  i = 0; i < lables.rows; i++ )
				((int*)(lables.data + lables.step * i))[0] = gc.whatLabel(i);

#ifdef BTM_DEBUG
			{
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
#endif

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
			maskA[0] = maskA[0] & _combinedHairAndFaceMask;
			//maskA[1] = maskA[1] & _combinedHairAndFaceMask;
			takeBiggestCC(maskA[1],bias);
			}

			//back mask is derived from hair and face
			maskA[2] = Mat(maskA[2].rows,maskA[2].cols,CV_8UC1,Scalar(255)); // - maskA[0] - maskA[1];

#ifdef BTM_DEBUG
			{
				Mat _tmp;
				vector<Mat> v; for(int _ii=0;_ii<3;_ii++) v.push_back(maskA[_ii]);
				cv::merge(v,_tmp);
				imshow("tmp",_tmp);
				waitKey(params.wait_time);
			}
#endif
			//gc->setSmoothCost((GCoptimization::EnergyType*)0);
			//delete gc;
		}
#ifdef BTM_DEBUG
		imshow("tmp",_it_faceMask);
		imshow("gabor",_it_hairMask);
		imshow("tmp1",_it_backMask);
		waitKey(params.wait_time);
#endif
	}

	resize(_it_faceMask,maskFace,maskFace.size(),0,0,INTER_NEAREST);
	resize(_it_hairMask,hairMask,hairMask.size(),0,0,INTER_NEAREST);
	//resize(_it_backMask,backMask,backMask.size(),0,0,INTER_NEAREST);

	} //end iterative proccess


	if(params.doScore) {
		Mat gt_im = imread(params.groundtruth,0);
		int groundTruth_count = countNonZero(gt_im);
		gt_im = (gt_im ^ hairMask);
#ifdef BTM_DEBUG
		imshow("gabor",gt_im);
#endif
		//double segscore = sum(gt_im > 0).val[0];
		int xor_count = countNonZero(gt_im);
		double segscore = (double)xor_count / (double)groundTruth_count;
		cout << "seg score: " << segscore << "(" << xor_count << "/" << groundTruth_count << ")" << endl;

	#ifdef BTM_DEBUG
		waitKey(params.wait_time);
	#endif
	}

	maskFace = maskFace | hairMask;

	takeBiggestCC(maskFace);

	face_grab_cut(im,maskFace,2,10);

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
		matting->Solve();

		Mat(matting->alphamap).copyTo(maskFace);

		//maskFace.convertTo(maskFace,CV_8UC1,255);

		delete matting;
	}

	}

//#ifdef BTM_DEBUG
	imshow("tmp",maskFace);
	
	Mat imMasked;
	{
		if(maskFace.type()!=CV_32FC1) 
			maskFace.convertTo(maskFace,CV_32FC1,1.0/255.0);

		Mat unMask = Mat::ones(maskFace.size(),CV_32FC1) - maskFace; //Mat(maskFace.size(),CV_8UC1,Scalar(255)) - maskFace;
		imshow("gabor",unMask);

		vector<Mat> v;
		im.convertTo(im,CV_32FC3,1.0/255.0);
		split(im,v);
		v[0] = v[0].mul(maskFace) + (unMask);
		v[1] = v[1].mul(maskFace);
		v[2] = v[2].mul(maskFace);
		cv::merge(v,imMasked);
	}
	imshow("tmp1",imMasked);
	waitKey(params.wait_time);
//#endif

	return 0;
}
