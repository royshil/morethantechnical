#include "cv.h"
//#include "cvaux.h"
#include "highgui.h"

using namespace cv;

#include <vector>
using namespace std;

int match_template_main(int argc, char** argv) {
	Mat templ = imread("../neck_template1.png",0);
	Mat img = imread("40406598_fd4e74d51c_d.jpg",0);

	resize(templ,templ,Size(templ.cols/1.5,templ.rows/1.5));

	Mat result(img.rows - templ.rows + 1,img.cols - templ.cols + 1,CV_32FC1);
	matchTemplate(img,templ,result,CV_TM_SQDIFF_NORMED);
	namedWindow("tmp");
	imshow("tmp",result);
	waitKey();

	double maxv; Point maxp;
	minMaxLoc(result,0,&maxv,0,&maxp);
	circle(img,Point(maxp.x+templ.cols/2,maxp.y+templ.rows/2),3,Scalar(0,0,255),CV_FILLED);
	imshow("tmp",img);
	waitKey();

	return 0;
}