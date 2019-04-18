/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/primitive/intersector.h
**	\brief Intersector Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
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

#ifndef __SYNFIG_RENDERING_INTERSECTOR_H
#define __SYNFIG_RENDERING_INTERSECTOR_H

/* === H E A D E R S ======================================================= */

#include <vector>

#include <ETL/handle>

#include <synfig/vector.h>
#include <synfig/rect.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class Intersector: public etl::shared_object
{
public:
	typedef etl::handle<Intersector> Handle;

	class MonoSegment;
	class CurveArray;
	typedef std::vector<MonoSegment> MonoSegmentList;
	typedef std::vector<CurveArray> CurveArrayList;

private:
	enum IntersectorFlags {
		NotClosed = 0x8000
	};

	enum PrimitiveType {
		TYPE_NONE = 0,
		TYPE_LINE,
		TYPE_CURVE
	};
	
	Rect aabb;
	bool initaabb; //<! true iff aabb hasn't been initialized yet
	int flags;
	int prim;

	Point cur_pos;
	Point close_pos;

	MonoSegmentList segs;  //<! monotonically increasing
	CurveArrayList curves; //<! big array of consecutive curves

public:
	Intersector();
	void clear();

	inline bool closed() const
		{ return !(flags & NotClosed) && cur_pos[0] == close_pos[0] && cur_pos[1] == close_pos[1]; }

	void move_to(const Point &p);
	void line_to(const Point &p);
	void conic_to(const Point &p, const Point &p1);
	void cubic_to(const Point &p, const Point &p1, const Point &p2);
	void close();
	
	int	intersect(const Point &p) const;

	Rect get_bounds() const
		{ return initaabb ? Rect::zero() : aabb; }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
