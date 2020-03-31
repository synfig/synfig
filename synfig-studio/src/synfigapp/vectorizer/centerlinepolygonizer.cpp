/* === S Y N F I G ========================================================= */
/*!	\file centerlinepolygonizer.cpp
**
**	$Id$
**
**	\legal
**	This file uses code from OpenToonz open-source animation software  (https://github.com/opentoonz/opentoonz/), which is developed from Toonz, a software originally created by Digital Video, S.p.A., Rome Italy Digital Video, S.p.A., Rome Italy.
**
**	Copyright (c) 2016 - 2019, DWANGO Co., Ltd.
**	Copyright (c) 2016 Toshihiro Shimizu - https://github.com/meso
**	Copyright (c) 2016 Shinya Kitaoka - https://github.com/skitaoka
**	Copyright (c) 2016 shun-iwasawa - https://github.com/shun-iwasawa
**	Copyright (c) 2016 Campbell Barton - https://github.com/ideasman42
**	Copyright (c) 2019 luzpaz - https://github.com/luzpaz
**
**	Copyright (c) 2019 - 2020, Ankit Kumar Dwivedi - https://github.com/ankit-kumar-dwivedi
**
**	LICENSE
** 
**	BSD 3-Clause "New" or "Revised" License
** 
**	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
**
**	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
**
**	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
**
**	3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
**
**	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#include <synfig/vector.h>


/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace studio;
using namespace synfig;

typedef etl::handle<synfig::Layer_Bitmap> Handle;
/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

class RawBorderPoint 
{
  synfig::PointInt m_position;
  int m_ambiguousTurn;  // used to remember cases of multiple turning directions
  // in a RawBorder extraction.

public:
  RawBorderPoint() : m_ambiguousTurn(0) {}
  RawBorderPoint(int i, int j) : m_position(i,j), m_ambiguousTurn(0) {}

  inline synfig::PointInt pos() const { return m_position; }
  inline int x() const { return m_position[0]; }
  inline int y() const { return m_position[1]; }

  enum { left = 1, right = 2 };  // Direction taken at ambiguous turning point
  inline int getAmbiguous() const { return m_ambiguousTurn; }
  inline void setAmbiguous(int direction) { m_ambiguousTurn = direction; }
};

//--------------------------------------------------------------------------

class studio::RawBorder final : public std::vector<RawBorderPoint> 
{
  int m_xExternal;  // x coordinate of a specific vertex in the outer
  // RawBorder which contains this inner one.

  synfig::Point *m_coordinateSums;
  synfig::Point *m_coordinateSquareSums;
  double *m_coordinateMixedSums;

public:
  RawBorder()
	  : m_xExternal(0),
		m_coordinateSums(nullptr),
		m_coordinateSquareSums(nullptr),
		m_coordinateMixedSums(nullptr)
  {}
  ~RawBorder() {}

  void setXExternalPixel(int a) { m_xExternal = a; }
  int xExternalPixel() { return m_xExternal; }
  synfig::Point *&sums() { return m_coordinateSums; }
  synfig::Point *&sums2() { return m_coordinateSquareSums; }
  double *&sumsMix() { return m_coordinateMixedSums; }
};

//--------------------------------------------------------------------------

// Of course we don't want RawBorders to be entirely copied whenever STL
// requires to resize a BorderFamily...
typedef std::vector<RawBorder *> BorderFamily;
typedef std::vector<BorderFamily> BorderList;

//============================
//    Polygonizer Locals
//============================

namespace 
{
// Const names
enum { white = 0, black = 1 };
enum { inner = 0, outer = 1, none = 2, invalid = 3 };
}

//=======================================================================================

//-------------------------------
//    Raster Data Functions
//-------------------------------

//--------------------------------------------------------------------------
class PixelEvaluator 
{
  const Surface &m_surface;
  int m_threshold;

public:
  PixelEvaluator(const Surface &surface, int threshold)
      : m_surface(surface), m_threshold(threshold) {}

  inline unsigned char getBlackOrWhite(int x, int y);
};

// //--------------------------------------------------------------------------

inline unsigned char PixelEvaluator::getBlackOrWhite(int x, int y)
{
	const int Y = m_surface.get_h() - y -1; 
	const Color &color = m_surface[Y][x];
	int r = 255.99*color.get_r();
	int g = 255.99*color.get_g();
	int b = 255.99*color.get_b();
	int a = 255.99*color.get_a();
	return std::max(r,std::max(g,b)) < m_threshold * (a / 255.0);
}

//--------------------------------------------------------------------------

// Signaturemap format:
//  stores a map of bytes, whose first bit represents the color (black/white) of
//  corresponding pixel, and
//  the rest its 'signature', used as an int to store information.

class Signaturemap 
{
  std::unique_ptr<unsigned char[]> m_array;
  int m_rowSize;
  int m_colSize;

public:
  Signaturemap(const Handle &ras, int threshold);

  //not needed 
  //void readRasterData(const Handle &ras, int threshold);

  inline int getRowSize() const { return m_rowSize; }
  inline int getColSize() const { return m_colSize; }

  unsigned char *pixelByte(int x, int y) 
  {
    return &m_array[(y + 1) * m_rowSize + x + 1];
  }

  bool getBitmapColor(int x, int y) const 
  {
    return m_array[(y + 1) * m_rowSize + x + 1] & 1;
  }

  inline unsigned char getSignature(int x, int y) const 
  {
    return m_array[(y + 1) * m_rowSize + x + 1] >> 1;
  }

  void setSignature(int x, int y, int val) 
  {
    unsigned char *pixel = pixelByte(x, y);
    *pixel &= 1;
    *pixel |= (val << 1);  
  }
};

//--------------------------------------------------------------------------

Signaturemap::Signaturemap(const Handle &ras, int threshold)
{
	// read the raster data
	unsigned char *currByte;
	int x, y;

	rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lock( ras->rendering_surface );
	const Surface &surface = lock->get_surface(); 
	PixelEvaluator evaluator(surface, threshold);//evaluator object with surface, threshold as constructor args
	m_rowSize = surface.get_w() + 2;
	m_colSize = surface.get_h() + 2;
	m_array.reset(new unsigned char[m_rowSize * m_colSize]);

	memset(m_array.get(), none << 1, m_rowSize);

	currByte = m_array.get() + m_rowSize;
	for (y = 0; y <m_colSize - 2 ; ++y) 
	{
		*currByte = none << 1;
		currByte++;

		for (x = 0; x < m_rowSize - 2; ++x, ++currByte)
			*currByte = evaluator.getBlackOrWhite(x, y) | (none << 1);
		*currByte = none << 1;
		currByte++;
	}

	memset(currByte, none << 1, m_rowSize);
}

//--------------------------------------------------------------------------

// Minority check for ambiguous turning directions
inline bool getMinorityCheck(const Signaturemap &ras, int x, int y) 
{
  // Assumes (x,y) is ambiguous case: 2 immediate surrounding pixels are white
  // and 2 black
  return (ras.getBitmapColor(x + 1, y) + ras.getBitmapColor(x + 1, y - 1) +
          ras.getBitmapColor(x - 2, y) + ras.getBitmapColor(x - 2, y - 1) +
          ras.getBitmapColor(x - 1, y + 1) + ras.getBitmapColor(x - 1, y - 2) +
          ras.getBitmapColor(x, y + 1) + ras.getBitmapColor(x, y - 2)) > 4;
}

//--------------------------------------------------------------------------

// Sets signature of a given border
inline void setSignature(Signaturemap &ras, const RawBorder &border, int val) 
{
  unsigned int j;
  int yOld;

  // Set border's alpha channel
  yOld = border.back().y();
  for (j = 0; j < border.size(); ++j) 
  {
    if (border[j].y() < yOld) 
    {
      ras.setSignature(border[j].x(), border[j].y(), val);
    } 
    else if (border[j].y() > yOld) 
    {
      ras.setSignature(border[j].x(), yOld, val);
    }
    yOld = border[j].y();
  }
}
//==========================================================================

//-------------------------------
//    Raw Borders Extraction
//-------------------------------

// RawBorderPoints correspond to lower-left pixel corners.
// EXAMPLE: (0,0) is the lower-left *corner* of the image, whereas (0,0) also
// represents coordinates of the lower-left *pixel*.
// NOTE: 'Ambiguous turning' vertices are those of kind:
//
//    B|W           W|B
//    -x-    -or-   -x-
//    W|B           B|W
//
// Keeping B on the right of our path-seeking direction, we may either turn
// left or right at these points.

static RawBorder *extractPath(Signaturemap &ras, int x0, int y0, int pathType,
                              int xOuterPixel, int despeckling) 
{
  RawBorder *path = new RawBorder;
  int x, y;
  short dirX, dirY;
  long int area = 0;
  bool nextLeftPixel, nextRightPixel;

  if (pathType == outer) 
  {
    dirX = 0;
    dirY = 1;
  }
  else 
  {
    dirX = 1;
    dirY = 0;
    area += y0;
    path->setXExternalPixel(xOuterPixel);
  }

  path->push_back(RawBorderPoint(x0, y0));

  // Check here if (x0, y0) is an ambiguous-direction point
  nextLeftPixel = ras.getBitmapColor(x0 + (dirY - dirX - 1) / 2,
                                     y0 + (-dirY - dirX - 1) / 2);
  nextRightPixel = ras.getBitmapColor(x0 + (-dirX - dirY - 1) / 2,
                                      y0 + (dirX - dirY - 1) / 2);
  if ((nextRightPixel == black) && (nextLeftPixel == white))
    path->back().setAmbiguous(dirX ? RawBorderPoint::left : RawBorderPoint::right);

  // Begin path extraction

  for (x = x0 + dirX, y = y0 + dirY; !(x == x0 && y == y0); x += dirX, y += dirY) 
  {
    path->push_back(RawBorderPoint(x, y));

    // Calculate next direction
    nextLeftPixel = ras.getBitmapColor(x + (dirX - dirY - 1) / 2,
                                       y + (dirY + dirX - 1) / 2);
    nextRightPixel = ras.getBitmapColor(x + (dirX + dirY - 1) / 2,
                                        y + (dirY - dirX - 1) / 2);

    if ((nextRightPixel == black) && (nextLeftPixel == black)) {
      // Left Turn
      std::swap(dirY, dirX);
      dirX = -dirX;
    } 
    else if ((nextRightPixel == white) && (nextLeftPixel == white)) 
    {
      // Right Turn
      std::swap(dirY, dirX);
      dirY = -dirY;
    } 
    else if ((nextRightPixel == white) && (nextLeftPixel == black)) {
      // path->back().setAmbiguous();

      // Do a surrounding check and connect minority color
      if (getMinorityCheck(ras, x, y) == black) 
      {
        std::swap(dirY, dirX);
        dirY = -dirY;
        path->back().setAmbiguous(RawBorderPoint::right);
      }  // right turn
      else 
      {
        std::swap(dirY, dirX);
        dirX = -dirX;
        path->back().setAmbiguous(RawBorderPoint::left);
      }  // left turn
    }

    // Also calculate border area
    area += y * dirX;

    // And sign treated pixel
    if (dirY != 0) ras.setSignature(x, y + (dirY - 1) / 2, pathType);
  }

  // If the inner region's overall area is under a given threshold,
  // then erase it (intended as image noise).
  if (abs(area) < despeckling) 
  {
    setSignature(ras, *path, invalid);
    delete path;
    path = 0;
  }

  return path;
}

//--------------------------------------------------------------------------

static BorderList *extractBorders(const Handle &ras, int threshold, int despeckling)
{
  Signaturemap byteImage(ras, threshold);

  BorderList *borderHierarchy = new BorderList;
  std::vector<RawBorder *> outerBorders;
  std::list<RawBorder *> innerBorders;
  RawBorder *foundPath;

  int x, y;
  bool Color, oldColor;
  int xOuterPixel = 0;
  bool enteredRegionType;
  unsigned char signature;

  rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lock( ras->rendering_surface );
	const Surface &surface = lock->get_surface(); 
	int width = surface.get_w(); 
	int height = surface.get_h(); 
	
  for (y = 0; y < height; ++y) 
  {
    oldColor          = white;
    enteredRegionType = outer;
    for (x = 0; x < width; ++x) 
    {
      if (oldColor ^ (Color = byteImage.getBitmapColor(x, y))) 
      {
        // Region type changes
        enteredRegionType = !enteredRegionType;

        if ((signature = byteImage.getSignature(x, y)) == none) 
        {
          if ((foundPath = extractPath(byteImage, x, y, !enteredRegionType,
                                       xOuterPixel, despeckling)))
            if (enteredRegionType == outer)
              innerBorders.push_back(foundPath);
            else
              outerBorders.push_back(foundPath);
        }

        // If leaving a white region, remember it - in order to establish
        // border hierarchy in the future
        if (enteredRegionType == inner && signature != invalid) xOuterPixel = x;

        // Invalid pixels got signed by a cut-out path, due to insufficient area
        if (signature == invalid)
          byteImage.setSignature(x, y, none);  // Restore them now

        oldColor = Color;
      }
    }
  }

  // Now, we have all borders found, but no hierarchy between them.

  unsigned int i;
  std::list<RawBorder *>::iterator l;

  // Build hierarchy

  innerBorders.push_front(0);  // Just to keep a fixed list head

  for (i = 0; i < outerBorders.size(); ++i) 
  {
    // Initialize a border family
    borderHierarchy->push_back(BorderFamily());
    borderHierarchy->back().push_back(outerBorders[i]);

    // Reset outerBorders[i]'s signature
    setSignature(byteImage, *outerBorders[i], none);

    // Now check inner borders for insideness - check if the outerPixel
    // remembered in path extraction has been cleared
    for (l = innerBorders.begin(), ++l; l != innerBorders.end(); ++l) 
    {
      if (byteImage.getSignature((*l)->xExternalPixel(), (**l)[0].y()) == none) 
      {
        borderHierarchy->back().push_back(*l);
        setSignature(byteImage, **l, none);
        l = innerBorders.erase(l);
        --l;
      }
    }
  }

  return borderHierarchy;
}

//--------------------------------------------------------------------------

//==================================
//    Calculate optimal polygons
//==================================

// The optimal polygon for a given original border is found like:

// 1) Find couples (i,k(i)), so that k(i) be the largest k:
//        d(j,ik) <= 1;  for *all* i<j<k.  (d is infinite-norm distance)
//    It can be shown that such a condition is equivalent to:
//      exists line l : d(l,j)<=1/2, for all i<=j<=k(i).
// 2) Clean the above couples - find couples (i,l(i)):
//        l(i)=min{k(j)},  j=i..n.
// 3) Calculate clipped couples (i',l'); where i'=i+1, l'=l(i)-1.
// 4) Calculate sums for path penalties.
// 5) Apply optimality algorithm.

// NOTE: Weak simpleness reads like: a set of polygons is weak-simple if no edge
//      *crosses* another edge. Superposition and collision of edges with
//      vertices
//      are still admitted.
//        =>  It can be shown that due to 1) and special conditions on ambiguous
//            turnings applied in both 1) and 3), weak simpleness is insured in
//            our polygonization.

//--------------------------------------------------------------------------

// Helper functions/classes: circular-indexed vectors

// returns 1 whenever the triple (a,b,c) is 'circular' mod n.
// NOTE: We'll find useful taking (i,i,j) as 1 and (i,j,j) as 0.
inline bool isCircular(int a, int b, int c) 
{
  return a <= c ? a <= b && b < c : c > b || b >= a;
}

//--------------------------------------------------------------------------

// Extracts a 'next corner' array - helps improving overall speed
inline std::unique_ptr<int[]> findNextCorners(RawBorder &path)
{
  std::unique_ptr<int[]> corners(new int[path.size()]);

  // NOTE: 0 is a corner, due to the path extraction procedure.
  int currentCorner = 0;
  for (int i = path.size() - 1; i >= 0; --i) {
    if (path[currentCorner].x() != path[i].x() &&
        path[currentCorner].y() != path[i].y())
      currentCorner = i + 1;
    corners[i]      = currentCorner;
  }

  return corners;
}

inline int cross(const synfig::PointInt &a, const synfig::PointInt &b) 
{
  return a[0] * b[1] - a[1] * b[0];
}

// Calculate furthest k satisfying 1) for all fixed i.
inline std::unique_ptr<int[]> furthestKs(RawBorder &path, std::unique_ptr<int[]> &nextCorners) 
{
  int n = path.size();
  std::unique_ptr<int[]> kVector(new int[n]);

  enum { left, up, right, down };
  int directionsOccurred[4];

  nextCorners = findNextCorners(path);

  int i, j, k;
  synfig::PointInt shift;
  synfig::PointInt leftConstraint, rightConstraint, violatedConstraint;
  synfig::PointInt newLeftConstraint, newRightConstraint;
  synfig::PointInt jPoint, jNextPoint, iPoint, temp;
  synfig::Point tempD;
  synfig::PointInt direction;

  int directionSignature;

  for (i = 0; i < n; ++i) 
  {
    // Initialize search
    leftConstraint = rightConstraint = synfig::PointInt();
    directionsOccurred[0] = directionsOccurred[1] = directionsOccurred[2] = directionsOccurred[3] = 0;
    j = i;
    jNextPoint = iPoint = path[i].pos();

    // Search for k(i)
    while (1) {
      // NOTE: Here using TPoint::operator= is less effective than setting
      // its x and y components directly...

      jPoint     = jNextPoint;
      jNextPoint = path[nextCorners[j]].pos();

      // Update directions count
      directionSignature = jNextPoint[0] > jPoint[0]
                               ? right
                               : jNextPoint[0] < jPoint[0]
                                     ? left
                                     : jNextPoint[1] > jPoint[1] ? up : down;
      directionsOccurred[directionSignature] = 1;

      // If all 4 axis directions occurred, quit
      if (directionsOccurred[left] && directionsOccurred[right] &&
          directionsOccurred[up] && directionsOccurred[down]) {
        k = j;
        goto foundK;
      }

      // Update displacement from i
      shift = jNextPoint - iPoint;

      // Test j against constraints
      // if(cross(shift, leftConstraint)<0 || cross(shift, rightConstraint)>0)
      if (cross(shift, leftConstraint) < 0) {
        violatedConstraint = leftConstraint;
        break;
      }
      if (cross(shift, rightConstraint) > 0) {
        violatedConstraint = rightConstraint;
        break;
      }

      // Update constraints
      if (abs(shift[0]) > 1 || abs(shift[1]) > 1) {
        newLeftConstraint[0] =
            shift[0] + (shift[1] < 0 || (shift[1] == 0 && shift[0] < 0) ? 1 : -1);
        newLeftConstraint[1] =
            shift[1] + (shift[0] > 0 || (shift[0] == 0 && shift[1] < 0) ? 1 : -1);

        if (cross(newLeftConstraint, leftConstraint) >= 0)
          leftConstraint = newLeftConstraint;

        newRightConstraint[0] =
            shift[0] + (shift[1] > 0 || (shift[1] == 0 && shift[0] < 0) ? 1 : -1);

        newRightConstraint[1] =
            shift[1] + (shift[0] < 0 || (shift[0] == 0 && shift[1] < 0) ? 1 : -1);

        if (cross(newRightConstraint, rightConstraint) <= 0)
          rightConstraint = newRightConstraint;
      }

      // Imposing strict constraint for ambiguous turnings, to ensure polygons'
      // weak simpleness.
      // Has to be defined *outside* abs checks.
      if (path[nextCorners[j]].getAmbiguous()) {
        if (path[nextCorners[j]].getAmbiguous() == RawBorderPoint::left)
          rightConstraint = shift;
        else
          leftConstraint = shift;
      }

      j = nextCorners[j];
    }

    // At this point, constraints are violated by the next corner.
    // Then, search for the last k between j and corners[j] not violating them.
    temp =  jNextPoint - jPoint;
    tempD[0]=temp[0];
    tempD[1]=temp[1];
    tempD = tempD.norm();
    direction[0]=round(tempD[0]);
    direction[1]=round(tempD[1]) ;

    k = (j + cross(jPoint - iPoint, violatedConstraint) / cross(violatedConstraint, direction)) % n;

  foundK:

    kVector[i] = k;
  }

  return kVector;
}

//--------------------------------------------------------------------------

// Now find the effective intervals inside which we can define possible
// arcs approximating the given raw border:
//    for every a in [i,res[i]], the arc connecting border[i] and
//    border[a] will be a possible one.
inline std::unique_ptr<int[]> calculateForwardArcs(RawBorder &border, bool ambiguitiesCheck) 
{
  int const n = (int)border.size();

  std::unique_ptr<int[]> nextCorners;
  std::unique_ptr<int[]> k = furthestKs(border, nextCorners);
  std::unique_ptr<int[]> K(new int[n]);
  std::unique_ptr<int[]> res(new int[n]);

  // find K[i]= min {k[j]}, j=i..n-1.
  for (int i = 0; i < n; ++i) 
  {
    int j;
    for (j = i, K[i] = k[i]; isCircular(i, j, K[i]); j = (j + 1) % n)
      if (isCircular(j, k[j], K[i])) K[i] = k[j];
  }

  // Finally, we perform the following clean-up operations:
  //  first, extremities of [i,K[i]] are clipped away, to obtain a
  //    smoother optimal polygon (and deal with cases like the unitary
  //    square);
  //  second, arcs of the kind [i,j] with j<i, become [i,n].

  for (int i = n - 1, j = 0; j < n; i = j, ++j) 
  {
    res[j] = K[i] < j ? (K[i] == 0 ? n - 1 : n) : K[i] - 1;
  }

  // Amibiguities check for vertex and edge superpositions. Prevent problems in
  // the forecoming
  // straight-skeleton thinning process.

  if (ambiguitiesCheck) 
  {
    for (int i = 1; nextCorners[i] > 0; i = nextCorners[i]) 
    {
      if (border[i].getAmbiguous() == RawBorderPoint::right) 
      {
        // Check vertices from i (excluded) to res[res[i]]; if in it there
        // exists vertex k so that pos(k)==pos(i)...
        // This prevents the existence of 0 degree angles in the optimal
        // polygon.

        int rrPlus1 = (res[res[i] % n] + 1) % n;

        // remember that isCircular(a,a,b) == 1 ...
        for (int j = nextCorners[i]; isCircular(i, j, rrPlus1) && j != i; j = nextCorners[j]) 
        {
          if (border[j].getAmbiguous() && (border[j].pos() == border[i].pos())) 
          {
            res[res[i] % n] = j - 1;
            assert((res[i] % n) != j - 1);

            // Further, ensure res is increasing
            for (int k = res[i] % n; res[k] >= j - 1 && k >= 0; --k) 
            {
              res[k] = j - 1;
              assert(k != j - 1);
            }

            break;
          }
        }
      }
    }
  }

  return res;
}

//--------------------------------------------------------------------------

// Let sum[i] and sum2[i] be respectively the sums of vertex coordinates
// from 0 to i, and the sums of their squares; sumsMix contain sums of
// xy terms.
inline void calculateSums(RawBorder &path) 
{
  unsigned int i, n = path.size();
  synfig::PointInt temp;
  path.sums()    = new synfig::Point[n + 1];
  path.sums2()   = new synfig::Point[n + 1];
  path.sumsMix() = new double[n + 1];

  path.sums()[0][0] = path.sums()[0][1] = path.sums2()[0][0] = path.sums2()[0][1] = 0;
  for (i = 1; i < path.size(); ++i) 
  {
    temp = path[i].pos() - path[0].pos();
    synfig::Point currentRelativePos(temp[0],temp[1]);

    path.sums()[i] = path.sums()[i - 1] + currentRelativePos;

    path.sums2()[i][0] = path.sums2()[i - 1][0] + currentRelativePos[0] * currentRelativePos[0];
    path.sums2()[i][1] = path.sums2()[i - 1][1] + currentRelativePos[1] * currentRelativePos[1];

    path.sumsMix()[i] = path.sumsMix()[i - 1] + currentRelativePos[0] * currentRelativePos[1];
  }

  // path[n] is virtually intended as path[0], but we prefer to introduce
  // it in the optimality algorithm's count
  path.sums()[n][0] = path.sums()[n][1] = path.sums2()[n][0] = path.sums2()[n][1] = 0;
}

//--------------------------------------------------------------------------

// Let a,b the index-extremities of an arc of this path.
// Then return its penalty.
inline double penalty(RawBorder &path, int a, int b) 
{
  synfig::PointInt p;

  int n = b - a + 1;
  p = path[b == path.size() ? 0 : b].pos() - path[a].pos();

  synfig::Point v(-p[1],p[0]);//convert(rotate90(p))
  synfig::Point sum   = path.sums()[b] - path.sums()[a];
  synfig::Point sum2  = path.sums2()[b] - path.sums2()[a];
  double sumMix = path.sumsMix()[b] - path.sumsMix()[a];

  double F1 = sum2[0] - 2 * sum[0] * path[a].x() + n * path[a].x() * path[a].x();
  double F2 = sum2[1] - 2 * sum[1] * path[a].y() + n * path[a].y() * path[a].y();
  double F3 = sumMix - sum[0] * path[a].y() - sum[1] * path[a].x() +
              n * path[a].x() * path[a].y();
  return sqrt((v[1] * v[1] * F1 + v[0] * v[0] * F2 - 2 * v[0] * v[1] * F3) / n);
}

//--------------------------------------------------------------------------

// NOTA: Il seguente algoritmo di riduzione assicura la semplicita' (debole) dei
// poligoni prodotti.
//

inline void reduceBorder(RawBorder &border, Contour &res, bool ambiguitiesCheck) 
{
  int n = border.size();
  int minPenaltyNext;
  std::unique_ptr<int[]> minPenaltyNextArray(new int[n]);

  // Calculate preliminary infos
  std::unique_ptr<int[]> longestArcFrom = calculateForwardArcs(border, ambiguitiesCheck);
  calculateSums(border);

  std::unique_ptr<double[]> penaltyToEnd(new double[n + 1]);

  // EXPLANATION:
  // The fastest way to extract the optimal reduced border is based on the
  // weakly monotonic property of longestArc[].
  // The minimal number of its vertices 'm' is easily found by
  // traversing the path with the longest step allowed. Let b[] be that
  // succession; then, given res[i], it has to be reached by a vertex in
  // the interval: {a[i-1], .. , b[i-1]}, where longestArc[a[i-1]]=a[i],
  // longestArc[a[i-1]-1]<a[i], and a[m]=n.

  // Calculate m
  int m = 0;
  for (int i = 0; i < n; i = longestArcFrom[i]) ++m;

  // Calculate b[]
  std::unique_ptr<int[]> b(new int[m + 1]);
  b[m] = n;
  for (int i = 0, j = 0; j < m; i = longestArcFrom[i], ++j) b[j] = i;

  // NOTE: a[] need not be completely found - we just remember the
  // a=a[j+1] currently needed.

  // Now, build the optimal polygon
  for (int j = m - 1, a = n; j >= 0; --j) 
  {
    int k;
    for (k = b[j]; k >= 0 && longestArcFrom[k] >= a; --k) 
    {
      penaltyToEnd[k] = infinity;
      for (int i = a; i <= longestArcFrom[k]; ++i) 
      {
        double newPenalty = penaltyToEnd[i] + penalty(border, k, i);
        
        if (newPenalty < penaltyToEnd[k]) 
          penaltyToEnd[k] = newPenalty;
        
        minPenaltyNext = i;
      }
      minPenaltyNextArray[k] = minPenaltyNext;
    }
    a = k + 1;
  }

  // Finally, read off the optimal polygon

  res.resize(m);
  for (int i = 0, j = 0; i < n; i = minPenaltyNextArray[i], ++j) {
    res[j] = ContourNode(border[i].x(), border[i].y());

    // Ambiguities are still remembered in the output polygon.
    if (border[i].getAmbiguous() == RawBorderPoint::left)
      res[j].setAttribute(ContourNode::AMBIGUOUS_LEFT);
    if (border[i].getAmbiguous() == RawBorderPoint::right)
      res[j].setAttribute(ContourNode::AMBIGUOUS_RIGHT);
  }

  delete[] border.sums();
  delete[] border.sums2();
  delete[] border.sumsMix();
}

//--------------------------------------------------------------------------

// Reduction caller and list copier.
inline void reduceBorders(BorderList &borders, Contours &result,bool ambiguitiesCheck) 
{
  unsigned int i, j;

  // Initialize output container
  result.resize(borders.size());

  // Copy results
  for (i = 0; i < borders.size(); ++i) 
  {
    result[i].resize(borders[i].size());
    for (j = 0; j < borders[i].size(); ++j) 
    {
      reduceBorder(*borders[i][j], result[i][j], ambiguitiesCheck);
      delete borders[i][j];
    }
  }
}

/* === E N T R Y P O I N T ================================================= */
//===========================
//    Polygonization Main
//===========================

//Extracts a polygonal, minimal yet faithful representation of image contours

void studio::polygonize(const etl::handle<synfig::Layer_Bitmap> &ras, Contours &polygons, VectorizerCoreGlobals &g)
{
	
	BorderList *borders;
	
	borders = extractBorders(ras, g.currConfig->m_threshold, g.currConfig->m_despeckling);
	
	reduceBorders(*borders, polygons, g.currConfig->m_maxThickness > 0.0);

	delete borders;
}


