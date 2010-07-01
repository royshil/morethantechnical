#pragma once

#include "cv.h"

#define _PI 3.14159265

#include <string>
using namespace std;

namespace VirtualSurgeon {

	class VirtualSurgeonFaceData {
	public:
		string filename;

		//face data
		cv::Point li,ri;
		double yaw;
		double roll;
		double pitch;

		void FaceDotComDetection(cv::Mat& im);
	};

typedef class VirtualSurgeonParams : public VirtualSurgeonFaceData {
public:
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
	double hair_ellipse_size_mult;
	bool do_eq_hist;
	bool consider_pixel_neighbourhood;

	double snake_snap_weight_edge;
	double snake_snap_weight_direction;
	double snake_snap_weight_consistency;
	int snake_snap_edge_len_thresh;
	int snale_snap_total_width_coeff;

	bool use_hist_match_hs;
	bool use_hist_match_rgb;
	bool use_overlay;
	bool use_warp_rigid;
	bool use_warp_affine;
	bool use_double_warp;

	bool no_gui;
	int wait_time;

	void InitializeDefault();
	void ParseParams(int argc, char** argv);
	void PrintParams();
	void face_grab_cut(cv::Mat& orig, cv::Mat& mask, int iters, int dilate_size = 30);

} VIRTUAL_SURGEON_PARAMS;


}//ns