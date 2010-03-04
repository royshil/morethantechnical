#include <cv.h>
#include <highgui.h>
#include <cxcore.h>
//#include <cvaux.h>

#include <vector>
#include <fstream>


//l 25x56
//r 56x56
int main(int argc, char** argv) {
	IplImage* im = cvLoadImage("D:/Documents and Settings/rshilkr/My Documents/workspace/BeTheModel/ads/path.png",0);
	std::vector<CvPoint> pts;
	cvAbsDiffS(im,im,cvScalarAll(255));

	for(int y=0;y<im->height;y++) {
		char* linePtr = im->imageData + y * im->widthStep;
		for(int x=0;x<im->width;x++) {
			char* ptr = linePtr + x;
			if((uchar)(*ptr) > 0) {
				pts.push_back(cvPoint(x,y));			
			}
		}
	}

	IplImage* imc = cvCreateImage(cvGetSize(im),8,3);
	cvCvtColor(im,imc,CV_GRAY2RGB);

	std::ofstream ofs("path.txt");
	ofs << pts.size() << "\n";
	for(std::vector<CvPoint>::iterator i = pts.begin(); i != pts.end(); ++i) {
		cvCircle(imc,*i,2,CV_RGB(255,0,0));
		ofs << (*i).x << " " << (*i).y << " ";
	}
	ofs.close();

	cvNamedWindow("pts");
	cvShowImage("pts",imc);
	printf("%d pts\n",pts.size());
	cvWaitKey(0);
	cvReleaseImage(&im);
	cvReleaseImage(&imc);
}