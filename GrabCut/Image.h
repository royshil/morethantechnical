/*
 * GrabCut implementation source code Copyright(c) 2005-2006 Justin Talbot
 *
 * All Rights Reserved.
 * For educational use only; commercial use expressly forbidden.
 * NO WARRANTY, express or implied, for this software.
 */

#ifndef IMAGE_H
#define IMAGE_H

#include "Global.h"

// Images, really just a templatized 2D array. We use this for all the image variables.
namespace GrabCutNS {

template<class T>
class Image
{

public:
	Image(unsigned int width, unsigned int height);
	~Image();

	T* ptr() { return m_image; }

	T& operator() (int x, int y) { clampX(x); clampY(y); return m_image[y*m_width+x]; }
	const T& operator() (int x, int y) const { clampX(x); clampY(y); return m_image[y*m_width+x]; }

	void fillRectangle(int x1, int y1, int x2, int y2, const T& t);
	void fill(const T& t);

	unsigned int width() const { return m_width; }
	unsigned int height() const { return m_height; }

private:

	void clampX(int& x) const { if (x < 0) x = 0; if (x >= (int)m_width)  x = m_width-1; }
	void clampY(int& y) const { if (y < 0) y = 0; if (y >= (int)m_height) y = m_height-1; }

	T* m_image;
	unsigned int m_width, m_height;
};


// Image member functions
template<class T>
Image<T>::Image(unsigned int width, unsigned int height) : m_width(width), m_height(height)
{
	m_image = new T[m_width*m_height];
}

template<class T>
Image<T>::~Image()
{
	if (m_image)
		delete [] m_image;
}

template<class T>
void Image<T>::fillRectangle(int x1, int y1, int x2, int y2, const T& t)
{
	clampX(x1); clampY(y1);
	clampX(x2); clampY(y2);
	
	if(y1>y2) {int t=y1; y1=y2; y2=t;}
	if(x1>x2) {int t=x1; x1=x2; x2=t;}

	for (int i = y1; i <= y2; ++i)
	{
		for (int j = x1; j <= x2; ++j)
		{
			m_image[i*m_width+j] = t;
		}
	}
}

template<class T>
void Image<T>::fill(const T& t)
{
	for (unsigned int i = 0; i < m_width*m_height; ++i) 
	{
		m_image[i] = t;
	}
}


}//ns

#endif