/* === S Y N F I G ========================================================= */
/*!	\file centerlinecolors.cpp
**	\brief centerlinecolors File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */
#include "polygonizerclasses.h"

#include <synfig/surface.h>
#include <synfig/rendering/software/surfacesw.h>
#include <math.h>
#include <ETL/handle>
#include <synfig/layers/layer_bitmap.h>


/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace studio;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

synfig::Color pixelToColor(const Surface &rsurface, int x, int y)
{
  const int Y = rsurface.get_h() - y -1; 
	const Color *row = rsurface[Y]; 
  return row[x];
}

bool checkPixelThreshold(const Surface &rsurface, int x, int y,int threshold)
{
  const int Y = rsurface.get_h() - y -1; 
	const Color *row = rsurface[Y]; 
	int r = 255.0*pow(row[x].get_r(),1/2.2);
	int g = 255.0*pow(row[x].get_g(),1/2.2);
	int b = 255.0*pow(row[x].get_b(),1/2.2);
  int a = 255.0*pow(row[x].get_a(),1/2.2);
  return std::max(r,std::max(g,b)) < threshold * (a / 255.0);

}

//------------------------------------------------------------------------
static synfig::Point3 firstInkChangePosition(const Surface &ras,
                                        const synfig::Point3 &start, const synfig::Point3 &end, int threshold) 
{
  double dist = (end - start).mag();

  int sampleMax = ceil_to_int(dist), sampleCount = sampleMax + 1;
  double sampleMaxD = double(sampleMax);

  // Get first ink color
  int s;
  synfig::Color color ;

  for (s = 0; s != sampleCount; ++s) 
  {
    synfig::Point3 p  = start * (1 - s/sampleMaxD) +  end * (s/sampleMaxD);
    // const TPixelCM32 &pix = pixel(*ras, p.x, p.y);
    if (checkPixelThreshold(ras,p[0],p[1], threshold)) 
    {
      color = pixelToColor(ras,p[0],p[1]);
      break;
    }
  }

  // Get second color
  for (; s != sampleCount; ++s) 
  {
    synfig::Point3 p = start *(1 - s/sampleMaxD) +  end * (s/sampleMaxD);
    // const TPixelCM32 &pix = pixel(*ras, p.x, p.y);

    if (checkPixelThreshold(ras,p[0],p[1],threshold) && pixelToColor(ras,p[0],p[1]) != color)
     break;
  }

  // Return middle position between s-1 and s
  if (s < sampleCount)
    return start * (1 - ((s - 0.5) / sampleMaxD)) + end * ((s - 0.5) / sampleMaxD);

  return synfig::Point3::nan();
}

// Find color of input sequence. Will be copied to its equivalent stroke.
// Currently in use only on colormaps

// Summary: It is better to test the color to be assigned to the strokes
// to check
// sequences * before * convert them to TStroke (since you lose part
// of the original grip
// to the line). You specify a number of 'taste points' of the broken line
// equidistant from each other,
// on which the value of the corresponding pixel input is taken. If
// identifies a change
// of color, the sequence breaking procedure is launched: yes
// identifies the point
// of breaking, and the sequence s is blocked there; a new one is built
// sequence newSeq e
// sampleColor is re-launched (ras, newSeq, sOpposite). Sequences between two points
// of breaking up
// are inserted into the vector 'globals-> singleSequences'.
// In the case of circular sequences there is a small change: the first point of
// splitting
// * only redefines * the s-node, without introducing new sequences.
// The sequence sOpposite, 'inverse' of s, remains and becomes 'forward-oriented'
// after updating
// of the tail.
// Notice that the break nodes are entered with the signature
// 'SAMPLECOLOR_SIGN'.
// NOTE: The J-S 'upper' graph structure is not altered in here.
// Eventualm. to do outside.

