/* === S Y N F I G ========================================================= */
/*!	\file curve_helper.cpp
**	\brief Curve Helper File
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

#include "curve_helper.h"

#include <algorithm>
#include <vector>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */
#define ERR	1e-11
const Real ERROR = 1e-11;

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Real synfig::find_closest(const etl::bezier<Point> &curve, const Point &point,
				float step, Real *dout, float *tout)
{
#if 0
	float time(curve.find_closest(point,4));
	Real dist((curve(time)-point).mag());
	if(dout) *dout=dist;
	if(tout) *tout=time;
	return time;
#else
	Real d,closest = 1.0e50;
	float t,time,closestt = -1;
	Vector p0,p1,end;

	if(dout && *dout > 0)
		closest = *dout;

	p0 = curve[0];
	end = curve[3];

	for(t = step; t < 1; t+=step, p0=p1)
	{
		p1 = curve(t);
		d = line_point_distsq(p0,p1,point,time);

		if(d<closest)
		{
			closest=d;
			closestt = t-step + time*step;//t+(time-1)*step; //time between [t-step,t]
		}
	}

	d = line_point_distsq(p0,end,point,time);
	if(d<closest)
	{
		closest = d;
		closestt= t-step + time*(1-t+step); //time between [t-step,1.0]
	}

	//set the time value if we found a closer point
	if(closestt >=0)
	{
		if(tout) *tout = closestt;
	}

	return closest;
#endif
}

// Line and BezHull Definitions
void BezHull::Bound(const etl::bezier<Point> &b)
{
	#if 1

	//with a starting vertex, find the only vertex that has all other vertices on its right
	int i,j;
	int first,cur,last;

	float d,ds;

	Vector n,vi;
	Vector::value_type	deqn;

	//get left most vertex
	d = b[0][0];
	first = 0;
	for(i = 1; i < 4; ++i)
	{
		if(b[i][0] < d)
		{
			d = b[i][0];
			first = i;
		}
	}
	cur = last = first;
	size = 0;

	//find the farthest point with all points on right
	ds = 0;
	do //should reassign cur so it won't break on first step
	{
		for(i = 0; i < 4; ++i)
		{
			if(i == cur || i == last) continue;

			//rotate vector to right to make normal
			vi = -(b[i] - b[cur]).perp();
			d = vi.mag_squared();

			//we want only the farthest (solves the case with many points on a line)
			if(d > ds)
			{
				ds = d;
				deqn = n*b[cur];
				for(j = 0; j < 4; ++j)
				{
					d = n*b[i] - deqn;
					if(d < 0) break; //we're on left, nope!
				}

				//everyone is on right... yay! :)
				if(d >= 0)
				{
					//advance point and add last one into hull
					p[size++] = p[last];
					last = cur;
					cur = i;
				}
			}
		}
	}while(cur != first);

	#else

	//will work but does not keep winding order

	//convex hull alg.
	//build set of line segs which have no points on other side...
	//start with initial normal segments

	//start with single triangle
	p[0] = b[0];
	p[1] = b[1];
	p[2] = b[2];
	p[3] = b[3];

	//initial reject (if point is inside triangle don't care)
	{
		Vector v1,v2,vp;

		v1 = p[1]-p[0];
		v2 = p[2]-p[0];

		vp = p[3]-p[0];

		float 	s = (vp*v1) / (v1*v1),
				t = (vp*v2) / (v2*v2);

		//if we're inside the triangle we don't this sissy point
		if( s >= 0 && s <= 1 && t >= 0 && t <= 1 )
		{
			size = 3;
			return;
		}
	}

	//expand triangle based on info...
	bool line;
	int index,i,j;
	float ds,d;

	//distance from point to vertices
	line = false;
	index = 0;
	ds = (p[0]-b[3]).mag_squared();
	for(i = 1; i < 3; ++i)
	{
		d = (p[3]-p[i]).mag_squared();
		if(d < ds)
		{
			index = i;
			ds = d;
		}
	}

	//distance to line
	float t;
	j = 2;
	for(i = 0; i < 3; j = i++)
	{
		d = line_point_distsq(p[j],p[i],b[4],t);
		if(d < ds)
		{
			index = j;
			ds = d;
			line = true;
		}
	}

	//We don't need no stinkin extra vertex, just replace
	if(!line)
	{
		p[index] = p[3];
		size = 3;
	}else
	{
		//must expand volume to work with point...
		//	after the index then

		/* Pattern:
			0 - push 1,2 -> 2,3
			1 - push 2 -> 3
			2 - none
		*/
		for(i = 3; i > index+1; --i)
		{
			p[i] = p[i-1];
		}

		p[index] = b[3]; //recopy b3
		size = 4;
	}

	#endif
}

