#include "cv.h"
#include "highgui.h"

using namespace cv;

#include "util.h"

#include "../VirtualSurgeon_Utils/VirtualSurgeon_Utils.h"

#include "../VirtualSurgeon_HeadExtractor/head_extractor.h"

#include "../VirtualSurgeon_NeckFinder/VirtualSurgeon_NeckFinder.h"

using namespace VirtualSurgeon;

int main( int argc, char** argv )
{
	VIRTUAL_SURGEON_PARAMS params;

	ParseParams(params,argc,argv);

	Mat im;

	FaceDotComDetection(params,im);
	PrintParams(params);

	HeadExtractor he(params);
	Mat im_extracted; im.copyTo(im_extracted);
	he.ExtractHead(im_extracted);

	NeckFinder nf(params);
	Mat im_neck; im.copyTo(im_neck);
	nf.FindNeck(im_neck);
}