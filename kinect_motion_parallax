/*****************************************************************************
*                                                                            *
*  OpenNI 1.0 Alpha                                                          *
*  Copyright (C) 2010 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  OpenNI is free software: you can redistribute it and/or modify            *
*  it under the terms of the GNU Lesser General Public License as published  *
*  by the Free Software Foundation, either version 3 of the License, or      *
*  (at your option) any later version.                                       *
*                                                                            *
*  OpenNI is distributed in the hope that it will be useful,                 *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
*  GNU Lesser General Public License for more details.                       *
*                                                                            *
*  You should have received a copy of the GNU Lesser General Public License  *
*  along with OpenNI. If not, see <http://www.gnu.org/licenses/>.            *
*                                                                            *
*****************************************************************************/




//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------

#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glut.h>
#else
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#endif

#include <math.h>
#include <iostream>

#include <XnOpenNI.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>
#include "SceneDrawer.h"
#include "glm.h"
#include <set>

using namespace std;

#include "time.h"

#include <boost/numeric/ublas/vector.hpp>
//#include <boost/numeric/ublas/vector_proxy.hpp>
//#include <boost/numeric/ublas/matrix.hpp>
//#include <boost/numeric/ublas/triangular.hpp>
//#include <boost/numeric/ublas/lu.hpp>
//#include <boost/numeric/ublas/io.hpp>
using namespace boost::numeric::ublas;

//#include "no_face_track.h"

//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------
xn::Context g_Context;
xn::DepthGenerator g_DepthGenerator;
xn::UserGenerator g_UserGenerator;

XnBool g_bNeedPose = FALSE;
XnChar g_strPose[20] = "";
XnBool g_bDrawBackground = TRUE;
XnBool g_bDrawPixels = TRUE;
XnBool g_bDrawSkeleton = TRUE;
XnBool g_bPrintID = TRUE;
XnBool g_bPrintState = TRUE;

#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#define GL_WIN_SIZE_X 1024
#define GL_WIN_SIZE_Y 768

int win_w=1024,win_h=768,_tex_size=730;

#define TEX_SIZE _tex_size
#define BUFSIZE 512

#define OBJ_POS_X 0
#define OBJ_POS_Y -50.0
#define OBJ_POS_Z 500.0

double z_near = 1.0;

GLuint v,f,f2,p;

XnBool g_bPause = false;
XnBool g_bRecord = false;

XnBool g_bQuit = false;

XnSkeletonJointPosition Head;
XnSkeletonJointPosition lHand;
XnSkeletonJointPosition rHand;

GLMmodel *objmodel_ptr, *hand_objmodel_ptr, *hand_closed_objmodel_ptr;

//extern GLuint MyTexture;										// An Unsigned Int To Store The Texture Number
GLuint MyTexture;
float face_loc[3] = {0,0,2050};

GLfloat eye[4] = {0,200,1050,0};
double kinectHeight = 300;
GLdouble tlv[3] = {-530, -kinectHeight, 90}, 
		trv[3] = {530, -kinectHeight, 90}, 
		brv[3] = {530, -kinectHeight, 955}, 
		blv[3] = {-530, -kinectHeight, 955};
GLdouble obj[3] = {-200, tlv[1], 522.5};
GLdouble tl[3],tr[3],bl[3],br[3];
GLdouble tlnorm[3],trnorm[3],blnorm[3],brnorm[3];
float blvf[3] = {blv[0],blv[1],blv[2]};
float brvf[3] = {brv[0],brv[1],brv[2]};
float tlvf[3] = {tlv[0],tlv[1],tlv[2]};
//float eyef[3] = {eye[0],eye[1],eye[2]};
float lightp[3] = {0,1000,1000};

GLfloat selectColor[4] = {0.0,0.0,1.0,1.0},		//blue
		neutralColor[4] = {1.0,0.0,0.0,1.0},	//red
		grabColor[4] = {1.0,0.0,1.0,1.0},	//red
		green[4] = {0.0,1.0,0.0,1.0},
		yellow[4] = {1.0,1.0,0.0,1.0},
		blue[4] = {1.0,.0,.0,1.0};	

set<int> selectedTriangles;

bool isMousePressed = false;
bool grabbing = false;

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
//extern void idle();
void idle() { glutPostRedisplay(); }

