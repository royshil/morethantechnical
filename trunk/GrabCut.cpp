/*
 * GrabCut implementation source code Copyright(c) 2005-2006 Justin Talbot
 *
 * All Rights Reserved.
 * For educational use only; commercial use expressly forbidden.
 * NO WARRANTY, express or implied, for this software.
 */

#include "GrabCut.h" 
#include <conio.h>
#include "GL/glut.h"

namespace GrabCutNS {

GrabCut::GrabCut( Image<Color>* image )
{
	m_image = image;

	m_w = m_image->width();
	m_h = m_image->height();

	m_trimap = new Image<TrimapValue>( m_w, m_h );
	m_trimap->fill(TrimapUnknown);

	m_GMMcomponent = new Image<unsigned int>( m_w, m_h );

	m_hardSegmentation = new Image<SegmentationValue>( m_w, m_h );
	m_hardSegmentation->fill(SegmentationBackground);

	m_softSegmentation = 0;		// Not yet implemented

	m_TLinksImage = new Image<Color>(m_w, m_h);
	m_TLinksImage->fill(Color(0,0,0));
	m_NLinksImage = new Image<Real>(m_w, m_h);
	m_NLinksImage->fill(0);
	m_GMMImage = new Image<Color>(m_w, m_h);
	m_GMMImage->fill(Color(0,0,0));
	m_AlphaImage = new Image<Real>(m_w, m_h);
	m_AlphaImage->fill(0);

	m_foregroundGMM = new GMM(5);
	m_backgroundGMM = new GMM(5);

	//set some constants
	m_lambda = 50;
	computeL();
	computeBeta();
	
	m_NLinks = new Image<NLinks>( m_w, m_h );
	computeNLinks();

	m_graph = 0;
	m_nodes = new Image<Graph::node_id>( m_w, m_h );
}

GrabCut::~GrabCut()
{
	if (m_trimap)
		delete m_trimap;
	if (m_GMMcomponent)
		delete m_GMMcomponent;
	if (m_hardSegmentation)
		delete m_hardSegmentation;
	if (m_softSegmentation)
		delete m_softSegmentation;
	if (m_foregroundGMM)
		delete m_foregroundGMM;
	if (m_backgroundGMM)
		delete m_backgroundGMM;
	if (m_NLinks)
		delete m_NLinks;
	if (m_nodes)
		delete m_nodes;
	if (m_TLinksImage)
		delete m_TLinksImage;
	if (m_NLinksImage)
		delete m_NLinksImage;
	if (m_GMMImage)
		delete m_GMMImage;
	if (m_AlphaImage)
		delete m_AlphaImage;
}


void GrabCut::initialize(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
	// Step 1: User creates inital Trimap with rectangle, Background outside, Unknown inside
	m_trimap->fill(TrimapBackground);
	m_trimap->fillRectangle(x1, y1, x2, y2, TrimapUnknown);
  
	// Step 2: Initial segmentation, Background where Trimap is Background, Foreground where Trimap is Unknown.
	m_hardSegmentation->fill(SegmentationBackground);
	m_hardSegmentation->fillRectangle(x1, y1, x2, y2, SegmentationForeground);
}

void GrabCut::initializeWithMask(Image<Color>* mask) {
	m_trimap->fill(TrimapBackground);
	m_hardSegmentation->fill(SegmentationBackground);
	for(int x=0;x<mask->width();x++) {
		for(int y=0;y<mask->height();y++) {
			if((*mask)(x,y).b > 0.0 || (*mask)(x,y).g > 0.0 || (*mask)(x,y).r > 0.0) {
				(*m_trimap)(x,y) = TrimapUnknown;
				(*m_hardSegmentation)(x,y) = SegmentationForeground;
			}
		}
	}
}

void GrabCut::fitGMMs()
{
	// Step 3: Build GMMs using Orchard-Bouman clustering algorithm
	buildGMMs(*m_backgroundGMM, *m_foregroundGMM, *m_GMMcomponent, *m_image, *m_hardSegmentation);

	// Initialize the graph for graphcut (do this here so that the T-Link debugging image will be initialized)
	initGraph();

	// Build debugging images
	buildImages();
}

int GrabCut::refineOnce()
{
	Real flow = 0;

	// Steps 4 and 5: Learn new GMMs from current segmentation
	learnGMMs(*m_backgroundGMM, *m_foregroundGMM, *m_GMMcomponent, *m_image, *m_hardSegmentation);

	// Step 6: Run GraphCut and update segmentation
	initGraph();
	if (m_graph)
		flow = m_graph->maxflow();
	
	int changed = updateHardSegmentation();
	printf("%d pixels changed segmentation (max flow = %f)\n", changed, flow ); 

	// Build debugging images
	buildImages();

	return changed;
}

void GrabCut::refine()
{
	int changed = m_w*m_h;

	while (changed)
		changed = refineOnce();
}

int GrabCut::updateHardSegmentation()
{
	int changed = 0;

	for (unsigned int y = 0; y < m_h; ++y)
	{
		for (unsigned int x = 0; x < m_w; ++x)
		{
			SegmentationValue oldValue = (*m_hardSegmentation)(x,y);

			if ((*m_trimap)(x,y) == TrimapBackground)
				(*m_hardSegmentation)(x,y) = SegmentationBackground;
			else if ((*m_trimap)(x,y) == TrimapForeground)
				(*m_hardSegmentation)(x,y) = SegmentationForeground;
			else	// TrimapUnknown
			{
				if (m_graph->what_segment((*m_nodes)(x,y)) == Graph::SOURCE)
					(*m_hardSegmentation)(x,y) = SegmentationForeground;
				else
					(*m_hardSegmentation)(x,y) = SegmentationBackground;
			}

			if (oldValue != (*m_hardSegmentation)(x,y))
				changed++;
		}
	}
	return changed;
}

void GrabCut::setTrimap(int x1, int y1, int x2, int y2, const TrimapValue& t)
{
	(*m_trimap).fillRectangle(x1, y1, x2, y2, t);

	// Immediately set the segmentation as well so that the display will update.
	if (t == TrimapForeground)
		(*m_hardSegmentation).fillRectangle(x1, y1, x2, y2, SegmentationForeground);
	else if (t == TrimapBackground)
		(*m_hardSegmentation).fillRectangle(x1, y1, x2, y2, SegmentationBackground);

	// Build debugging images
	buildImages();
}

void GrabCut::display( int t )
{
	switch(t)
	{
	case 0:					// Display the image
		GrabCutNS::display(*m_image);
		break;

	case 1:					// Display Gaussian component memberships
		GrabCutNS::display(*m_GMMImage);
		break;

	case 2:					// Display N-link weights
		GrabCutNS::display(*m_NLinksImage, GL_LUMINANCE);
		break;

	case 3:					// Display T-link weights
		GrabCutNS::display(*m_TLinksImage);		
		break;

	default:
		// Do nothing
		break;
	}
}

void GrabCut::overlayAlpha()
{
	GrabCutNS::display(*m_AlphaImage, GL_ALPHA);
}


//private functions

void GrabCut::initGraph()
{
	// Set up the graph (it can only be used once, so we have to recreate it each time the graph is updated)
	if (m_graph)
		delete m_graph;
	m_graph = new Graph();

	for (unsigned int y = 0; y < m_h; ++y)
	{
		for(unsigned int x = 0; x < m_w; ++x)
		{
			(*m_nodes)(x,y) = m_graph->add_node();
		}
	}
	
	// Set T-Link weights
	for (unsigned int y = 0; y < m_h; ++y)
	{
		for(unsigned int x = 0; x < m_w; ++x)
		{
			Real back, fore;

			if ((*m_trimap)(x,y) == TrimapUnknown )
			{
				fore = -log(m_backgroundGMM->p((*m_image)(x,y)));
				back = -log(m_foregroundGMM->p((*m_image)(x,y)));
			}
			else if ((*m_trimap)(x,y) == TrimapBackground )
			{
				fore = 0;
				back = m_L;
			}
			else		// TrimapForeground
			{
				fore = m_L;
				back = 0;
			}

			m_graph->set_tweights((*m_nodes)(x,y), fore, back);

			(*m_TLinksImage)(x,y).r = pow((Real)fore/m_L, (Real)0.25);
			(*m_TLinksImage)(x,y).g = pow((Real)back/m_L, (Real)0.25);
		}
	}

	// Set N-Link weights from precomputed values
	for (unsigned int y = 0; y < m_h; ++y)
	{
		for (unsigned int x = 0; x < m_w; ++x)
		{
			if( x > 0 && y < m_h-1 )
				m_graph->add_edge((*m_nodes)(x,y), (*m_nodes)(x-1,y+1), (*m_NLinks)(x,y).upleft, (*m_NLinks)(x,y).upleft);

			if( y < m_h-1 )
				m_graph->add_edge((*m_nodes)(x,y), (*m_nodes)(x,y+1), (*m_NLinks)(x,y).up, (*m_NLinks)(x,y).up);

			if( x < m_w-1 && y < m_h-1 )
				m_graph->add_edge((*m_nodes)(x,y), (*m_nodes)(x+1,y+1), (*m_NLinks)(x,y).upright, (*m_NLinks)(x,y).upright);

			if( x < m_w-1 )
				m_graph->add_edge((*m_nodes)(x,y), (*m_nodes)(x+1,y), (*m_NLinks)(x,y).right, (*m_NLinks)(x,y).right);
		}
	}
}

void GrabCut::computeNLinks()
{
	for( unsigned int y = 0; y < m_h; ++y )
	{
		for( unsigned int x = 0; x < m_w; ++x )
		{
			if( x > 0 && y < m_h-1 )
				(*m_NLinks)(x,y).upleft = computeNLink( x, y, x-1, y+1 );
				
			if( y < m_h-1 )
				(*m_NLinks)(x,y).up = computeNLink( x, y, x, y+1 );
				
			if( x < m_w-1 && y < m_h-1 )
				(*m_NLinks)(x,y).upright = computeNLink( x, y, x+1, y+1 );
				
			if( x < m_w-1 )
				(*m_NLinks)(x,y).right = computeNLink( x, y, x+1, y );
		}
	}
}

Real GrabCut::computeNLink(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
	return m_lambda * exp( -m_beta * distance2((*m_image)(x1,y1),(*m_image)(x2,y2)) ) / distance(x1,y1,x2,y2);
}

void GrabCut::computeBeta()
{
	Real result = 0;
	int edges = 0;

	for (unsigned int y = 0; y < m_h; ++y)
	{
		for (unsigned int x = 0; x < m_w; ++x)
		{
			if (x > 0 && y < m_h-1)					// upleft
			{
				result += distance2( (*m_image)(x,y), (*m_image)(x-1,y+1) );
				edges++;
			}

			if (y < m_h-1)							// up
			{
				result += distance2( (*m_image)(x,y), (*m_image)(x,y+1) );
				edges++;
			}

			if (x < m_w-1 && y < m_h-1)				// upright
			{
				result += distance2( (*m_image)(x,y), (*m_image)(x+1,y+1) );
				edges++;
			}

			if (x < m_w-1)							// right
			{
				result += distance2( (*m_image)(x,y), (*m_image)(x+1,y) );
				edges++;
			}
		}
	}

	m_beta = (Real)(1.0/(2*result/edges));
}

void GrabCut::computeL()
{
	m_L = 8*m_lambda + 1;
}

void GrabCut::buildImages()
{
	m_NLinksImage->fill(0);

	for (unsigned int y = 0; y < m_h; ++y)
	{
		for (unsigned int x = 0; x < m_w; ++x)
		{
			// T-Links image is populated in initGraph since we have easy access to the link values there.

			// N-Links image
			if( x > 0 && y < m_h-1 )
				{
				(*m_NLinksImage)(x,y) += (*m_NLinks)(x,y).upleft/m_L;
				(*m_NLinksImage)(x-1,y+1) += (*m_NLinks)(x,y).upleft/m_L;
				}

			if( y < m_h-1 )
				{
				(*m_NLinksImage)(x,y) += (*m_NLinks)(x,y).up/m_L;
				(*m_NLinksImage)(x,y+1) += (*m_NLinks)(x,y).up/m_L;
				}

			if( x < m_w-1 && y < m_h-1 )
				{
				(*m_NLinksImage)(x,y) += (*m_NLinks)(x,y).upright/m_L;
				(*m_NLinksImage)(x+1,y+1) += (*m_NLinks)(x,y).upright/m_L;
				}

			if( x < m_w-1 )
				{
				(*m_NLinksImage)(x,y) += (*m_NLinks)(x,y).right/m_L;
				(*m_NLinksImage)(x+1,y) += (*m_NLinks)(x,y).right/m_L;
				}

			// GMM image
			if ((*m_hardSegmentation)(x,y) == SegmentationForeground)
				(*m_GMMImage)(x,y) = Color((Real)((*m_GMMcomponent)(x,y)+1)/m_foregroundGMM->K(),0,0);
			else
				(*m_GMMImage)(x,y) = Color(0,(Real)((*m_GMMcomponent)(x,y)+1)/m_backgroundGMM->K(),0);
			
			//Alpha image
			if ((*m_hardSegmentation)(x,y) == SegmentationForeground)
				(*m_AlphaImage)(x,y) = 0.0;
			else
				(*m_AlphaImage)(x,y) = 0.75;
		}
	}
}
}