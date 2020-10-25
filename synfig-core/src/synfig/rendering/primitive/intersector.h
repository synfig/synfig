/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/primitive/intersector.h
**	\brief Intersector Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
**	......... ... 2019 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

/// Helper class to check if a point is inside a complex shape.
///
/// This class uses the Winding Number Algorithm to check if
/// a given point is inside a shape. For that, it computes how
/// many intersections a line from the given point to negative
/// X-axis direction.
///
/// To use this class, first define the shape with the methods
/// move_to(), line_to(), conic_to(), cubic_to() and close().
/// Then, call intersect() with, as argument, the point you want
/// to know if it is inside or not the shape.
class Intersector: public etl::shared_object
{
public:
	typedef etl::handle<Intersector> Handle;

private:
	class MonoSegment;
	class CurveArray;
	typedef std::vector<MonoSegment> MonoSegmentList;
	typedef std::vector<CurveArray> CurveArrayList;

	enum IntersectorFlags {
		NotClosed = 0x8000
	};

	enum PrimitiveType {
		TYPE_NONE = 0,
		TYPE_LINE,
		TYPE_CURVE
	};
	
	Rect aabb; //<! the Axis-Aligned Bounding Box (AABB) for the stored shape
	bool invalid_aabb; //<! true iff aabb hasn't been initialized yet
	int flags; //<! bitwise IntersectorFlags
	PrimitiveType previous_primitive_type; //<! last primitive type drawn

	Point cur_pos;
	Point close_pos;

	MonoSegmentList segs;  //<! monotonically increasing list of line segments of the stored shape
	CurveArrayList curves; //<! big array of consecutive curves that compounds the stored shape

public:
	Intersector();
	~Intersector();
	/// Clear the stored shape
	void clear();

	/// Check if it stores a closed shape
	inline bool closed() const
		{ return !(flags & NotClosed) && cur_pos[0] == close_pos[0] && cur_pos[1] == close_pos[1]; }

	/// Move current position to p
	void move_to(const Point &p);
	/// Create a line from current position to p
	void line_to(const Point &p);
	/// Create a conic bezier curve from current position to p with control point p1
	void conic_to(const Point &p, const Point &p1);
	/// Create a conic bezier curve from current position to p with control points p1 and p2
	void cubic_to(const Point &p, const Point &p1, const Point &p2);
	/// Close the shape (from current point to first one)
	void close();
	
	/// Count the number of intersections from p to (-inf,p[y]).
	/// A zero value means the point is outside the shape.
	int	intersect(const Point &p) const;

	/// The Axis-Aligned Bounding Box (AABB) for the stored shape
	Rect get_bounds() const
		{ return invalid_aabb ? Rect::zero() : aabb; }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
