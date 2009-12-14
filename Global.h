/*
 * GrabCut implementation source code Copyright(c) 2005-2006 Justin Talbot
 *
 * All Rights Reserved.
 * For educational use only; commercial use expressly forbidden.
 * NO WARRANTY, express or implied, for this software.
 */

#ifndef GLOBAL_H
#define GLOBAL_H

#include <string>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>

// Choose accuracy of computations (double doesn't seem to work for me yet.)
//#define USE_DOUBLE

#ifdef USE_DOUBLE
	#define Real double
#else
	#define Real float
#endif


// User supplied Trimap values
enum TrimapValue { TrimapUnknown, TrimapForeground, TrimapBackground };

// Grabcut derived hard segementation values
enum SegmentationValue { SegmentationForeground, SegmentationBackground };

// Storage for N-link weights, each pixel stores links to only four of its 8-neighborhood neighbors.
// This avoids duplication of links, while still allowing for relatively easy lookup.
struct NLinks
{
	Real upleft;
	Real up;
	Real upright;
	Real right;
};


// Helper function, finds distance between two pixels
inline Real distance(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
	return sqrt((Real)((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)));
}

#endif
