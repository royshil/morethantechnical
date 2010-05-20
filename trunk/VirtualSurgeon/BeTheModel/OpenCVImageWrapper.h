#ifndef _OPENCVIMAGEWRAPPER_H
#define _OPENCVIMAGEWRAPPER_H

#include "ImageEditingUtils.h"
#include <cv.h>

class OpenCVImage : public ImageEditingUtils::IImage
{
private:
	IplImage* _img;
public:
	OpenCVImage(IplImage* i) { _img = i;}

	int getRGB(int x,int y) {
		uchar* ptr = cvPtr2D(_img,y,x);
		int rgb;
		if(_img->nChannels == 1) {
			rgb = ptr[0] | ((int)ptr[0]) << 8 | ((int)ptr[0]) << 16;
		} else if(_img->nChannels == 3 || _img->nChannels == 4) {
			rgb = ptr[0] | ((int)ptr[1]) << 8 | ((int)ptr[2]) << 16;
		}
		return rgb;
	}
	void setRGB(int x,int y,int rgb) {
		uchar* ptr = cvPtr2D(_img,y,x);
		if(_img->nChannels == 1) {
			*ptr = (uchar)(rgb & 0x000000ff);
		} else if(_img->nChannels == 3 || _img->nChannels == 4) {
			ptr[0] = (uchar)(rgb & 0x000000ff);
			ptr[1] = (uchar)((rgb & 0x0000ff00) >> 8);
			ptr[2] = (uchar)((rgb & 0x00ff0000) >> 16);
		}
	}
};

#endif