#include "NyAR_core.h"

#define CODE_FILE "../../data/patt.hiro"
#define DATA_FILE "../../data/320x240ABGR.raw"
#define CAMERA_FILE "../../data/logitech.dat"
#include <cstdio>

#include <cv.h>
#include <highgui.h>
#include <cxcore.h>

#include <windows.h>   // Standard Header For Most Programs
#include <gl/gl.h>     // The GL Header File
#include <gl/glu.h>
#include <gl/openglut.h>   // The GL Utility Toolkit (Glut) Header

#include "objloader.h"
#include "util.h"

using namespace NyARToolkitCPP;
using namespace std;

NyARParam ap;
NyARCode* code;
NyARRgbRaster_BGRA* ra;
NyARSingleDetectMarker* ar;
NyARTransMatResult result_mat;
IplImage* frame, *image, *gray, *bgra_img, *flipped;
CvMat* dft;
CvCapture* capture = 0;
int win_w, win_h;
std::vector<Triangle> mesh;
double camera_proj[16] = {0.0f};
double view_distance_min = 0.1;
double view_distance_max = 100.0;
float xrot = 0.0f;
//GLfloat LightAmbient[]=		{ 0.5f, 0.5f, 0.1f, 1.0f };
//GLfloat LightDiffuse[]=		{ 1.0f, 1.0f, 1.0f, 1.0f };
//GLfloat LightPosition[]=	{ 0.0f, 0.0f, 10.0f, 1.0f };

    GLfloat   mat_ambient[]     = {0.0, 0.0, 1.0, 1.0};
    GLfloat   mat_flash[]       = {0.0, 0.0, 0, 1.0};
    GLfloat   mat_flash_shiny[] = {50.0};
    GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
    GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
    GLfloat   lightZeroColor[]  = {0.1, 0.1, 0.1, 0.1};


void arglCameraFrustumRH(const NyARParam& cparam, const double focalmin, const double focalmax, GLdouble m_projection[16]);



void init ( GLvoid )     // Create Some Everyday Functions
{

	ap.setEndian(TNyAREndian_LITTLE);
	ap.loadARParamFromFile(CAMERA_FILE);

	code= new NyARCode(16, 16);
	code->loadARPattFromFile(CODE_FILE);
	
	capture = cvCaptureFromCAM('0');
	//cvNamedWindow("input");

	frame = cvQueryFrame( capture );
	if( frame ) {
		if(ra == NULL) {
			ra = new NyARRgbRaster_BGRA(frame->width, frame->height);
			ap.changeScreenSize(frame->width, frame->height);
			ar = new NyARSingleDetectMarker(ap, code, 80.0);
			code=NULL;/*codeの所有権はNyARSingleDetectMarkerへ渡移動*/
			ar->setContinueMode(false);

			arglCameraFrustumRH(ap,1.0,100.0,camera_proj);
		}

		if( !image )
		{
			CvSize s = cvGetSize(frame);
			image = cvCreateImage(s , 8, 3 );
			image->origin = frame->origin;
			bgra_img = cvCreateImage(s,8,4);
			bgra_img->origin = frame->origin;
			gray = cvCreateImage(s,8,1);
			gray->origin = frame->origin;
			flipped = cvCreateImage(s,8,3);

			dft = cvCreateMat(gray->height,gray->width,CV_32FC1);

			ra->setBuffer((NyARToolkitCPP::NyAR_BYTE_t*)bgra_img->imageData);
		}
		win_w = frame->width;
		win_h = frame->height;
	}

	mesh = loadFromFile("c:/downloads/apple.obj");

	glShadeModel(GL_FLAT);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}



