#pragma once

#include "cv.h"
#include "ml.h"
#include "highgui.h"
using namespace cv;

#include <vector>
using namespace std;

#include "../VirtualSurgeon_Utils/VirtualSurgeon_Utils.h"

namespace VirtualSurgeon {

class Recoloring
{
private:
	void TrainGMM(CvEM& source_model, Mat& source, Mat& source_mask);
	vector<int> Recoloring::MatchGaussians(CvEM& source_model, CvEM& target_model);

	VirtualSurgeonParams m_p;
public:
	void Recolor(Mat& source, Mat& source_mask, Mat& target, Mat& target_mask);
	Recoloring(VirtualSurgeonParams& _p):m_p(_p) {};
	~Recoloring(void);
};


}//ns