#include "cv.h"
#include "highgui.h"

using namespace cv;

#include "util.h"

#include "../VirtualSurgeon_Utils/VirtualSurgeon_Utils.h"

#include "../VirtualSurgeon_HeadExtractor/head_extractor.h"

#include "../VirtualSurgeon_NeckFinder/VirtualSurgeon_NeckFinder.h"

int main( int argc, char** argv )
{
	VIRTUAL_SURGEON_PARAMS params;

	ParseParams(params,argc,argv);

	Mat im;

	FaceDotComDetection(params,im);
	PrintParams(params);

	ExtractHead(im,params);

	FindNeck(params,im);
}