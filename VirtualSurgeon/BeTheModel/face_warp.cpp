#include "../VirtualSurgeon_Warp/VirtualSurgeon_Warp/stdafx.h"

#include "util.h"

#include "../VirtualSurgeon_Warp/VirtualSurgeon_Warp/vs_warp.h"
namespace VirtualSurgeon {

void VirtualSurgeon_Warp::face_warp_read_points(std::string& str, Size im_size, vector<Point2d>& points) {
	int x = -1,y = -1;
	FILE* f;
	fopen_s(&f,str.c_str(),"r");

	while(!feof(f)) {
		fscanf_s(f,"%d %d\n",&x,&y);
		//fscanf_s(f,"%d",&y);
		points.push_back(Point2d(
			max(0,min(im_size.width-1,x)),
			max(0,min(im_size.height-1,y))
			));
	}

	fclose(f);
}

void VirtualSurgeon_Warp::doWarp(VirtualSurgeonFaceData& model_p, 
								 vector<Point2d> face_points,
								 vector<Point2d> model_points,
								 Mat& model_im,
								 Mat& model_mask,
								 Mat& im,
								 Mat& im_mask) {
	//warp
	//double model_dist = norm(model_p.li - model_p.ri);
	//model_dist *= cos(model_p.yaw / 180.0 * CV_PI);
	//double face_dist = norm(p.li - p.ri);
	//face_dist *= cos(p.yaw / 180.0 * CV_PI);

	//double scaleFromFaceToBack = model_dist / face_dist;

	////double scaleFromFaceToBack = norm(model_p.li - model_p.ri) / norm(p.li - p.ri);
	////Point2d model_midp = (model_p.li + model_p.ri) * .5;
	////Point2d face_midp = (p.li + p.ri) * .5;
	////Point2d diff = model_midp - face_midp;

	//Mat facePts(face_points);
	//facePts = facePts * scaleFromFaceToBack;
	//Mat modelPts(model_points);
	//modelPts = modelPts - Scalar(diff.x,diff.y);

	vector<Mat* > imgs;
	Mat backM,maskM;
	model_im.copyTo(backM); model_mask.copyTo(maskM);

	imgs.push_back(&(backM));
	imgs.push_back(&(maskM));

	//warp back into face
	face_warp(imgs,
		face_points,
		model_points,
		im.size(),
		(p.use_warp_affine)?FACE_WARP_AFFINE:FACE_WARP_RIGID,
		(p.use_double_warp)?0.75:1.0);

	GaussianBlur(maskM,maskM,Size(7,7),1);
	//blur(maskM,maskM,Size(7,7));

	//facePts = Mat(face_points);
	//facePts = facePts / scaleFromFaceToBack;

	backM.copyTo(model_im); maskM.copyTo(model_mask);

	if(p.use_double_warp) {
		//warp face into back...
		imgs.clear();
		im.copyTo(backM);
		im_mask.copyTo(maskM);
		imgs.push_back(&(backM));
		imgs.push_back(&(maskM));
		face_warp(imgs,
			model_points,
			face_points,
			model_im.size(),
			(p.use_warp_affine)?FACE_WARP_AFFINE:FACE_WARP_RIGID,
			0.25);

		backM.copyTo(im); maskM.copyTo(im_mask);
	}
}


void VirtualSurgeon_Warp::face_warp(vector<Mat* >& imgs,
			   vector<Point2d>& points1,
			   vector<Point2d>& points2, 
			   Size im2_size, 
			   face_warp_method method,
			   double alpha) {

	//Move points to align one on top of the other
	Scalar m1 = mean(Mat(points1));
	Mat p2M(points2);
	Scalar m2 = mean(p2M);
	p2M = p2M - m2 + m1;

	vector<Point2d> restorePts2 = points2;

	//anchor first and last points
	Point2d tmpPts[2] = {points2[0],points2[points2.size()-1]};	//save for restore
	points2[0] = points1[0];
	points2[points2.size()-1] = points1[points1.size()-1];

	//use alpha to interpolate between points sets
	Mat p1M(points1);
	//Mat p2M(points2);
	p2M = p2M * alpha + p1M * (1 - alpha);
	
	Mat& tmpIm = *(imgs[0]);

	//put 4 anchors in corners of image for rigidity of deformation
	Point2d pts[4] = {Point2d(1,1),Point2d(1,tmpIm.rows-2),Point2d(tmpIm.cols-2,tmpIm.rows-2),Point2d(tmpIm.cols-2,1)};
	for(int i=0;i<4;i++) {
		points1.push_back(pts[i]);
		points2.push_back(pts[i]);
	}

	bool computed = false;
	vector<Point2d> mesh;

	for(unsigned int imIdx = 0;imIdx<imgs.size();imIdx++) {

	Mat& im1 = *(imgs[imIdx]);

	if(!p.no_gui) {
		Mat clone;
		clone.create(im1.size(),im1.type());
		im1.copyTo(clone);
		for(unsigned int i=0;i<points1.size();i++) {
			circle(clone,points1[i],2,Scalar(0,0,255),CV_FILLED);
			circle(clone,points2[i],2,Scalar(0,255,255),CV_FILLED);
			line(clone,points1[i],points2[i],Scalar(255,0,0),2);
		}
		imshow("tmp",clone);
		int c = waitKey(p.wait_time);
	}

	if(!computed) {
		mesh = vector<Point2d>(im1.rows*im1.cols,Point2d(0,0));
		for(int y=0;y<im1.rows;y++) {
			for(int x=0;x<im1.cols;x++) {
				mesh[y*im1.cols + x] = Point2d(x,y);
			}
		}

		if(!p.no_gui) {
			printf("Precompute weights...\n");
		}
		if(method == FACE_WARP_AFFINE) {
			Affine_Precompute(points2,mesh);
//		} else {
//			Rigid_Precompute(points2,mesh);
//		}
//
//#ifdef BTM_DEBUG
//		printf("Warp mesh...\n");
//#endif
//		if(method == FACE_WARP_AFFINE) {
			Affine_doWarp(points1,mesh);
//		} else {
//			Rigid_doWarp(points2,points1,mesh);
//			Rigid_release();
		} else {
			Rigid_doAll(points2,points1,mesh);
		}

		computed = true;
	}

	Mat warped(im1.size(),im1.type());
	int warpedCn = warped.channels();
	int im1Cn = im1.channels();
	for(int y=0;y<im1.rows;y++) {
		uchar* warpedRowPtr = warped.ptr<uchar>(y);

		for(int x=0;x<im1.cols;x++) {
			uchar* pxlP = warpedRowPtr + x * warpedCn;

			Point2d meshP = mesh[y*im1.cols + x];
			int xMsh = max(min((int)floor(meshP.x),im1.cols-1),0);
			int yMsh = max(min((int)floor(meshP.y),im1.rows-1),0);
			uchar* origP = im1.data + yMsh * im1.step + xMsh * im1Cn;

			for(int cn=0;cn<warpedCn;cn++) {
				pxlP[cn] = origP[cn];
			}
		}
	}
	
	if(!p.no_gui) {
		//namedWindow("warped");
		Mat _tmp;
		warped.copyTo(_tmp);
		for(unsigned int i=0;i<points2.size();i++) circle(_tmp,points2[i],2,Scalar(0,0,255),CV_FILLED);
		imshow("tmp",_tmp);
		waitKey(p.wait_time);
	}
	warped.copyTo(im1);

	}

	for(int i=0;i<4;i++) {points1.pop_back();points2.pop_back();}

	//points2[0] = tmpPts[0];
	//points2[points2.size()-1] = tmpPts[1];
	points2 = restorePts2;

	p2M = Mat(points2);
	p2M = p2M - m1 + m2;	//restore original position of points
}

}//ns