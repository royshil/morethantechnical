/*
 * GrabCut implementation source code Copyright(c) 2005-2006 Justin Talbot
 *
 * All Rights Reserved.
 * For educational use only; commercial use expressly forbidden.
 * NO WARRANTY, express or implied, for this software.
 */

#include "GMM.h"

namespace GrabCutNS {

GMM::GMM(unsigned int K) : m_K(K)
{
	m_gaussians = new Gaussian[m_K];
}

GMM::~GMM()
{
	if (m_gaussians)
		delete [] m_gaussians;
}

Real GMM::p(Color c)
{
	Real result = 0;

	if (m_gaussians)
	{
		for (unsigned int i=0; i < m_K; i++)
		{
			result += m_gaussians[i].pi * p(i, c);
		}
	}

	return result;
}

Real GMM::p(unsigned int i, Color c)
{
	Real result = 0;

	if( m_gaussians[i].pi > 0 )
	{
		if (m_gaussians[i].determinant > 0)
		{
			Real r = c.r - m_gaussians[i].mu.r;
			Real g = c.g - m_gaussians[i].mu.g;
			Real b = c.b - m_gaussians[i].mu.b;
			
			Real d = r * (r*m_gaussians[i].inverse[0][0] + g*m_gaussians[i].inverse[1][0] + b*m_gaussians[i].inverse[2][0]) +
					g * (r*m_gaussians[i].inverse[0][1] + g*m_gaussians[i].inverse[1][1] + b*m_gaussians[i].inverse[2][1]) +
					b * (r*m_gaussians[i].inverse[0][2] + g*m_gaussians[i].inverse[1][2] + b*m_gaussians[i].inverse[2][2]);

			result = (Real)(1.0/(sqrt(m_gaussians[i].determinant)) * exp(-0.5*d));
		}
	}

	return result;
}

void buildGMMs(GMM& backgroundGMM, GMM& foregroundGMM, Image<unsigned int>& components, const Image<Color>& image, const Image<SegmentationValue>& hardSegmentation)
{
	// Step 3: Build GMMs using Orchard-Bouman clustering algorithm

	// Set up Gaussian Fitters
	GaussianFitter* backFitters = new GaussianFitter[backgroundGMM.K()];
	GaussianFitter* foreFitters = new GaussianFitter[foregroundGMM.K()];

	unsigned int foreCount = 0, backCount = 0;

	// Initialize the first foreground and background clusters
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			components(x,y) = 0;

			if (hardSegmentation(x,y) == SegmentationForeground)
			{
				foreFitters[0].add(image(x,y));
				foreCount++;
			}
			else
			{
				backFitters[0].add(image(x,y));
				backCount++;
			}
		}
	}

	backFitters[0].finalize(backgroundGMM.m_gaussians[0], backCount, true);
	foreFitters[0].finalize(foregroundGMM.m_gaussians[0], foreCount, true);

	unsigned int nBack = 0, nFore = 0;		// Which cluster will be split
	unsigned int maxK = backgroundGMM.K() > foregroundGMM.K() ? backgroundGMM.K() : foregroundGMM.K();
	
	// Compute clusters
	for (unsigned int i = 1; i < maxK; i++)
	{
		// Reset the fitters for the splitting clusters
		backFitters[nBack] = GaussianFitter();
		foreFitters[nFore] = GaussianFitter();

		// For brevity, get references to the splitting Gaussians
		Gaussian& bg = backgroundGMM.m_gaussians[nBack];
		Gaussian& fg = foregroundGMM.m_gaussians[nFore];

		// Compute splitting points
		Real splitBack = bg.eigenvectors[0][0] * bg.mu.r + bg.eigenvectors[1][0] * bg.mu.g + bg.eigenvectors[2][0] * bg.mu.b;
		Real splitFore = fg.eigenvectors[0][0] * fg.mu.r + fg.eigenvectors[1][0] * fg.mu.g + fg.eigenvectors[2][0] * fg.mu.b;

		// Split clusters nBack and nFore, place split portion into cluster i
		for (unsigned int y = 0; y < image.height(); ++y)
		{
			for(unsigned int x = 0; x < image.width(); ++x)
			{
				Color c = image(x,y);

				// For each pixel
				if (i < foregroundGMM.K() && hardSegmentation(x,y) == SegmentationForeground && components(x,y) == nFore)
				{
					if (fg.eigenvectors[0][0] * c.r + fg.eigenvectors[1][0] * c.g + fg.eigenvectors[2][0] * c.b > splitFore)
					{
						components(x,y) = i;
						foreFitters[i].add(c);
					}
					else
					{
						foreFitters[nFore].add(c);
					}
				}
				else if (i < backgroundGMM.K() && hardSegmentation(x,y) == SegmentationBackground && components(x,y) == nBack)
				{
					if (bg.eigenvectors[0][0] * c.r + bg.eigenvectors[1][0] * c.g + bg.eigenvectors[2][0] * c.b > splitBack)
					{
						components(x,y) = i;
						backFitters[i].add(c);
					}
					else
					{
						backFitters[nBack].add(c);
					}
				}
			}
		}

		
		// Compute new split Gaussians
		backFitters[nBack].finalize(backgroundGMM.m_gaussians[nBack], backCount, true);
		foreFitters[nFore].finalize(foregroundGMM.m_gaussians[nFore], foreCount, true);

		if (i < backgroundGMM.K())
			backFitters[i].finalize(backgroundGMM.m_gaussians[i], backCount, true);
		if (i < foregroundGMM.K())
			foreFitters[i].finalize(foregroundGMM.m_gaussians[i], foreCount, true);

		// Find clusters with highest eigenvalue
		nBack = 0;
		nFore = 0;

		for (unsigned int j = 0; j <= i; j++ )
		{
			if (j < backgroundGMM.K() && backgroundGMM.m_gaussians[j].eigenvalues[0] > backgroundGMM.m_gaussians[nBack].eigenvalues[0])
				nBack = j;

			if (j < foregroundGMM.K() && foregroundGMM.m_gaussians[j].eigenvalues[0] > foregroundGMM.m_gaussians[nFore].eigenvalues[0])
				nFore = j;
		}
	}

	delete [] backFitters;
	delete [] foreFitters;
}

