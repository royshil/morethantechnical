#include "ImageEditingUtils.h"

namespace ImageEditingUtils {

typedef gmm::row_matrix< gmm::rsvector<double> > SparseMatrix;
typedef std::vector<double > Vector;

int getSingleColor(int rgb, int color) {
	return ((rgb >> (color * 8)) & 0x000000ff);
}

int getColorInPixel(IImage& image, int y, int x, int color) {
	int rgb = image.getRGB(x, y);
	return getSingleColor(rgb, color);
}

int getUpper(int pixel, int n) {
	int upper = (pixel - n);
	return (upper >= 0) ? upper : -1;
}

int getLower(int pixel, int n, int mn) {
	int lower = (pixel + n);
	return (lower < mn) ? lower : -1;
}

int getLeft(int pixel, int n) {
	return ((pixel % n) != 0) ? (pixel - 1) : -1;
}

int getRight(int pixel, int n) {
	int pixel1 = (pixel + 1);
	return (((pixel1 % n) != 0) ? pixel1 : -1);
}

void matrixCreate(SparseMatrix& outMatrix, int n, int mn, IImage& maskImage) {
	fprintf(stdout,"ImageEditingUtils::matrixCreate(%X,%d,%d,%X)\n",&outMatrix,n,mn,&maskImage);
	for (int pixel = 0; pixel < mn; pixel++) {
		int pxlX = pixel % n;
		int pxlY = (int) floor((float)pixel / (float)n);
		int maskRGB = maskImage.getRGB(pxlX, pxlY);
		if ((maskRGB & 0x00ffffff) == BLACK) {
			outMatrix(pixel,pixel) = 1;
		} else {
			// add 1s in sides
			int numOfNeighbors = 0;
			int neighborsArray[4] = {0};
			neighborsArray[0] = getUpper(pixel, n);
			neighborsArray[1] = getLower(pixel, n, mn);
			neighborsArray[2] = getRight(pixel, n);
			neighborsArray[3] = getLeft(pixel, n);
			for (int j = 0; j < 4; j++) {
				if (neighborsArray[j] >= 0) {
					outMatrix(pixel,neighborsArray[j]) = 1;
					numOfNeighbors++;
				}
			}

			//add -4, -3 or -2 in middle
			outMatrix(pixel,pixel) = (-1) * numOfNeighbors;
		}
	}
}

/**
 * Solve linear equation system
 */
int solveLinear(const SparseMatrix& M, Vector& X, const Vector& B) {
	gmm::ilut_precond<SparseMatrix > PR(M, 10, 1e-2);

	gmm::iteration iter(1E-8);  // defines an iteration object, with a max residu of 1E-8

	gmm::gmres(M, X, B, PR, 50, iter);  // execute the GMRES algorithm

	return 1;
}

int getUpdatedRGBValue(int currentValue, int updateValue, int color) {
	int colorOffest = (color * 8);
	int colorInRGB = (updateValue << colorOffest);
	int bitmask = 0xffffffff ^ (0x000000ff << colorOffest);
	int roomForUpdated = (currentValue & bitmask);
	int updatedRGB = (roomForUpdated | colorInRGB);
	return updatedRGB;
}

void solveAndPaintOutput(int x0, int y0, int n, int mn,
        Vector rgbVector[3], const SparseMatrix& matrix, Vector solutionVectors[3],
		IImage& outputImage) {
	for (int color = 0; color < 3; color++) {
		fprintf(stdout,"Solving... \n");
		// solve equations set for current color
		if (solveLinear(matrix, solutionVectors[color], rgbVector[color]) == 0) {
			fprintf(stderr,"FAIL main(): matrix.solve() failed with color: %d",color);
			return;
		} else {
			fprintf(stdout,"Done solving color %d\n",color);
		}

		// fill output image
		for (int pixel = 0; pixel < mn; pixel++) {
			int y = pixel / n;
			int x = pixel - n * y;

			int updateVal = solutionVectors[color][pixel];

			if (updateVal > 255) {
				updateVal = 255;
			}
			else if (updateVal < 0) {
				updateVal = 0;
			}
			int updatedRGBVal = getUpdatedRGBValue(outputImage.getRGB(x + x0, y + y0), updateVal, color);
			outputImage.setRGB(x + x0, y + y0, updatedRGBVal);
		}
		fprintf(stdout,"Done applying color %d to output\n",color);
	}
}


}
