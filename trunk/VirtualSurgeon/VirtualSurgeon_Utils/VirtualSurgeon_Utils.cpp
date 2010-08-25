// VirtualSurgeon_Utils.cpp : Defines the entry point for the console application.
//

#include "VirtualSurgeon_Utils.h"

#include "cv.h"
#include "highgui.h"
using namespace cv;

#include <iostream>
#include <fstream>
using namespace std;

#include "sys/types.h"
#include "sys/stat.h"

#include "../tclap-1.2.0/include/tclap/CmdLine.h"
using namespace TCLAP;

#include "../BeTheModel/util.h"
#include "../BeTheModel/ImageEditingUtils.h"
#include "OpenCV2ImageWrapper.h"

int curl_get(std::string& s, std::string& _s = std::string(""));
int btm_wait_time = 0;

namespace VirtualSurgeon {

// Define the command line object.
CmdLine cmd("Command description message", ' ', "0.9");
bool cmd_initialized = false;
UnlabeledValueArg<string> filename_arg("filename","file to work on",true,"","string",cmd);
UnlabeledValueArg<string> groundtruth_arg("groundtruth","ground truth file",false,"","string",cmd);

ValueArg<double> gb_freq_arg(		"f","gabor-freq","Gabor func frequency",false,0.15,"float",cmd);
ValueArg<double> gb_sig_arg(		"s","gabor-sigma","Gabor func sigma",false,1.0,"float",cmd);
ValueArg<double> gb_phase_arg(		"p","gabor-phase","Gabor func phase",false,_PI/2.0,"float",cmd);
ValueArg<double> gb_gamma_arg(		"g","gabor-gamma","Gabor func gamma",false,1.0,"float",cmd);
ValueArg<int> gb_nstds_arg(			"n","gabor-nstds","Gabor func number of std devs",false,3,"int",cmd);
ValueArg<int> gb_size_arg(			"z","gabor-size","Gabor filter bank size",false,2,"int",cmd);
ValueArg<int> km_numc_arg(			"c","kmeans-num-centers","K-Means number of clusters",false,10,"int",cmd);
ValueArg<int> km_numt_arg(			"m","kmeans-num-tries","K-Means number of tries",false,2,"int",cmd);
ValueArg<int> com_winsize_arg(		"w","combine-win-size","Hist combine window size",false,5,"int",cmd);
ValueArg<double> com_thresh_arg(	"t","combine-threshold","Hist combine threshold",false,0.15,"float",cmd);
ValueArg<int> com_add_type_arg(		"y","combine-add-type","Hist combine scores add type (0=L2, 1=MAX, 2=MAX+, 3=just +)",false,2,"int [0-3]",cmd);
ValueArg<int> com_calc_type_arg(	"l","combine-type","Hist combine scores calc type (0=COREL, 1=CHISQR, 2=INTERSECT, 3=BAHAT.)",false,0,"int [0-3]",cmd);
ValueArg<double> im_scale_by_arg(	"b","image-scale-by","Scale down image by factor",false,2.0,"float",cmd);
ValueArg<int> gc_iter_arg(			"r","grabcut-iterations","Number of grabcut iterations",false,1,"int",cmd);
ValueArg<int> relable_type_arg(		"e","relable-type","Type of relabeling in iterative procecess (0 = hist max, 1 = graph cut)",false,1,"int",cmd);
SwitchArg position_in_km_arg(		"q","position-in-km","Include position in K-Means?",cmd,false);
SwitchArg initialization_step_arg(	"a","initialization_step","Do initialization step?",cmd,false);
ValueArg<int> num_iters_arg(		"x","num-iters","Number of cut-backp iterations",false,3,"int",cmd);
ValueArg<int> btm_wait_time_arg(	"d","wait-time","Time in msec to wait on debug pauses",false,1,"int",cmd);
SwitchArg do_alphamatt_arg(			"u","do-alpha-matting","Do alpha matting?",cmd,false);
ValueArg<int> alpha_matt_dilate_arg("i","alpha-matt-dilate-size","Size in pixels to dilate mask for alpha matting",false,10,"int",cmd);

SwitchArg use_hist_match_hs_arg(string(),"use-hist-match-hs","Use histogram matching over HS space for recoloring?",cmd,false);
SwitchArg use_hist_match_rgb_arg(string(),"use-hist-match-rgb","Use histogram matching over RGB space for recoloring?",cmd,false);
SwitchArg use_overlay_arg(string(),"use-overlay","Use overlay for recoloring?",cmd,false);
SwitchArg use_warp_rigid_arg(string(),"use-warp-rigid","Use rigid warp for neck warping?",cmd,false);
SwitchArg use_warp_affine_arg(string(),"use-warp-affine","Use affine warp for neck warping?",cmd,false);
SwitchArg use_double_warp_arg(string(),"use-double-warp","Use 2-way warping?",cmd,false);

void VirtualSurgeonParams::InitializeDefault() {
	VirtualSurgeonParams& params = *this;
	params.gb_sig = 1.0;
	params.gb_freq = 0.15;
	params.gb_phase = _PI/2.0;
	params.gb_gamma = 1.0;
	params.gb_nstds = 3;
	params.gb_size = 2;
	params.km_numc = 20;
	params.com_winsize = 5;
	params.com_thresh = 0.25;
	params.com_add_type = 0;
	params.com_calc_type = 1;
	params.im_scale_by = 1;
	params.gc_iter = 0;
	params.km_numt = 1;
	params.doScore = false;
	params.relable_type = 1;
	params.doPositionInKM = false;
	params.doInitStep = false;
	params.num_cut_backp_iters = 2;
	params.do_alpha_matt = false;
	params.alpha_matt_dilate_size = 5;
	params.use_hist_match_hs = false;
	params.use_hist_match_rgb = false;
	params.use_overlay = false;
	params.use_warp_rigid = false;
	params.use_warp_affine = false;
	params.use_double_warp = false;	
	params.hair_ellipse_size_mult = 1;
	params.do_eq_hist = false;
	params.consider_pixel_neighbourhood = false;
	params.do_two_segments = false;
	params.do_kmeans = false;
	params.head_mask_size_mult = 1.0;
	params.num_DoG = 2;
	params.do_two_back_kernels = false;

	params.poisson_cloning_band_size = 4;

	params.snake_snap_weight_edge = 5000.0;
	params.snake_snap_weight_direction = 12.0;
	params.snake_snap_weight_consistency = 12.0;
	params.snake_snap_edge_len_thresh = 200;
	params.snale_snap_total_width_coeff = 10;

	params.no_gui = false;
	params.wait_time = 1;
}

void VirtualSurgeonParams::ParseParams(int argc, char** argv) {
	VirtualSurgeonParams& params = *this;

	InitializeDefault();

	try {  

	cmd.reset();

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
	params.wait_time = btm_wait_time_arg.getValue();
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

void VirtualSurgeonParams::PrintParams() {
	VirtualSurgeonParams& p = *this;

	cout<<"file to work on: "<<p.filename<<endl;
	cout<<"ground truth file: "<<p.groundtruth<<endl;
	cout<<"Gabor func frequency: "<<p.gb_freq<<endl;
	cout<<"Gabor func sigma: "<<p.gb_sig<<endl;
	cout<<"Gabor func phase: "<<p.gb_phase<<endl;
	cout<<"Gabor func gamma: "<<p.gb_gamma<<endl;
	cout<<"Gabor func number of std devs: "<<p.gb_nstds<<endl;
	cout<<"Gabor filter bank size: "<<p.gb_size;
	cout<<"K-Means number of clusters: "<<p.km_numc<<endl;
	cout<<"K-Means number of tries: "<<p.km_numt<<endl;
	cout<<"Hist combine window size: "<<p.com_winsize<<endl;
	cout<<"Hist combine threshold: "<<p.com_thresh<<endl;
	cout<<"Hist combine scores add type (0=L2, 1=MAX, 2=MAX+, 3=just +): "<<p.com_add_type<<endl;
	cout<<"Hist combine scores calc type (0=COREL, 1=CHISQR, 2=INTERSECT, 3=BAHAT.): "<<p.com_calc_type<<endl;
	cout<<"Scale down image by factor: "<<p.im_scale_by<<endl;
	cout<<"Number of grabcut iterations: "<<p.gc_iter<<endl;
	cout<<"Type of relabeling in iterative procecess (0 = hist max, 1 = graph cut): "<<p.relable_type<<endl;
	cout<<"Include position in K-Means?: "<<p.doPositionInKM<<endl;
	cout<<"Do initialization step?: "<<p.doInitStep<<endl;
	cout<<"Number of cut-backp iterations: "<<p.num_cut_backp_iters<<endl;
	cout<<"Time in msec to wait on debug pauses: "<<p.wait_time<<endl;
	cout<<"Do alpha matting?: "<<p.do_alpha_matt<<endl;
	cout<<"Size in pixels to dilate mask for alpha matting: "<<p.alpha_matt_dilate_size<<endl;
	cout<<"Use histogram matching over HS space for recoloring?: "<<p.use_hist_match_hs<<endl;
	cout<<"Use histogram matching over RGB space for recoloring?: "<<p.use_hist_match_rgb<<endl;
	cout<<"Use overlay for recoloring?: "<<p.use_overlay<<endl;
	cout<<"Use rigid warp for neck warping?: "<<p.use_warp_rigid<<endl;
	cout<<"Use affine warp for neck warping?: "<<p.use_warp_affine<<endl;
	cout<<"Use 2-way warping?: "<<p.use_double_warp<<endl;
	cout<<"Use only 2 segments for segmentation?"<<p.do_two_segments<<endl;

	cout<<"Face:"<<endl<<"------"<<endl;
	cout<<"Left eye: "<<p.li.x<<","<<p.li.y<<endl;
	cout<<"Right eye: "<<p.ri.x<<","<<p.ri.y<<endl;
	cout<<"Pitch: "<<p.pitch<<endl;
	cout<<"Roll: "<<p.roll<<endl;
	cout<<"Yaw: "<<p.yaw<<endl;

}

/**
This function goes to Face.com APIs (using CURL) to get the facial features of the face in the image.
It will take out the image URL from params.filename and fill in the ri,li,yaw,roll,pitch params, an also
load the image into the im argument
**/
void VirtualSurgeonFaceData::FaceDotComDetection(Mat& im) {
	VirtualSurgeonFaceData& params = *this;

	struct stat f__stat;

	int indexofslash = params.filename.find_last_of("/");
	int indexofqmark = params.filename.find_last_of("?");
	string img_filename = params.filename;
	if(indexofslash >= 0) {
		img_filename = params.filename.substr(
			indexofslash+1,
			(indexofqmark>0)?indexofqmark-indexofslash-1:params.filename.length()-1
			);
	}

	//check if already downloaded before..
	if(stat(img_filename.c_str(),&f__stat)!=0) {
		cout << "Download..."<<endl;
		if (!curl_get(params.filename,img_filename)) {
			cerr << "Cannot download picture file"<<endl;
			return;
		}
	}

	im = imread(img_filename);

	//check if already got Face.com detection before..
	string img_fn_txtext = img_filename.substr(0,img_filename.find_last_of(".")) + ".txt";
	if(stat(img_fn_txtext.c_str(),&f__stat)!=0) {
		//file with "txt" extension doesn't exist, so call Face.com API
		cout << "Get Face.com detection..."<<endl;
		string u("http://api.face.com/faces/detect.json?api_key=4bc70b36e32fc&api_secret=4bc70b36e3334&urls=");
		u += params.filename;

		curl_get(u,img_fn_txtext);
	}

	//TODO: handle bad file format, or no detection
	{
		ifstream ifs(img_fn_txtext.c_str());
		string line;
		string _line;
		while(!ifs.eof())
		{
			ifs >> _line;
			line += _line;
		}
		ifs.close();

		cout << line;

		stringstream ss(line);
		
		int p = line.find("eye_left\":{\"x\":");
		ss.seekg(p+strlen("eye_left\":{\"x\":"));
		double d;
		ss >> d;
		params.li.x = d*im.cols/100;
		p = ss.tellg();
		ss.seekg(p+5);
		ss >> d;
		params.li.y = d*im.rows/100;
		p = ss.tellg();
		ss.seekg(p+strlen("},\"eye_right\":{\"x\":"));
		ss >> d;
		params.ri.x = d*im.cols/100;
		p = ss.tellg();
		ss.seekg(p+5);
		ss >> d;
		params.ri.y = d*im.rows/100;

		ss.seekg(line.find("\"yaw\":") + strlen("\"yaw\":"));
		ss >> params.yaw;
		ss.seekg(line.find("\"roll\":") + strlen("\"roll\":"));
		ss >> params.roll;
		ss.seekg(line.find("\"pitch\":") + strlen("\"pitch\":"));
		ss >> params.pitch;
	}
}

void VirtualSurgeonParams::face_grab_cut(Mat& orig, Mat& mask, int iters, int dilate_size) {
	Mat tmpMask(mask.rows,mask.cols,CV_8UC1,Scalar(GC_BGD));

	//create "buffer" zones for probably BG and prob. FG.
	{
		Mat __tmp(mask.rows,mask.cols,CV_8UC1,Scalar(0));
		dilate(mask,__tmp,Mat::ones(dilate_size,dilate_size,CV_8UC1),Point(-1,-1),1,BORDER_REFLECT);	//probably background
		tmpMask.setTo(Scalar(GC_PR_BGD),__tmp);

		dilate(mask,__tmp,Mat::ones(dilate_size/2,dilate_size/2,CV_8UC1),Point(-1,-1),1,BORDER_REFLECT); //probably foregroung
		tmpMask.setTo(Scalar(GC_PR_FGD),__tmp);

		erode(mask,__tmp,Mat::ones(dilate_size/3,dilate_size/3,CV_8UC1),Point(-1,-1),1,BORDER_REFLECT); // foreground
		tmpMask.setTo(Scalar(GC_FGD),__tmp);
	}

	//Mat(mask).copyTo(tmpMask);
	Mat bgdModel, fgdModel;

	if(!this->no_gui) {
		Mat _tmp;
		tmpMask.convertTo(_tmp,CV_32FC1);
		_tmp = tmpMask / 4.0f * 255.0f;
		imshow("tmp",_tmp);
		waitKey(BTM_WAIT_TIME);
	}

	cout << "Do grabcut... init... ";

	Rect mr = find_bounding_rect_of_mask(&((IplImage)mask));
	//initialize
	grabCut(
		orig,
		tmpMask,
		mr,
		bgdModel,
		fgdModel,
		1, GC_INIT_WITH_MASK);
#ifdef BTM_DEBUG
	cout << "run... ";
#endif
	for(int i=0;i<iters;i++) {
		//run one iteration
		grabCut(
			orig,
			tmpMask,
			mr,
			bgdModel,
			fgdModel,
			1);
#ifdef BTM_DEBUG
		cout << ".";
#endif
	}

	//cvShowImage("result",image);
	//cvCopy(__GCtmp,mask);
	//Mat(mask).setTo(Scalar(255),tmpMask);
	//cvSet(mask,cvScalar(255),&((IplImage)tmpMask));
	Mat __tm = tmpMask & GC_FGD;
	__tm.setTo(Scalar(255),__tm);
	__tm.copyTo(Mat(mask));

	if(!this->no_gui) {
		cout << "Done" << endl;
		imshow("tmp",mask);
		waitKey(BTM_WAIT_TIME);
	}

}

void FindBoundingRect(Rect& faceRect, Mat* headMask) {
	Mat arow(1,headMask->cols,CV_32SC1);
	for(int i=0;i<headMask->cols;i++) {
		((int*)arow.data)[i] = i;
	}
	Mat allrows; repeat(arow,headMask->rows,1,allrows);
	Mat rows; allrows.copyTo(rows,(*headMask > 0));
	rows.setTo(Scalar(headMask->cols/2),(*headMask == 0));
	double minv,maxv;
	minMaxLoc(rows,&minv,&maxv);

	faceRect.x = minv;
	faceRect.width = maxv - minv;

	Mat acol(headMask->rows,1,CV_32SC1);
	for(int i=0;i<headMask->rows;i++) {
		((int*)acol.data)[i] = i;
	}
	Mat allcols; repeat(acol,1,headMask->cols,allcols);
	Mat cols; allcols.copyTo(cols,(*headMask > 0));
	cols.setTo(Scalar(headMask->rows/2),(*headMask == 0));
	minMaxLoc(cols,&minv,&maxv);

	faceRect.y = minv;
	faceRect.height = maxv - minv;
}

void takeBiggestCC(Mat& mask, Mat& bias) {
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

void VirtualSurgeonParams::PoissonImageEditing(Mat& back, Mat& _backMask, Mat& front, Mat& _frontMask, bool doLaplacian = true) {
	Mat backMask,frontMask;

	if(_backMask.type() != CV_8UC1)
		_backMask.convertTo(backMask,CV_8UC1,255.0);
	else
		backMask = _backMask;

	if(_frontMask.type() != CV_8UC1)
		_frontMask.convertTo(frontMask,CV_8UC1,255.0);
	else
		frontMask = _frontMask;

	assert(backMask.type() == frontMask.type());
	assert(backMask.type() == CV_8UC1);

	Mat _mask = backMask & frontMask;
	Rect bound; FindBoundingRect(bound,&_mask);
	bound.x = MAX(bound.x - this->poisson_cloning_band_size,0); 
	bound.y = MAX(bound.y - this->poisson_cloning_band_size,0); 
	bound.width = MIN(bound.width + this->poisson_cloning_band_size*2, _mask.cols - bound.x); 
	bound.height = MIN(bound.height + this->poisson_cloning_band_size*2, _mask.rows - bound.y);
	Mat mask; _mask(bound).copyTo(mask);

	assert(mask.type() == CV_8UC1);
	
	assert(back.channels() == 3);

	Mat back_64f; 
	//if(back.type() == CV_8UC3) {
	//	cout << "back.type() == CV_8UC3"<<endl;
		back(bound).convertTo(back_64f,CV_64FC3,1.0/255.0);
	//} else if(back.type() == CV_32FC3) {
	//	cout << "back.type() == CV_32FC3"<<endl;
	//	back(bound).convertTo(back_64f,CV_64FC3,1.0/255.0);
	//} else if(back.type() == CV_64FC3) {
	//	cout << "back.type() == CV_64FC3"<<endl;
	//	back(bound).copyTo(back_64f);
	//}

	if(!this->no_gui) {
		imshow("tmp",back_64f);
		waitKey(this->wait_time);
	}

	Mat front_64f; 
	//if(front.type() == CV_8UC3)
		front(bound).convertTo(front_64f,CV_64FC3,1.0/255.0);
	//else
	//	front(bound).convertTo(front_64f,CV_64FC3);

	if(!this->no_gui) {
		imshow("tmp",front_64f);
		waitKey(this->wait_time);
	}
	Laplacian(front_64f,front_64f,-1);
	//Mat lap_back_64; Laplacian(back_64f,lap_back_64,-1);

	vector<Mat> v_b(3), v_f(3);//, v_b_lap(3); 
	split(back_64f,v_b); 
	split(front_64f,v_f);
	//split(lap_back_64,v_b_lap);
	
	int n = back_64f.cols;
	int m = back_64f.rows;
	int mn = m*n;
	
	if(!this->no_gui) {
		imshow("tmp",backMask);
		waitKey(this->wait_time);
		imshow("tmp",frontMask);
		waitKey(this->wait_time);
		imshow("tmp",mask);
		waitKey(this->wait_time);
	}

	gmm::row_matrix< gmm::rsvector<double> > M(mn,mn);
	//Mat eroded; dilate(mask,eroded,Mat::ones(3,3,CV_64FC1));
	OpenCV2ImageWrapper<uchar> maskImage(mask);
	ImageEditingUtils::matrixCreate(M, n, mn, maskImage);

	vector<double > solutionVectors(mn);
	vector<double> v_color(mn); 
	Mat v_c_mat(back.size(),CV_64FC1);

	for(int color = 0; color < 3; color++) {
		v_c_mat.setTo(Scalar(0));
		solutionVectors.assign(mn,0.0);
		//v_color.assign(mn,0.0);

		if(doLaplacian)
			v_f[color].copyTo(v_c_mat,mask);

		//Mat tmp = v_b_lap[c] > v_c_mat;
		//v_b_lap[c].copyTo(v_c_mat,tmp);

		v_b[color].copyTo(v_c_mat,~mask);

		if(!this->no_gui) {
			imshow("tmp",v_c_mat);
			waitKey(this->wait_time);
		}

		v_c_mat.reshape(1,mn).copyTo(Mat(v_color));

		ImageEditingUtils::solveLinear(M,solutionVectors,v_color);

		Mat(solutionVectors).reshape(1,m).convertTo(v_b[color],back.type(),255.0);
		cout <<"done with color " << color << endl;

		if(!this->no_gui) {
			imshow("tmp",v_b[color]);
			waitKey(this->wait_time);
		}
	}

	Mat output; cv::merge(v_b,output);
	
	if(!this->no_gui) {
		imshow("tmp",output);
		waitKey(this->wait_time);
	}
	output.copyTo(back(bound));
}

}//ns

#ifdef UTILS_MAIN
int main(int argc, char** argv) {
	/*Mat blue(200,200,CV_8UC3,Scalar(255,0,0));*/
	Mat blue(200,200,CV_8UC3); 
	//RNG& rng = theRNG();
	//rng.fill(blue,RNG::UNIFORM,Scalar(0,0,0),Scalar(256,256,256));
	Mat im = imread("../images/40406598_fd4e74d51c_d.jpg");
	im(Rect(200,200,200,200)).copyTo(blue);

	Mat red(200,200,CV_8UC3,Scalar(0,0,255));
	im = imread("../images/59600641_acd478ae71_d.jpg");
	im(Rect(200,200,200,200)).copyTo(red);

	Mat blumask = Mat::zeros(blue.size(),CV_8UC1);
	Mat redmask = Mat::zeros(blue.size(),CV_8UC1);
	circle(blumask,Point(100,75),50,Scalar(255),CV_FILLED);
	circle(redmask,Point(100,125),50,Scalar(255),CV_FILLED);

	namedWindow("tmp");
	imshow("tmp",blue);
	waitKey();
	imshow("tmp",red);
	waitKey();

	VirtualSurgeonParams p;
	p.InitializeDefaults();
	p.no_gui = false;
	p.PoissonImageEditing(red,redmask,blue,blumask);
}
#endif