void learnGMMs(GMM& backgroundGMM, GMM& foregroundGMM, Image<unsigned int>& components, const Image<Color>& image, const Image<SegmentationValue>& hardSegmentation)
{
	// Step 4: Assign each pixel to the component which maximizes its probability
	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for (unsigned int x = 0; x < image.width(); ++x)
		{
			Color c = image(x,y);

			if (hardSegmentation(x,y) == SegmentationForeground)
			{
				int k = 0;
				Real max = 0;

				for (unsigned int i = 0; i < foregroundGMM.K(); i++)
				{
					Real p = foregroundGMM.p(i, c);
					if (p > max)
					{
						k = i;
						max = p;
					}
				}

				components(x, y) = k;
			}
			else 
			{
				int k = 0;
				Real max = 0;

				for (unsigned int i = 0; i < backgroundGMM.K(); i++)
				{
					Real p = backgroundGMM.p(i, c);
					if (p > max)
					{
						k = i;
						max = p;
					}
				}

				components(x, y) = k;
			}
		}
	}

	// Step 5: Relearn GMMs from new component assignments

	// Set up Gaussian Fitters
	GaussianFitter* backFitters = new GaussianFitter[backgroundGMM.K()];
	GaussianFitter* foreFitters = new GaussianFitter[foregroundGMM.K()];

	unsigned int foreCount = 0, backCount = 0;

	for (unsigned int y = 0; y < image.height(); ++y)
	{
		for(unsigned int x = 0; x < image.width(); ++x)
		{
			Color c = image(x,y);

			if(hardSegmentation(x,y) == SegmentationForeground)
			{
				foreFitters[components(x,y)].add(c);
				foreCount++;
			}
			else
			{
				backFitters[components(x,y)].add(c);
				backCount++;
			}
		}
	}

	for (unsigned int i = 0; i < backgroundGMM.K(); i++)
		backFitters[i].finalize(backgroundGMM.m_gaussians[i], backCount, false);

	for (unsigned int i = 0; i < foregroundGMM.K(); i++)
		foreFitters[i].finalize(foregroundGMM.m_gaussians[i], foreCount, false);

	delete [] backFitters;
	delete [] foreFitters;
}


// GaussianFitter functions
GaussianFitter::GaussianFitter()
{
	s = Color();

	p[0][0] = 0; p[0][1] = 0; p[0][2] = 0;
	p[1][0] = 0; p[1][1] = 0; p[1][2] = 0;
	p[2][0] = 0; p[2][1] = 0; p[2][2] = 0;

	count = 0;
}

// Add a color sample
void GaussianFitter::add(Color c)
{
	s.r += c.r; s.g += c.g; s.b += c.b;

	p[0][0] += c.r*c.r; p[0][1] += c.r*c.g; p[0][2] += c.r*c.b;
	p[1][0] += c.g*c.r; p[1][1] += c.g*c.g; p[1][2] += c.g*c.b;
	p[2][0] += c.b*c.r; p[2][1] += c.b*c.g; p[2][2] += c.b*c.b;

	count++;
}

