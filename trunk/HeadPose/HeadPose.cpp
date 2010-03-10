// HeadPose.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "cv.h"
#include "highgui.h"

using namespace cv;

#include <vector>

using namespace std;

#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/freeglut.h"

void loadNext();
void loadWithPoints(Mat& ip, Mat& img);

const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

double rot[9] = {0};
Vec3d eav;
GLuint textureID;
Mat backPxls;
vector<double> rv(3), tv(3);
Mat rvec(rv),tvec(tv);
Mat camMatrix;

void resize(int width, int height)
{
    const float ar = (float) width / (float) height;

    glViewport(0, 0, width, height);

	////FileStorage fs("D:/OpenCV2.0/samples/c/cam.out",CV_STORAGE_READ);
	////FileNode fn = fs["camera_matrix"];
	////Mat camera_matrix((CvMat*)fn.readObj(),true);
	////fn = fs["distortion_coefficients"];
	////Mat distortion_coefficients((CvMat*)fn.readObj(),true);
	//double fovx,fovy,focalength,aspectR;
	//Point2d princpPt;
	//calibrationMatrixValues(camera_matrix,Size(352,288),1.0,1.0,fovx,fovy,focalength,princpPt,aspectR);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);
	gluPerspective(40,1.0,0.01,1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity() ;
}

int __w=250,__h=250;

void key(unsigned char key, int x, int y)
{
	//static int counter = 0;

    switch (key)
    {
    case 27 :
    case 'Q':
    case 'q': 
		glutLeaveMainLoop();
		break;
	case 'w':
	case 'W':
		__w++;
		__w = __w%251;
		break;
	case 'h':
	case 'H':
		__h++;
		__h = __h%250;
		break;
	case ' ':
		loadNext();

		glBindTexture(GL_TEXTURE_2D, textureID);//This binds the texture to a texture target
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);//set our filter
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);    //set our filter
		glTexImage2D(GL_TEXTURE_2D, 0, 3, backPxls.cols, backPxls.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, backPxls.data);

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
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

	glShadeModel(GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);
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

	glGenTextures(1, &textureID);
}



void display(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

	//background
	//glMatrixMode(GL_PROJECTION);
	//glPushMatrix();
	//gluOrtho2D(0,250,0,250);
	//glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();
	//glDisable(GL_DEPTH_TEST);
	//
	////glDrawPixels(250,250,GL_RGB,GL_INT,backPxls.data);

	//// Dynamically create a big one-dimensional array for the pixels
	//uchar *pixels = new uchar[250*250];

	//for (int i=0; i < 250*250; i++) {

	//	//pixels[i] = 250 - (i%250);
	//	pixels[i] = i%250;
	//	/*
	//	pixels[i] = pixels[i] | 0x00bb0000;
	//	pixels[i] = pixels[i] | 0x0000cc00;*/
	//}
	//
	//glDrawPixels(__w,__h,GL_LUMINANCE,GL_UNSIGNED_BYTE,pixels);

	//delete[] pixels;

	//glEnable(GL_DEPTH_TEST);
	//glPopMatrix();
	//glMatrixMode(GL_PROJECTION);
	//glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//curCam[0] = cam[0]; curCam[1] = cam[1]; curCam[2] = cam[2];
	//glTranslated(-curCam[0]+0.5,-curCam[1]+0.7,-curCam[2]);

	gluLookAt(0,0,0,0,0,1,0,1,0);

	//----------Axes
	glPushMatrix();
	//gluLookAt(0,0,-5,0,0,0,0,1,0);
	glTranslated(0,0,5);

	glPushMatrix();
	//glRotated(eav[0],1,0,0);
	//glRotated(eav[1],0,1,0);
	//glRotated(eav[2],0,0,1);
	double _d[16] = {	rot[0],rot[1],rot[2],0,
						rot[3],rot[4],rot[5],0,
						rot[6],rot[7],rot[8],0,
						0,	   0,	  0		,1};
	glMultMatrixd(_d);
	glRotated(180,1,0,0);

	//glPushMatrix();
	//glScaled(1,1.5,0.5);
	//glColor4b(150,150,150,50);
	//glutSolidSphere(0.5,20,20);
	//glPopMatrix();

	/********************************/
	/*			draw picture		*/

	glEnable(GL_TEXTURE_2D);//(NEW) Enable 2D texturing
    glBindTexture(GL_TEXTURE_2D, textureID);// (NEW) used to select the texture you wanted to use

    glPushMatrix();

    //glRotatef(rotation, 0,0,1);//Rotate the square on the Z axis according to the rotation variable

    glBegin(GL_QUADS);// Just create a simple square
    glTexCoord2f(0,1);//(NEW) This is the coordinate position on the texture we want to draw at this vertex (This is the upper left corner)
    glVertex2f(-1,1);//This is the upper left corner of the square
    glTexCoord2f(1,1);//This is the upper right corner of the bitmap
    glVertex2f(1,1);
    glTexCoord2f(1,0);//This is the lower right corner of the bitmap
    glVertex2f(1,-1);
    glTexCoord2f(0,0);//The lower left corner
    glVertex2f(-1,-1);
    glEnd();

    glPopMatrix();

    glDisable(GL_TEXTURE_2D);//(NEW) Disable 2D texturing
	/*			end draw picture		*/
	/************************************/


	//Z = red
	glPushMatrix();
	glRotated(180,0,1,0);
	glColor3d(1,0,0);
	glutSolidCylinder(0.05,1,15,20);
	glTranslated(0,0,1);
	glScaled(.1,.1,.1);
	glutSolidTetrahedron();
	glPopMatrix();

	//Y = green
	glPushMatrix();
	glRotated(-90,1,0,0);
	glColor3d(0,1,0);
	glutSolidCylinder(0.05,1,15,20);
	glTranslated(0,0,1);
	glScaled(.1,.1,.1);
	glutSolidTetrahedron();
	glPopMatrix();

	//X = blue
	glPushMatrix();
	glRotated(-90,0,1,0);
	glColor3d(0,0,1);
	glutSolidCylinder(0.05,1,15,20);
	glTranslated(0,0,1);
	glScaled(.1,.1,.1);
	glutSolidTetrahedron();
	glPopMatrix();

	glPopMatrix();
	glPopMatrix();
	//----------End axes

	glutSwapBuffers();
}

