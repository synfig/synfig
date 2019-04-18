/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/primitive/intersector.cpp
**	\brief Intersector
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007-2008 Chris Moore
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "intersector.h"

#include <synfig/general.h>

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

// super segment
class Intersector::MonoSegment
{
public:
	Rect aabb;
	int ydir;
	std::vector<Point> pointlist;

	explicit MonoSegment(int dir = 0, const Point &p0 = Point(), const Point &p1 = Point())
	{
		aabb.minx = p0[0];
		aabb.maxx = p1[0];
		aabb.miny = p0[1];
		aabb.maxy = p1[1];
		ydir = dir;
	}

	int intersect(const Point &p) const
	{
		if ( pointlist.empty()
		  || p[1] < aabb.miny + real_precision<Real>()
		  || p[1] > aabb.maxy
		  || p[0] < aabb.minx )
			return 0;
		if(p[0] > aabb.maxx) return ydir;

		//assumes that the rect culled away anything that would be beyond the edges
		std::vector<Point>::const_iterator i = pointlist.begin();
		if (ydir > 0) while(p[1] > (*++i)[1]);
		         else while(p[1] < (*++i)[1]);

		//for the loop to break there must have been a slope (straight line would do nothing)
		Vector d = *(i-1) - (*i);
		assert(d[1]);

		Real xi = (*i)[0] + (p[1] - (*i)[1])*d[0]/d[1];
		return (p[0] > xi)*ydir;
	}
};

class Intersector::CurveArray
{
public:
	Rect aabb; //!< not necessarily as effective - can only reject values
	std::vector<Point> pointlist; //!< run length - p0, p1, p2, p3 = p10, p11, p12, p13 = p20 ...
	std::vector<char> degrees;

	explicit CurveArray(const Point &p0 = Point(), const Point &p1 = Point())
		{ aabb.set(p0[0], p0[1], p1[0], p1[1]); }
	
	void reset(const Point &p0 = Point(), const Point &p1 = Point())
	{
		aabb.set(p0[0], p0[1], p1[0], p1[1]);
		pointlist.clear();
		degrees.clear();
	}
	
	int size() const
		{ return degrees.size(); }

	void start(const Point &m)
	{
		reset(m, m);
		pointlist.push_back(m);
	}

	void add_cubic(Point dest, Point p1, Point p2)
	{
		aabb.expand(p1[0], p1[1]);
		aabb.expand(p2[0], p2[1]);
		aabb.expand(dest[0], dest[1]);

		pointlist.push_back(p1);
		pointlist.push_back(p2);
		pointlist.push_back(dest);

		degrees.push_back(3);
	}

	void add_conic(Point dest, Point p1)
	{
		aabb.expand(p1[0], p1[1]);
		aabb.expand(dest[0], dest[1]);

		pointlist.push_back(p1);
		pointlist.push_back(dest);

		degrees.push_back(2);
	}

