#include <stdio.h>
#include "NyAR_core.h"

typedef double GLdouble;

using namespace NyARToolkitCPP;

void toCameraViewRH(NyARTransMatResult i_ny_result, float o_gl_result[]);
void arglCameraFrustumRH(const NyARParam& cparam, const double focalmin, const double focalmax, GLdouble m_projection[16]);
int  arParamDecompMat( double source[3][4], double cpara[3][4], double trans[3][4] );
static double dot( double a1, double a2, double a3,
		   double b1, double b2, double b3 );
static double norm( double a, double b, double c );
unsigned char* load_file_image(const char* i_filename);