static void reshape_fund(int _w,int _h) {
	win_w = _w;
	win_h = _h;
}

void processHits(GLint hits, GLuint buffer[])
{
   unsigned int i, j;
   GLuint names, *ptr;

   printf("hits = %d\n", hits);
   ptr = (GLuint *) buffer;
   for (i = 0; i < hits; i++) {  /* for each hit  */
      names = *ptr;
      printf(" number of names for hit = %d\n", names); ptr++;
      printf("  z1 is %g;", (float) *ptr/0x7fffffff); ptr++;
      printf(" z2 is %g\n", (float) *ptr/0x7fffffff); ptr++;
      printf("   the name is ");
      for (j = 0; j < names; j++) {  /* for each name */
         printf("%d ", *ptr); ptr++;
      }
      printf("\n");
   }
}

void drawScene(GLenum renderMode) {
	
	glPushMatrix();
	glTranslated(obj[0]-10,obj[1]+80,obj[2]);
	glColor4f(1.0, 0.0, 0.0, 1.0);
	glScaled(80,80,80);
	glmDraw(objmodel_ptr,GLM_SMOOTH,renderMode,selectedTriangles,selectColor,grabbing ? grabColor : neutralColor);
	glPopMatrix();
	
	if (renderMode != GL_SELECT) {
		glColor4fv(green);	//draw the table
		glBegin(GL_QUADS);
		glNormal3d(0.0, 1.0, 0.0);
		glVertex3dv(tlv);
		glNormal3d(0.0, 1.0, 0.0);
		glVertex3dv(trv);
		glNormal3d(0.0, 1.0, 0.0);
		glVertex3dv(brv);
		glNormal3d(0.0, 1.0, 0.0);
		glVertex3dv(blv);
		
		glEnd();

		if(rHand.position.X != .0f || rHand.position.Y != .0f || rHand.position.Z != .0f)
		{
			glPushMatrix();
			glTranslated(rHand.position.X,rHand.position.Y,rHand.position.Z);
			glColor4f(1.0, 1.0, 0, 1.0);
			glRotated(-30.0,1.0,.0,.0);
			glScaled(20,20,20);
			glmDraw(grabbing ? hand_closed_objmodel_ptr : hand_objmodel_ptr,GLM_SMOOTH,GL_RENDER);
			glPopMatrix();

			/*
			//boost::numeric::ublas::vector<double> vec_rHand(3);
			//vec_rHand[0] = rHand.position.X - eye[0];
			//vec_rHand[1] = rHand.position.Y - eye[1];
			//vec_rHand[2] = rHand.position.Z - eye[2];

			//vec_rHand /= norm_2(vec_rHand);
			//vec_rHand *=100000;

			//glPushMatrix();
			//glLoadIdentity();
			//glTranslated(rHand.position.X - eye[0], rHand.position.Y - eye[1], rHand.position.Z - eye[2]);
			//glColor3f(0.0f,1.0f,0.0f);
			//glBegin(GL_LINE_LOOP);
			//{
			//	glVertex3d(eye[0], eye[1], eye[2]) ; 
			//	glVertex3d(eye[0] + vec_rHand[0], eye[1] + vec_rHand[1], eye[2] + vec_rHand[2]) ;
			//}
			//glEnd();
			//glPopMatrix();
			 */
		}
		
		glEnable(GL_BLEND);
		glPushMatrix();
		glTranslated(tlv[0] + 600, tlv[1]+10, tlv[3]+200);
		glScaled(10.0, 3.0, 10.0);
		glColor4f(.0, .0, 1.0, .5);
		glutSolidCube(10);
		glPopMatrix();
		glDisable(GL_BLEND);
	}
}

void subtract(float u[3], float v[3], float n[3]) {
	u[0] = v[0] - n[0];
	u[1] = v[1] - n[1];
	u[2] = v[2] - n[2];
}

