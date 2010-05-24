#pragma once

#include "cv.h"

using namespace cv;

#define _PI 3.14159265

typedef struct VirtualSurgeonParams {
	//algorithm data
	double gb_sig;
	double gb_freq;
	double gb_phase;
	double gb_gamma;
	int gb_nstds;
	int gb_size;
	int km_numc;
	int com_winsize;
	double com_thresh;
	string filename;
	string groundtruth;
	int com_add_type;
	int com_calc_type;
	double im_scale_by;
	int gc_iter;
	int km_numt;
	bool doScore;
	int relable_type;
	bool doPositionInKM;
	bool doInitStep;
	int num_cut_backp_iters;
	bool do_alpha_matt;
	int alpha_matt_dilate_size;

	bool use_hist_match_hs;
	bool use_hist_match_rgb;
	bool use_overlay;
	bool use_warp_rigid;
	bool use_warp_affine;
	bool use_double_warp;

	int wait_time;

	//face data
	Point li,ri;
	double yaw;
	double roll;
	double pitch;
} VIRTUAL_SURGEON_PARAMS;


void ParseParams(VIRTUAL_SURGEON_PARAMS& params, int argc, char** argv);
void FaceDotComDetection(VIRTUAL_SURGEON_PARAMS& params, Mat& im);
void PrintParams(VIRTUAL_SURGEON_PARAMS& p);