void display ( void )   // Create The Display Function
{
/******************************************************************
OpenCV & NyARToolkit
*/
        int c;
		bool doMult = false;

        frame = cvQueryFrame( capture );
		if( frame ) {

			cvCopy( frame, image);
			cvCvtColor(image,gray,CV_RGB2GRAY);
			cvCvtColor(gray,bgra_img,CV_GRAY2BGRA);	
			cvCvtColor(frame,flipped,CV_RGB2BGR);
			cvFlip(flipped);

			//cvShowImage("input",gray);

			if(ar->detectMarkerLite(*ra, 100)) {
				ar->getTransmationMatrix(result_mat);
			//	printf("Marker confidence\n cf=%f,direction=%d\n",ar->getConfidence(),ar->getDirection());
				//printf("Transform Matrix\n");
				//printf(
				//	"% 4.8f,% 4.8f,% 4.8f,% 4.8f\n"
				//	"% 4.8f,% 4.8f,% 4.8f,% 4.8f\n"
				//	"% 4.8f,% 4.8f,% 4.8f,% 4.8f\n",
				//	result_mat.m00,result_mat.m01,result_mat.m02,result_mat.m03,
				//	result_mat.m10,result_mat.m11,result_mat.m12,result_mat.m13,
				//	result_mat.m20,result_mat.m21,result_mat.m22,result_mat.m23);
				doMult = true;
			}
		}

/*******************************************************
OpenGL
*/
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

  glDisable(GL_LIGHTING);

	//draw background
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0,win_w, 0.0,win_h);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDrawPixels(win_w,win_h,GL_RGB,GL_UNSIGNED_BYTE,flipped->imageData);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

	//check if marker was found
	if(doMult) {
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixd(camera_proj);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		float m[16] = {0.0f}; m[0] = 1.0f; m[5] = 1.0f; m[10] = 1.0f; m[15] = 1.0f;
		toCameraViewRH(result_mat,m);
		glLoadMatrixf(m);
	} else {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
    //glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
    //glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);	
    //glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMatrixMode(GL_MODELVIEW);

	//draw 3D model
	//glPushMatrix();

	//glBegin(GL_LINES);
	//	glColor3d(1,0,0);
	//	glVertex3i(0,0,0);
	//	glVertex3i(0,0,10);
	//	glColor3d(0,1,0);
	//	glVertex3i(0,0,0);
	//	glVertex3i(0,10,0);
	//	glColor3d(0,0,1);
	//	glVertex3i(0,0,0);
	//	glVertex3i(10,0,0);
	//glEnd();

	glPushMatrix();
	glScalef(.02f,.02f,.02f);

	glRotatef(xrot,0.0f,0.0f,1.0f);
	xrot+=5.0f;

	glRotated(90,1,0,0);
	//glTranslated(-55,0,0);

	glutSolidCube(50.0);

	//glBegin(GL_TRIANGLES);								// Drawing Using Triangles
	//for(std::vector<Triangle>::iterator itr = mesh.begin();	itr != mesh.end(); ++itr) {
	//	glNormal3f((*itr).a.normal.x,(*itr).a.normal.y,(*itr).a.normal.z);
	//	glVertex3f((*itr).a.location.x,(*itr).a.location.y,(*itr).a.location.z);
	//	glNormal3f((*itr).b.normal.x,(*itr).b.normal.y,(*itr).b.normal.z);
	//	glVertex3f((*itr).b.location.x,(*itr).b.location.y,(*itr).b.location.z);
	//	glNormal3f((*itr).c.normal.x,(*itr).c.normal.y,(*itr).c.normal.z);
	//	glVertex3f((*itr).c.location.x,(*itr).c.location.y,(*itr).c.location.z);
	//}
	//glEnd();
	glPopMatrix();

	//glPopMatrix();
    glDisable( GL_LIGHTING );

    glDisable( GL_DEPTH_TEST );

	glutSwapBuffers ( );
  // Swap The Buffers To Not Be Left With A Clear Screen
}

void reshape ( int w, int h )   // Create The Reshape Function (the viewport)
{
  glViewport     ( 0, 0, w, h );
  glMatrixMode   ( GL_PROJECTION );  // Select The Projection Matrix
  glLoadIdentity ( );                // Reset The Projection Matrix
  if ( h==0 )  // Calculate The Aspect Ratio Of The Window
     gluPerspective ( 80, ( float ) w, 1.0, 5000.0 );
  else
     gluPerspective ( 80, ( float ) w / ( float ) h, 1.0, 5000.0 );
  glMatrixMode   ( GL_MODELVIEW );  // Select The Model View Matrix
  glLoadIdentity ( );    // Reset The Model View Matrix
}

void keyboard ( unsigned char key, int x, int y )  // Create Keyboard Function
{
  switch ( key ) {
    case 27:        // When Escape Is Pressed...
      exit ( 0 );   // Exit The Program
      break;        // Ready For Next Case
    default:        // Now Wrap It Up
      break;
  }
}

void arrow_keys ( int a_keys, int x, int y )  // Create Special Function (required for arrow keys)
{
  switch ( a_keys ) {
    case GLUT_KEY_UP:     // When Up Arrow Is Pressed...
      glutFullScreen ( ); // Go Into Full Screen Mode
      break;
    case GLUT_KEY_DOWN:               // When Down Arrow Is Pressed...
      glutReshapeWindow ( 500, 500 ); // Go Into A 500 By 500 Window
      break;
    default:
      break;
  }
}

void main(int argc,char* argv[])
{
	{
	code = NULL; ra = NULL; ar = NULL; frame = NULL; image = NULL; bgra_img = NULL;

	/**
	GLUT
	*/
	glutInit            ( &argc, argv ); // Erm Just Write It =)
	init ();
	glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE ); // Display Mode
	glutInitWindowSize  ( win_w, win_h ); // If glutFullScreen wasn't called this is the window size
	glutCreateWindow    ( "NeHe's OpenGL Framework" ); // Window Title (argv[0] for current directory as title)
//	glutFullScreen      ( );          // Put Into Full Screen
	glutDisplayFunc     ( display );  // Matching Earlier Functions To Their Counterparts
	glutIdleFunc(display);
	glutReshapeFunc     ( reshape );
	glutKeyboardFunc    ( keyboard );
	glutSpecialFunc     ( arrow_keys );
	glutMainLoop        ( );          // Initialize The Main Loop

	//DWORD st=GetTickCount();

	// マーカーを検出1000回分の処理時間を計測
	//for (int i = 0; i < 1000; i++) {
	//	// 変換行列を取得
	//	ar->detectMarkerLite(*ra, 100);
	//	ar->getTransmationMatrix(result_mat);
	//}
	//printf("done.\ntotal=%u[ms]\n",GetTickCount()-st);

	cvReleaseCapture(&capture);
	cvReleaseImage(&image);
	//cvReleaseImage(&frame);
	cvReleaseImage(&bgra_img);
	cvDestroyAllWindows();

	delete ra;
	delete ar;
	//delete buf;
	}
	_CrtDumpMemoryLeaks();
	return;
}