	static int intersect_conic(const Point &p, const Point *points)
	{
		Real ymin, ymax, xmin, xmax;
		int intersects = 0;

		// sort the overall curve ys - degenerate detection
		ymin = std::min(points[0][1], points[2][1]);
		ymax = std::max(points[0][1], points[2][1]);

		xmin = std::min( std::min(points[0][0], points[1][0]), points[2][0] );
		xmax = std::max( std::max(points[0][0], points[1][0]), points[2][0] );

		// to the left, to the right and out of range y, or completely out of range y
		// or degenerate line max
		if ( (p[0] < xmin)
		  || (p[0] > xmax && (p[1] > ymax || p[1] < ymin))
		  || (p[1] > ymax && p[1] > points[1][1])
		  || (p[1] < ymin && p[1] < points[1][1])
		  || (ymin == ymax && ymax == points[1][1]) )
			return 0;

		// degenerate accept - to the right and crossing the base line
		if (p[0] > xmax)
			return (p[1] <= ymax && p[1] >= ymin);

		// solve for curve = y

		// real roots:
		//   0 roots - 0 intersection
		//   1 root  - get x, and figure out x
		//   2 roots (non-double root) - get 2 xs, and count xs to the left

		//for conic we can assume 1 intersection for monotonic curve
		Real a = points[2][1] - 2*points[1][1] +   points[0][1],
			 b =                2*points[1][1] - 2*points[0][1],
			 c =                                   points[0][1] - p[1];
		Real t1 = -1, t2 = -1;

		if (a == 0) {
			// linear - easier :)
			if (b == 0) return 0; // may not need this check
			t1 = -c/b;            // bt + c = 0 solved
		} else {
			// 2 degree polynomial
			Real b2_4ac = b*b - 4*a*c;

			// if there are double/no roots - no intersections (in real #s that is)
			if (b2_4ac <= 0) return 0;
			b2_4ac = sqrt(b2_4ac);
			t1 = (-b - b2_4ac)/2*a,
			t2 = (-b + b2_4ac)/2*a;
		}

		// calculate number of intersections
		if (t1 >= 0 && t1 <= 1) {
			const Real t = t1;
			const Real invt = 1 - t;

			// find x val and it counts if it's to the left of the point
			const Real xi = invt*invt*points[0][0] + 2*t*invt*points[1][0] + t*t*points[2][0];
			const Real dy_t = 2*a*t + b;
			if (dy_t)
				intersects += (p[0] >= xi) * ( dy_t > 0 ? 1 : -1);
		}

		if (t2 >= 0 && t2 <= 1) {
			const Real t = t2;
			const Real invt = 1 - t;

			// find x val and it counts if it's to the left of the point
			const Real xi = invt*invt*points[0][0] + 2*t*invt*points[1][0] + t*t*points[2][0];
			const Real dy_t = 2*a*t + b;
			if(dy_t)
				intersects += (p[0] >= xi) * ( dy_t > 0 ? 1 : -1);
		}

		return intersects;
	}

	static int quadratic_eqn(Real a, Real b, Real c, Real *t0, Real *t1)
	{
		const Real b2_4ac = b*b - 4*a*c;

		// degenerate reject (can't take sqrt)
		if (b2_4ac < 0)
			return 0;

		const Real sqrtb2_4ac = sqrt(b2_4ac);
		const Real signb = b < 0 ? -1 : 1;
		const Real q = - 0.5 * (b + signb * sqrtb2_4ac);

		*t0 = q/a;
		*t1 = c/q;

		return sqrtb2_4ac == 0 ? 1 : 2;
	}

