#include "cv.h"

using namespace cv;

namespace myselfsim
{

class SelfSimDescriptor
{
public:
    SelfSimDescriptor();
    SelfSimDescriptor(int _ssize, int _lsize,
        int _startDistanceBucket=DEFAULT_START_DISTANCE_BUCKET,
        int _numberOfDistanceBuckets=DEFAULT_NUM_DISTANCE_BUCKETS,
        int _nangles=DEFAULT_NUM_ANGLES);
	SelfSimDescriptor(const SelfSimDescriptor& ss);
	virtual ~SelfSimDescriptor();
    SelfSimDescriptor& operator = (const SelfSimDescriptor& ss);

    size_t getDescriptorSize() const;
    Size getGridSize( Size imgsize, Size winStride ) const;

    virtual void compute(const Mat& img, vector<float>& descriptors, Size winStride=Size(),
                         vector<Point>& locations=vector<Point>(),bool do_border = false) const;
    virtual void computeLogPolarMapping(Mat& mappingMask) const;
    virtual void SSD(const Mat& img, Point pt, Mat& ssd) const;

	int smallSize;
	int largeSize;
    int startDistanceBucket;
    int numberOfDistanceBuckets;
    int numberOfAngles;

    enum { DEFAULT_SMALL_SIZE = 5, DEFAULT_LARGE_SIZE = 41,
        DEFAULT_NUM_ANGLES = 20, DEFAULT_START_DISTANCE_BUCKET = 3,
        DEFAULT_NUM_DISTANCE_BUCKETS = 7 };
};

}