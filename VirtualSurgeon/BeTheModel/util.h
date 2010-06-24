#ifndef _UTIL_H
#define _UTIL_H


#include "cv.h"
#include "highgui.h"

using namespace cv;



#define BTM_WAIT_TIME btm_wait_time

#define EPSILON 0.0001
//
//#include <Global.h>
//#include <GrabCut.h>


typedef struct CLONE_INFO {
	IplImage* back;
	IplImage* back_for_poisson;
	IplImage* back_mask;
	IplImage* back_mask_for_poisson;
	int eye_distance;
	CvPoint left_eye;
	CvPoint right_eye;
	CvRect hue_measure;
	int grayscale;
	std::string neck_path_file;
} CloneInfo;

typedef struct _FACE_INFO {
	IplImage* face;
	IplImage* face_mask;
	CvPoint lEye;
	CvPoint rEye;
	CvRect face_rect;
} FaceInfo;

typedef struct _FACE_PATH {
	CvPoint2D32f* pts;
	int num;
	CvPoint left_eye;
	CvPoint right_eye;
} FacePath;



void detect_and_draw(cv::CascadeClassifier& cascade, cv::CascadeClassifier& nestedCascade, 
					 IplImage* small_img, IplImage* gray, IplImage* paint_over,
					 CvPoint& eye1, CvPoint& eye2, float& output_phi, double scale);
void find_face_path(CvPoint eye1, CvPoint eye2, IplImage* edge_8bit, CvPoint2D32f* points, int numPathPts, IplImage* paint_over);
double calc_angle_with_x_axis(CvPoint2D32f a, CvPoint2D32f b);
void create_ellipse_path(int numPathPts,CvPoint2D32f* points, CvPoint eye1, CvPoint eye2);
double distance(CvPoint a, CvPoint b);
void readCloneInfoFile(char* filename, CloneInfo& info);
float calc_angle(CvPoint2D32f a, CvPoint2D32f b);
void rotate_around_point(IplImage* im, CvPoint pt, double ang, CvPoint* pt1, CvPoint* pt2);
void refine_pupil_point(IplImage* im, CvPoint& pupil, int);
void paste_face_on_clone(FaceInfo* faceI, CloneInfo* cloneI);
void read_path_from_file(char* filename, FacePath& fp);
void draw_points_poly_mask(CvPoint2D32f* points, int num_points_on_path, IplImage* mask, IplImage* paint_over = 0);
void refine_face_path_with_LLSQ(int num_points_on_path, CvPoint2D32f* points, IplImage* paint_over);
void drawArrow(CvPoint , CvPoint , IplImage* );
void prune_points_set(int* num_points_ptr, CvPoint2D32f** points_ptr);
void get_center_connected_component(cv::Ptr<CvMemStorage>& storage, IplImage* mask, IplImage* image, CvScalar midPt);
CvRect find_bounding_rect_of_mask(IplImage* mask);
//void pyramid_histogram_backprojection_RGB(IplImage* orig,IplImage* image,IplImage* mask,IplImage* mask_inner,CvScalar midPt,bool recalc_face_hist);
void find_face_path_homography(IplImage* edge_8bit, CvPoint2D32f* points, int numPathPts, IplImage* paint_over);

//void GCCallback(const GrabCutNS::GrabCut* GC);

extern "C" void bezier(int npts,float b[],int cpts,float p[]);

inline double doOverlay(double I, double M) {
	return (I / 255.0) * (I + (2.0*M/255.0)*(255.0 - I));
}

inline float norm2D(CvPoint2D32f p) { return sqrt(p.x*p.x + p.y*p.y);}

inline void normalize2D(CvPoint2D32f& p) {
	float norm = norm2D(p); p.x /= norm; p.y /= norm;
}

inline float dotp(CvPoint2D32f u, CvPoint2D32f v) {
	return u.x*v.x + u.y*u.y;
}

inline IplImage* cloneIm(IplImage* im) { return cvCreateImage(cvGetSize(im),im->depth,im->nChannels);}

extern int btm_wait_time;

extern void face_grab_cut(Mat& orig, Mat& mask);

//extern void face_warp(vector<Mat* >& imgs,
//			   vector<Point2d>& points1,
//			   vector<Point2d>& points2, 
//			   Size im2_size, 
//			   face_warp_method method,
//			   double alpha);
//extern void face_warp_read_points(std::string& str, Size im_size, vector<Point2d>& points);

#endif