void projection( float *pa,
				float *pb,
				float *pc,
				float *pe, float n, float f)
{
	float va[3], vb[3], vc[3];
	float vr[3], vu[3], vn[3];
	float l, r, b, t, d, M[16];
	
	// Compute an orthonormal basis for the screen.
	subtract(vr, pb, pa);
	subtract(vu, pc, pa);
	
	
	glmNormalize(vr);
	glmNormalize(vu);
	glmCross(vr, vu, vn);
	glmNormalize(vn);
	
	// Compute the screen corner vectors.
	subtract(va, pa, pe);
	subtract(vb, pb, pe);
	subtract(vc, pc, pe);
	
	// Find the distance from the eye to screen plane.
	d = -glmDot(va, vn);
	
	// Find the extent of the perpendicular projection.
	l = glmDot(vr, va) * n / d;
	r = glmDot(vr, vb) * n / d;
	b = glmDot(vu, va) * n / d;
	t = glmDot(vu, vc) * n / d;
	// Load the perpendicular projection.
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glFrustum(l, r, b, t, n, f);
	// Rotate the projection to be non-perpendicular.
	memset(M, 0, 16 * sizeof (float));
	M[0] = vr[0]; M[4] = vr[1]; M[ 8] = vr[2];
	M[1] = vu[0]; M[5] = vu[1]; M[ 9] = vu[2];
	M[2] = vn[0]; M[6] = vn[1]; M[10] = vn[2];
	M[15] = 1.0f;
	glMultMatrixf(M);
	// Move the apex of the frustum to the origin.
	glTranslatef(-pe[0], -pe[1], -pe[2]);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
}

time_t start,end_t;

void offAxisView() {
	projection(blvf, brvf, tlvf, eye, 1.0f, 10000.0f);
	
	glLightfv(GL_LIGHT0, GL_POSITION, lightp);
	drawScene(GL_RENDER);
	
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	if(grabbing) {
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0,1,0,1);

		glMatrixMode(GL_MODELVIEW);
		time (&end_t);
		double dif = difftime (end_t,start);

		glPushMatrix();
		glTranslated(0.05,0.05,0.01);
		glScaled(2.0,2.0,1.0);
		glColor3f(1.0,0.0,1.0); 
		char buf[100]; sprintf(buf,"%.1f seconds\0",dif);
		int num = strlen(buf);
		glRasterPos2f(0,0);
		for(int i=0;i<num;i++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, buf[i]);
		}
		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}
}

