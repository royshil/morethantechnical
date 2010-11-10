///*
// *  opticalFlow.cpp
// *  OpenCVTries1
// *
// *  Created by Roy Shilkrot on 11/3/10.
// *  Copyright 2010. All rights reserved.
// *
// */
//
#include "opticalFlow.h"

#include <GL/glut.h>
#include <pthread.h>

//#define USE_DRAWPIXELS

GLfloat light_ambient[] = {1.0, 1.0, 1.0, 1.0};  /* Red diffuse light. */
GLfloat light_diffuse[] = {1.0, 0.0, 0.0, 1.0};  /* Red diffuse light. */
GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};  /* Infinite light location. */
GLfloat n[6][3] = {  /* Normals for the 6 faces of a cube. */
	{-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0},
	{0.0, -1.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0} };
GLint faces[6][4] = {  /* Vertex indices for the 6 faces of a cube. */
	{0, 1, 2, 3}, {3, 2, 6, 7}, {7, 6, 5, 4},
	{4, 5, 1, 0}, {5, 6, 2, 1}, {7, 4, 0, 3} };
GLfloat v[8][3];  /* Will be filled in with X,Y,Z vertexes. */
int glutwin = -1;

vector<KeyPoint> imgPointsOnPlane;
Mat img_to_show;

vector<Point2f> points1,points2;
vector<uchar> status;
vector<float> err;

double camD[] = {6.7649431228632795e+02, 0., 3.8262188058832749e+02, 0.,
	5.9941193806780484e+02, 1.6894241981264270e+02, 0., 0., 1.};
double distCoeffD[] = {5.5318827974857022e-02, -1.0129523116603711e+00,
	3.8895464611792836e-02, 2.5365684020675693e-02,
	2.6020235726385716e+00, 0., 0., 8.1013197871984710e-01};
Mat camera_matrix = Mat(3,3,CV_64FC1,camD);
Mat distortion_coefficients = Mat(5,1,CV_64FC1,distCoeffD);
Mat objPM;
vector<double> rv(3), tv(3);
Mat rvec(rv),tvec(tv); 
double _d[9] = {1,	0,	0,
	0,	-1,	0,
	0,	0,	-1}; //rotation: looking at -x axis
Mat rotM(3,3,CV_64FC1,_d);
double theta = 0.0,theta1 = 0.0,theta2 = 0.0,phi = 0.0,phi1 = 0.0,phi2 = 0.0,psi = 0.0,psi1 = 0.0,psi2 = 0.0;

Mat outImg(Size(640,480),CV_8UC3);
int frameNum = 0;

double zt = 0.0;

Mat texttmp(640,480,CV_8UC3);
Mat textmp(Size(1024,512),CV_8UC3);

void keyboard(uchar key, int x, int y) { glutPostRedisplay(); }
void idle() { glutPostRedisplay(); }

void
drawBox(void)
{
	int i;
	
	for (i = 0; i < 6; i++) {
		glBegin(GL_QUADS);
		glNormal3fv(&n[i][0]);
		glVertex3fv(&v[faces[i][0]][0]);
		glVertex3fv(&v[faces[i][1]][0]);
		glVertex3fv(&v[faces[i][2]][0]);
		glVertex3fv(&v[faces[i][3]][0]);
		glEnd();
	}
}

void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(img_to_show.data != NULL) {
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
//		glLoadIdentity();
//		gluOrtho2D(0.0, 640.0, 480.0, 0.0);	//Toying around with gluOrtho..
//		glOrtho(0.0, 640.0, 480.0, 0.0, -10, 1);
		
		glMatrixMode(GL_MODELVIEW);
		
#ifdef USE_DRAWPIXELS
		if(img_to_show.step == img_to_show.cols)
			cvtColor(img_to_show, textmp, CV_GRAY2RGB);
		else if(img_to_show.step == img_to_show.cols * 3)
			cvtColor(img_to_show, textmp, CV_BGR2RGB);
		flip(textmp,textmp,0);

		glDrawPixels(640, 480, GL_RGB, GL_UNSIGNED_BYTE, textmp.data);
#else
		if(img_to_show.step == img_to_show.cols)
			cvtColor(img_to_show, texttmp, CV_GRAY2RGB);
		else if(img_to_show.step == img_to_show.cols * 3)
			cvtColor(img_to_show, texttmp, CV_BGR2RGB);
		flip(texttmp,texttmp,0);
		resize(texttmp,textmp,Size(1024,512));
		
		glEnable(GL_TEXTURE_2D);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, textmp.data);
		
		glPushMatrix();
		glTranslated(0, 0, -30);
		glScaled(1.0/640.0, 1.0/480.0, 1.0);
		glScaled(30, 30, 1);
		glTranslated(-320, -240, 0.0);
		glBegin(GL_QUADS);
		glTexCoord2i(0, 0); glVertex2i(0,	0);
		glTexCoord2i(1, 0); glVertex2i(640, 0);
		glTexCoord2i(1, 1); glVertex2i(640, 480);
		glTexCoord2i(0, 1); glVertex2i(0,	480);
		glEnd();
		glPopMatrix();
