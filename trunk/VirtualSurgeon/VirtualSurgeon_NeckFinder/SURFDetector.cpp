#include "cv.h"
#include "cvaux.h"
#include "highgui.h"

using namespace cv;

#include <vector>
using namespace std;

int surf_main(int argc, char** argv) {
	Mat img = imread("../neck_template1.png",0);

	cv::SURF s;
	vector<KeyPoint> keypoints;
	s(img,Mat::ones(img.size(),CV_8UC1),keypoints);

	Mat img1 = imread("40406598_fd4e74d51c_d.jpg",0);
	//vector<KeyPoint> keypoints1;
	//s(img1,mask,keypoints1);

	StarDetector sd;
	sd(img1,keypoints);

	for (int i=0;i<keypoints.size();i++) {
		circle(img1,keypoints[i].pt,2,Scalar(0,0,255),CV_FILLED);
	}
	namedWindow("tmp");
	imshow("tmp",img1);
	waitKey();

	//PlanarObjectDetector p;
	//p.train(

	return 0;
}