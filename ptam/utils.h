/**
 * License...
 *
 *	All globals, function declarations
 *
 *
 */

#define VIDEO_MODE

/** general utility functions **/
void read_frame_and_points(const char* pntsFile, const char* frameFile, vector<Point2d>& pts, Mat& frame);
void draw_points(Mat& image, vector<Point2d> points);
int intersect3D_SegmentPlane( Vec3d S_P0, Vec3d S_P1, Vec3d P_V0, Vec3d P_N, Vec3d& I );
void draw_frame_points_w_status(const string& win_name, const Mat& i, vector<Point2d>& p, vector<uchar>& status);
int show_2_frame_correlation(int i,char** c);
/******************************/

int capture_data(int, char**);

/** OPENGL **/
const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

extern void resize(int width, int height);
extern void key(unsigned char key, int x, int y);
extern void idle(void);
void myGLinit();

void loadFrame(int counter);

int start_opengl_with_stereo(int argc,char** argv);
/**************/

/** stereo **/
void findExtrinsics(vector<Point2d>& points, vector<double>& rv, vector<double>& tv);
void findExtrinsics(vector<Point2f>& points1,
					vector<Point2f>& points2,
					vector<double>& rv, vector<double>& tv);
void stereoInit();
double keepGood2D3DMatch(vector<Point2d>& trackedPoints, vector<double>& rv, vector<double>& tv, vector<uchar>& status);
void drawReprojectedOnImage(Mat& image, vector<double>& rv, vector<double>& tv, vector<Point2d>& tracked, vector<uchar>& status);
bool reprojectInivibles(vector<Point2f>& trackedPoints, vector<double> rv, vector<double> tv);
/************/

extern Mat backPxls;
extern Mat frames[5];
extern vector<Point2d> _points[5],points[5],pointsOrig[5];
extern vector<uchar> tri_status;

extern vector<double> rots[5];
extern vector<double> cams[5];
extern double cam[3];
extern double rot[9];

extern double curCam[3];

extern vector<Point3d> points1Proj; 
extern vector<Point3f> points1ProjF; 
extern Mat points1projMF;
extern vector<Point3d> points1Proj_invisible;


extern Vec3d u,v;

extern bool running;

extern HANDLE ghMutex; 
