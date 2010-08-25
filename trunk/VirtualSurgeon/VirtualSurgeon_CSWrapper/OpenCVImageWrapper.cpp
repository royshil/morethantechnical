#include "StdAfx.h"
#include "OpenCVImageWrapper.h"

OpenCVImageWrapper::OpenCVImageWrapper(void)
{
}

OpenCVImageWrapper^ OpenCVImageWrapper::getFromOpenCV() {
Mat im; Mat tmp = imread("C:/Users/Roy/Documents/VirtualSurgeon/images/40406598_fd4e74d51c_d.jpg");

namedWindow("tmp");
resize(tmp,im,Size(),0.5,0.5);
putText(im,"In OpenCV!",Point(50,50),FONT_HERSHEY_COMPLEX,0.85,Scalar(0,0,255),2);
imshow("tmp",im);
waitKey(1);

OpenCVImageWrapper^ i = gcnew OpenCVImageWrapper();

i->buf = gcnew cli::array<byte>(im.rows * im.step);
System::Runtime::InteropServices::Marshal::Copy(System::IntPtr::IntPtr(im.data),i->buf,0,im.rows * im.step);

i->width = im.cols;
i->height = im.rows;
i->row_stride = im.step;
i->channels = im.channels();
i->buf_size = im.rows * im.step;

return i;
}