//Line Intersection
int
synfig::intersect(const Point &p1, const Vector &v1, float &t1,
					const Point &p2, const Vector &v2, float &t2)
{
	/* Parametric intersection:
		l1 = p1 + tv1, l2 = p2 + sv2

		0 = p1+tv1-(p2+sv2)
		group parameters: sv2 - tv1 = p1-p2

		^ = transpose
		invert matrix (on condition det != 0):
		A[t s]^ = [p1-p2]^

		A = [-v1 v2]

		det = v1y.v2x - v1x.v2y

		if non 0 then A^-1 = invdet * | v2y -v2x |
									  | v1y -v1x |

		[t s]^ = A^-1 [p1-p2]^
	*/

	Vector::value_type det = v1[1]*v2[0] - v1[0]*v2[1];

	//is determinant valid?
	if(det > ERR || det < -ERR)
	{
		Vector p_p = p1-p2;

		det = 1/det;

		t1 = det*(v2[1]*p_p[0] - v2[0]*p_p[1]);
		t2 = det*(v1[1]*p_p[0] - v1[0]*p_p[1]);

		return 1;
	}

	return 0;
}

//Returns the true or false intersection of a rectangle and a line
int intersect(const Rect &r, const Point &p, const Vector &v)
{
	float t[4] = {0};

	/*get horizontal intersections and then vertical intersections
		and intersect them

		Vertical planes - n = (1,0)
		Horizontal planes - n = (0,1)

		so if we are solving for ray with implicit line
	*/

	//solve horizontal
	if(v[0] > ERR || v[0] < -ERR)
	{
		//solve for t0, t1
		t[0] = (r.minx - p[0])/v[0];
		t[1] = (r.maxx - p[0])/v[0];
	}else
	{
		return (int)(p[1] >= r.miny && p[1] <= r.maxy);
	}

	//solve vertical
	if(v[1] > ERR || v[1] < -ERR)
	{
		//solve for t0, t1
		t[2] = (r.miny - p[1])/v[1];
		t[3] = (r.maxy - p[1])/v[1];
	}else
	{
		return (int)(p[0] >= r.minx && p[0] <= r.maxx);
	}

	return (int)(t[0] <= t[3] && t[1] >= t[2]);
}

int synfig::intersect(const Rect &r, const Point &p)
{
	return (p[1] < r.maxy && p[1] > r.miny) && p[0] > r.minx;
}

//returns 0 or 1 for true or false number of intersections of a ray with a bezier convex hull
int intersect(const BezHull &bh, const Point &p, const Vector &v)
{
	float mint = 0, maxt = 1e20;

	//polygon clipping
	Vector n;
	Vector::value_type	nv;

	Point last = bh.p[3];
	for(int i = 0; i < bh.size; ++i)
	{
		n = (bh.p[i] - last).perp(); //rotate 90 deg.

		/*
			since rotated left
			if n.v 	< 0 - going in
					> 0 - going out
					= 0 - parallel
		*/
		nv = n*v;

		//going OUT
		if(nv > ERR)
		{
			maxt = min(maxt,(float)((n*(p-last))/nv));
		}else
		if( nv < -ERR) //going IN
		{
			mint = max(mint,(float)((n*(p-last))/nv));
		}else
		{
			if( n*(p-last) > 0 ) //outside entirely
			{
				return 0;
			}
		}

		last = bh.p[i];
	}

	return 0;
}

