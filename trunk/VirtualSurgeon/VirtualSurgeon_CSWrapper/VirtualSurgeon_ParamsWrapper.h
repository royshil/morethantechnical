#pragma once

namespace VirtualSurgeon_CSWrapper {

public ref class VirtualSurgeon_ParamsWrapper
{
public:
	property VirtualSurgeonParams* p;

	property double gb_sig {
		double get() { return p->gb_sig;}
		void set(double v) { p->gb_sig = v;}
	}

	property double gb_freq {
		double get() { return p->gb_freq;}
		void set(double v) { p->gb_freq = v;}
	}

	property double gb_phase {
		double get() { return p->gb_phase;}
		void set(double v) { p->gb_phase = v;}
	}

	property double gb_gamma {
		double get() { return p->gb_gamma;}
		void set(double v) { p->gb_gamma = v;}
	}

	property int gb_nstds {
		int get() { return p->gb_nstds;}
		void set(int v) { p->gb_nstds = v;}
	}

	property int gb_size {
		int get() { return p->gb_size;}
		void set(int v) { p->gb_size = v;}
	}

	property int km_numc {
		int get() { return p->km_numc;}
		void set(int v) { p->km_numc = v;}
	}

	property int com_winsize {
		int get() { return p->com_winsize;}
		void set(int v) { p->com_winsize = v;}
	}

	property double com_thresh {
		double get() { return p->com_thresh;}
		void set(double v) { p->com_thresh = v;}
	}


	property string groundtruth {
		string get() { return p->groundtruth;}
		void set(string v) { p->groundtruth = v;}
	}

	property int com_add_type {
		int get() { return p->com_add_type;}
		void set(int v) { p->com_add_type = v;}
	}

	property int com_calc_type {
		int get() { return p->com_calc_type;}
		void set(int v) { p->com_calc_type = v;}
	}

	property double im_scale_by {
		double get() { return p->im_scale_by;}
		void set(double v) { p->im_scale_by = v;}
	}

	property int gc_iter {
		int get() { return p->gc_iter;}
		void set(int v) { p->gc_iter = v;}
	}

	property int km_numt {
		int get() { return p->km_numt;}
		void set(int v) { p->km_numt = v;}
	}

	property bool doScore {
		bool get() { return p->doScore;}
		void set(bool v) { p->doScore = v;}
	}

	property int relable_type {
		int get() { return p->relable_type;}
		void set(int v) { p->relable_type = v;}
	}

	property bool doPositionInKM {
		bool get() { return p->doPositionInKM;}
		void set(bool v) { p->doPositionInKM = v;}
	}

	property bool doInitStep {
		bool get() { return p->doInitStep;}
		void set(bool v) { p->doInitStep = v;}
	}

	property int num_cut_backp_iters {
		int get() { return p->num_cut_backp_iters;}
		void set(int v) { p->num_cut_backp_iters = v;}
	}

	property bool do_alpha_matt {
		bool get() { return p->do_alpha_matt;}
		void set(bool v) { p->do_alpha_matt = v;}
	}

	property int alpha_matt_dilate_size {
		int get() { return p->alpha_matt_dilate_size;}
		void set(int v) { p->alpha_matt_dilate_size = v;}
	}

	property bool use_hist_match_hs {
		bool get() { return p->use_hist_match_hs;}
		void set(bool v) { p->use_hist_match_hs = v;}
	}

	property bool use_hist_match_rgb {
		bool get() { return p->use_hist_match_rgb;}
		void set(bool v) { p->use_hist_match_rgb = v;}
	}

	property bool use_overlay {
		bool get() { return p->use_overlay;}
		void set(bool v) { p->use_overlay = v;}
	}

	property bool use_warp_rigid {
		bool get() { return p->use_warp_rigid;}
		void set(bool v) { p->use_warp_rigid = v;}
	}

	property bool use_warp_affine {
		bool get() { return p->use_warp_affine;}
		void set(bool v) { p->use_warp_affine = v;}
	}

	property bool use_double_warp {
		bool get() { return p->use_double_warp;}
		void set(bool v) { p->use_double_warp = v;}
	}

	property bool no_gui {
		bool get() { return p->no_gui;}
		void set(bool v) { p->no_gui = v;}
	}

	property int wait_time {
		int get() { return p->wait_time;}
		void set(int v) { p->wait_time = v;}
	}

	property double yaw {
		double get() { return p->yaw;}
		void set(double v) { p->yaw = v;}
	}

	property double roll {
		double get() { return p->roll;}
		void set(double v) { p->roll = v;}
	}

	property double pitch {
		double get() { return p->pitch;}
		void set(double v) { p->pitch = v;}
	}

	property double weight_edge {
		double get() { return p->snake_snap_weight_edge;}
		void set(double v) { p->snake_snap_weight_edge = v;}
	}

	property double weight_direction {
		double get() { return p->snake_snap_weight_direction;}
		void set(double v) { p->snake_snap_weight_direction = v;}
	}

	property double weight_consistency {
		double get() { return p->snake_snap_weight_consistency;}
		void set(double v) { p->snake_snap_weight_consistency = v;}
	}

	property int snake_total_width_mult {
		int get() { return p->snale_snap_total_width_coeff;}
		void set(int v) { p->snale_snap_total_width_coeff = v;}
	}
	property int snake_edge_len_thresh {
		int get() { return p->snake_snap_edge_len_thresh;}
		void set(int v) { p->snake_snap_edge_len_thresh = v;}
	}
	property double hair_ellipse_size_mult {
		double get() { return p->hair_ellipse_size_mult;}
		void set(double v) { p->hair_ellipse_size_mult = v;}
	}

	property bool do_eq_hist {
		bool get() { return p->do_eq_hist;}
		void set(bool v) { p->do_eq_hist = v;}
	}

	property bool consider_pixel_neighbourhood {
		bool get() { return p->consider_pixel_neighbourhood;}
		void set(bool v) { p->consider_pixel_neighbourhood = v;}
	}

	property bool do_two_segments {
		bool get() { return p->do_two_segments;}
		void set(bool v) { p->do_two_segments = v;}
	}
	property int poisson_cloning_band_size {
		int get() { return p->poisson_cloning_band_size;}
		void set(int v) { p->poisson_cloning_band_size = v;}
	}
	property bool do_kmeans {
		bool get() { return p->do_kmeans;}
		void set(bool v) { p->do_kmeans = v;}
	}
	property double head_mask_size_mult {
		double get() { return p->head_mask_size_mult;}
		void set(double v) { p->head_mask_size_mult = v;}
	}
	property int num_DoG {
		int get() { return p->num_DoG;}
		void set(int v) { p->num_DoG = v;}
	}
	property bool do_two_back_kernels {
		bool get() { return p->do_two_back_kernels;}
		void set(bool v) { p->do_two_back_kernels = v;}
	}

	VirtualSurgeon_ParamsWrapper(void);
};

}