// Build the gaussian out of all the added colors
void GaussianFitter::finalize(Gaussian& g, unsigned int totalCount, bool computeEigens) const
{
	// Running into a singular covariance matrix is problematic. So we'll add a small epsilon
	// value to the diagonal elements to ensure a positive definite covariance matrix.
	const Real Epsilon = (Real)0.0001;

	if (count==0)
	{
		g.pi = 0;
	}
	else
	{
		// Compute mean of gaussian
		g.mu.r = s.r/count;
		g.mu.g = s.g/count;
		g.mu.b = s.b/count;

		// Compute covariance matrix
		g.covariance[0][0] = p[0][0]/count-g.mu.r*g.mu.r + Epsilon; g.covariance[0][1] = p[0][1]/count-g.mu.r*g.mu.g; g.covariance[0][2] = p[0][2]/count-g.mu.r*g.mu.b;
		g.covariance[1][0] = p[1][0]/count-g.mu.g*g.mu.r; g.covariance[1][1] = p[1][1]/count-g.mu.g*g.mu.g + Epsilon; g.covariance[1][2] = p[1][2]/count-g.mu.g*g.mu.b;
		g.covariance[2][0] = p[2][0]/count-g.mu.b*g.mu.r; g.covariance[2][1] = p[2][1]/count-g.mu.b*g.mu.g; g.covariance[2][2] = p[2][2]/count-g.mu.b*g.mu.b + Epsilon;

		// Compute determinant of covariance matrix
		g.determinant = g.covariance[0][0]*(g.covariance[1][1]*g.covariance[2][2]-g.covariance[1][2]*g.covariance[2][1]) 
						- g.covariance[0][1]*(g.covariance[1][0]*g.covariance[2][2]-g.covariance[1][2]*g.covariance[2][0]) 
						+ g.covariance[0][2]*(g.covariance[1][0]*g.covariance[2][1]-g.covariance[1][1]*g.covariance[2][0]);

		// Compute inverse (cofactor matrix divided by determinant)
		g.inverse[0][0] =  (g.covariance[1][1]*g.covariance[2][2] - g.covariance[1][2]*g.covariance[2][1]) / g.determinant;
		g.inverse[1][0] = -(g.covariance[1][0]*g.covariance[2][2] - g.covariance[1][2]*g.covariance[2][0]) / g.determinant;
		g.inverse[2][0] =  (g.covariance[1][0]*g.covariance[2][1] - g.covariance[1][1]*g.covariance[2][0]) / g.determinant;
		g.inverse[0][1] = -(g.covariance[0][1]*g.covariance[2][2] - g.covariance[0][2]*g.covariance[2][1]) / g.determinant;
		g.inverse[1][1] =  (g.covariance[0][0]*g.covariance[2][2] - g.covariance[0][2]*g.covariance[2][0]) / g.determinant;
		g.inverse[2][1] = -(g.covariance[0][0]*g.covariance[2][1] - g.covariance[0][1]*g.covariance[2][0]) / g.determinant;
		g.inverse[0][2] =  (g.covariance[0][1]*g.covariance[1][2] - g.covariance[0][2]*g.covariance[1][1]) / g.determinant;
		g.inverse[1][2] = -(g.covariance[0][0]*g.covariance[1][2] - g.covariance[0][2]*g.covariance[1][0]) / g.determinant;
		g.inverse[2][2] =  (g.covariance[0][0]*g.covariance[1][1] - g.covariance[0][1]*g.covariance[1][0]) / g.determinant;

		// The weight of the gaussian is the fraction of the number of pixels in this Gaussian to the number of 
		// pixels in all the gaussians of this GMM.
		g.pi = (Real)count/totalCount;

		if (computeEigens)
		{

#ifdef USE_DOUBLE
	#define CV_TYPE CV_64FC1
#else
	#define CV_TYPE CV_32FC1
#endif
			// Build OpenCV wrappers around our data.
			CvMat mat = cvMat(3, 3, CV_TYPE, g.covariance);
			CvMat eval = cvMat(3, 1, CV_TYPE, g.eigenvalues);
			CvMat evec = cvMat(3, 3, CV_TYPE, g.eigenvectors);
			
			// Compute eigenvalues and vectors using SVD
			cvSVD( &mat, &eval, &evec );
		}
	}
} 

}