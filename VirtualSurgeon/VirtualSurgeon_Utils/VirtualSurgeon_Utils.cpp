// VirtualSurgeon_Utils.cpp : Defines the entry point for the console application.
//

#include "VirtualSurgeon_Utils.h"

#include "highgui.h"
using namespace cv;

#include <iostream>
#include <fstream>
using namespace std;

#include "sys/types.h"
#include "sys/stat.h"

#include "../tclap-1.2.0/include/tclap/CmdLine.h"
using namespace TCLAP;

int curl_get(std::string& s, std::string& _s = std::string(""));

// Define the command line object.
CmdLine cmd("Command description message", ' ', "0.9");
bool cmd_initialized = false;
UnlabeledValueArg<string> filename_arg("filename","file to work on",true,"","string",cmd);
UnlabeledValueArg<string> groundtruth_arg("groundtruth","ground truth file",false,"","string",cmd);
ValueArg<double> gb_freq_arg("f","gabor-freq","Gabor func frequency",false,0.15,"float",cmd);
ValueArg<double> gb_sig_arg("s","gabor-sigma","Gabor func sigma",false,1.0,"float",cmd);
ValueArg<double> gb_phase_arg("p","gabor-phase","Gabor func phase",false,_PI/2.0,"float",cmd);
ValueArg<double> gb_gamma_arg("g","gabor-gamma","Gabor func gamma",false,1.0,"float",cmd);
ValueArg<int> gb_nstds_arg("n","gabor-nstds","Gabor func number of std devs",false,3,"int",cmd);
ValueArg<int> gb_size_arg("z","gabor-size","Gabor filter bank size",false,2,"int",cmd);
ValueArg<int> km_numc_arg("c","kmeans-num-centers","K-Means number of clusters",false,10,"int",cmd);
ValueArg<int> km_numt_arg("m","kmeans-num-tries","K-Means number of tries",false,2,"int",cmd);
ValueArg<int> com_winsize_arg("w","combine-win-size","Hist combine window size",false,5,"int",cmd);
ValueArg<double> com_thresh_arg("t","combine-threshold","Hist combine threshold",false,0.15,"float",cmd);
ValueArg<int> com_add_type_arg("y","combine-add-type","Hist combine scores add type (0=L2, 1=MAX, 2=MAX+, 3=just +)",false,2,"int [0-3]",cmd);
ValueArg<int> com_calc_type_arg("l","combine-type","Hist combine scores calc type (0=COREL, 1=CHISQR, 2=INTERSECT, 3=BAHAT.)",false,0,"int [0-3]",cmd);
ValueArg<double> im_scale_by_arg("b","image-scale-by","Scale down image by factor",false,2.0,"float",cmd);
ValueArg<int> gc_iter_arg("r","grabcut-iterations","Number of grabcut iterations",false,1,"int",cmd);
ValueArg<int> relable_type_arg("e","relable-type","Type of relabeling in iterative procecess (0 = hist max, 1 = graph cut)",false,1,"int",cmd);
SwitchArg position_in_km_arg("q","position-in-km","Include position in K-Means?",cmd,false);
SwitchArg initialization_step_arg("a","initialization_step","Do initialization step?",cmd,false);
ValueArg<int> num_iters_arg("x","num-iters","Number of cut-backp iterations",false,3,"int",cmd);
ValueArg<int> btm_wait_time_arg("d","wait-time","Time in msec to wait on debug pauses",false,1,"int",cmd);
SwitchArg do_alphamatt_arg("u","do-alpha-matting","Do alpha matting?",cmd,false);
ValueArg<int> alpha_matt_dilate_arg("i","alpha-matt-dilate-size","Size in pixels to dilate mask for alpha matting",false,10,"int",cmd);
SwitchArg use_hist_match_hs_arg(string(),"use-hist-match-hs","Use histogram matching over HS space for recoloring?",cmd,false);
SwitchArg use_hist_match_rgb_arg(string(),"use-hist-match-rgb","Use histogram matching over RGB space for recoloring?",cmd,false);
SwitchArg use_overlay_arg(string(),"use-overlay","Use overlay for recoloring?",cmd,false);
SwitchArg use_warp_rigid_arg(string(),"use-warp-rigid","Use rigid warp for neck warping?",cmd,false);
SwitchArg use_warp_affine_arg(string(),"use-warp-affine","Use affine warp for neck warping?",cmd,false);
SwitchArg use_double_warp_arg(string(),"use-double-warp","Use 2-way warping?",cmd,false);

void ParseParams(VIRTUAL_SURGEON_PARAMS& params, int argc, char** argv) {
	params.gb_sig = 1.0;
	params.gb_freq = 0.15;
	params.gb_phase = _PI/2.0;
	params.gb_gamma = 1.0;
	params.gb_nstds = 3;
	params.gb_size = 2;
	params.km_numc = 10;
	params.com_winsize = 5;
	params.com_thresh = 0.25;
	params.com_add_type = 0;
	params.com_calc_type = 1;
	params.im_scale_by = 2.0;
	params.gc_iter = 1;
	params.km_numt = 2;
	params.doScore = false;
	params.relable_type = 1;
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

void PrintParams(VIRTUAL_SURGEON_PARAMS& p) {
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
void FaceDotComDetection(VIRTUAL_SURGEON_PARAMS& params, Mat& im) {
	struct stat f__stat;

	int indexofslash = params.filename.find_last_of("/");
	int indexofqmark = params.filename.find_last_of("?");
	string img_filename = params.filename.substr(
		indexofslash+1,
		(indexofqmark>0)?indexofqmark-indexofslash-1:params.filename.length()-1
		);

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
		ifs >> line;
		ifs.close();

		//cout << line;
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