	static int intersect_cubic(const Point &p, const Point *points)
	{
		const Real INVALIDROOT = -1.0/(real_high_precision<Real>()*real_high_precision<Real>());
		int intersects = 0;

		// sort the overall curve ys and xs - degenerate detection

		// open span for the two end points
		Real ymin = std::min(points[0][1], points[3][1]);
		Real ymax = std::max(points[0][1], points[3][1]);

		// other points etc.
		Real ymin2 = std::min(points[1][1], points[2][1]);
		Real ymax2 = std::max(points[1][1], points[2][1]);

		Real ymintot = std::min(ymin, ymin2);
		Real ymaxtot = std::max(ymax, ymax2);

		// the entire curve control polygon is in this x range
		Real xmin = std::min( std::min(points[0][0], points[1][0]), std::min(points[2][0], points[3][0]) );
		Real xmax = std::max( std::max(points[0][0], points[1][0]), std::max(points[2][0], points[3][0]) );

		// outside all y boundaries (no intersect)
		if (p[1] > ymaxtot || p[1] < ymintot) return 0;

		// left of curve (no intersect)
		if (p[0] < xmin) return 0;

		// right of curve (and outside base range)
		if (p[0] > xmax) {
			if (p[1] > ymax || p[1] < ymin) return 0;
			// degenerate accept - to the right and inside the [ymin,ymax] range (already rejected if out of range)
			const Real n = points[3][1] - points[0][1];
			// extract the sign from the value (we need valid data)
			return n < 0 ? -1 : 1;
		}

		// degenerate horizontal line max -- doesn't happen enough to check for
		if (ymintot == ymaxtot) return 0;

		// calculate roots:
		//   can have 0,1,2, or 3 real roots
		//   if any of them are double then reject the two...

		// coefficients, f_y(t) - y = 0
		Vector a = points[3] - points[2]*3 + points[1]*3 - points[0],
		       b =             points[2]*3 - points[1]*6 + points[0]*3,
		       c =                           points[1]*3 - points[0]*3,
		       d =                                         points[0];
		d[1] -= p[1];
		Real t1 = INVALIDROOT, t2 = INVALIDROOT, t3 = INVALIDROOT;
		Real t, dydt;

		if (a[1] == 0) {
			// only 2nd degree
			if (b[1] == 0) {
				// linear
				if (c[1] == 0) return 0;
				t1 = d[1]/c[1]; // equation devolved into: ct + d = 0 - solve...
			} else {
				// 0 roots = 0 intersections, 1 root = 2 intersections at the same place (0 effective)
				if (quadratic_eqn(a[1], b[1], c[1], &t1, &t2) != 2) return 0;
			}
		} else {
			// cubic - sigh....
			// algorithm courtesy of Numerical Recipes in C (algorithm copied from pg. 184/185)
			Real an = b[1]/a[1],
			     bn = c[1]/a[1],
			     cn = d[1]/a[1];

			// if cn is 0 (or really really close), then we can simplify this...
			if (approximate_equal(cn, Real(0))) {
				t3 = 0;
				// 0 roots = 0 intersections, 1 root = 2 intersections at the same place (0 effective)
				if (quadratic_eqn(a[1], b[1], c[1], &t1, &t2) != 2)
					t1 = t2 = INVALIDROOT;
			} else {
				// otherwise run the normal cubic root equation
				Real Q = (an*an - 3.0*bn) / 9.0;
				Real R = ((2.0*an*an - 9.0*bn)*an + 27.0*cn)/54.0;
				if (R*R < Q*Q*Q) {
					Real theta = acos(R / sqrt(Q*Q*Q));

					t1 = -2.0*sqrt(Q)*cos(theta/3) - an/3.0;
					t2 = -2.0*sqrt(Q)*cos((theta+2*PI)/3.0) - an/3.0;
					t3 = -2.0*sqrt(Q)*cos((theta-2*PI)/3.0) - an/3.0;

					// don't need to reorder,l just need to eliminate double/triple roots
					//if(t3 == t2 && t1 == t2) t2 = t3 = INVALIDROOT;
					if(t3 == t2) t2 = t3 = INVALIDROOT;
					if(t1 == t2) t1 = t2 = INVALIDROOT;
					if(t1 == t3) t1 = t3 = INVALIDROOT;
				} else {
					Real signR = R < 0 ? -1 : 1;
					Real A = - signR * pow(signR*R + sqrt(R*R - Q*Q*Q),1/3.0);
					Real B = A == 0 ? 0 : Q/A;
					// single real root in this case
					t1 = (A + B) - an/3.0;
				}
			}
		}

		//if (t1 != INVALIDROOT)
		{
			t = t1;
			if (t >= 0 && t < 1) {
				// find x val and it counts if it's to the left of the point
				const Real xi = ((a[0]*t + b[0])*t + c[0])*t + d[0];
				dydt = (3*a[1]*t + 2*b[1])*t + c[1];
				if (dydt && p[0] >= xi)
					intersects += dydt > 0 ? 1 : -1;
			}
		}

		//if(t2 != INVALIDROOT)
		{
			t = t2;
			if (t >= 0 && t < 1) {
				// find x val and it counts if it's to the left of the point
				const Real xi = ((a[0]*t + b[0])*t + c[0])*t + d[0];
				dydt = (3*a[1]*t + 2*b[1])*t + c[1];
				if (dydt && p[0] >= xi)
					intersects += dydt > 0 ? 1 : -1;
			}
		}

		//if(t3 != INVALIDROOT)
		{
			t = t3;
			if(t >= 0 && t < 1) {
				// find x val and it counts if it's to the left of the point
				const Real xi = ((a[0]*t + b[0])*t + c[0])*t + d[0];
				dydt = (3*a[1]*t + 2*b[1])*t + c[1];
				if (dydt && p[0] >= xi)
					intersects += dydt > 0 ? 1 : -1;
			}
		}

		return intersects;
	}

