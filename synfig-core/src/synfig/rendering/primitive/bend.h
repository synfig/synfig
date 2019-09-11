/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/primitive/bend.h
**	\brief Bend Header
**
**	$Id$
**
**	\legal
**	......... ... 2019 Ivan Mahonin
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_BEND_H
#define __SYNFIG_RENDERING_BEND_H

/* === H E A D E R S ======================================================= */

#include <vector>

#include <ETL/handle>

#include <synfig/vector.h>
#include <synfig/matrix.h>

#include "contour.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class Bend: public etl::shared_object
{
public:
	typedef etl::handle<Bend> Handle;

	enum Mode
	{
		NONE   = 0,
		FLAT   = 1,
		ROUND  = 2,
		CORNER = 3
	};
	
	typedef unsigned int Hints;

	class Point {
	public:
		Vector p;
		Vector t0, t1;
		Vector tn0, tn1;
		Mode mode;
		bool e0, e1;
		Real l;
		Real length;
		Point(): mode(NONE), e0(), e1(), l() { }
	};

	typedef std::vector<Point> PointList;

	PointList points;

	void add(const Vector &p, const Vector &t0, const Vector &t1, Mode mode, bool calc_length, int segments);
	void loop(bool calc_length, int segments);
	void tails();
	
	Real l0() const
		{ return points.empty() ? Real() : points.front().l; }
	Real l1() const
		{ return points.empty() ? Real() : points.back().l; }

	Real length0() const
		{ return points.empty() ? Real() : points.front().length; }
	Real length1() const
		{ return points.empty() ? Real() : points.back().length; }
	
	PointList::const_iterator find_by_l(Real l) const;
	PointList::const_iterator find(Real length) const;
	Real length_by_l(Real length) const;
	Point interpolate(Real length) const;
	
	void bend(Contour &dst, const Contour &src, const Matrix &matrix, int segments) const;
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif

