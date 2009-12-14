/*
 * GrabCut implementation source code Copyright(c) 2005-2006 Justin Talbot
 *
 * All Rights Reserved.
 * For educational use only; commercial use expressly forbidden.
 * NO WARRANTY, express or implied, for this software.
 */

#include "Global.h"
#include "GrabCut.h"

#include <stdlib.h>
#include "GL/glut.h"

Image<Color> *displayImage;
GrabCut *grabCut = 0;

// Some state variables for the UI
Real xstart, ystart, xend, yend;
bool box = false;
bool initialized = false;
bool left = false, right = false;
bool refining = false;
bool showMask = false;
int displayType = 0;
int edits = 0;

void init()
{
	//set the background color to black (RGBA)
	glClearColor(0.0,0.0,0.0,0.0);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}

// Draw the image and paint the chosen mask over the top.
void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	grabCut->display(displayType);

	if (showMask)
	{
		grabCut->overlayAlpha();
	}

	if (box)
	{
		glColor4f( 1, 1, 1, 1 );
		glBegin( GL_LINE_LOOP );
		glVertex2d( xstart, ystart );
		glVertex2d( xstart, yend );
		glVertex2d( xend, yend );
		glVertex2d( xend, ystart );
		glEnd();
	}

	glFlush();
	glutSwapBuffers();
}

void idle()
{
	int changed = 0;

	if (refining)
		{
		changed = grabCut->refineOnce();
		glutPostRedisplay();
		}

	if (!changed)
		{
		refining = false;
		glutIdleFunc(NULL);
		}
}

void mouse(int button, int state, int x, int y)
{  
	y = displayImage->height() - y;

	switch(button) 
	{
	case GLUT_LEFT_BUTTON:
		if (state==GLUT_DOWN)
		{
			left = true;

			if (!initialized)
			{
				xstart = x; ystart = y;
				box = true;
			}
		}

		if( state==GLUT_UP )
		{
			left = false;

			if( initialized )
			{
				grabCut->refineOnce();
				glutPostRedisplay();
			}

			else
			{
				xend = x; yend = y;
				grabCut->initialize(xstart, ystart, xend, yend);
				grabCut->fitGMMs();
				box = false;
				initialized = true;
				showMask = true;
				glutPostRedisplay();
			}           
		}
		break;

	case GLUT_RIGHT_BUTTON:
		if( state==GLUT_DOWN )
		{
			right = true;
		}
		if( state==GLUT_UP )
		{
			right = false;

			if( initialized )
			{
				grabCut->refineOnce();
				glutPostRedisplay();
			}
		}
		break;

	default:
		break;
	}
}

void motion(int x, int y)
{
	y = displayImage->height() - y;  

	if( box == true )
	{
		xend = x; yend = y;
		glutPostRedisplay();
	}

	if( initialized )
	{      
		if( left )
			grabCut->setTrimap(x-2,y-2,x+2,y+2,TrimapForeground);
			
		if( right )
			grabCut->setTrimap(x-2,y-2,x+2,y+2,TrimapBackground);

		glutPostRedisplay();
	}
}

void keyboard(unsigned char key, int x, int y)
{
	y = displayImage->height() - y;

	switch (key)
	{
	case 'q':case 'Q':				// quit
		exit(0);
		break;
	  
	case ' ':						// space bar show/hide alpha mask
		showMask = !showMask;
		break;

	case '1': case 'i': case 'I':	// choose the image
		displayType = 0;
		break;

	case '2': case 'g': case 'G':	// choose GMM index mask
		displayType = 1;
		break;

	case '3': case 'n': case 'N':	// choose N-Link mask
		displayType = 2;
		break;

	case '4': case 't': case 'T':	// choose T-Link mask
		displayType = 3;
		break;

	case 'r':						// run GrabCut refinement
		refining = true;
		glutIdleFunc(idle);
		break;

	case 'o':						// run one step of GrabCut refinement
		grabCut->refineOnce();
		glutPostRedisplay();
		break;

	case 'l':
		grabCut->fitGMMs();			// rerun the Orchard-Bowman GMM clustering
		glutPostRedisplay();
		break;

	case 27:
		refining = false;
		glutIdleFunc(NULL);

	default:
		break;    
	}

	glutPostRedisplay();
}

int main(int argc, char** argv) {
	Image<Color>* image = load( argv[1] );
	Image<Color>* mask = load(argv[2]);
	displayImage = image;

	grabCut = new GrabCut( image );
	grabCut->initializeWithMask(mask);
	grabCut->fitGMMs();
	initialized = true;

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	glutInitWindowSize(displayImage->width(),displayImage->height());
	glutInitWindowPosition(100,100);
	  
	glutCreateWindow("GrabCut - Justin Talbot");
	  
	glOrtho(0,displayImage->width(),0,displayImage->height(),-1,1);
	  
	init();

	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);

	glutMainLoop();		//note: this will NEVER return.
  
  return 0;
}

int __main (int argc,char **argv)
{
  Image<Color>* image = load( argv[1] );

  if (image)
  {
	displayImage = image;

	grabCut = new GrabCut( image );

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	glutInitWindowSize(displayImage->width(),displayImage->height());
	glutInitWindowPosition(100,100);
	  
	glutCreateWindow("GrabCut - Justin Talbot");
	  
	glOrtho(0,displayImage->width(),0,displayImage->height(),-1,1);
	  
	init();

	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);

	glutMainLoop();		//note: this will NEVER return.
  }
  
  return 0;

}

