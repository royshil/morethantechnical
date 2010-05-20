#include "cv.h"
#include "highgui.h"

void do1ChnHist(const Mat& _i, const Mat& mask, double* h, double* cdf) {
	Mat _t = _i.reshape(1,1);
	Mat _tm;
	mask.copyTo(_tm);
	_tm = _tm.reshape(1,1);
	for(int p=0;p<_t.cols;p++) {
		if(_tm.at<uchar>(0,p) > 0) {
			uchar c = _t.at<uchar>(0,p);
			h[c] += 1.0;
		}
	}

	//normalize hist
	Mat _tmp(1,256,CV_64FC1,h);
	double minVal,maxVal;
	minMaxLoc(_tmp,&minVal,&maxVal);
	_tmp = _tmp / maxVal;

	cdf[0] = h[0];
	for(int j=1;j<256;j++) {
		cdf[j] = cdf[j-1]+h[j];
	}

	//normalize CDF
	_tmp.data = (uchar*)cdf;
	minMaxLoc(_tmp,&minVal,&maxVal);
	_tmp = _tmp / maxVal;
}

void histMatch(const Mat& src, const Mat& src_mask, const Mat& dst, const Mat& dst_mask) {
	double histr[256] = {0}; double histg[256] = {0};	double histb[256] = {0};
	vector<double*> src_hists(3); src_hists[0] = histr; src_hists[1] = histg; src_hists[2] = histb;
	double histr1[256] = {0}; double histg1[256] = {0};	double histb1[256] = {0};
	vector<double*> dst_hists(3); dst_hists[0] = histr1; dst_hists[1] = histg1; dst_hists[2] = histb1;
	double cdfr[256] = {0}; double cdfg[256] = {0};	double cdfb[256] = {0};
	vector<double*> src_cdfs(3); src_cdfs[0] = cdfr; src_cdfs[1] = cdfg; src_cdfs[2] = cdfb;
	double cdfr1[256] = {0}; double cdfg1[256] = {0};	double cdfb1[256] = {0};
	vector<double*> dst_cdfs(3); dst_cdfs[0] = cdfr1; dst_cdfs[1] = cdfg1; dst_cdfs[2] = cdfb1;

	namedWindow("original source",0);
	imshow("original source",src);
	namedWindow("original query",0);
	imshow("original query",dst);

	vector<Mat> chns;
	split(src,chns);
	vector<Mat> chns1;
	split(dst,chns1);
	for(int i=0;i<3;i++) {
		do1ChnHist(chns[i],src_mask,src_hists[i],src_cdfs[i]);
		do1ChnHist(chns1[i],dst_mask,dst_hists[i],dst_cdfs[i]);
	}

	for(int i=0;i<3;i++) {
		uchar M[255] = {0};	//the replacement function
		uchar last = 0;
		for(int j=0;j<255;j++) {
			double F1j = src_cdfs[i][j];

			for(uchar k = last; k<255; k++) {
				double F2k = dst_cdfs[i][k];
				if(abs(F2k - F1j) < HISTMATCH_EPSILON || F2k > F1j) {
					M[j] = k;
					last = k;
					break;
				}
			}
		}

		Mat lut(1,256,CV_8UC1,M);
		LUT(chns[i],lut,chns[i]);
	}

	Mat res;
	merge(chns,res);

	namedWindow("matched",0);
	imshow("matched",res);

	waitKey();
}