/* === S Y N F I G ========================================================= */
/*!	\file curve_helper.h
**	\brief Curve Helper Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_CURVE_HELPER_H
#define __SYNFIG_CURVE_HELPER_H

/* === H E A D E R S ======================================================= */
#include <ETL/bezier>

#include "rect.h"
#include "real.h"
#include "vector.h"

#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

//line helper functions
inline Real line_point_distsq(const Point &p1, const Point &p2,
										const Point &p, float &t)
{
	Vector v,vt;

	v = p2 - p1;
	vt = p - p1;

	t = v.mag_squared() > 1e-12 ? (vt*v)/v.mag_squared() : 0; //get the projected time value for the current line

	//get distance to line segment with the time value clamped 0-1
	if(t >= 1)	//use p+v
	{
		vt += v; //makes it pp - (p+v)
		t = 1;
	}else if(t > 0)	//use vt-proj
	{
		vt -= v * t; // vt - proj_v(vt)	//must normalize the projection vector to work
	}else
	{
		t = 0;
	}

	//else use p
	return vt.mag_squared();
}


//----- RAY CLASS AND FUNCTIONS --------------
struct Ray
{
	Point	p;
	Vector	v;

	Ray() {}
	Ray(const Point &pin, const Vector &vin):p(pin), v(vin) {}
};

/* This algorithm calculates the INTERSECTION of 2 line segments
	(not the closest point or anything like that, just intersection)
	//parameter values returned are [0,1]
*/
int intersect(const Point &p1, const Vector &v1, float &t1,
				const Point &p2, const Vector &v2, float &t2);

inline bool intersect_line_segments(const Point &a, const Point &b, float &tout,
										const Point &c, const Point &d, float &sout)
{
	Vector v1(b-a), v2(d-c);

	//ok so treat both lines as parametric (so we can find the time values simultaneously)
	float t,s;

	if( intersect(a,v1,t, b,v2,s) && t >= 0 && t <= 1 && s >= 0 && s <= 1 )
	{
		tout = t;
		sout = s;
		return true;
	}

	return false;
}

//Find the closest point on the curve to a point (and return its distance, and time value)
Real find_closest(const etl::bezier<Point> &curve, const Point &point, float step, Real *closest, float *t);

//----------- Rectangle helper functions ---------------

template < typename T >
inline void Bound(synfig::rect<T> &r, const etl::bezier<Point> &b)
{
	r.set_point(b[0][0],b[0][1]);
	r.expand(b[1][0],b[1][1]);
	r.expand(b[2][0],b[2][1]);
	r.expand(b[3][0],b[3][1]);
}

/*template < typename T >
inline bool intersect(const etl::rect<T> &r1, const etl::rect<T> &r2)
{
	return (r1.minx < r2.maxx) &
			(r2.minx < r1.maxx) &
			(r1.miny < r2.maxy) &
			(r2.miny < r1.maxy);
}*/

//----- Convex Hull of a Bezier Curve --------------
struct BezHull
{
	Point	p[4];
	int		size;

	void Bound(const etl::bezier<Point> &b);
};

//Line Intersection
int intersect(const Rect &r1, const Point &p, const Vector &v);
int intersect(const Rect &r1, const Point &p); //inside or to the right
int intersect(const BezHull &bh, const Point &p, const Vector &v);
//int intersect(const etl::bezier<Point> &b, const Point &p, const Vector &v);
int intersect(const etl::bezier<Point> &b, const Point &p); //for use in containment tests for regions

//Curve intersection object
class CIntersect
{
public:
	struct SCurve;
private:
	void recurse_intersect(const SCurve &left, const SCurve &right, int depth = 0);

public:
	//size should be equal
	typedef std::vector< std::pair<float,float > >	intersect_set;
	intersect_set	times;

	int		max_depth;

	CIntersect();

	bool operator()(const etl::bezier<Point> &b1, const etl::bezier<Point> &b2);
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
