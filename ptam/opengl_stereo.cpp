#include "stdafx.h"
#include "utils.h"

static double a = 0.0;

void display(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

	//background
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	gluOrtho2D(0.0,352.0,288.0,0.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glDisable(GL_DEPTH_TEST);
	
	if(WaitForSingleObject(ghMutex, INFINITE) == WAIT_OBJECT_0) {
		glDrawPixels(352,288,GL_RGB,GL_UNSIGNED_BYTE,backPxls.data);
		ReleaseMutex(ghMutex);
	}
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();


    const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	a = t*20.0;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//make camera look in the proper direction (- z axis)
	//gluLookAt(	cam[0],cam[1],cam[2],
	//		cam[0],	cam[1],	-1,
	//		0,	1,	0);
	////glRotated(60,1,0,0);
	//glGetDoublev(GL_MODELVIEW_MATRIX,rot);

	//double xc = cam[0] - curCam[0];
	//double yc = cam[1] - curCam[1];
	//double zc = cam[2] - curCam[2];
	//if(fabs(xc) > 0.001 || fabs(yc) > 0.001 || fabs(zc) > 0.001) {
	//	curCam[0] += xc * t * 0.08;
	//	curCam[1] += yc * t * 0.08;
	//	curCam[2] += zc * t * 0.08;
	//	//printf("move %.3f %.3f %.3f by %.5f %.5f %.5f\n",curCam[0],curCam[1],curCam[2],xc,yc,zc);
	//}

	//curCam[0] = cam[0]; curCam[1] = cam[1]; curCam[2] = cam[2];
	//glTranslated(-curCam[0]+0.5,-curCam[1]+0.7,-curCam[2]);

	////double _d[16];
	////glGetDoublev(GL_MODELVIEW_MATRIX,_d);

	//double _d[16] = {	rot[0],rot[1],rot[2],0,
	//					rot[3],rot[4],rot[5],0,
	//					rot[6],rot[7],rot[8],0,
	//					0,	   0,	  0		,1};
	//glMultMatrixd(_d);

	//glRotated(180,1,0,0);

	double m[16] = {	rot[0],-rot[3],-rot[6],0,
		rot[1],-rot[4],-rot[7],0,
		rot[2],-rot[5],-rot[8],0,
		cam[0],-cam[1],-cam[2],1};

	glLoadMatrixd(m);


	/*draw features*/
	glPushMatrix();
	////glScaled(((double*)(camera_matrix.ptr()))[2],((double*)(camera_matrix.ptr()))[5],1.0);
	//gluLookAt(0,0,-2,0,0,1,0,1,0);
	//glScaled(.1,.1,.1);
	//glBegin(GL_POINTS);
	glColor4d(1.0,0.0,0.0,1.0);
	for(unsigned int i=0;i<points1Proj.size();i++) {
	//for(unsigned int i=0;i<points1.size();i++) {
		glPushMatrix();
	//	//glVertex3d(points1Proj[i].x,points1Proj[i].y,points1Proj[i].z);
		//glTranslated(points1Proj[i].x+2.5,points1Proj[i].y+3,points1Proj[i].z-9);
		glTranslated(points1Proj[i].x,points1Proj[i].y,points1Proj[i].z);
		//glTranslated(points1[i].x,points1[i].y,0);
		glutSolidSphere(0.03,15,15);
	//	//glVertex2d(points1[i].x,points1[i].y);
		glPopMatrix();
	}
	//glEnd();
	glPopMatrix();
	/**/

	/*draw plane grid..
	glPushMatrix();
	//move to estimated plane location
	gluLookAt(	0.0,0.0,-15.0,
				0.0,0.0,-1.0,
				0.0,1.0,0.0);

	glPushMatrix();
	//rotate to plane normal
	gluLookAt(	0.0,0.0,0.0,
				u[0],u[1],u[2], 	//normal of planar surface
				v[0],v[1],v[2]);

	//glRotated(a,0,0,1);

	//------- draw axes of plane --------
	glPushMatrix();
	glTranslated(0.1,0.1,0.1);
	glScaled(2.0,2.0,2.0);
	glBegin(GL_LINES);
	glColor4f(1.0f,0.0f,0.0f,1.0f); //RED = X
	glVertex3i(0,0,0);
	glVertex3i(1,0,0);
	glColor4f(0.0f,1.0f,0.0f,1.0f); //GREEN = Y
	glVertex3i(0,0,0);
	glVertex3i(0,1,0);
	glColor4f(0.0f,0.0f,1.0f,1.0f); //BLUE = Z
	glVertex3i(0,0,0);
	glVertex3i(0,0,1);
	glEnd();
	glPopMatrix();

	//-------- draw plane grid ---------
	glScalef(0.5,0.5,0.5);
	glTranslatef(-5,-5,0);
	glColor4f(1.0f,0.0f,1.0f,1.0f);
	glBegin(GL_LINES);
	int rows = 10, columns = 10;
		// Horizontal lines. 
		for (int i=0; i<=rows; i++) {
		  glVertex2i(0, i);
		  glVertex2i(columns, i);
		}
		// Vertical lines. 
		for (int i=0; i<=columns; i++) {
		  glVertex2i(i, 0);
		  glVertex2i(i, rows);
		}
	glEnd();

	glPopMatrix();

	glPopMatrix();
	/**/

	glutSwapBuffers();

	if(!running) {
		glutLeaveMainLoop();
	}

	Sleep(25);
}


int start_opengl_with_stereo(int argc,char** argv) {
	glutInitWindowSize(352,288);
    glutInitWindowPosition(40,40);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutCreateWindow("roy's p.t.a.m");

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