int Clip(const Rect &r, const Point &p1, const Point &p2, Point *op1, Point *op2)
{
	float t1=0,t2=1;
	Vector v=p2-p1;

	/*get horizontal intersections and then vertical intersections
		and intersect them

		Vertical planes - n = (1,0)
		Horizontal planes - n = (0,1)

		so if we are solving for ray with implicit line
	*/

	//solve horizontal
	if(v[0] > ERR || v[0] < -ERR)
	{
		//solve for t0, t1
		float 	tt1 = (r.minx - p1[0])/v[0],
				tt2 = (r.maxx - p1[0])/v[0];

		//line in positive direction (normal comparisons
		if(tt1 < tt2)
		{
			t1 = max(t1,tt1);
			t2 = min(t2,tt2);
		}else
		{
			t1 = max(t1,tt2);
			t2 = min(t2,tt1);
		}
	}else
	{
		if(p1[1] < r.miny || p1[1] > r.maxy)
			return 0;
	}

	//solve vertical
	if(v[1] > ERR || v[1] < -ERR)
	{
		//solve for t0, t1
		float 	tt1 = (r.miny - p1[1])/v[1],
				tt2 = (r.maxy - p1[1])/v[1];

		//line in positive direction (normal comparisons
		if(tt1 < tt2)
		{
			t1 = max(t1,tt1);
			t2 = min(t2,tt2);
		}else
		{
			t1 = max(t1,tt2);
			t2 = min(t2,tt1);
		}
	}else
	{
		if(p1[0] < r.minx || p1[0] > r.maxx)
			return 0;
	}

	if(op1) *op1 = p1 + v*t1;
	if(op2) *op2 = p1 + v*t2;

	return 1;
}

static void clean_bez(const bezier<Point> &b, bezier<Point> &out)
{
	bezier<Point> temp;

	temp = b;
	temp.set_r(0);
	temp.set_s(1);

	if(b.get_r() != 0)
		temp.subdivide(0,&temp,b.get_r());

	if(b.get_s() != 1)
		temp.subdivide(&temp,0,b.get_s());

	out = temp;
}

// CIntersect Definitions

CIntersect::CIntersect()
	: max_depth(10) //depth of 10 means timevalue parameters will have an approx. error bound of 2^-10
{
}

struct CIntersect::SCurve
{
	bezier<Point> 	b;		//the current subdivided curve
	float rt,st;
	//float 			mid,	//the midpoint time value on this section of the subdivided curve
	//				scale;	//the current delta in time values this curve would be on original curve

	float 	mag;			//approximate sum of magnitudes of each edge of control polygon
	Rect	aabb;			//Axis Aligned Bounding Box for quick (albeit less accurate) collision

	SCurve(): b(), rt(), st(), mag() {}

	SCurve(const bezier<Point> &c,float rin, float sin)
	:b(c),rt(rin),st(sin),mag(1)
	{
		Bound(aabb,b);
	}

	void Split(SCurve &l, SCurve &r) const
	{
		b.subdivide(&l.b,&r.b);

		l.rt = rt;
		r.st = st;
		l.st = r.rt = (rt+st)/2;

		Bound(l.aabb,l.b);
		Bound(r.aabb,r.b);
	}
};

//Curve to the left of point test
static int recurse_intersect(const CIntersect::SCurve &b, const Point &p1, int depthleft = 10)
{
	//reject when the line does not intersect the bounding box
	if(!intersect(b.aabb,p1)) return 0;

	//accept curves (and perform super detailed check for intersections)
	//if the values are below tolerance

	//NOTE FOR BETTERING OF ALGORITHM: SHOULD ALSO/IN-PLACE-OF CHECK MAGNITUDE OF EDGES (or approximate)
	if(depthleft <= 0)
	{
		//NOTE FOR IMPROVEMENT: Polish roots based on original curve
		//						(may be too expensive to be effective)
		int turn = 0;

		for(int i = 0; i < 3; ++i)
		{
			//intersect line segments

			//solve for the y_value
			Vector v = b.b[i+1] - b.b[i];

			if(v[1] > ERROR || v[1] < -ERROR)
			{
				Real xi = (p1[1] - b.b[i][1])/v[1];

				//and add in the turn (up or down) if it's valid
				if(xi < p1[0]) turn += (v[1] > 0) ? 1 : -1;
			}
		}

		return turn;
	}

	//subdivide the curve and continue
	CIntersect::SCurve l1,r1;
	b.Split(l1,r1);	//subdivide left

	//test each subdivision against the point
	return recurse_intersect(l1,p1) + recurse_intersect(r1,p1);
}

