/* === S Y N F I G ========================================================= */
/*!	\file template.cpp
**	\brief Template File
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/vector.h>
#include "polygonizerclasses.h"

/*
** Insert headers here
*/

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


///////////////////////////////////////////

// ...
// using namespace synfig;
// ...
// etl::handle<Layer_Bitmap> my_layer_bitmap;
// ...
// // lock surface for read
// // in most cases surface will be in different format,
// // but it will be automatically converted to synfig::Surface while locking
// // after that SurfaceResource will contain both original and converted surfaces
// if ( rendering::SurfaceResource::LockRead<Surface> lock(my_layer_bitmap->rendering_surface) ) {
//     // operator 'lock->' returns a pointer to synfig::Surface,
//     // so see methods of synfig::Surface (aka etl::surface<synfig::Color>)
//     int width = lock->get_w();
//     int height = lock->get_h();
//     int pitch = lock->get_pitch(); // offset from one row to next row
    
//     // returns an array of pixels of row #5 (pointer to first pixel in row)
//     // see also synfig::Color, it represents pixel in four float32 values (RGBA)
//     const Color *row5 = (*lock)[5];
//}


///////////////////////////////////////////
class RawBorderPoint {
  PointInt m_position;
  int m_ambiguousTurn;  // used to remember cases of multiple turning directions
  // in a RawBorder extraction.

public:
  RawBorderPoint() : m_ambiguousTurn(0) {}
  RawBorderPoint(int i, int j) : m_position(i,j), m_ambiguousTurn(0) {}

  inline PointInt pos() const { return m_position; }
  inline int x() const { return m_position[0]; }
  inline int y() const { return m_position[1]; }

  enum { left = 1, right = 2 };  // Direction taken at ambiguous turning point
  inline int getAmbiguous() const { return m_ambiguousTurn; }
  inline void setAmbiguous(int direction) { m_ambiguousTurn = direction; }
};

//--------------------------------------------------------------------------

class RawBorder final : public std::vector<RawBorderPoint> {
  int m_xExternal;  // x coordinate of a specific vertex in the outer
  // RawBorder which contains this inner one.

  Point *m_coordinateSums;
  Point *m_coordinateSquareSums;
  double *m_coordinateMixedSums;

public:
  RawBorder() {}
  ~RawBorder() {}

  void setXExternalPixel(int a) { m_xExternal = a; }
  int xExternalPixel() { return m_xExternal; }
  Point *&sums() { return m_coordinateSums; }
  Point *&sums2() { return m_coordinateSquareSums; }
  double *&sumsMix() { return m_coordinateMixedSums; }
};

//--------------------------------------------------------------------------

// Of course we don't want RawBorders to be entirely copied whenever STL
// requires to resize a BorderFamily...
typedef std::vector<RawBorder *> BorderFamily;
typedef std::vector<BorderFamily> BorderList;


/* === E N T R Y P O I N T ================================================= */
//===========================
//    Polygonization Main
//===========================

// Extracts a polygonal, minimal yet faithful representation of image contours
// Contours* polygonize(const TRasterP &ras){
// void polygonize(const TRasterP &ras, Contours &polygons,
//                 VectorizerCoreGlobals &g) {
//   BorderList *borders;

//   borders = extractBorders(ras, g.currConfig->m_threshold,
//                            g.currConfig->m_despeckling);
//   reduceBorders(*borders, polygons, g.currConfig->m_maxThickness > 0.0);
//}