#endif

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}
	
	glPushMatrix();
//	glTranslated(-tv[0]+0.5, -tv[1]+0.7, -tv[2]); 
	//psi, phi, theta
//	glRotated(-psi2 / (2*CV_PI) * 180.0, 1.0, 0.0, 0.0);
//	glRotated(-phi2 / (2*CV_PI) * 180.0, 0.0, 1.0, 0.0);
//	glRotated(-theta2 / (2*CV_PI) * 180.0, 0.0, 0.0, 1.0);

	double m[16] = {	_d[0],-_d[3],-_d[6],0,
						_d[1],-_d[4],-_d[7],0,
						_d[2],-_d[5],-_d[8],0,
						tv[0],-tv[1],-tv[2],1};

	glLoadMatrixd(m);
	
	glDisable(GL_TEXTURE_2D);
	glColor3b(255, 0, 0);
	glTranslated(2.5, -2.5, 0.0);
	glutSolidCube(2);
//	glColor3b(0, 0, 255);
//	glBegin(GL_QUADS);
//	glVertex2i(0, 0);
//	glVertex2i(5, 0);
//	glVertex2i(5, 5);
//	glVertex2i(0, 5);
//	glEnd();
	
//	glScaled(0.35, 0.35, 0.35);
//	drawBox();
	glPopMatrix();
	
//	glReadPixels(0, 0, 640, 480, GL_BGR, GL_UNSIGNED_BYTE, outImg.data);
//	flip(outImg,outImg,0);
//	char _str[64] = {0}; sprintf(_str, "img%05d.png",frameNum);
//	imwrite(_str, outImg);
//	frameNum++;

	glutSwapBuffers();
}

void mouse(int button, int state, int x, int y) {
	zt = ((double)y/480.0) * 10.0;
	glutPostWindowRedisplay(glutwin);
}


