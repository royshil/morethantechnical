#ifndef _HEAD_EXTRACTOR_H
#define _HEAD_EXTRACTOR_H

#pragma once

#include "cv.h"
#include "highgui.h"

using namespace cv;

#include "../VirtualSurgeon_Utils/VirtualSurgeon_Utils.h"

class GCoptimizationGridGraph;

namespace VirtualSurgeon {

class HeadExtractor {
public:
	Mat ExtractHead(Mat& im);
	HeadExtractor(VIRTUAL_SURGEON_PARAMS& _p):params(_p) {};
//void FaceDotComDetection(VIRTUAL_SURGEON_PARAMS& params, Mat& im);
private:
	void calcHistogramWithMask(vector<MatND>& hist, Mat &im, vector<Mat>& mask, float _max, int win_size = 10, int histCompareMethod = CV_COMP_CORREL, vector<Mat>& backProj = vector<Mat>(), vector<Mat>& hists = vector<Mat>());
	void create2DGaussian(Mat& im, double sigma_x, double sigma_y, Point mean);
	Mat gabor_fn(double sigma, int n_stds, double theta, double freq, double phase, double gamma);
	void getSobels(Mat& gray, Mat& grayInt, Mat& grayInt1);
	int head_extract_main(int argc, char** argv);
	void make_gabor_bank(vector<Mat>& filter_bank, int bank_size, double sigma, int n_stds, double freq, double phase, double gamma);
	void NaiveRelabeling(Size s, vector<Mat>& backP, vector<Mat>& maskA);
	void takeBiggestCC(Mat& mask, Mat& bias = Mat());

	void calcSegmentsLikelihood(Mat& labled_im, 
							   vector<Mat> masks, 
							   int bins,
							   GCoptimizationGridGraph* gc,
							   Mat& vert_edge_score,
							   Mat& horiz_edge_score, int* score_matrix);

	void getEdgesForGC(Mat& gray, Mat& horiz, Mat& vert);

	VIRTUAL_SURGEON_PARAMS params;
};

}//ns

#endif