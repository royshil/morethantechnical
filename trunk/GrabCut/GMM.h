/*
 * GrabCut implementation source code Copyright(c) 2005-2006 Justin Talbot
 *
 * All Rights Reserved.
 * For educational use only; commercial use expressly forbidden.
 * NO WARRANTY, express or implied, for this software.
 */

#ifndef GMM_H
#define GMM_H

#include "Color.h"
#include "Image.h"

#include "cv.h"

namespace GrabCutNS {

struct Gaussian
{
	Color mu;					// mean of the gaussian
	Real covariance[3][3];		// covariance matrix of the gaussian
	Real determinant;			// determinant of the covariance matrix
	Real inverse[3][3];			// inverse of the covariance matrix
	Real pi;					// weighting of this gaussian in the GMM.

	// These are only needed during Orchard and Bouman clustering.
	Real eigenvalues[3];		// eigenvalues of covariance matrix
	Real eigenvectors[3][3];	// eigenvectors of   "          "
};

class GMM
{
public:

	// Initialize GMM with number of gaussians desired.
	GMM(unsigned int K);
	~GMM();

	unsigned int K() const { return m_K; }

	// Returns the probability density of color c in this GMM
	Real p(Color c);

	// Returns the probability density of color c in just Gaussian k
	Real p(unsigned int i, Color c);

private:

	unsigned int m_K;		// number of gaussians
	Gaussian* m_gaussians;	// an array of K gaussians

	friend void buildGMMs(GMM& backgroundGMM, GMM& foregroundGMM, Image<unsigned int>& components, const Image<Color>& image, const Image<SegmentationValue>& hardSegmentation);
	friend void learnGMMs(GMM& backgroundGMM, GMM& foregroundGMM, Image<unsigned int>& components, const Image<Color>& image, const Image<SegmentationValue>& hardSegmentation);
};

// Build the initial GMMs using the Orchard and Bouman color clustering algorithm
void buildGMMs(GMM& backgroundGMM, GMM& foregroundGMM, Image<unsigned int>& components, const Image<Color>& image, const Image<SegmentationValue>& hardSegmentation);

// Iteratively learn GMMs using GrabCut updating algorithm
void learnGMMs(GMM& backgroundGMM, GMM& foregroundGMM, Image<unsigned int>& components, const Image<Color>& image, const Image<SegmentationValue>& hardSegmentation);


// Helper class that fits a single Gaussian to color samples
class GaussianFitter
{
public:
	GaussianFitter();
	
	// Add a color sample
	void add(Color c);
	
	// Build the gaussian out of all the added color samples
	void finalize(Gaussian& g, unsigned int totalCount, bool computeEigens = false) const;
	
private:

	Color s;			// sum of r,g, and b
	Real  p[3][3];		// matrix of products (i.e. r*r, r*g, r*b), some values are duplicated.

	unsigned int count;	// count of color samples added to the gaussian
};
}
#endif //GMM_H