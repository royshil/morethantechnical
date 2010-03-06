/**
 * License...
 *
 *	Main program globals, declarations
 *
 *
 */
#include "utils.h"

Mat backPxls;
Vec3d u,v;
vector<Point2d> points1,points2;

//3D features store
vector<Point3d> points1Proj; 
vector<Point3f> points1ProjF; 
Mat points1projMF;
vector<Point3d> points1Proj_invisible;

Mat frames[5];
vector<Point2d> _points[5],points[5],pointsOrig[5];
vector<uchar> tri_status;

void findExtrinsics(vector<Point2d>& points, vector<double>& rv, vector<double>& tv);
void findExtrinsics(vector<Point2f>& points1,
					vector<Point2f>& points2,
					vector<double>& rv, vector<double>& tv);

vector<double> rots[5];
vector<double> cams[5];
double cam[3] = {0,0,0};
double rot[9] = {-1,0,0,0,1,0,0,0,-1};

double curCam[3] = {0,0,0};

bool running = true;

HANDLE ghMutex; 
