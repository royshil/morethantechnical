#include "cv.h"

using namespace cv;

float face_path[60] = {81,180,
					84,143,
					74,136,
					68,126,
					69,106,
					69,90,
					80,55,
					102,36,
					134,31,
					162,46,
					176,78,
					173,110,
					179,120,
					174,134,
					169,143,
					160,146,
					159,181,
					123,205};
float face_path_leye[2] = {104,113};
float face_path_reye[2] = {144,113};
int face_path_num_points = 30;

Mat face_path_M(1,30,CV_32FC2,face_path);