void
init(void)
{
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glEnable(GL_TEXTURE_2D);
	
	
	/* Setup cube vertex data. */
	v[0][0] = v[1][0] = v[2][0] = v[3][0] = 0;
	v[4][0] = v[5][0] = v[6][0] = v[7][0] = 1;
	v[0][1] = v[1][1] = v[4][1] = v[5][1] = 0;
	v[2][1] = v[3][1] = v[6][1] = v[7][1] = 1;
	v[0][2] = v[3][2] = v[4][2] = v[7][2] = 1;
	v[1][2] = v[2][2] = v[5][2] = v[6][2] = 0;
	
	/* Enable a single OpenGL light. */
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	
	/* Use depth buffering for hidden surface elimination. */
	glEnable(GL_DEPTH_TEST);
	
	/* Setup the view of the cube. */
	glMatrixMode(GL_PROJECTION);
	
	double fovx,fovy,focalLength,aspectRatio; Point2d principalPt;
	calibrationMatrixValues(camera_matrix, Size(640,480), 0.0, 0.0, fovx, fovy, focalLength, principalPt, aspectRatio);
	
	gluPerspective( /* field of view in degree */ fovy,
				   /* aspect ratio */ 1.0/aspectRatio,
				   /* Z near */ 1.0, /* Z far */ 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

//	gluLookAt(0.0, 0.0, 0.0,  /* eye is at (0,0,5) */
//			  0.0, 0.0, -10.0,      /* center is at (0,0,0) */
//			  0.0, 1.0, 0.);      /* up is in positive Y direction */
//	
//	/* Adjust cube position to be asthetic angle. */
//	glTranslatef(0.0, 0.0, -1.0);
//	glRotatef(60, 1.0, 0.0, 0.0);
//	glRotatef(-20, 0.0, 0.0, 1.0);
}

void* startGL(void* arg)
{
	glutMainLoop();
	
	return NULL;
}


void on_mouse(int event,int x,int y,int flag, void *param)
{
	if(event==CV_EVENT_LBUTTONDOWN)
	{
		if (imgPointsOnPlane.size() < 4) {
			imgPointsOnPlane.push_back(KeyPoint(x,y,1));
			Mat out;
			drawKeypoints(img_to_show, imgPointsOnPlane, out, Scalar(255));
			imshow("main2", out);
		}
	}
	if(event==CV_EVENT_MOUSEMOVE)
	{
	}
}

void cvtKeyPtoP(vector<KeyPoint>& kpts, vector<Point2f>& points) {
	points.clear();
	for (int i=0; i<kpts.size(); i++) points.push_back(kpts[i].pt);
}
void cvtPtoKpts(vector<KeyPoint>& kpts, vector<Point2f>& points) {
	kpts.clear();
	for (int i=0; i<points.size(); i++) kpts.push_back(KeyPoint(points[i],1));
}

void getPlanarSurface(vector<Point2f>& imgP) {
	
	Rodrigues(rotM,rvec);
	
	solvePnP(objPM, Mat(imgP), camera_matrix, distortion_coefficients, rvec, tvec, true);
//	memcpy(cam, tvec.data, sizeof(double)*3);
	
	Rodrigues(rvec,rotM);
	
//	double* _r = _d;
//	printf("rotation mat: \n %.4f %.4f %.4f\n%.4f %.4f %.4f\n%.4f %.4f %.4f\n",
//		   _r[0],_r[1],_r[2],_r[3],_r[4],_r[5],_r[6],_r[7],_r[8]);
	printf("traslation: %.3f %.3f %.3f\n",tv[0],tv[1],tv[2]);
	
	{
		double R[3][3]; memcpy(R, Mat(rotM.inv()).data, sizeof(double)*9);
		
		if (R[2][0] != -1 && R[2][0] != 1) {
			theta1 = -asin(R[2][0]);
			theta2 = CV_PI-theta1;
			psi1 = atan2(R[2][1] / cos(theta1) , R[2][2] / cos(theta1));
			psi2 = atan2(R[2][1] / cos(theta2) , R[2][2] / cos(theta2));
			phi1 = atan2(R[1][0] / cos(theta1), R[0][0] / cos(theta1));
			phi2 = atan2(R[1][0] / cos(theta2), R[0][0] / cos(theta2));
		} else {
			phi = 0; //can set to 0 
			if (R[2][0] == -1) {
				theta = CV_PI/2.0;
				psi = phi + atan2(R[0][1], R[0][2]);
			} else {
				theta = -CV_PI/2.0;
				psi = -phi + atan2(-R[0][1], -R[0][2]);
			}
		}
		
//		printf("euler angles: theta = %.3f phi = %.3f psi = %.3f\n theta1 = %.3f phi1 = %.3f psi1 = %.3f\n theta2 = %.3f phi2 = %.3f psi2 = %.3f\n",
//			   theta,phi,psi,theta1,phi1,psi1,theta2,phi2,psi2);
	}
}

void markPoints(Mat& img) {
	cvtColor(img, img_to_show, CV_BGR2GRAY);
	imshow("main2", img_to_show);
	imgPointsOnPlane.clear();
	while(imgPointsOnPlane.size() < 4) 
		waitKey(0);
		
	cvtKeyPtoP(imgPointsOnPlane,points1);			
//	cout << "Points: " << endl;
//	for (int i=0; i<4; i++) {
//		cout << points1[i].x << "," << points1[i].y << endl;
//	}
}	

VideoCapture cap;
Mat frame,img,prev,next;

void* initOCV(void* arg) {
	namedWindow("main2",CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback( "main2",on_mouse,NULL );

	tv[0]=0;tv[1]=0;tv[2]=4;

	cap.open("../../mov4.mp4");
	cap.set(CV_CAP_PROP_POS_FRAMES, 0);
			
	vector<Point3d> objP;
	objP.push_back(Point3d(0,0,0));
	objP.push_back(Point3d(5,0,0));
	objP.push_back(Point3d(5,5,0));
	objP.push_back(Point3d(0,5,0));
	Mat(objP).convertTo(objPM,CV_32F);

	cap >> frame;
	frame.copyTo(img);

	markPoints(img);
//	points1.push_back(Point2f(261,215)); //debug: no need to mark points every time..
//	points1.push_back(Point2f(414,214));
//	points1.push_back(Point2f(417,359));
//	points1.push_back(Point2f(245,357));
	cvtPtoKpts(imgPointsOnPlane,points1);
	
	return NULL;
}
	
void* startOCV(void* arg) {
	while (1) {
		cvtColor(img, prev, CV_BGR2GRAY);

		cap >> frame;
		if(frame.data == NULL) break;
		
		frame.copyTo(img);

		cvtColor(img, next, CV_BGR2GRAY);
		
		calcOpticalFlowPyrLK(prev, next, points1, points2, status, err, Size(30,30));
		cvtPtoKpts(imgPointsOnPlane, points2);
		points1.clear();
		points1 = points2;
		
		getPlanarSurface(points1);
		glutPostWindowRedisplay(glutwin);
		
//		{
			drawKeypoints(next, imgPointsOnPlane, img_to_show, Scalar(255));
			imshow("main2", img_to_show);
			int c = waitKey(30);
			if (c == ' ') {
				waitKey(0);
//				markPoints(img);
			}
//		}
	}
	return NULL;
}

void initGL(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitWindowSize(640, 480);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutwin = glutCreateWindow("red 3D lighted cube");
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	glutMouseFunc(mouse);
	init();
}	

int main(int argc, char** argv) {
	initGL(argc,argv);
	initOCV(NULL);
	
	pthread_t tId;
	pthread_attr_t tAttr;
	pthread_attr_init(&tAttr);
	pthread_create(&tId, &tAttr, startOCV, NULL);

	startGL(NULL);	
}