static void display(GLenum mode)
{
	if(Head.position.X != 0.0f || Head.position.Y != 0.0f || Head.position.Z != 0.0f)
	{
		eye[0] = Head.position.X;
		eye[1] = Head.position.Y;
		eye[2] = Head.position.Z;
	}
	
	//GL...
	
	//glDisable(GL_TEXTURE_2D);
	
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	/*glViewport(0, 0, TEX_SIZE,TEX_SIZE);*/
		
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluPerspective( /* field of view in degree */ 90.0,
	//			   /* aspect ratio */ 1.0,
	//			   /* Z near */ 1.0, /* Z far */ 10000.0);
	//
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();	
	//
	//glLightfv(GL_LIGHT0, GL_POSITION, eye);
	//
	////-------------- eye view ----------------*/
	//glPushMatrix();
	//gluLookAt(eye[0], eye[1], eye[2], obj[0],obj[1],obj[2], 0, 1, 0);
	//drawScene(GL_RENDER);
	//glPopMatrix();

	//	//------------ perspective view ----------------
	//glViewport(0, 0, 400, 400);
	offAxisView();
	//----------------------------------------------

	/*
	if(lHand.position.X != NULL)
	{
		glPushMatrix();
		glTranslated(lHand.position.X,lHand.position.Y,lHand.position.Z);
		glColor4f(1.0, 1.0, 0, 1.0);
		glutSolidSphere(20,20,20);
		glPopMatrix();

		vector<double> vec_lHand(3);
		vec_lHand[0] = lHand.position.X - eye[0];
		vec_lHand[1] = lHand.position.Y - eye[1];
		vec_lHand[2] = lHand.position.Z - eye[2];

		vec_lHand /= norm_2(vec_lHand);
		vec_lHand *=100000;

		glPushMatrix();
		glLoadIdentity();
		glTranslated(rHand.position.X - eye[0], rHand.position.Y - eye[1], rHand.position.Z - eye[2]);
		glColor3f(0.0f,1.0f,0.0f);
		glBegin(GL_LINE_LOOP);
		{
			glVertex3d(eye[0], eye[1], eye[2]) ; 
			glVertex3d(eye[0] + vec_lHand[0], eye[1] + vec_lHand[1], eye[2] + vec_lHand[2]) ;
		}
		glEnd();
		glPopMatrix();
	}
*/


	glutSwapBuffers();
}

struct openni_stuff {
	xn::DepthGenerator* dg;
	xn::UserGenerator* ug;
	xn::Context* ctx;
	XnSkeletonJointPosition* Head;
	XnSkeletonJointPosition* rh;
	XnSkeletonJointPosition* lh;
};

// this function is called each frame
void glutDisplay (struct openni_stuff s)
{

	xn::SceneMetaData sceneMD;
	xn::DepthMetaData depthMD;
	//g_DepthGenerator.GetMetaData(depthMD);
	s.dg->GetMetaData(depthMD);

	if (!g_bPause)
	{
		// Read next available data
		//g_Context.WaitAndUpdateAll();
		s.ctx->WaitAndUpdateAll();
	}

	// Process the data
	g_DepthGenerator.GetMetaData(depthMD);
	s.dg->GetMetaData(depthMD);
	rHand.position.X = NULL;
	//g_UserGenerator.GetUserPixels(0, sceneMD);
	s.ug->GetUserPixels(0, sceneMD);
	DrawDepthMap(depthMD, sceneMD, *s.Head, *s.rh, *s.lh);

	/*
	if(rHand.position.X != NULL && isMousePressed == true)
	{
	   GLuint selectBuf[BUFSIZE];
	   GLint hits;
	   GLint viewport[4];

	   glGetIntegerv(GL_VIEWPORT, viewport);

	   glSelectBuffer(BUFSIZE, selectBuf);
	   (void) glRenderMode(GL_SELECT);

	   glInitNames();
	   glPushName(0);

	   //gluProject to get hand coords in screen format
		GLdouble _model[16],_proj[16];
		GLdouble _coors[3];
		GLint _view[4];
		glGetDoublev(GL_MODELVIEW_MATRIX, _model);
		glGetDoublev(GL_PROJECTION_MATRIX, _proj);
		glGetIntegerv(GL_VIEWPORT, _view);
		gluProject(rHand.position.X,rHand.position.Y,rHand.position.Z, _model, _proj, _view, &_coors[0], &_coors[1], &_coors[2]);


	   glMatrixMode(GL_PROJECTION);
	   glPushMatrix();
	   glLoadIdentity();
	   //  create 5x5 pixel picking region near cursor location 
	   gluPickMatrix((GLdouble) _coors[0], (GLdouble) (viewport[3] - _coors[1]),
					 5.0, 5.0, viewport);
	   //glOrtho(0.0, 8.0, 0.0, 8.0, -0.5, 2.5);
	   gluPerspective(  90.0,
				1.0,
			   1.0,  10000.0);
	   display(GL_SELECT);
	   glPopMatrix();
	   glFlush();

	   hits = glRenderMode(GL_RENDER);
	   processHits(hits, selectBuf);
	}
	else
*/
	//{
	//	display(GL_RENDER);
	//}
}
void CleanupExit()
{
	g_Context.Shutdown();

	exit (1);
}

// Callback: New user was detected
void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	printf("New User %d\n", nId);
	// New user found
	if (g_bNeedPose)
	{
		g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
	}
	else
	{
		g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
	}
}
// Callback: An existing user was lost
void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	printf("Lost user %d\n", nId);
}
// Callback: Detected a pose
void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie)
{
	printf("Pose %s detected for user %d\n", strPose, nId);
	g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
	g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}
// Callback: Started calibration
void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie)
{
	printf("Calibration started for user %d\n", nId);
}
// Callback: Finished calibration
void XN_CALLBACK_TYPE UserCalibration_CalibrationEnd(xn::SkeletonCapability& capability, XnUserID nId, XnBool bSuccess, void* pCookie)
{
	if (bSuccess)
	{
		// Calibration succeeded
		printf("Calibration complete, start tracking user %d\n", nId);
		g_UserGenerator.GetSkeletonCap().StartTracking(nId);
	}
	else
	{
		// Calibration failed
		printf("Calibration failed for user %d\n", nId);
		if (g_bNeedPose)
		{
			g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
		}
		else
		{
			g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
		}
	}
}

