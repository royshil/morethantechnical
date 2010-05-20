#ifndef _HEAD_EXTRACTOR_H
#define _HEAD_EXTRACTOR_H

#pragma once

#include "cv.h"
#include "highgui.h"

using namespace cv;

#include "../VirtualSurgeon_Utils/VirtualSurgeon_Utils.h"

int ExtractHead(Mat& im, VIRTUAL_SURGEON_PARAMS& params);

void FaceDotComDetection(VIRTUAL_SURGEON_PARAMS& params, Mat& im);

#endif