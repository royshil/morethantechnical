#pragma once

#include "cv.h"
#include "ml.h"
using namespace cv;

#include <vector>
using namespace std;

class Recoloring
{
private:
	void TrainGMM(CvEM& source_model, Mat& source, Mat& source_mask);
	vector<int> MatchGaussians(CvEM& source_model, CvEM& target_model);

	VirtualSurgeonParams m_p;
public:
	void Recolor(Mat& source, Mat& source_mask, Mat& target, Mat& target_mask);
	Recoloring() {};
	~Recoloring(void);
};
