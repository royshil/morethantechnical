#include "util.h"

#include <fstream>
#include <iostream>

using namespace std;

unsigned char* load_file_image(const char* i_filename)
{
	ifstream inf;
	unsigned char* result=NULL;
	try {
		inf.open(i_filename,ios::binary+ios::in);
		int size=320*240*4;
		result=new unsigned char[size];
		inf.read((char*)result,size);
		if(size!=inf.gcount()){
			throw exception();
		}
		inf.close();
	} catch(exception e){
		if(result!=NULL){
			delete result;
		}
		throw;
	}
	return result;
}

static double norm( double a, double b, double c )
{
    return( sqrt( a*a + b*b + c*c ) );
}

static double dot( double a1, double a2, double a3,
		   double b1, double b2, double b3 )
{
    return( a1 * b1 + a2 * b2 + a3 * b3 );
}

int  arParamDecompMat( double source[3][4], double cpara[3][4], double trans[3][4] )
{
    int       r, c;
    double    Cpara[3][4];
    double    rem1, rem2, rem3;

    if( source[2][3] >= 0 ) {
        for( r = 0; r < 3; r++ ){
	    for( c = 0; c < 4; c++ ){
                Cpara[r][c] = source[r][c];
            }
        }
    }
    else {
        for( r = 0; r < 3; r++ ){
	    for( c = 0; c < 4; c++ ){
                Cpara[r][c] = -(source[r][c]);
            }
        }
    }

    for( r = 0; r < 3; r++ ){
	for( c = 0; c < 4; c++ ){
            cpara[r][c] = 0.0;
	}
    }
    cpara[2][2] = norm( Cpara[2][0], Cpara[2][1], Cpara[2][2] );
    trans[2][0] = Cpara[2][0] / cpara[2][2];
    trans[2][1] = Cpara[2][1] / cpara[2][2];
    trans[2][2] = Cpara[2][2] / cpara[2][2];
    trans[2][3] = Cpara[2][3] / cpara[2][2];
	
    cpara[1][2] = dot( trans[2][0], trans[2][1], trans[2][2],
                       Cpara[1][0], Cpara[1][1], Cpara[1][2] );
    rem1 = Cpara[1][0] - cpara[1][2] * trans[2][0];
    rem2 = Cpara[1][1] - cpara[1][2] * trans[2][1];
    rem3 = Cpara[1][2] - cpara[1][2] * trans[2][2];
    cpara[1][1] = norm( rem1, rem2, rem3 );
    trans[1][0] = rem1 / cpara[1][1];
    trans[1][1] = rem2 / cpara[1][1];
    trans[1][2] = rem3 / cpara[1][1];

    cpara[0][2] = dot( trans[2][0], trans[2][1], trans[2][2],
                       Cpara[0][0], Cpara[0][1], Cpara[0][2] );
    cpara[0][1] = dot( trans[1][0], trans[1][1], trans[1][2],
                       Cpara[0][0], Cpara[0][1], Cpara[0][2] );
    rem1 = Cpara[0][0] - cpara[0][1]*trans[1][0] - cpara[0][2]*trans[2][0];
    rem2 = Cpara[0][1] - cpara[0][1]*trans[1][1] - cpara[0][2]*trans[2][1];
    rem3 = Cpara[0][2] - cpara[0][1]*trans[1][2] - cpara[0][2]*trans[2][2];
    cpara[0][0] = norm( rem1, rem2, rem3 );
    trans[0][0] = rem1 / cpara[0][0];
    trans[0][1] = rem2 / cpara[0][0];
    trans[0][2] = rem3 / cpara[0][0];

    trans[1][3] = (Cpara[1][3] - cpara[1][2]*trans[2][3]) / cpara[1][1];
    trans[0][3] = (Cpara[0][3] - cpara[0][1]*trans[1][3]
                               - cpara[0][2]*trans[2][3]) / cpara[0][0];

    for( r = 0; r < 3; r++ ){
	for( c = 0; c < 3; c++ ){
            cpara[r][c] /= cpara[2][2];
	}
    }

    return 0;
}