	int intersect(const Point &p) const
	{
		if (p[1] < aabb.miny || p[1] > aabb.maxy || p[0] < aabb.minx) return 0;
		int intersects = 0;
		std::vector<Point>::const_iterator ip = pointlist.begin();
		for(int i = 0, count = (int)degrees.size(); i < count; ip += degrees[i++]) {
			switch(degrees[i]) {
				case 2:
					intersects += intersect_conic(p, &*ip);
					break;
				case 3:
					intersects += intersect_cubic(p, &*ip);
					break;
				default:
					warning("Invalid degree (%d) inserted into the list (index: %d)\n", degrees[i], i);
					return 0;
			}
		}
		return intersects;
	}
};



Intersector::Intersector()
	{ clear(); }

void
Intersector::clear()
{
	segs.clear();
	curves.clear();
	flags = 0;
	cur_pos = close_pos = Point();
	prim = TYPE_NONE;
	initaabb = true;
}


void
Intersector::move_to(const Point &p)
{
	close();
	close_pos = cur_pos = p;
	if (initaabb) {
		aabb.set_point(p[0], p[1]);
		initaabb = false;
	} else aabb.expand(p[0], p[1]);
	prim = TYPE_NONE;
}

void
Intersector::line_to(const Point &p)
{
	int dir = p[1] > cur_pos[1] ?  1
	        : p[1] < cur_pos[1] ? -1 : 0;

	// check for context (if not line start a new segment)
	if (prim != TYPE_LINE || (dir && segs.back().ydir != dir)) {
		// if we're not in line mode (covers 0 set case), or if directions are different (not valid for 0 direction)
		segs.push_back(MonoSegment(dir, cur_pos, cur_pos));
		segs.back().pointlist.push_back(cur_pos);
	}
	// add to the last segment, because it works
	segs.back().pointlist.push_back(p);
	segs.back().aabb.expand(p[0], p[1]);

	cur_pos = p;
	aabb.expand(cur_pos[0], cur_pos[1]); // expand the entire thing's bounding box
	flags |= NotClosed;
	prim = TYPE_LINE;
}

void
Intersector::conic_to(const Point &p, const Point &p1)
{
	// if we're not already a curve start one
	if (prim != TYPE_CURVE) {
		curves.push_back(CurveArray());
		curves.back().start(cur_pos);
	}
	curves.back().add_conic(p, p1);

	cur_pos = p;
	aabb.expand(p[0], p[1]);
	aabb.expand(p1[0], p1[1]);
	flags |= NotClosed;
	prim = TYPE_CURVE;
}

void
Intersector::cubic_to(const Point &p, const Point &p1, const Point &p2)
{
	// if we're not already a curve start one
	if (prim != TYPE_CURVE) {
		curves.push_back(CurveArray());
		curves.back().start(cur_pos);
	}
	curves.back().add_cubic(p, p1, p2);

	cur_pos = p;
	aabb.expand(p[0], p[1]);
	aabb.expand(p1[0], p1[1]);
	aabb.expand(p2[0], p2[1]);
	flags |= NotClosed;
	prim = TYPE_CURVE;
}

void
Intersector::close()
{
	if (flags & NotClosed) {
		if(cur_pos[0] != close_pos[0] || cur_pos[1] != close_pos[1])
			line_to(close_pos);
		flags &= ~NotClosed;
	}
}

// assumes the line to count the intersections with is (-1,0)
int
Intersector::intersect(const Point &p) const
{
	int intersects = 0;
	for(MonoSegmentList::const_iterator i = segs.begin(); i != segs.end(); ++i)
		intersects += i->intersect(p);
	for(CurveArrayList::const_iterator i = curves.begin(); i != curves.end(); ++i)
		intersects += i->intersect(p);
	return intersects;
}

/* === E N T R Y P O I N T ================================================= */
