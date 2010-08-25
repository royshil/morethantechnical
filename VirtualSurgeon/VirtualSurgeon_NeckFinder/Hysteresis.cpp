#include "cv.h"
#include "highgui.h"
using namespace cv;

#include <iostream>
using namespace std;

#define MAXVAL 255
#define MAXLINE 2000

Mat Hysteresis(Mat& strength, double low, double high, int rows, int cols, int radius)
{
	int x, y, start, end, nbr[2][8] = {{ 1,  1,  1,  0, -1, -1, -1, 0},
	{ 1,  0, -1, -1, -1,  0,  1, 1}};
	int edge, i, xp, yp, X[MAXLINE], Y[MAXLINE];
	//it_image *img;
	Mat img = Mat::zeros(strength.size(),CV_8UC1);

	/* img = i_create_image(cols, rows, IT_BYTE, IM_CONTIG);
	if (!img)
	Error("Could not create output image\n");*/

	/* First, mark all pixels above the high and below the low */	
	/* We don't check the borders because we didn't in NonMaximalSuppression */
	//for (x = 0; x < strength.rows; x++) {
	//	for (y = 0; y < strength.cols; y++) {
	//		if (x == 0 || y == 0 || x == strength.rows - 1 || 
	//			y == strength.cols - 1)
	//			strength.data[y * strength.rows + x] = 0.0;
	//		if (strength.data[y * strength.rows + x] >= high) {
	//			//im_byte_value(img, y + radius, x + radius) = MAXVAL;
	//			img.at<uchar>(y+radius,x+radius) = MAXVAL;
	//			strength.data[y * strength.rows + x] = 0.0;
	//		} else if (strength.data[y * strength.rows + x] < low)
	//			strength.data[y * strength.rows + x] = 0.0;
	//	}
	//}
	imshow("tmp",strength);
	waitKey();
	strength(Range(0,1),Range(0,strength.cols)).setTo(Scalar(0));	//1st row
	strength(Range(strength.rows-1,strength.rows),Range(0,strength.cols)).setTo(Scalar(0)); //last row
	strength(Range(0,strength.rows),Range(0,1)).setTo(Scalar(0));	//1st col
	strength(Range(0,strength.rows),Range(strength.cols-1,strength.cols)).setTo(Scalar(0));	//1st col
	imshow("tmp",strength);
	waitKey();
	imshow("tmp",strength < low);
	waitKey();
	strength.setTo(Scalar(0),strength < low);
	imshow("tmp",strength);
	waitKey();
	img.setTo(Scalar(MAXVAL),strength > high);
	imshow("tmp",img);
	waitKey();
	strength.setTo(Scalar(0), strength > high);
	imshow("tmp",strength);
	waitKey();

	/* The only pixels left with non-zero strength fall between the two
	* thresholds.  Group adjacent pixels together, and if they touch a
	* marked edge, they all become marked.
	*/
	float* s_ptr = ((float*)strength.data);
	for (x = 1; x < strength.rows - 1; x++) {
		for (y = 1; y < strength.cols - 1; y++) {
			if (s_ptr[y * strength.rows + x] > 0.0) {
				s_ptr[y * strength.rows + x] = 0.0;
				start = 0;
				edge = 0;
				X[start] = x;
				Y[start] = y;
				end = 1;
				while (start != end) {
					for (i = 0; i < 8; i++) {
						xp = X[start] + nbr[0][i];
						yp = Y[start] + nbr[1][i];
						//edge = edge || (im_byte_value(img, yp + radius, xp + radius) == MAXVAL);
						edge = edge || (img.at<uchar>(yp + radius, xp + radius) == MAXVAL);
						if (s_ptr[yp * strength.rows + xp] > 0.0) {
							s_ptr[yp * strength.rows + xp] = 0.0;
							X[end] = xp;
							Y[end] = yp;
							if (++end == MAXLINE)
								cerr << "Edge Buffer Full";
						}
					}
					start++;
				}

				if (edge)
					for (i = 0; i < end; i++)
						img.at<uchar>(Y[i] + radius, X[i] + radius) = MAXVAL;
						//im_byte_value(img, Y[i] + radius, X[i] + radius) = MAXVAL;
			}
		}
	}
	return img;
}