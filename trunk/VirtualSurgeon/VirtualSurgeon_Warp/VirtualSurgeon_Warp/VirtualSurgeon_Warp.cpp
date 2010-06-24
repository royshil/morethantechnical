// VirtualSurgeon_Warp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "vs_warp.h"

using namespace VirtualSurgeon;

#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

void read_points(std::string& str, Mat& im, vector<Point2d>& points) {
	double x = -1,y = -1;
	ifstream ifs(str.c_str());
	char c;

	std::string line;
	while(!ifs.eof()) {
		getline(ifs,line);
		istringstream i(line);
		i >> x;
		i.read(&c,1);
		i >> y;
		points.push_back(Point2d(
			MAX(0,MIN(im.cols-1,x)),
			MAX(0,MIN(im.rows-1,y))
			));
	}

	//FILE* f;
	//fopen_s(&f,str.c_str(),"r");

	//while(!feof(f)) {
	//	fscanf_s(f,"%f",&x);
	//	fscanf_s(f,"%f",&y);
	//	points.push_back(Point2d(
	//		MAX(0,MIN(im.cols-1,x)),
	//		MAX(0,MIN(im.rows-1,y))
	//		));
	//}

	//fclose(f);
}


int _tmain(int argc, _TCHAR* argv[])
{
	VIRTUAL_SURGEON_PARAMS p;
	p.filename = std::string("..\\..\\images\\40406598_fd4e74d51c_d.jpg");
	p.InitializeDefault();
	Mat im1;
	p.FaceDotComDetection(im1);

	VirtualSurgeonFaceData mp;
	mp.filename = "..\\..\\images\\chinese_dude.jpg";
	Mat im2;
	mp.FaceDotComDetection(im2);
	im2 = imread("..\\..\\images\\chinese_dude.png");

	//while(true) {
	//Mat im1 = imread(std::string("../../images/40406598_fd4e74d51c_d.jpg"));
	vector<Point2d> points1;
	read_points(std::string("../../images/40406598_fd4e74d51c_d.points"),im1,points1);

	//Mat im2 = imread("../../images/2095271621_e709ff4bd0_d.jpg");
	vector<Point2d> points2;
	read_points(std::string("../../images/chinese_dude.points"),im2,points2);

	//Scalar m1 = mean(Mat(points1));
	//Mat p2M(points2);
	//Scalar m2 = mean(p2M);
	//p2M = p2M - m2 + m1;	

	namedWindow("1");
	Mat clone;
	im1.copyTo(clone);
	for(int i=0;i<points1.size();i++) {
		circle(clone,points1[i],2,Scalar(0,0,255),CV_FILLED);
		circle(clone,points2[i],2,Scalar(0,255,255),CV_FILLED);
		line(clone,points1[i],points2[i],Scalar(255,0,0),2);
	}
	imshow("1",clone);
	int c = waitKey();
	//if(c=='q'||c=='Q') continue;


	//vector<Point2d> mesh(im2.rows*im2.cols,Point2d(0,0));
	//for(int y=0;y<im2.rows;y++) {
	//	for(int x=0;x<im2.cols;x++) {
	//		mesh[y*im2.cols + x] = Point2d(x,y);
	//	}
	//}


	Mat im1_mask = Mat::zeros(im1.size(),CV_8UC1);
	Mat im2_mask = Mat::zeros(im2.size(),CV_8UC1);

	p.no_gui = false;
	p.use_warp_rigid = false;
	p.use_warp_affine = true;
	p.use_double_warp = false;

	VirtualSurgeon_Warp warpobj(p);
	warpobj.doWarp(mp,points2,points1,im2,im2_mask,im1,im1_mask);
		
	Mat warped; im2.copyTo(warped);

	//warpobj

	//VirtualSurgeon::Rigid_Precompute(points2,mesh);

	//printf("Warp mesh...\n");
	////Affine_doWarp(points1,mesh);
	//VirtualSurgeon::Rigid_doWarp(points2,points1,mesh);

	/*printf("Redraw...");
	Mat warped(im1.size(),im1.type());
	for(int y=0;y<im1.rows;y++) {
		uchar* warpedRowPtr = warped.ptr<uchar>(y);

		for(int x=0;x<im1.cols;x++) {
			uchar* pxlP = warpedRowPtr + x * 3;

			Point2d meshP = mesh[y*im1.cols + x];
			int xMsh = max(min((int)floor(meshP.x),im1.cols-1),0);
			int yMsh = max(min((int)floor(meshP.y),im1.rows-1),0);
			uchar* origP = im1.data + yMsh * im1.step + xMsh * 3;

			pxlP[0] = origP[0];
			pxlP[1] = origP[1];
			pxlP[2] = origP[2];
		}
	}*/
	

	namedWindow("warped");
	for(int i=0;i<points2.size();i++) circle(warped,points2[i],2,Scalar(0,0,255),CV_FILLED);
	imshow("warped",warped);

	c = waitKey();
	//if(c=='q'||c=='Q') break;
	//}

	return 0;
}
