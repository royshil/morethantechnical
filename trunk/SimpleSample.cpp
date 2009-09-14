/*	SimpleSample plugin for Avisynth -- a simple sample

	Copyright (C) 2002-2003 Simon Walters, All Rights Reserved

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.


	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	The author can be contacted at:
	Simon Walters
	siwalters@hotmail.com

	V1.0 - 1st release.
	V1.0a - Revised version to put colourspace checking in the right place.
  V1.0b - Added detailed comments.  // sh0dan

*/
//following 2 includes needed
#include <windows.h>
#include <avisynth.h>
#include "glee.h"
#include <openglut.h>
#include <stdio.h>

typedef unsigned char uchar;
/****************************
 * The following is the header definitions.
 * For larger projects, move this into a .h file
 * that can be included.
 ****************************/

	int width;		// The hight of the texture we'll be rendering to
	int height;		// The width of the texture we'll be rendering to

	// Used for drawing the 3D cube with our rendered texture on it
	GLfloat	xrot;			// X Rotation
	GLfloat	yrot;			// Y Rotation
	GLfloat xspeed;		// X Rotation Speed
	GLfloat yspeed;		// Y Rotation Speed
	GLuint tex;
	int staylength;
	int zoomlength;
	int length;
	int frame;
	GLfloat ratio;

	BYTE* img_data;
	uchar* texBuf;

void display();

class SimpleSample : public GenericVideoFilter {   
  // SimpleSample defines the name of your filter class. 
  // This name is only used internally, and does not affect the name of your filter or similar.
  // This filter extends GenericVideoFilter, which incorporates basic functionality.
  // All functions present in the filter must also be present here.

public:
	~SimpleSample();
  // This defines that these functions are present in your class.
  // These functions must be that same as those actually implemented.
  // Since the functions are "public" they are accessible to other classes.
  // Otherwise they can only be called from functions within the class itself.

	SimpleSample(PClip _child, IScriptEnvironment* env);
  // This is the constructor. It does not return eny value, and is always used, 
  //  when an instance of the class is created.
  // Since there is no code in this, this is the definition.

	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  // This is the function that AviSynth calls to get a given frame.
  // So when this functions gets called, the filter is supposed to return frame n.

	//void display();
};


SimpleSample::~SimpleSample() {
	free(img_data);
	free(texBuf);
}
/***************************
 * The following is the implementation 
 * of the defined functions.
 ***************************/

SimpleSample::SimpleSample(PClip _child, IScriptEnvironment* env) : GenericVideoFilter(_child) {
  // This is the implementation of the constructor.
  // The child clip (source clip) is inherited by the GenericVideoFilter,
  //  where the following variables gets defined:
  //   PClip child;   // Contains the source clip.
  //   VideoInfo vi;  // Contains videoinfo on the source clip.

	if (vi.IsPlanar()) // is input planar? 
		env->ThrowError("SimpleSample: input to filter must be in YUY2 or RGB");   
    // This filter does not not support planar images.

	PVideoFrame src = child->GetFrame(1, env);
	width = src->GetRowSize() / 3;
	height = src->GetHeight();

	img_data = (BYTE*)malloc(sizeof(BYTE) * width * height * 3);
	texBuf = (uchar*)malloc(width * height * 3);

	// Used for drawing the 3D cube with our rendered texture on it
	xrot = 0;			// X Rotation
	yrot = 0;			// Y Rotation
	int num_frames = child->GetVideoInfo().num_frames;

	length = num_frames;
	int tmp = (float)length / 5.0f;
	zoomlength = tmp;
	staylength = tmp * 3;

	xspeed = 360.0f / (float)staylength;		// X Rotation Speed
	yspeed = 360.0f / (float)staylength;		// Y Rotation Speed
}

void display() {
	glViewport(0,0,width,height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();

	//The "scene": zoom out, rotate, zoom back in
	if(frame <= zoomlength) {
		//zoom out
		glTranslatef(0.0f,0.0f,-2.0f - (float)(frame)/(float)(zoomlength));
	} else if(frame > staylength + zoomlength && frame <= length) {
		//zoom in
		glTranslatef(0.0f,0.0f,-3.0f + (float)(frame - staylength - zoomlength)/(float)zoomlength);
	} else {
		//rotate
		glTranslatef(0.0f,0.0f,-3.0f);
		glRotatef(xrot,1.0f,0.0f,0.0f);
		glRotatef(yrot,0.0f,1.0f,0.0f);
		xrot+=xspeed;
		yrot+=yspeed;
	}

	glBindTexture(GL_TEXTURE_2D,tex);

	glBegin(GL_QUADS);
		// Front Face
		glNormal3f( 0.0f, 0.0f, 1.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0,  1.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0, -1.0,  1.0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0,  1.0,  1.0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0,  1.0,  1.0);
		// Back Face
		glNormal3f( 0.0f, 0.0f,-1.0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0, -1.0, -1.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0,  1.0, -1.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0,  1.0, -1.0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0, -1.0, -1.0);
		// Top Face
		glNormal3f( 0.0f, 1.0, 0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0,  1.0, -1.0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0,  1.0,  1.0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0,  1.0,  1.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0,  1.0, -1.0);
		// Bottom Face
		glNormal3f( 0.0f,-1.0, 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0, -1.0, -1.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0, -1.0, -1.0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0, -1.0,  1.0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0, -1.0,  1.0);
		// Right face
		glNormal3f( 1.0, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0, -1.0, -1.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0,  1.0, -1.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0,  1.0,  1.0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0, -1.0,  1.0);
		// Left Face
		glNormal3f(-1.0, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0, -1.0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0, -1.0,  1.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0,  1.0,  1.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0,  1.0, -1.0);
		
	glEnd();

	glFlush();
	glFinish();

	glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,img_data);

	glutLeaveMainLoop();
}
void reshape(int w,int h)			
{
	glViewport( 0, 0, w, h );
	glMatrixMode(GL_PROJECTION);	
	glLoadIdentity();					
	if ( h==0 )							
		gluPerspective(80,(float)w,1.0,5000.0);
	else
		gluPerspective(80,(float)w/(float)h,1.0,5000.0);
	glMatrixMode(GL_MODELVIEW);	
	glLoadIdentity();					
}

