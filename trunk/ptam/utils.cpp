/**
 * License...
 *
 *	Utility functions
 *
 *
 */
#include "stdafx.h"
#include "utils.h"

extern Mat camera_matrix;
extern Mat distortion_coefficients;
extern double cam[3];
extern double rot[9];

extern Mat frames[5];
extern vector<Point2d> _points[5],points[5],pointsOrig[5];
extern vector<double> cams[5];
extern vector<double> rots[5];

extern Mat backPxls;

void read_frame_and_points(const char* pntsFile, const char* frameFile, vector<Point2d>& pts, Mat& frame) {
	FILE* f = fopen(pntsFile,"r");
	float x,y;
	while(true) {
		fscanf(f,"%f",&x);
		fscanf(f,"%f",&y);
		pts.push_back(Point2d(x,y));
		if(feof(f)) break;
	}
	fclose(f);
	frame = imread(frameFile);
}

void draw_points(Mat& image, vector<Point2d> points) {
	for(unsigned int i=0;i<points.size();i++)
	{
		circle(image,points[i],2,CV_RGB(0,255,0),CV_FILLED);
	}
}

void resize(int width, int height)
{
    const float ar = (float) width / (float) height;

    glViewport(0, 0, width, height);

	//FileStorage fs("D:/OpenCV2.0/samples/c/cam.out",CV_STORAGE_READ);
	//FileNode fn = fs["camera_matrix"];
	//Mat camera_matrix((CvMat*)fn.readObj(),true);
	//fn = fs["distortion_coefficients"];
	//Mat distortion_coefficients((CvMat*)fn.readObj(),true);
	double fovx,fovy,focalength,aspectR;
	Point2d princpPt;
	calibrationMatrixValues(camera_matrix,Size(352,288),1.0,1.0,fovx,fovy,focalength,princpPt,aspectR);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);
	gluPerspective(fovy,aspectR,0.01,1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity() ;
}

void loadFrame(int counter) {
	vector<double> v = cams[counter];
	cam[0] = v[0];
	cam[1] = v[1];
	cam[2] = v[2];
	//printf("switch cam: %.3f %.3f %.3f\n",cam[0],cam[1],cam[2]);

	//Vec3d ea;
	//float _d[9];
	//Mat pm(3,4,CV_64FC1);
	Mat _r(3,3,CV_64FC1,rot); 
	Rodrigues(Mat(rots[counter]),_r);

	//Mat _rf(3,3,CV_32FC1,_d);
	//_r.convertTo(_rf,CV_32FC1);

	//_r.copyTo(pm.colRange(0,2));
	//ea = RQDecomp3x3(_rf,Mat(),Mat(),Mat(),Mat(),Mat());
	//decomposeProjectionMatrix(pm,Mat(),Mat(),Mat(4,1,CV_64FC1),Mat(),Mat(),Mat(),ea);

	//update background pixels, but only when no one else needs them...
	if(WaitForSingleObject(ghMutex, INFINITE) == WAIT_OBJECT_0) {
		//Mat &f = frames[counter];
		frames[counter].copyTo(backPxls);
		cvtColor(backPxls,backPxls,CV_BGR2RGB);
		cvFlip(&cvMat(backPxls.rows,backPxls.cols,CV_8UC3,backPxls.data),0,0);

		ReleaseMutex(ghMutex);
	}
}

void key(unsigned char key, int x, int y)
{
	static int counter = 0;

    switch (key)
    {
    case 27 :
    case 'Q':
    case 'q': 
		glutLeaveMainLoop () ;      
		break;
	case ' ':
		counter = (counter + 1) % 5;
		loadFrame(counter);

		break;
    default:
        break;
    }

    glutPostRedisplay();
}

void idle(void)
{
    glutPostRedisplay();
}

void myGLinit() {
    glutSetOption ( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION ) ;

    glClearColor(1,1,1,1);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

	glShadeModel(GL_SMOOTH);

    //glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial ( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

	glEnable(GL_LIGHTING);
}

// http://softsurfer.com/Archive/algorithm_0104/algorithm_0104B.htm
// intersect3D_SegmentPlane(): intersect a segment and a plane
//    Input:  S = a segment, and Pn = a plane = {Point V0; Vector n;}
//    Output: *I0 = the intersect point (when it exists)
//    Return: 0 = disjoint (no intersection)
//            1 = intersection in the unique point *I0
//            2 = the segment lies in the plane
int
intersect3D_SegmentPlane( Vec3d S_P0, Vec3d S_P1, Vec3d P_V0, Vec3d P_N, Vec3d& I )
{
    Vec3d   u = S_P1 - S_P0;
    Vec3d	w = S_P0 - P_V0;

	double	D = P_N.ddot(u);
	double	N = -P_N.ddot(w);

    if (fabs(D) < 0.00001) {          // segment is parallel to plane
        if (N == 0)                     // segment lies in plane
            return 2;
        else
            return 0;                   // no intersection
    }
    // they are not parallel
    // compute intersect param
    double sI = N / D;
    if (sI < 0 || sI > 1)
        return 0;                       // no intersection

    I = S_P0 + sI * u;                 // compute segment intersect point
    return 1;
}

void draw_frame_points_w_status(const string& win_name, const Mat& i, vector<Point2d>& p, vector<uchar>& status) {
	namedWindow(win_name,1);
	Mat __tmpI;
	i.copyTo(__tmpI);
	for(unsigned int i=0;i<p.size();i++) {
		if(status[i] == 1) {
			circle(__tmpI,p[i],2,CV_RGB(0,255,0),CV_FILLED);
		} else {
			circle(__tmpI,p[i],2,CV_RGB(255,0,0),CV_FILLED);
		}
	}
	imshow(win_name,__tmpI);
}

int show_2_frame_correlation(int i,char** c) {
	vector<Point2d> p[2];
	Mat f[2];

	//for(int i=0;i<2;i++) {
		//std::stringstream st;
		//st << "../ptam_orig/5/frame" << (i+2) << ".png";
		//std::stringstream st1;
		//st1 << "../ptam_orig/5/frame" << (i+2) << ".points";
		//p[i].clear();
		//read_frame_and_points(st1.str().c_str(),st.str().c_str(),p[i],f[i]);
	//}
	read_frame_and_points("../ptam_orig/5/frame1.points","../ptam_orig/5/frame1.png",p[0],f[0]);
	read_frame_and_points("../ptam_orig/5/frame3.points","../ptam_orig/5/frame3.png",p[1],f[1]);
	namedWindow("tmp",1);
	Mat im(f[0].rows,f[0].cols+f[1].cols,f[0].type());
	uchar* pt = im.ptr<uchar>();
	uchar* pf0 = f[0].ptr<uchar>();
	uchar* pf1 = f[1].ptr<uchar>();
	int f0s = f[0].step;
	int f1s = f[1].step;
	for(int i=0;i<im.rows;i++) {
		memcpy(pt + i*im.step,pf0 + i*f0s,f0s);
		memcpy(pt + i*im.step + f0s,pf1 + i*f1s,f1s);
	}
	draw_points(im.colRange(0,f[0].cols-1),p[0]);
	draw_points(im.colRange(f[0].cols,im.cols-1),p[1]);
	for(unsigned int i=0;i<p[0].size();i++) {
		line(im,p[0][i],Point((int)f[0].cols+(int)p[1][i].x,(int)p[1][i].y),Scalar(255));
	}

	imshow("tmp",im);
	waitKey();
	imwrite("frames_correl.png",im);

	return 1;
}