static void sampleColor(const etl::handle<synfig::Layer_Bitmap> &ras, int threshold, Sequence &seq,
                        Sequence &seqOpposite, SequenceList &singleSequences) 
{
  SkeletonGraph *currGraph = seq.m_graphHolder;

  // Calculate sequence parametrization
  std::vector<unsigned int> nodes;
  std::vector<double> params;

  // Meanwhile, ensure each point belong to ras. Otherwise, typically an error
  // occured in the thinning process and it's better avoid sampling procedure. Only
  // exception, when
  // a point has x==rsurface.get_w() || y==rsurface.get_h(); that is accepted.
  synfig::rendering::SurfaceResource::LockRead<synfig::rendering::SurfaceSW> lock( ras->rendering_surface ); 
	const Surface &rsurface = lock->get_surface(); 
  {
    const synfig::Point3 &headPos = *currGraph->getNode(seq.m_head);
    // get bounds rectangle with = 0,0,lx -1, ly-1
    if (headPos[0] < 0 || rsurface.get_w() < headPos[0] || headPos[1] < 0 || rsurface.get_h() < headPos[1])// check again and return
        return;
    
  }

  unsigned int curr, currLink, next;
  double meanThickness = currGraph->getNode(seq.m_head)->operator[](2);

  params.push_back(0);
  nodes.push_back(seq.m_head);

  for (curr = seq.m_head, currLink = seq.m_headLink;
       curr != seq.m_tail || params.size() == 1; seq.next(curr, currLink)) 
  {
    next = currGraph->getNode(curr).getLink(currLink).getNext();

    const synfig::Point3 &nextPos = *currGraph->getNode(next);
    if (nextPos[0] < 0 || rsurface.get_w() < nextPos[0] || nextPos[1] < 0 || rsurface.get_h() < nextPos[1]) 
    {
      return;
    }

    params.push_back(params.back() + (*currGraph->getNode(next) - *currGraph->getNode(curr)).mag());
    nodes.push_back(next);

    meanThickness += currGraph->getNode(next)->operator[](2);
  }

  meanThickness /= params.size();

  // Exclude 0-length sequences
  if (params.back() < 0.01) 
  {
    seq.m_color = pixelToColor(rsurface, currGraph->getNode(seq.m_head)->operator[](0),
                        currGraph->getNode(seq.m_head)->operator[](1));
    return;
  }

  // Prepare sampling procedure
  int paramCount = params.size(), paramMax = paramCount - 1;

  int sampleMax = std::max(params.back() / std::max(meanThickness, 1.0),3.0),    // Number of color samples depends on
      sampleCount = sampleMax + 1;  // the ratio params.back() / meanThickness

  std::vector<double> sampleParams(sampleCount);  // Sampling lengths
  std::vector<synfig::Point> samplePoints(sampleCount);  // Image points for color sampling
  std::vector<int> sampleSegments(sampleCount);  // Sequence segment index for the above

  // Sample colors
  for (int s = 0, j = 0; s != sampleCount; ++s) 
  {
    double samplePar = params.back() * (s / double(sampleMax));

    while (j != paramMax && params[j + 1] < samplePar)  // params[j] < samplePar <= params[j+1]
      ++j;

    double t = (samplePar - params[j]) / (params[j + 1] - params[j]);

    synfig::Point3 samplePoint(*currGraph->getNode(nodes[j]) * (1 - t) +
                          *currGraph->getNode(nodes[j + 1]) * t);

    sampleParams[s] = samplePar;
    samplePoints[s] = synfig::Point(
        std::min(samplePoint[0],
                 double(rsurface.get_w() - 1)),  // This deals with sample points at
        std::min(samplePoint[1],
                 double(rsurface.get_h() - 1)));  // the top/right raster border
    sampleSegments[s] = j;
  }

  // NOTE: Extremities of a sequence are considered unreliable: they typically
  // happen
  //       to be junction points shared between possibly different-colored
  //       strokes.

  // Find first and last extremity-free sampled points
  synfig::Point3 first(*currGraph->getNode(seq.m_head));
  synfig::Point3 last(*currGraph->getNode(seq.m_tail));

  int i, k;

  for (i = 1;
       params.back() * i / double(sampleMax) <= first[2] && i < sampleCount; ++i)
    ;
  for (k = sampleMax - 1;
       params.back() * (sampleMax - k) / double(sampleMax) <= last[2] && k >= 0;
       --k)
    ;

  // Give s the first sampled ink color found

  // Initialize with a last-resort reasonable color - not just 0
  seq.m_color = seqOpposite.m_color = pixelToColor(rsurface,samplePoints[0][0],samplePoints[0][1]);

  int l;

  for (l = i - 1; l >= 0; --l) 
  {
    if (checkPixelThreshold(rsurface,samplePoints[l][0],samplePoints[l][1], threshold))
    {
      seq.m_color = seqOpposite.m_color = pixelToColor(rsurface,samplePoints[l][0],samplePoints[l][1]);
      break;
    }
  }

  // Then, look for the first reliable ink
  for (l = i; l <= k; ++l) 
  {
    if (checkPixelThreshold(rsurface,samplePoints[l][0],samplePoints[l][1], threshold))
    {
      seq.m_color = seqOpposite.m_color = pixelToColor(rsurface, samplePoints[l][0], samplePoints[l][1]);

      break;
    }
  }

  if (i >= k) goto _getOut;  // No admissible segment found for splitting
                             // check.
  // Find color changes between sampled colors
  for (l = i; l < k; ++l) 
  {
    const int x1 = samplePoints[l + 1][0], y1 = samplePoints[l + 1][1],
              x2 = samplePoints[l + 2][0], y2 = samplePoints[l + 2][1];
    
    // const TPixelCM32
    //     &nextSample = ras->pixels(x1)[y1],
    //     &nextSample2 = ras->pixels(x2)[y2];  // l < k < sampleMax - so +2 is ok

    if (checkPixelThreshold(rsurface,x1,y1,threshold) &&
        pixelToColor(rsurface,x1,y1) != seq.m_color &&
        checkPixelThreshold(rsurface,x2,y2,threshold) &&
        pixelToColor(rsurface,x2,y2) == pixelToColor(rsurface,x1,y1))  // Ignore single-sample color changes
    {
      // Found a color change - apply splitting procedure
      // NOTE: The function RETURNS BEFORE THE FOR IS CONTINUED!

      synfig::Color nextColor = pixelToColor(rsurface,x1,y1);

      // Identify split segment
      int u;

      for (u = sampleSegments[l]; u < sampleSegments[l + 1]; ++u) 
      {
        const int x = currGraph->getNode(nodes[u + 1])->operator[](0),
                  y = currGraph->getNode(nodes[u + 1])->operator[](1);
        if (checkPixelThreshold(rsurface,x,y,threshold) && pixelToColor(rsurface,x,y) != seq.m_color)
         break;
      }

      // Now u indicates the splitting segment. Search for splitting point by
      // binary subdivision.
      const synfig::Point3 &nodeStartPos = *currGraph->getNode(nodes[u]),
                           &nodeEndPos   = *currGraph->getNode(nodes[u + 1]);

      synfig::Point3 splitPoint = firstInkChangePosition(rsurface, nodeStartPos, nodeEndPos, threshold);

      if (splitPoint == synfig::Point3::nan())
        splitPoint = (nodeStartPos + nodeEndPos) * 0.5; 
      // A color change was found, but could not be precisely located. Just take
      // a reasonable representant. Insert a corresponding new node in basic graph structure.

      unsigned int splitNode = currGraph->newNode(splitPoint);

      unsigned int nodesLink =
          currGraph->getNode(nodes[u]).linkOfNode(nodes[u + 1]);
      currGraph->insert(splitNode, nodes[u], nodesLink);
      *currGraph->node(splitNode).link(0) =
          *currGraph->getNode(nodes[u]).getLink(nodesLink);

      nodesLink = currGraph->getNode(nodes[u + 1]).linkOfNode(nodes[u]);
      currGraph->insert(splitNode, nodes[u + 1], nodesLink);
      *currGraph->node(splitNode).link(1) =
          *currGraph->getNode(nodes[u + 1]).getLink(nodesLink);

      currGraph->node(splitNode).setAttribute(SAMPLECOLOR_SIGN);  // Sign all split-inserted nodes

      if (seq.m_head == seq.m_tail &&
          currGraph->getNode(seq.m_head).getLinksCount() == 2 &&
          !currGraph->getNode(seq.m_head).hasAttribute(SAMPLECOLOR_SIGN)) 
      {
        // Circular case: we update s to splitNode and relaunch this very
        // procedure on it.
        seq.m_head = seq.m_tail = splitNode;
        sampleColor(ras, threshold, seq, seqOpposite, singleSequences);
      } 
      else 
      {
        // Update upper (Joint-Sequence) graph data
        Sequence newSeq;
        newSeq.m_graphHolder = currGraph;
        newSeq.m_head        = splitNode;
        newSeq.m_headLink    = 0;
        newSeq.m_tail        = seq.m_tail;
        newSeq.m_tailLink    = seq.m_tailLink;

        seq.m_tail     = splitNode;
        seq.m_tailLink = 1;  // (link from splitNode to nodes[u] inserted for
                             // second by 'insert')

        seqOpposite.m_graphHolder = seq.m_graphHolder;  // Inform that a split was found

        // NOTE: access on s terminates at newSeq's push_back, due to possible
        // reallocation of globals->singleSequences

        if ((!(seq.m_head == newSeq.m_tail &&
               currGraph->getNode(seq.m_head).getLinksCount() == 2)) &&
            currGraph->getNode(seq.m_head).hasAttribute(SAMPLECOLOR_SIGN))
          singleSequences.push_back(seq);

        sampleColor(ras, threshold, newSeq, seqOpposite, singleSequences);
      }

      return;
    }
  }

_getOut:

  // Color changes not found (and therefore no newSeq got pushed back); if a
  // split happened, update sOpposite.
  if (currGraph->getNode(seq.m_head).hasAttribute(SAMPLECOLOR_SIGN)) {
    seqOpposite.m_color    = seq.m_color;
    seqOpposite.m_head     = seq.m_tail;
    seqOpposite.m_headLink = seq.m_tailLink;
    seqOpposite.m_tail     = seq.m_head;
    seqOpposite.m_tailLink = seq.m_headLink;
  }
}


