// recoloring.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>

using namespace cv;

int _tmain(int argc, _TCHAR* argv[])
{
    Mat src;
	if(!(src=cvLoadImage("../BeTheModel/pic.jpg")).data)
        return -1;

    Mat hsv;
    cvtColor(src, hsv, CV_BGR2HSV);

    // let's quantize the hue to 30 levels
    // and the saturation to 32 levels
    int hbins = 60, sbins = 64;
    int histSize[] = {hbins, sbins};
    // hue varies from 0 to 179, see cvtColor
    float hranges[] = { 0, 180 };
    // saturation varies from 0 (black-gray-white) to
    // 255 (pure spectrum color)
    float sranges[] = { 0, 256 };
    float* ranges[] = { hranges, sranges };
    MatND hist;
    // we compute the histogram from the 0-th and 1-st channels
    int channels[] = {0, 1};

	calcHist(&hsv,1,(int*)channels,Mat(),hist,2,(const int*)histSize,(const float**)ranges);
	Mat dilated;
	dilate(hist,dilated,Mat(15,15,CV_8U));
	//bitwise_and(dilated,hist,dilated);
	Mat dilated1;
	compare(dilated,Mat(hist),dilated1,CV_CMP_EQ); //keep interesting points
	dilated1 = dilated1 & (hist > 0);	
	Mat dilated2;
	dilated.copyTo(dilated2,dilated1); //get interesting points values
	Mat pts;
	sortIdx(dilated2.reshape(1,1),pts,CV_SORT_DESCENDING);
	int* p_pts = pts.ptr<int>(0);
	std::cout << p_pts[0] << "," << p_pts[1] << "," << p_pts[2] << "\n";

	Mat centersForKM(2,5,CV_8UC1);
	uchar* c_ptr = centersForKM.ptr<uchar>(0);
	for(int i=0;i<5;i++) {
		c_ptr[i] = ((int)floor(p_pts[i] / 64.0)) * 3;
	}
	c_ptr = centersForKM.ptr<uchar>(1);
	for(int i=0;i<5;i++) {
		c_ptr[i] = (p_pts[i] % 64) * 4;
	}

	Mat samples(2,src.rows*src.cols,CV_8UC1);
	Mat flatIm = src.reshape(src.channels(),1);
	vector<Mat> im_ch(3);
	split(flatIm,im_ch);
	im_ch[0].copyTo(samples.row(0),Mat()); 
	im_ch[1].copyTo(samples.row(1),Mat()); 

	Mat samplesT = samples.t();
	Mat lables(1,samples.cols,CV_8UC1);
	Mat scores = Mat_<double>(samplesT) * Mat_<double>(centersForKM);
	Point maxIdx;
	uchar* l_ptr = lables.ptr<uchar>();
	for(int i=0;i<scores.rows;i++) {
		Mat arow = scores.row(i);
		minMaxLoc(arow,0,0,0,&maxIdx);
		l_ptr[i] = maxIdx.x;
	}

	Mat centers;
	kmeans(samples.t(),5,lables,
		cv::TermCriteria(TermCriteria::MAX_ITER+TermCriteria::EPS,10,1.0),
		1,KMEANS_USE_INITIAL_LABELS,&centers);


    double maxVal=0;
    minMaxLoc(dilated2, 0, &maxVal, 0, 0);

    int scale = 10;
    Mat histImg = Mat::zeros(sbins*scale, hbins*10, CV_8UC3);

    for( int h = 0; h < hbins; h++ )
        for( int s = 0; s < sbins; s++ )
        {
            float binVal = dilated2.at<float>(h, s);
            int intensity = cvRound(binVal*255/maxVal);
            rectangle( histImg, Point(h*scale, s*scale),
                         Point( (h+1)*scale - 1, (s+1)*scale - 1),
                         Scalar::all(intensity),
                         CV_FILLED );
        }
	

    namedWindow( "Source", 1 );
    imshow( "Source", src );

    namedWindow( "H-S Histogram", 1 );
    imshow( "H-S Histogram", histImg );

    waitKey();

	return 0;
}

