#include "head_extractor.h"

#include "matting.h"


#ifdef HEAD_EXTRACTOR_MAIN
int main(int argc, char** argv) {
	VirtualSurgeon::VirtualSurgeonParams p;
	p.InitializeDefault();
	p.im_scale_by = 1.5;
	p.no_gui = false;
	p.wait_time = 0;
	p.gc_iter = 0;
	p.num_DoG = 0;
	p.gb_size = 4;
	p.km_numc = 20;
	p.hair_ellipse_size_mult = 1.0;
	p.do_alpha_matt = false;
	p.consider_pixel_neighbourhood = true;
	p.do_two_segments = false;
	p.do_kmeans = true;
	p.do_two_back_kernels = true;

	p.filename = std::string(argv[1]);
	Mat _tmp,im;
	p.FaceDotComDetection(_tmp);
	p.PrintParams();

	//resize(_tmp,im,Size(),0.75,0.75);
	//p.li *= 0.75;
	//p.ri *= 0.75;

	_tmp.copyTo(im);

	//p.pitch = 15.16;
	//p.yaw = -21.31;
	//_tmp.copyTo(im);

	double li_ri = norm(p.li - p.ri);// / (double)(faceMask.cols);
	int li_ri_m_3 = (int)(li_ri*3.0);
	int li_ri_t_6_5 = (int)(li_ri*6.5);
	Rect r(MIN(im.cols,MAX(0,p.li.x - li_ri_m_3)),
			MIN(im.rows,MAX(0,p.li.y - li_ri_m_3)),
			MIN(im.cols-MAX(0,p.li.x - li_ri_m_3),MAX(0,li_ri_t_6_5)),
			MIN(im.rows-MAX(0,p.li.y - li_ri_m_3),MAX(0,li_ri_t_6_5)));

	im(r).copyTo(_tmp);

	Point orig_li = p.li;
	Point orig_ri = p.ri;
	p.li = p.li - r.tl(); 
	p.ri = p.ri - r.tl(); 

	VirtualSurgeon::HeadExtractor he(p);
	he.ExtractHead(_tmp);

	return 0;

	Mat mask1(_tmp.size(),CV_8UC1,Scalar(0)),mask2(_tmp.size(),CV_8UC1,Scalar(0));
	he.CreateEllipses(_tmp,mask1,mask2,Point());
	Mat maskfinal = mask1 | mask2;

	im(r).copyTo(_tmp);
	namedWindow("tmp");
	namedWindow("tmp1");
	p.face_grab_cut(_tmp,maskfinal,1);

	vector<Mat> v; split(im(r),v);
	//v[0] = Mat::zeros(maskfinal.size(),CV_8UC1);
	v[1] = v[1] * 0.7 + maskfinal * 0.3;
	v[2] = v[2] * 0.7 + ~maskfinal * 0.3;// + 255.0;
	cv::merge(v,_tmp);
	imshow("tmp",_tmp);
	_tmp.setTo(Scalar(0));
	im(r).copyTo(_tmp,maskfinal);
	imshow("tmp1",_tmp);
	waitKey();

	{
		Mat tmpMask(maskfinal.rows,maskfinal.cols,CV_8UC1,Scalar(0));
		int dilate_size = 5; //p.alpha_matt_dilate_size;

		//prepare trimap
		{
			Mat __tmp(maskfinal.rows,maskfinal.cols,CV_8UC1,Scalar(0));
			dilate(maskfinal,__tmp,Mat::ones(dilate_size,dilate_size,CV_8UC1),Point(-1,-1),1,BORDER_REFLECT);	//probably background
			tmpMask.setTo(Scalar(128),__tmp);

			erode(maskfinal,__tmp,Mat::ones((int)((double)(dilate_size)*1.5),
				(int)((double)(dilate_size)*1.5),CV_8UC1),Point(-1,-1),1,BORDER_REFLECT); // foreground
			tmpMask.setTo(Scalar(255),__tmp);
		}

		Mat tmpim; im(r).copyTo(tmpim);

		imshow("tmp",tmpMask);
		imshow("tmp1",tmpim);
		waitKey();

		Matting *matting = new BayesianMatting( &((IplImage)tmpim), &((IplImage)tmpMask) );
		//Matting *matting = new RobustMatting( &((IplImage)im), &((IplImage)tmpMask) );
		matting->Solve(!p.no_gui);

		Mat(matting->alphamap).copyTo(maskfinal);

		//maskFace.convertTo(maskFace,CV_8UC1,255);

		delete matting;
	}
	Mat unMask = Mat::ones(maskfinal.size(),CV_32FC1) - maskfinal; //Mat(maskFace.size(),CV_8UC1,Scalar(255)) - maskFace;
	vector<Mat> v1;
	Mat im1;
	im(r).convertTo(im1,CV_32FC3,1.0/255.0);
	split(im1,v1);
	v1[0] = v1[0].mul(maskfinal) + (unMask);
	v1[1] = v1[1].mul(maskfinal);
	v1[2] = v1[2].mul(maskfinal);
	Mat imMasked;
	cv::merge(v1,imMasked);

	imshow("tmp1",imMasked);
	waitKey();
}
#endif