/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

void studio::calculateSequenceColors(const etl::handle<synfig::Layer_Bitmap> &ras, VectorizerCoreGlobals &g) 
{
  int threshold                           = g.currConfig->m_threshold;
  SequenceList &singleSequences           = g.singleSequences;
  JointSequenceGraphList &organizedGraphs = g.organizedGraphs;

  unsigned int i, j, k;
  int l;

  if (ras && g.currConfig->m_maxThickness > 0.0) 
  {
    // singleSequence is traversed back-to-front because new, possibly splitted
    // sequences
    // are inserted at back - and don't have to be re-sampled.
    for (l = singleSequences.size() - 1; l >= 0; --l) 
    {
      Sequence rear;
      sampleColor(ras, threshold, singleSequences[l], rear, singleSequences);
      // If rear is built, a split occurred and the rear of this
      // single sequence has to be pushed back.
      if (rear.m_graphHolder) singleSequences.push_back(rear);
    }

    for (i = 0; i < organizedGraphs.size(); ++i)
    {
        for (j = 0; j < organizedGraphs[i].getNodesCount(); ++j)
        {
          if (!organizedGraphs[i].getNode(j).hasAttribute(JointSequenceGraph::ELIMINATED))
          {
              for (k = 0; k < organizedGraphs[i].getNode(j).getLinksCount(); ++k) 
              {
                  Sequence &s = *organizedGraphs[i].node(j).link(k);
                  if (s.isForward() && !s.m_graphHolder->getNode(s.m_tail).hasAttribute(SAMPLECOLOR_SIGN))
                  {
                    unsigned int next = organizedGraphs[i].node(j).link(k).getNext();
                    unsigned int nextLink = organizedGraphs[i].tailLinkOf(j, k);
                    Sequence &sOpposite = *organizedGraphs[i].node(next).link(nextLink);
                    sampleColor(ras, threshold, s, sOpposite, singleSequences);
                  }
              }
          }  

        }
    }
      
  }
}