int intersect(const bezier<Point> &b, const Point &p)
{
	CIntersect::SCurve	sb;
	clean_bez(b,sb.b);

	sb.rt = 0; sb.st = 1;
	sb.mag = 1; Bound(sb.aabb,sb.b);

	return recurse_intersect(sb,p);
}

//Curve curve intersection
void CIntersect::recurse_intersect(const SCurve &left, const SCurve &right, int depth)
{
	//reject curves that do not overlap with bounding boxes
	if(!intersect(left.aabb,right.aabb)) return;

	//accept curves (and perform super detailed check for intersections)
	//if the values are below tolerance

	//NOTE FOR BETTERING OF ALGORITHM: SHOULD ALSO/IN-PLACE-OF CHECK MAGNITUDE OF EDGES (or approximate)
	if(depth >= max_depth)
	{
		//NOTE FOR IMPROVEMENT: Polish roots based on original curve with the Jacobian
		//						(may be too expensive to be effective)

		//perform root approximation
		//collide line segments

		float t,s;

		for(int i = 0; i < 3; ++i)
		{
			for(int j = 0; j < 3; ++j)
			{
				//intersect line segments
				if(intersect_line_segments(left.b[i],left.b[i+1],t,right.b[j],right.b[j+1],s))
				{
					//We got one Jimmy
					times.push_back(intersect_set::value_type(t,s));
				}
			}
		}

		return;
	}

	//NOTE FOR IMPROVEMENT: only subdivide one curve and choose the one that has
	//						the highest approximated length
	//fast approximation to curve length may be hard (accurate would
	// involve 3 square roots), could sum the squares which would be
	// quick but inaccurate

	SCurve l1,r1,l2,r2;
	left.Split(l1,r1);	//subdivide left
	right.Split(l2,r2); //subdivide right

	//Test each candidate against each other
	recurse_intersect(l1,l2);
	recurse_intersect(l1,r2);
	recurse_intersect(r1,l2);
	recurse_intersect(r1,r2);
}



bool CIntersect::operator()(const etl::bezier<Point> &c1, const etl::bezier<Point> &c2)
{
	times.clear();

	//need to subdivide and check recursive bounding regions against each other
	//so track a list of dirty curves and compare compare compare


	//temporary curves for subdivision
	CIntersect			intersector;
	CIntersect::SCurve	left,right;

	//Make sure the parameters are normalized (so we don't compare unwanted parts of the curves,
	//	and don't miss any for that matter)

	//left curve
	//Compile information about curve
	clean_bez(c1,left.b);
	left.rt = 0; left.st = 1;
	Bound(left.aabb, left.b);

	//right curve
	//Compile information about right curve
	clean_bez(c2,right.b);
	right.rt = 0; right.st = 1;
	Bound(right.aabb, right.b);

	//Perform Curve intersection
	intersector.recurse_intersect(left,right);

	//Get information about roots (yay! :P)
	return times.size() != 0;
}

//point inside curve - return +/- hit up or down edge
int intersect_scurve(const CIntersect::SCurve &b, const Point &p)
{
	//initial reject/approve etc.

	/*
			*-----------*---------
			|			|
			|			|
			|			|
			|	  1		|    2
			|			|
			|			|
			|			|
			|			|
			*-----------*--------
		1,2 are only regions not rejected
	*/
	if(p[0] < b.aabb.minx || p[1] < b.aabb.miny || p[1] > b.aabb.maxy)
		return 0;

	//approve only if to the right of rect around 2 end points
	{
		Rect 	r;
		r.set_point(b.b[0][0],b.b[0][1]);
		r.expand(b.b[3][0],b.b[3][1]);

		if(p[0] >= r.maxx && p[1] <= r.maxy && p[1] >= r.miny)
		{
			float df = b.b[3][1] - b.b[0][1];

			return df >= 0 ? 1 : -1;
		}
	}

	//subdivide and check again!
	CIntersect::SCurve	l,r;
	b.Split(l,r);
	return	intersect_scurve(l,p) + intersect_scurve(r,p);
}

int synfig::intersect(const bezier<Point> &b, const Point &p)
{
	CIntersect::SCurve	c(b,0,1);

	return intersect_scurve(c,p);
}
