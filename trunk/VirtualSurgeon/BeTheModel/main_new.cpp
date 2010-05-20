#include "cv.h"
#include "highgui.h"

using namespace cv;

#include "util.h"

#include "../VirtualSurgeon_Utils/VirtualSurgeon_Utils.h"

#include "../VirtualSurgeon_HeadExtractor/head_extractor.h"

int main( int argc, char** argv )
{
	VIRTUAL_SURGEON_PARAMS params;

	ParseParams(params,argc,argv);

	Mat im;

	if(params.filename.substr(0,6).compare("http://")) {
		FaceDotComDetection(params,im);
	} else {
		im = imread(params.filename);

		params.li = Point(253*0.4122,338*0.2624);
		params.ri = Point(253*0.5129,338*0.2603);
	}

	ExtractHead(im,params);
}