//==========================================================================
//==========================================================================

// Take samples of image colors to associate each stroke to its corresponding
// palette color. Currently working on colormaps, closest-to-black strokes
// otherwise.
/*
void applyStrokeColors(std::vector<TStroke *> &strokes, const TRasterP &ras,
                       TPalette *palette, VectorizerCoreGlobals &g) {
  JointSequenceGraphList &organizedGraphs = g.organizedGraphs;
  SequenceList &singleSequences           = g.singleSequences;

  TRasterCM32P cm = ras;
  unsigned int i, j, k, n;

  if (cm && g.currConfig->m_maxThickness > 0.0) 
  {
    applyStrokeIndices(&g);

    // Treat single sequences before, like conversionToStrokes(..)
    for (i = 0; i < singleSequences.size(); ++i)
      strokes[i]->setStyle(singleSequences[i].m_color);

    // Then, treat remaining graph-strokes
    n = i;

    for (i = 0; i < organizedGraphs.size(); ++i)
      for (j = 0; j < organizedGraphs[i].getNodesCount(); ++j)
        if (!organizedGraphs[i].getNode(j).hasAttribute(
                JointSequenceGraph::ELIMINATED))  // due to junction recovery
          for (k = 0; k < organizedGraphs[i].getNode(j).getLinksCount(); ++k) {
            Sequence &s = *organizedGraphs[i].node(j).link(k);
            if (s.isForward()) {
              // vi->getStroke(n)->setStyle(s.m_color);
              strokes[n]->setStyle(s.m_color);
              ++n;
            }
          }

    // Order vector image according to actual color-coverings at junctions.
    orderColoredStrokes(organizedGraphs, strokes, cm, palette);
  }
  else
  {
    // Choose closest-to-black palette color
    int blackStyleId = palette->getClosestStyle(TPixel32::Black);

    unsigned int i;
    for (i = 0; i < strokes.size(); ++i) strokes[i]->setStyle(blackStyleId);
  }
}

*/