int start_opengl_with_stereo(int argc,char** argv) {
	glutInitWindowSize(250,250);
    glutInitWindowPosition(40,40);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutCreateWindow("head pose");

	//stereoInit();

	myGLinit();

    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    //glutSpecialFunc(special);
    glutIdleFunc(idle);

    glutMainLoop();

	return 1;
}

Mat op;

void loadNext() {
	static int counter = 1;
	
	printf("load %d\n",counter);

	char buf[256] = {0};
	//sprintf(buf,"Angelina_Jolie/Angelina_Jolie_%04d.txt",counter);
	sprintf(buf,"vam8_img/%05d.txt",counter);

	vector<Point2f > imagePoints;
	FILE* f;
	//ss << ".txt";
	fopen_s(&f,buf,"r");
	for(int i=0;i<7;i++) {
		int x,y;
		fscanf_s(f,"%d",&x); fscanf_s(f,"%d",&y);
		imagePoints.push_back(Point2f((float)x,(float)y));
	}
	fclose(f);

	Mat ip(imagePoints);

	//sprintf(buf,"Angelina_Jolie/Angelina_Jolie_%04d.jpg",counter);
	sprintf(buf,"vam8_img/%05d.png",counter);

	Mat img = imread(buf);
	for(unsigned int i=0;i<imagePoints.size();i++) circle(img,imagePoints[i],2,Scalar(255,0,255),CV_FILLED);


	loadWithPoints(ip,img);

	counter = (counter+1);
}

void loadWithPoints(Mat& ip, Mat& img) {
	double _dc[] = {0,0,0,0};
	solvePnP(op,ip,camMatrix,Mat(1,4,CV_64FC1,_dc),rvec,tvec,true);

	Mat rotM(3,3,CV_64FC1,rot);
	Rodrigues(rvec,rotM);
	double* _r = rotM.ptr<double>();
	printf("rotation mat: \n %.3f %.3f %.3f\n%.3f %.3f %.3f\n%.3f %.3f %.3f\n",
		_r[0],_r[1],_r[2],_r[3],_r[4],_r[5],_r[6],_r[7],_r[8]);


	Mat tmp,tmp1,tmp2,tmp3,tmp4,tmp5;
	double _pm[12] = {_r[0],_r[1],_r[2],0,
					  _r[3],_r[4],_r[5],0,
					  _r[6],_r[7],_r[8],0};
	decomposeProjectionMatrix(Mat(3,4,CV_64FC1,_pm),tmp,tmp1,tmp2,tmp3,tmp4,tmp5,eav);

	printf("euler angles: %.5f %.5f %.5f\n",eav[0],eav[1],eav[2]);
	
	imshow("tmp",img);
	//waitKey();
	img.copyTo(backPxls);
	cvtColor(backPxls,backPxls,CV_BGR2RGB);
	cvFlip(&cvMat(250,250,CV_8UC3,backPxls.data),0,-1);
}


int main(int argc, char** argv)
{

	vector<Point3f > modelPoints;
	modelPoints.push_back(Point3f(-36.9522f,39.3518f,47.1217f));	//l eye
	modelPoints.push_back(Point3f(35.446f,38.4345f,47.6468f));		//r eye
	modelPoints.push_back(Point3f(-0.0697709f,18.6015f,87.9695f)); //nose
	modelPoints.push_back(Point3f(-27.6439f,-29.6388f,73.8551f));	//l mouth
	modelPoints.push_back(Point3f(28.7793f,-29.2935f,72.7329f));	//r mouth
	modelPoints.push_back(Point3f(-87.2155f,15.5829f,-45.1352f));	//l ear
	modelPoints.push_back(Point3f(85.8383f,14.9023f,-46.3169f));	//r ear

	op = Mat(modelPoints);
	op = op / 35;

	rvec = Mat(rv);
	double _d[9] = {1,0,0,
					0,-1,0,
					0,0,-1};
	Rodrigues(Mat(3,3,CV_64FC1,_d),rvec);
	tv[0]=0;tv[1]=0;tv[2]=1;
	tvec = Mat(tv);
	double _cm[9] = { 20, 0, 160,
					  0, 20, 120,
					  0,  0,   1 };
	camMatrix = Mat(3,3,CV_64FC1,_cm);

//	VideoCapture cap("vam8-1.wmv"); // open the default camera
//	if(!cap.isOpened()) {  // check if we succeeded
//		fprintf(stderr,"Can't open video\n");
//        return -1;
//	}
////
////    Mat edges;
//    namedWindow("edges",1);
//    for(;;)
//    {
//        Mat frame;
//        cap >> frame; // get a new frame from camera
//        imshow("edges", frame);
//        if(waitKey(30) >= 0) break;
////    }
    // the camera will be deinitialized automatically in VideoCapture destructor

	namedWindow("tmp",1);
	loadNext();

	start_opengl_with_stereo(argc,argv);

	return 0;
}