void arglCameraFrustumRH(const NyARParam& cparam, const double focalmin, const double focalmax, GLdouble m_projection[16])
{
	double   icpara[3][4];
    double   trans[3][4];
    double   p[3][3], q[4][4];
	int      width, height;
    int      i, j;
	
	width  = cparam.getScreenSize()->w;
    height = cparam.getScreenSize()->h;

	double cparam_m[3][4] = {0.0};
	const NyARPerspectiveProjectionMatrix* proj = cparam.getPerspectiveProjectionMatrix();
	cparam_m[0][0] = proj->m00;
	cparam_m[0][1] = proj->m01;
	cparam_m[0][2] = proj->m02;
	cparam_m[0][3] = proj->m03;
	cparam_m[1][0] = proj->m10;
	cparam_m[1][1] = proj->m11;
	cparam_m[1][2] = proj->m12;
	cparam_m[1][3] = proj->m13;
	cparam_m[2][0] = proj->m20;
	cparam_m[2][1] = proj->m21;
	cparam_m[2][2] = proj->m22;
	cparam_m[2][3] = proj->m23;
	
	if (arParamDecompMat(cparam_m, icpara, trans) < 0) {
        printf("arglCameraFrustum(): arParamDecompMat() indicated parameter error.\n"); // Windows bug: when running multi-threaded, can't write to stderr!
        return;
    }
	for (i = 0; i < 4; i++) {
        icpara[1][i] = (height - 1)*(icpara[2][i]) - icpara[1][i];
    }
	
    for(i = 0; i < 3; i++) {
        for(j = 0; j < 3; j++) {
            p[i][j] = icpara[i][j] / icpara[2][2];
        }
    }
    q[0][0] = (2.0 * p[0][0] / (width - 1));
    q[0][1] = (2.0 * p[0][1] / (width - 1));
    q[0][2] = -((2.0 * p[0][2] / (width - 1))  - 1.0);
    q[0][3] = 0.0;
	
    q[1][0] = 0.0;
    q[1][1] = -(2.0 * p[1][1] / (height - 1));
    q[1][2] = -((2.0 * p[1][2] / (height - 1)) - 1.0);
    q[1][3] = 0.0;
	
    q[2][0] = 0.0;
    q[2][1] = 0.0;
    q[2][2] = (focalmax + focalmin)/(focalmin - focalmax);
    q[2][3] = 2.0 * focalmax * focalmin / (focalmin - focalmax);
	
    q[3][0] = 0.0;
    q[3][1] = 0.0;
    q[3][2] = -1.0;
    q[3][3] = 0.0;
	
    for (i = 0; i < 4; i++) { // Row.
		// First 3 columns of the current row.
        for (j = 0; j < 3; j++) { // Column.
            m_projection[i + j*4] = q[i][0] * trans[0][j] +
			q[i][1] * trans[1][j] +
			q[i][2] * trans[2][j];
        }
		// Fourth column of the current row.
        m_projection[i + 3*4] = q[i][0] * trans[0][3] +
								q[i][1] * trans[1][3] +
								q[i][2] * trans[2][3] +
								q[i][3];
    }	
}

void toCameraViewRH(NyARTransMatResult i_ny_result, float o_gl_result[])
{
	float view_scale_factor = 0.025;
	o_gl_result[0 + 0 * 4] = i_ny_result.m00; 
	o_gl_result[0 + 1 * 4] = i_ny_result.m01;
	o_gl_result[0 + 2 * 4] = i_ny_result.m02;
	o_gl_result[0 + 3 * 4] = i_ny_result.m03;
	o_gl_result[1 + 0 * 4] = -i_ny_result.m10;
	o_gl_result[1 + 1 * 4] = -i_ny_result.m11;
	o_gl_result[1 + 2 * 4] = -i_ny_result.m12;
	o_gl_result[1 + 3 * 4] = -i_ny_result.m13;
	o_gl_result[2 + 0 * 4] = -i_ny_result.m20;
	o_gl_result[2 + 1 * 4] = -i_ny_result.m21;
	o_gl_result[2 + 2 * 4] = -i_ny_result.m22;
	o_gl_result[2 + 3 * 4] = -i_ny_result.m23;
	o_gl_result[3 + 0 * 4] = 0.0;
	o_gl_result[3 + 1 * 4] = 0.0;
	o_gl_result[3 + 2 * 4] = 0.0;
	o_gl_result[3 + 3 * 4] = 1.0;
	if (view_scale_factor != 0.0) {
		o_gl_result[12] *= view_scale_factor;
		o_gl_result[13] *= view_scale_factor;
		o_gl_result[14] *= view_scale_factor;
	}
	return;
}