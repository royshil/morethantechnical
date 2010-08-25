
namespace VirtualSurgeon {

class NeckFinder {
public:
	int FindNeck(Mat& _im);
	vector<Point> getNeckPoints() { return neck;}
	NeckFinder(VIRTUAL_SURGEON_PARAMS& _p):p(_p) {};

	vector<Point> neck;
private:
	int SnakeSnap(int argc, char** argv);
	int snake_snap_main(int argc, char** argv);
	void smear(Mat& im_dx, int levels);
	void calcHistogramWMask(Mat& im, Mat& mask, MatND& faceHist);

	VIRTUAL_SURGEON_PARAMS p;
};


}//ns