void keyboard(unsigned char key,int x,int y)  
{
	switch(key)
	{
	case 27:				// When Escape Is Pressed...
		//ShutDown();
		exit(0);			// Exit The Program
		break;				
	default:				
		break;
	}
}

void idle(void)
{
	glutPostRedisplay();
}

PVideoFrame __stdcall SimpleSample::GetFrame(int n, IScriptEnvironment* env) {
// This is the implementation of the GetFrame function.
// See the header definition for further info.

	PVideoFrame src = child->GetFrame(n, env);
   // Request frame 'n' from the child (source) clip.
	PVideoFrame dst = env->NewVideoFrame(vi);
   // Construct a frame based on the information of the current frame
   // contained in the "vi" struct.
	
	const unsigned char* srcp = src->GetReadPtr();
  // Request a Read pointer from the source frame.
  // This will return the position of the upperleft pixel in YUY2 images,
  // and return the lower-left pixel in RGB.
  // RGB images are stored upside-down in memory. 
  // You should still process images from line 0 to height.

	unsigned char* dstp = dst->GetWritePtr();
	// Request a Write pointer from the newly created destination image.
  // You can request a writepointer to images that have just been
  // created by NewVideoFrame. If you recieve a frame from PClip->GetFrame(...)
  // you must call env->MakeWritable(&frame) be recieve a valid write pointer.
	
	const int dst_pitch = dst->GetPitch();
  // Requests pitch (length of a line) of the destination image.
  // For more information on pitch see: http://www.avisynth.org/index.php?page=WorkingWithImages

	const int dst_width = dst->GetRowSize();
  // Requests rowsize (number of used bytes in a line.
  // See the link above for more information.

	const int dst_height = dst->GetHeight();
  // Requests the height of the destination image.

	const int src_pitch = src->GetPitch();
	const int src_width = src->GetRowSize();
	const int src_height = src->GetHeight();
	
	//int w, h;

	glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE /*| GLUT_OFFSCREEN*/);		// Display Mode
	glutInitWindowSize(width,height);
	glutCreateWindow( "FrameBuffer Object Example - Press ESC to exit" );

	glutDisplayFunc     ( display );  
	glutReshapeFunc     ( reshape );
	glutKeyboardFunc    ( keyboard );
	glutIdleFunc		( idle );

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);			
	glDepthFunc(GL_LEQUAL);				
	glEnable(GL_TEXTURE_2D);

	GLeeInit();

	//Shuffle the bytes from the input frame onto the texture buffer
	memset(texBuf,0,width*height*3);
	int line_length = width*3;
	for(int y=0;y<height;y++) {
		const unsigned char* line_srcp = srcp + y*src_pitch;
		unsigned char* line_dstp = texBuf + (height - 1 - y)*line_length;
		memcpy(line_dstp,line_srcp,line_length);
	}

	glGenTextures(1,&tex);
    glBindTexture(GL_TEXTURE_2D, tex);
	gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width,height,
                       GL_RGB, GL_UNSIGNED_BYTE, texBuf );

	glClearColor(0.0f, 0.0f, 0.2f, 0.5f);
	glClearDepth(1.0f);					
	glViewport(0,0,width,height);

	frame = n;

	glutMainLoop();

	//Shuffle the bytes back to the output image buffer
	for(int y=0;y<height;y++) {
		unsigned char* line_dstp = dstp + y*dst_pitch;
		unsigned char* line_srcp = img_data + y*width*3;
		memcpy(line_dstp,line_srcp,line_length);
		//for(int x=0;x<width;x++) {
		//	unsigned char p[3] = {0};
		//	p[0] = img_data[x + y*width*3];
		//	p[1] = img_data[x + y*width*3 + 1];
		//	p[2] = img_data[x + y*width*3 + 2];

		//	dstp[x + y*dst_pitch] = p[0];
		//	dstp[x + y*dst_pitch + 1] = p[1];
		//	dstp[x + y*dst_pitch + 2] = p[2];
		//}
	}

  // As we now are finished processing the image, we return the destination image.
	return dst;
}


// This is the function that created the filter, when the filter has been called.
// This can be used for simple parameter checking, so it is possible to create different filters,
// based on the arguments recieved.

AVSValue __cdecl Create_SimpleSample(AVSValue args, void* user_data, IScriptEnvironment* env) {
    return new SimpleSample(args[0].AsClip(),env);  
    // Calls the constructor with the arguments provied.
}


// The following function is the function that actually registers the filter in AviSynth
// It is called automatically, when the plugin is loaded to see which functions this filter contains.

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env) {
    env->AddFunction("SimpleSample", "c", Create_SimpleSample, 0);
    // The AddFunction has the following paramters:
    // AddFunction(Filtername , Arguments, Function to call,0);
    
    // Arguments is a string that defines the types and optional names of the arguments for your filter.
    // c - Video Clip
    // i - Integer number
    // f - Float number
    // s - String
    // b - boolean

    return "`SimpleSample' SimpleSample plugin";
    // A freeform name of the plugin.
}