void glutIdle (void)
{
	if (g_bQuit) {
		CleanupExit();
	}

	// Display the frame
	glutPostRedisplay();
}

float lastP[3] = {-1.0f};
void glutKeyboard (unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		CleanupExit();
	case 'b':
		// Draw background?
		g_bDrawBackground = !g_bDrawBackground;
		break;
	case 'x':
		// Draw pixels at all?
		g_bDrawPixels = !g_bDrawPixels;
		break;
	case 's':
		// Draw Skeleton?
		g_bDrawSkeleton = !g_bDrawSkeleton;
		break;
	case 'i':
		// Print label?
		g_bPrintID = !g_bPrintID;
		break;
	case 'l':
		// Print ID & state as label, or only ID?
		g_bPrintState = !g_bPrintState;
		break;
	case'p':
		g_bPause = !g_bPause;
		break;
	case' ':
		if(!grabbing) {
			grabbing = true;
			lastP[0] = rHand.position.X;
			lastP[1] = rHand.position.Y;
			lastP[2] = rHand.position.Z;
			printf("grabbing\n");
			time (&start);
		} else {
			//printf("movement: %.0f %.0f %.0f\n",rHand.position.X - lastP[0],rHand.position.Y - lastP[1],rHand.position.Z - lastP[2]);
			obj[0] += rHand.position.X - lastP[0];
			obj[2] += rHand.position.Z - lastP[2];  //XZ plane only
			lastP[0] = rHand.position.X;
			lastP[1] = rHand.position.Y;
			lastP[2] = rHand.position.Z;
		}
		break;
	}
}

void keyupfunc(unsigned char key, int x, int y) {
	if(key == ' ') { grabbing = false; printf("done grabbing\n");}
}

void OnMouseClick(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) 
	{ 
		isMousePressed = true;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) 
	{ 
		isMousePressed = false;
	}

}

static void init( void )
{
	//setShaders();
	
	glViewport(0, 0, TEX_SIZE,TEX_SIZE);

	glClearColor(0.0f, 0.0f, .5f, 0.5);
	
	GLfloat mat_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat lm_ambient[] = { 0.4, 0.4, 0.4, 1.0 };
	GLfloat diffuseMaterial[4] = { 0.5, 0.5, 0.5, 1.0 };
	
	//	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	//glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMaterial);
	//glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	//glMaterialf(GL_FRONT, GL_SHININESS, 25.0);
	
	//	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lm_ambient);
    
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	
	glShadeModel (GL_SMOOTH);
	
	/* Use depth buffering for hidden surface elimination. */
	//	glEnable (GL_BLEND);
	//	glEnable (GL_POLYGON_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	
	glColorMaterial(GL_FRONT, GL_DIFFUSE);// GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	
	glEnable(GL_NORMALIZE); //to fix lighting (normals) after scalng
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); 
}

void justDisplay() { display(GL_RENDER); }

DWORD WINAPI MyThreadFunction( LPVOID lpParam ) {
	struct openni_stuff s = *((struct openni_stuff*)lpParam);
	for(;;) {
		glutDisplay(s);
		Sleep(30);
	}
	return 0;
}

void glInit (int * pargc, char ** argv)
{
	glutInit(pargc, argv);
	
	//GLUT SCENE BEGIN
	glutInitWindowSize(TEX_SIZE,TEX_SIZE*3/4);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	int glutwin = glutCreateWindow("opengl scene");
	glutDisplayFunc(justDisplay); //glutDisplay);
	glutMouseFunc(OnMouseClick);
	glutIdleFunc(justDisplay);//glutIdle);
	glutKeyboardFunc(glutKeyboard);
	glutKeyboardUpFunc(keyupfunc);

	if (!objmodel_ptr)
	{
	   objmodel_ptr = glmReadOBJ("C:/Users/Owner/Dropbox/hpars/bunny1.obj");
	   if (!objmodel_ptr)
		   exit(0);

	   glmUnitize(objmodel_ptr);
	   //printf("welding...");
	   //glmWeld(objmodel_ptr,0.01);
	   //printf("DONE\n");
	   glmFacetNormals(objmodel_ptr);
	   glmVertexNormals(objmodel_ptr, 90.0);

	   hand_objmodel_ptr = glmReadOBJ("C:/Users/Owner/Dropbox/hpars/hand.obj");
	   if (!hand_objmodel_ptr)
		   exit(0);

	   glmUnitize(hand_objmodel_ptr);
	   glmFacetNormals(hand_objmodel_ptr);
	   glmVertexNormals(hand_objmodel_ptr, 90.0);

	   hand_closed_objmodel_ptr = glmReadOBJ("C:/Users/Owner/Dropbox/hpars/hand-closed.obj");
	   if (!hand_closed_objmodel_ptr)
		   exit(0);

	   glmUnitize(hand_closed_objmodel_ptr);
	   glmFacetNormals(hand_closed_objmodel_ptr);
	   glmVertexNormals(hand_closed_objmodel_ptr, 90.0);

	}

	init();
	//GLUT SCENE END
}

