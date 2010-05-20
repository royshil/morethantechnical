#include "cv.h"

using namespace cv;

#include "util.h"

void face_grab_cut(Mat& orig, Mat& mask) {
	Mat tmpMask(mask.rows,mask.cols,CV_8UC1,Scalar(GC_PR_BGD));
	//tmpMask.setTo(Scalar(GC_PR_BGD));
	tmpMask.setTo(Scalar(GC_FGD),mask);
	//Mat(mask).copyTo(tmpMask);
	Mat bgdModel, fgdModel;
#ifdef BTM_DEBUG
	printf("Do grabcut... init... ");
#endif

	Rect mr = find_bounding_rect_of_mask(&((IplImage)mask));
	//initialize
	grabCut(
		orig,
		tmpMask,
		mr,
		bgdModel,
		fgdModel,
		1, GC_INIT_WITH_MASK);
#ifdef BTM_DEBUG
	printf("run... ");
#endif
	//run one iteration
	grabCut(
		orig,
		tmpMask,
		mr,
		bgdModel,
		fgdModel,
		3);

	//cvShowImage("result",image);
	//cvCopy(__GCtmp,mask);
	//Mat(mask).setTo(Scalar(255),tmpMask);
	//cvSet(mask,cvScalar(255),&((IplImage)tmpMask));
	Mat __tm = tmpMask & 1;
	__tm.setTo(Scalar(255),__tm);
	__tm.copyTo(Mat(mask));
#ifdef BTM_DEBUG
	printf("Done\n");
	//cvShowImage("tmp",mask);
	imshow("tmp",mask);
	waitKey(BTM_WAIT_TIME);
#endif
}