#define SAMPLE_XML_PATH "C:/Program Files/OpenNI/Data/SamplesConfig.xml"

#define CHECK_RC(nRetVal, what)										\
	if (nRetVal != XN_STATUS_OK)									\
	{																\
		printf("%s failed: %s\n", what, xnGetStatusString(nRetVal));\
		return nRetVal;												\
	}

int main(int argc, char **argv)
{
	XnStatus nRetVal = XN_STATUS_OK;

	glInit(&argc, argv);

	if (argc > 1)
	{
		nRetVal = g_Context.Init();
		CHECK_RC(nRetVal, "Init");
		nRetVal = g_Context.OpenFileRecording(argv[1]);
		if (nRetVal != XN_STATUS_OK)
		{
			printf("Can't open recording %s: %s\n", argv[1], xnGetStatusString(nRetVal));
			return 1;
		}
	}
	else
	{
		xn::EnumerationErrors errors;
		nRetVal = g_Context.InitFromXmlFile(SAMPLE_XML_PATH, &errors);
		if (nRetVal == XN_STATUS_NO_NODE_PRESENT)
		{
			XnChar strError[1024];
			errors.ToString(strError, 1024);
			printf("%s\n", strError);
			return (nRetVal);
		}
		else if (nRetVal != XN_STATUS_OK)
		{
			printf("Open failed: %s\n", xnGetStatusString(nRetVal));
			return (nRetVal);
		}
	}

	nRetVal = g_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_DepthGenerator);
	CHECK_RC(nRetVal, "Find depth generator");
	nRetVal = g_Context.FindExistingNode(XN_NODE_TYPE_USER, g_UserGenerator);
	if (nRetVal != XN_STATUS_OK)
	{
		nRetVal = g_UserGenerator.Create(g_Context);
		CHECK_RC(nRetVal, "Find user generator");
	}

	XnCallbackHandle hUserCallbacks, hCalibrationCallbacks, hPoseCallbacks;
	if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
	{
		printf("Supplied user generator doesn't support skeleton\n");
		return 1;
	}
	g_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, hUserCallbacks);
	g_UserGenerator.GetSkeletonCap().RegisterCalibrationCallbacks(UserCalibration_CalibrationStart, UserCalibration_CalibrationEnd, NULL, hCalibrationCallbacks);

	if (g_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
	{
		g_bNeedPose = TRUE;
		if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
		{
			printf("Pose required, but not supported\n");
			return 1;
		}
		g_UserGenerator.GetPoseDetectionCap().RegisterToPoseCallbacks(UserPose_PoseDetected, NULL, NULL, hPoseCallbacks);
		g_UserGenerator.GetSkeletonCap().GetCalibrationPose(g_strPose);
	}

	g_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

	nRetVal = g_Context.StartGeneratingAll();
	CHECK_RC(nRetVal, "StartGenerating");

	DWORD threadid;
	struct openni_stuff s;
	s.ctx = &g_Context;
	s.dg = &g_DepthGenerator;
	s.ug = &g_UserGenerator;
	s.Head = &Head;
	s.rh = &rHand;
	s.lh = &lHand;
	CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
			MyThreadFunction,       // thread function name
            (LPVOID)(&s),          // argument to thread function 
            0,                      // use default creation flags 
            &threadid);   // returns the thread identifie

	glutMainLoop();
}