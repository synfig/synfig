/* === S Y N F I G ========================================================= */
/*!	\file layer_shape.cpp
**	\brief Implementation of the "Shape" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
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

#include "layer_shape.h"
#include "string.h"
#include "time.h"
#include "context.h"
#include "paramdesc.h"
#include "renddesc.h"
#include "surface.h"
#include "value.h"
#include "valuenode.h"
#include "float.h"
#include "blur.h"
#include "cairo_renddesc.h"


#include "curve_helper.h"

#include <vector>

#include <deque>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Shape);
SYNFIG_LAYER_SET_NAME(Layer_Shape,"shape");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Shape,N_("Shape"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Shape,N_("Internal"));
SYNFIG_LAYER_SET_VERSION(Layer_Shape,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Layer_Shape,"$Id$");

#define EPSILON	1e-12

template < class T >
inline bool IsZero(const T &n)
{
	return (n < EPSILON) && (n > -EPSILON);
}

/* === C L A S S E S ======================================================= */

//Assumes 64 byte aligned structures if at all
struct Primitive
{
	int		operation;
	int		number;

	//Point	data[0];

	enum Operations
	{
		NONE = -1,
		MOVE_TO = 0,		//(x,y)+ 				after first point treated as line_to
		CLOSE,				//						NOT RUNLENGTH enabled
		LINE_TO,			//(x,y)+				continuous func
		CONIC_TO,			//(x1,y1,x,y)+			"   "
		CONIC_TO_SMOOTH,	//(x,y)+				"   "
		CUBIC_TO,			//(x1,y1,x2,y2,x,y)+	"   "
		CUBIC_TO_SMOOTH,	//(x2,y2,x,y)+			"   "
		END
	};
};

//******** CURVE FUNCTIONS *****************
const int	MAX_SUBDIVISION_SIZE = 64;
const int	MIN_SUBDIVISION_DRAW_LEVELS = 4;

static void Subd_Conic_Stack(Point *arc)
{
	/*

	b0
	*		0+1 a
	b1 b	*		1+2*1+2 a
	*		1+2	b	*
	b2 		*
	*

	0.1.2 ->	0.1 2 3.4

	*/

	Real a,b;


	arc[4][0] = arc[2][0];
	b = arc[1][0];

	a = arc[1][0] = (arc[0][0] + b)/2;
	b = arc[3][0] = (arc[4][0] + b)/2;
	arc[2][0] = (a + b)/2;


	arc[4][1] = arc[2][1];
	b = arc[1][1];

	a = arc[1][1] = (arc[0][1] + b)/2;
	b = arc[3][1] = (arc[4][1] + b)/2;
	arc[2][1] = (a + b)/2;

	/* //USING SIMD

	arc[4] = arc[2];

	arc[3] = (arc[2] + arc[1])/2;
	arc[1] = (arc[0] + arc[1])/2;

	arc[2] = (arc[1] + arc[3])/2;

	*/

}

static void Subd_Cubic_Stack(Point *arc)
{
	Real a,b,c;

	/*

	b0
	*		0+1 a
	b1 b	*		1+2*1+2 a
	*		1+2	b	*			0+3*1+3*2+3
	b2 c	*		1+2*2+2	b	*
	*		2+3	c	*
	b3 		*
	*

	0.1 2.3 ->	0.1 2 3 4 5.6

	*/

	arc[6][0] = arc[3][0];

	b = arc[1][0];
	c = arc[2][0];

	a = arc[1][0] = (arc[0][0] + b)/2;
	b = (b + c)/2;
	c = arc[5][0] = (arc[6][0] + c)/2;

	a = arc[2][0] = (a + b)/2;
	b = arc[4][0] = (b + c)/2;

	arc[3][0] = (a + b)/2;


	arc[6][1] = arc[3][1];

	b = arc[1][1];
	c = arc[2][1];

	a = arc[1][1] = (arc[0][1] + b)/2;
	b = (b + c)/2;
	c = arc[5][1] = (arc[6][1] + c)/2;

	a = arc[2][1] = (a + b)/2;
	b = arc[4][1] = (b + c)/2;

	arc[3][1] = (a + b)/2;

	/* //USING SIMD
	temp

	arc[6] = arc[3];

	//backwards to avoid overwriting
	arc[5] = (arc[2] + arc[3])/2;
	temp = (arc[1] + arc[2])/2;
	arc[1] = (arc[0] + arc[1])/2;

	arc[4] = (temp + arc[5])/2;
	arc[2] = (arc[1] + temp)/2;

	arc[3] = (arc[2] + arc[4])/2;

	*/
}

//************** PARAMETRIC RENDERER SUPPORT STRUCTURES ****************

// super segment
struct MonoSegment
{
	Rect	aabb;
	int		ydir;
	vector<Point>	pointlist;

	MonoSegment(int dir = 0, Real x0 = 0, Real x1 = 0, Real y0 = 0, Real y1 = 0)
	{
		aabb.minx = x0;
		aabb.maxx = x1;
		aabb.miny = y0;
		aabb.maxy = y1;

		ydir = dir;
	}

	int intersect(Real x,Real y) const
	{
		if((y < aabb.miny+EPSILON) || (y > aabb.maxy) || (x < aabb.minx)) return 0;
		if(x > aabb.maxx) return ydir;

		//int i = 0;
		//int size = pointlist.size();
		//vector<Point>::const_iterator end = pointlist.end();
		vector<Point>::const_iterator p = pointlist.begin();

		//assumes that the rect culled away anything that would be beyond the edges
		if(ydir > 0)
		{
			while(y > (*++p)[1])
				;
		}
		else
		{
			while(y < (*++p)[1])
				;
		}

		//for the loop to break there must have been a slope (straight line would do nothing)
		//vector<Point>::const_iterator p1 = p-1;
		Real dy = p[-1][1] - p[0][1];
		Real dx = p[-1][0] - p[0][0];

		assert(dy != 0);

		Real xi = p[0][0] + (y - p[0][1]) * dx / dy;
		return (x > xi)*ydir;
	}
};

struct CurveArray
{
	Rect	aabb;	//not necessarily as effective - can only reject values
	vector<Point>	pointlist;	//run length - p0, p1, p2, p3 = p10, p11, p12, p13 = p20 ...
	vector<char>	degrees;

	CurveArray(Real x0 = 0, Real x1 = 0, Real y0 = 0, Real y1 = 0)
	{
		aabb.set(x0,y0,x1,y1);
	}

	void reset(Real x0 = 0, Real x1 = 0, Real y0 = 0, Real y1 = 0)
	{
		aabb.set(x0,y0,x1,y1);
		pointlist.clear();
		degrees.clear();
	}

	int size () const
	{
		return degrees.size();
	}

	void Start(Point m)
	{
		reset(m[0],m[0],m[1],m[1]);
		pointlist.push_back(m);
	}

	void AddCubic(Point p1, Point p2, Point dest)
	{
		aabb.expand(p1[0],p1[1]);
		aabb.expand(p2[0],p2[1]);
		aabb.expand(dest[0],dest[1]);

		pointlist.push_back(p1);
		pointlist.push_back(p2);
		pointlist.push_back(dest);

		degrees.push_back(3);
	}

	void AddConic(Point p1, Point dest)
	{
		aabb.expand(p1[0],p1[1]);
		aabb.expand(dest[0],dest[1]);

		pointlist.push_back(p1);
		pointlist.push_back(dest);

		degrees.push_back(2);
	}

	static int intersect_conic(Real x, Real y, Point *p, int /*level*/ = 0)
	{
		Real ymin,ymax,xmin,xmax;
		int intersects = 0;

		//sort the overall curve ys - degenerate detection
		ymin = min(p[0][1],p[2][1]);
		ymax = max(p[0][1],p[2][1]);

		xmin = min(min(p[0][0],p[1][0]),p[2][0]);
		xmax = max(max(p[0][0],p[1][0]),p[2][0]);

		//to the left, to the right and out of range y, or completely out of range y
		if( x < xmin ) return 0;
		if( x > xmax  && (y > ymax || y < ymin) ) return 0;
		if( (y > ymax && y > p[1][1]) || (y < ymin && y < p[1][1]) ) return 0;

		//degenerate line max
		if(ymin == ymax && ymax == p[1][1])
			return 0;

		//degenerate accept - to the right and crossing the base line
		if(x > xmax)
		{
			return (y <= ymax && y >= ymin);
		}

		//solve for curve = y

		//real roots:
		//0 roots 	- 0 intersection
		//1 root 	- get x, and figure out x
		//2 roots (non-double root)	- get 2 xs, and count xs to the left

		//for conic we can assume 1 intersection for monotonic curve
		Real 	a = p[2][1] - 	2*p[1][1] + 	p[0][1],
				b = 			2*p[1][1] - 	2*p[0][1],
				c = 							p[0][1]		-	y;

		Real t1 = -1, t2 = -1;

		if(a == 0)
		{
			//linear - easier :)
			if(b == 0) return 0; //may not need this check

			t1 = - c / b; //bt + c = 0 solved
		}else
		{
			//2 degree polynomial
			Real b2_4ac = b*b - 4*a*c;

			//if there are double/no roots - no intersections (in real #s that is)
			if(b2_4ac <= 0)
			{
				return 0;
			}

			b2_4ac = sqrt(b2_4ac);

			t1 = (-b - b2_4ac) / 2*a,
			t2 = (-b + b2_4ac) / 2*a;
		}

		//calculate number of intersections
		if(t1 >= 0 && t1 <= 1)
		{
			const Real t = t1;
			const Real invt = 1 - t;

			//find x val and it counts if it's to the left of the point
			const Real xi = invt*invt*p[0][0] + 2*t*invt*p[1][0] + t*t*p[2][0];
			const Real dy_t = 2*a*t + b;

			if(dy_t)
			{
				intersects += (x >= xi) * ( dy_t > 0 ? 1 : -1);
			}
		}

		if(t2 >= 0 && t2 <= 1)
		{
			const Real t = t2;
			const Real invt = 1 - t;

			//find x val and it counts if it's to the left of the point
			const Real xi = invt*invt*p[0][0] + 2*t*invt*p[1][0] + t*t*p[2][0];
			const Real dy_t = 2*a*t + b;

			if(dy_t)
			{
				intersects += (x >= xi) * ( dy_t > 0 ? 1 : -1);
			}
		}

		return intersects;
	}

	static int 	quadratic_eqn(Real a, Real b, Real c, Real *t0, Real *t1)
	{
		const Real b2_4ac = b*b - 4*a*c;

		//degenerate reject (can't take sqrt)
		if(b2_4ac < 0)
		{
			return 0;
		}

		const Real sqrtb2_4ac = sqrt(b2_4ac);
		const Real signb = b < 0 ? -1 : 1;
		const Real q = - 0.5 * (b + signb * sqrtb2_4ac);

		*t0 = q/a;
		*t1 = c/q;

		return sqrtb2_4ac == 0 ? 1 : 2;
	}

	//Newton-Raphson root polishing (we don't care about bounds, assumes very near the desired root)
	static Real polish_cubicroot(Real a, Real b, Real c, Real d, Real t, Real *dpdt)
	{
		const Real cn[4] = {a,b,c,d};
		Real p,dp,newt,oldpmag=FLT_MAX;

		//eval cubic eqn and its derivative
		for(;;)
		{
			p = cn[0]*t + cn[1];
			dp = cn[0];

			for(int i = 2; i < 4; i++)
			{
				dp = p + dp*t;
				p = cn[i] + p*t;
			}

			if(dp == 0)
			{
				synfig::warning("polish_cubicroot: Derivative should not vanish!!!");
				return t;
			}

			newt = t - p/dp;

			if(newt == t || fabs(p) >= oldpmag)
			{
				*dpdt = dp;
				return t;
			}

			t = newt;
			oldpmag = fabs(p);
		}
	}

	static int intersect_cubic(Real x, Real y, Point *p, int /*level*/ = 0)
	{
		const Real INVALIDROOT = -FLT_MAX;
		Real ymin,ymax,xmin,xmax;
		Real ymin2,ymax2,ymintot,ymaxtot;
		int intersects = 0;

		//sort the overall curve ys and xs - degenerate detection

		//open span for the two end points
		ymin = min(p[0][1],p[3][1]);
		ymax = max(p[0][1],p[3][1]);

		//other points etc.
		ymin2 = min(p[1][1],p[2][1]);
		ymax2 = max(p[1][1],p[2][1]);

		ymintot = min(ymin,ymin2);
		ymaxtot = max(ymax,ymax2);

		//the entire curve control polygon is in this x range
		xmin = min(min(p[0][0],p[1][0]),min(p[2][0],p[3][0]));
		xmax = max(max(p[0][0],p[1][0]),max(p[2][0],p[3][0]));

		//outside all y boundaries (no intersect)
		if( (y > ymaxtot) || (y < ymintot) ) return 0;

		//left of curve (no intersect)
		if(x < xmin) return 0;

		//right of curve (and outside base range)
		if( x > xmax )
		{
			if( (y > ymax) || (y < ymin) ) return 0;

			//degenerate accept - to the right and inside the [ymin,ymax] range (already rejected if out of range)
			const Real n = p[3][1] - p[0][1];

			//extract the sign from the value (we need valid data)
			return n < 0 ? -1 : 1;
		}

		//degenerate horizontal line max -- doesn't happen enough to check for
		if( ymintot == ymaxtot ) return 0;

		//calculate roots:
		// can have 0,1,2, or 3 real roots
		// if any of them are double then reject the two...

		// y-coefficients for f_y(t) - y = 0
		Real 	a = p[3][1] 	- 3*p[2][1]	+ 3*p[1][1]	-   p[0][1],
				b = 			  3*p[2][1]	- 6*p[1][1]	+ 3*p[0][1],
				c =							  3*p[1][1]	- 3*p[0][1],
				d = 										p[0][1]	- y;

		Real 	ax = p[3][0] 	- 3*p[2][0]	+ 3*p[1][0]	-   p[0][0],
				bx = 			  3*p[2][0]	- 6*p[1][0]	+ 3*p[0][0],
				cx =						  3*p[1][0]	- 3*p[0][0],
				dx = 										p[0][0];

		Real t1 = INVALIDROOT, t2 = INVALIDROOT, t3 = INVALIDROOT, t, dydt;

		if(a == 0)
		{
			//only 2nd degree
			if(b == 0)
			{
				//linear
				if(c == 0) return 0;

				t1 = - d / c; //equation devolved into: ct + d = 0 - solve...
			}else
			{
				//0 roots = 0 intersections, 1 root = 2 intersections at the same place (0 effective)
				if(quadratic_eqn(a,b,c,&t1,&t2) != 2) return 0;
			}
		}else
		{
			//cubic - sigh....

			//algorithm courtesy of Numerical Recipes in C (algorithm copied from pg. 184/185)
			Real an = b / a,
				 bn = c / a,
				 cn = d / a;

			//if cn is 0 (or really really close), then we can simplify this...
			if(IsZero(cn))
			{
				t3 = 0;

				//0 roots = 0 intersections, 1 root = 2 intersections at the same place (0 effective)
				if(quadratic_eqn(a,b,c,&t1,&t2) != 2)
				{
					t1 = t2 = INVALIDROOT;
				}
			}
			else
			{
				//otherwise run the normal cubic root equation
				Real Q = (an*an - 3.0*bn) / 9.0;
				Real R = ((2.0*an*an - 9.0*bn)*an + 27.0*cn)/54.0;

				if(R*R < Q*Q*Q)
				{
					Real theta = acos(R / sqrt(Q*Q*Q));

					t1 = -2.0*sqrt(Q)*cos(theta/3) - an/3.0;
					t2 = -2.0*sqrt(Q)*cos((theta+2*PI)/3.0) - an/3.0;
					t3 = -2.0*sqrt(Q)*cos((theta-2*PI)/3.0) - an/3.0;

					//don't need to reorder,l just need to eliminate double/triple roots
					//if(t3 == t2 && t1 == t2) t2 = t3 = INVALIDROOT;
					if(t3 == t2) t2 = t3 = INVALIDROOT;
					if(t1 == t2) t1 = t2 = INVALIDROOT;
					if(t1 == t3) t1 = t3 = INVALIDROOT;
				}else
				{
					Real signR = R < 0 ? -1 : 1;
					Real A = - signR * pow(signR*R + sqrt(R*R - Q*Q*Q),1/3.0);

					Real B;
					if(A == 0) B = 0;
					else B = Q / A;

					//single real root in this case
					t1 = (A + B) - an/3.0;
				}
			}
		}

		//if(t1 != INVALIDROOT)
		{
			t = t1;//polish_cubicroot(a,b,c,d,t1,&dydt);
			if(t >= 0 && t < 1)
			{
				//const Real invt = 1 - t;

				//find x val and it counts if it's to the left of the point
				const Real xi = ((ax*t + bx)*t + cx)*t + dx;
				dydt = (3*a*t + 2*b)*t + c;

				if(dydt)
				{
					intersects += (x >= xi) * ( dydt > 0 ? 1 : -1);
				}
			}
		}

		//if(t2 != INVALIDROOT)
		{
			t = t2;//polish_cubicroot(a,b,c,d,t2,&dydt);
			if(t >= 0 && t < 1)
			{
				//const Real invt = 1 - t;

				//find x val and it counts if it's to the left of the point
				const Real xi = ((ax*t + bx)*t + cx)*t + dx;
				dydt = (3*a*t + 2*b)*t + c;

				if(dydt)
				{
					intersects += (x >= xi) * ( dydt > 0 ? 1 : -1);
				}
			}
		}

		//if(t3 != INVALIDROOT)
		{
			t = t3;//polish_cubicroot(a,b,c,d,t3,&dydt);
			if(t >= 0 && t < 1)
			{
				//const Real invt = 1 - t;

				//find x val and it counts if it's to the left of the point
				const Real xi = ((ax*t + bx)*t + cx)*t + dx;
				dydt = (3*a*t + 2*b)*t + c;

				if(dydt)
				{
					intersects += (x >= xi) * ( dydt > 0 ? 1 : -1);
				}
			}
		}

		return intersects;
	}

	int intersect(Real x,Real y, Point *table) const
	{
		if((y < aabb.miny) || (y > aabb.maxy) || (x < aabb.minx)) return 0;

		int i, curdeg, intersects = 0;
		const int numcurves = degrees.size();

		vector<Point>::const_iterator	p = pointlist.begin();

		for(i=0; i < numcurves; i++)
		{
			curdeg = degrees[i];

			switch(curdeg)
			{
				case 2:
				{
					table[0] = *p++;
					table[1] = *p++;
					table[2] = *p;	//we want to include the last point for the next curve

					intersects += intersect_conic(x,y,table);

					break;
				}

				case 3:
				{
					table[0] = *p++;
					table[1] = *p++;
					table[2] = *p++;
					table[3] = *p;	//we want to include the last point for the next curve

					intersects += intersect_cubic(x,y,table);

					break;
				}

				default:
				{
					warning("Invalid degree (%d) inserted into the list (index: %d)\n", curdeg, i);
					return 0;
				}
			}
		}

		return intersects;
	}
};

struct Layer_Shape::Intersector
{
	Rect	aabb;

	//! true iff aabb hasn't been initialized yet
	bool	initaabb;

	int 	flags;

	enum IntersectorFlags
	{
		NotClosed = 0x8000
	};

	enum PrimitiveType
	{
		TYPE_NONE = 0,
		TYPE_LINE,
		TYPE_CURVE
	};

	Real	cur_x,cur_y;
	Real	close_x,close_y;

	vector<MonoSegment>				segs;	//monotonically increasing
	vector<CurveArray>				curves; //big array of consecutive curves

	int								prim;
	Vector							tangent;

	Intersector()
	{
		clear();
	}

	bool notclosed()
	{
		return (flags & NotClosed) || (cur_x != close_x) || (cur_y != close_y);
	}

	void move_to(Real x, Real y)
	{
		close();

		close_x = cur_x = x;
		close_y = cur_y = y;

		tangent[0] = tangent[1] = 0;

		if(initaabb)
		{
			aabb.set_point(x,y);
			initaabb = false;
		}else aabb.expand(x,y);

		prim = TYPE_NONE;
	}

	void line_to(Real x, Real y)
	{
		int dir = (y > cur_y)*1 + (-1)*(y < cur_y);

		//check for context (if not line start a new segment)
		//if we're not in line mode (covers 0 set case), or if directions are different (not valid for 0 direction)
		if(prim != TYPE_LINE || (dir && segs.back().ydir != dir))
		{
			MonoSegment		seg(dir,x,x,y,y);

			seg.aabb.expand(cur_x,cur_y);
			seg.pointlist.push_back(Point(cur_x,cur_y));
			seg.pointlist.push_back(Point(x,y));
			segs.push_back(seg);
		}
		//add to the last segment, because it works
		else
		{
			segs.back().pointlist.push_back(Point(x,y));
			segs.back().aabb.expand(x,y);
		}



		cur_x = x;
		cur_y = y;
		aabb.expand(x,y); //expand the entire thing's bounding box

		tangent[0] = x - cur_x;
		tangent[1] = x - cur_y;

		flags |= NotClosed;
		prim = TYPE_LINE;
	}

	void conic_to_smooth(Real x, Real y)
	{
		const Real x1 = tangent[0]/2.0 + cur_x;
		const Real y1 = tangent[1]/2.0 + cur_y;

		conic_to(x1,y1,x,y);
	}

	void conic_to(Real x1, Real y1, Real x, Real y)
	{
		//if we're not already a curve start one
		if(prim != TYPE_CURVE)
		{
			CurveArray	c;

			c.Start(Point(cur_x,cur_y));
			c.AddConic(Point(x1,y1),Point(x,y));

			curves.push_back(c);
		}else
		{
			curves.back().AddConic(Point(x1,y1),Point(x,y));
		}

		cur_x = x;
		cur_y = y;

		aabb.expand(x1,y1);
		aabb.expand(x,y);

		tangent[0] = 2*(x - x1);
		tangent[1] = 2*(y - y1);

		flags |= NotClosed;
		prim = TYPE_CURVE;
	}

	void curve_to_smooth(Real x2, Real y2, Real x, Real y)
	{
		Real x1 = tangent[0]/3.0 + cur_x;
		Real y1 = tangent[1]/3.0 + cur_y;

		curve_to(x1,y1,x2,y2,x,y);
	}

	void curve_to(Real x1, Real y1, Real x2, Real y2, Real x, Real y)
	{
		//if we're not already a curve start one
		if(prim != TYPE_CURVE)
		{
			CurveArray	c;

			c.Start(Point(cur_x,cur_y));
			c.AddCubic(Point(x1,y1),Point(x2,y2),Point(x,y));

			curves.push_back(c);
		}else
		{
			curves.back().AddCubic(Point(x1,y1),Point(x2,y2),Point(x,y));
		}

		cur_x = x;
		cur_y = y;

		//expand bounding box around ALL of it
		aabb.expand(x1,y1);
		aabb.expand(x2,y2);
		aabb.expand(x,y);

		tangent[0] = 3*(x - x2);
		tangent[1] = 3*(y - y2);

		flags |= NotClosed;
		prim = TYPE_CURVE;
	}

	void close()
	{
		if(flags & NotClosed)
		{
			if(cur_x != close_x || cur_y != close_y)
			{
				line_to(close_x,close_y);
			}

			flags &= ~NotClosed;
		}
	}

	//assumes the line to count the intersections with is (-1,0)
	int	intersect (Real x, Real y) const
	{
		int inter = 0;
		unsigned int i;
		vector<MonoSegment>::const_iterator s = segs.begin();
		vector<CurveArray>::const_iterator c = curves.begin();

		Point	memory[3*MAX_SUBDIVISION_SIZE + 1];

		for(i = 0; i < segs.size(); i++,s++)
		{
			inter += s->intersect(x,y);
		}

		for(i=0; i < curves.size(); i++,c++)
			inter += c->intersect(x,y,memory);

		return inter;
	}

	//intersect an arbitrary line
	//int	intersect (Real x, Real y, Real vx, Real vy) {return 0;}

	void clear()
	{
		segs.clear();
		curves.clear();

		flags = 0;
		cur_x = cur_y = close_x = close_y = 0;
		prim = TYPE_NONE;
		tangent[0] = tangent[1] = 0;
		initaabb = true;
	}
};

//*********** SCANLINE RENDERER SUPPORT STRUCTURES ***************
struct PenMark
{
	int y,x;
	Real cover,area;

	PenMark(){}
	PenMark(int xin, int yin, Real c, Real a)
		:y(yin),x(xin),cover(c),area(a) {}

	void set(int xin, int yin, Real c, Real a) 	{ y = yin; x = xin; cover = c; area = a;	}

	void setcoord(int xin, int yin)				{ y = yin; x = xin;	}

	void setcover(Real c, Real a)				{ cover	= c; area = a; }
	void addcover(Real c, Real a)				{ cover += c; area += a; }

	bool operator<(const PenMark &rhs) const
	{
		return y == rhs.y ? x < rhs.x : y < rhs.y;
	}
};

typedef rect<int> ContextRect;

class Layer_Shape::PolySpan
{
public:
	typedef	deque<PenMark> 	cover_array;

	Point			arc[3*MAX_SUBDIVISION_SIZE + 1];

	cover_array		covers;
	PenMark			current;

	int				open_index;

	//ending position of last primitive
	Real			cur_x;
	Real			cur_y;

	//starting position of current primitive list
	Real			close_x;
	Real			close_y;

	//flags for the current segment
	int				flags;

	//the window that will be drawn (used for clipping)
	ContextRect		window;

	//for assignment to flags value
	enum PolySpanFlags
	{
		NotSorted = 0x8000,
		NotClosed =	0x4000
	};

	//default constructor - 0 everything
	PolySpan() :current(0,0,0,0),flags(NotSorted)
	{
		cur_x = cur_y = close_x = close_y = 0;
		open_index = 0;
	}

	bool notclosed() const
	{
		return (flags & NotClosed) || (cur_x != close_x) || (cur_y != close_y);
	}

	//0 out all the variables involved in processing
	void clear()
	{
		covers.clear();
		cur_x = cur_y = close_x = close_y = 0;
		open_index = 0;
		current.set(0,0,0,0);
		flags = NotSorted;
	}

	//add the current cell, but only if there is information to add
	void addcurrent()
	{
		if(current.cover || current.area)
		{
			covers.push_back(current);
		}
	}

	//move to the next cell (cover values 0 initially), keeping the current if necessary
	void move_pen(int x, int y)
	{
		if(y != current.y || x != current.x)
		{
			addcurrent();
			current.set(x,y,0,0);
		}
	}

	//close the primitives with a line (or rendering will not work as expected)
	void close()
	{
		if(flags & NotClosed)
		{
			if(cur_x != close_x || cur_y != close_y)
			{
				line_to(close_x,close_y);
				addcurrent();
				current.setcover(0,0);
			}
			flags &= ~NotClosed;
		}
	}

	// Not recommended - destroys any separation of spans currently held
	void merge_all()
	{
		sort(covers.begin(),covers.end());
		open_index = 0;
	}

	//will sort the marks if they are not sorted
	void sort_marks()
	{
		if(flags & NotSorted)
		{
			//only sort the open index
			addcurrent();
			current.setcover(0,0);

			sort(covers.begin() + open_index,covers.end());
			flags &= ~NotSorted;
		}
	}

	//encapsulate the current sublist of marks (used for drawing)
	void encapsulate_current()
	{
		//sort the current list then reposition the open list section
		sort_marks();
		open_index = covers.size();
	}

	//move to start a new primitive list (enclose the last primitive if need be)
	void move_to(Real x, Real y)
	{
		close();
		if(isnan(x))x=0;
		if(isnan(y))y=0;
		move_pen((int)floor(x),(int)floor(y));
		close_y = cur_y = y;
		close_x = cur_x = x;
	}

	//primitive_to functions
	void line_to(Real x, Real y);
	void conic_to(Real x1, Real y1, Real x, Real y);
	void cubic_to(Real x1, Real y1, Real x2, Real y2, Real x, Real y);

	void draw_scanline(int y, Real x1, Real y1, Real x2, Real y2);
	void draw_line(Real x1, Real y1, Real x2, Real y2);

	Real ExtractAlpha(Real area, WindingStyle winding_style)
	{
		if (area < 0)
			area = -area;

		if (winding_style == WINDING_NON_ZERO)
		{
			// non-zero winding style
			if (area > 1)
				return 1;
		}
		else // if (winding_style == WINDING_EVEN_ODD)
		{
			// even-odd winding style
			while (area > 1)
				area -= 2;

			// want pyramid like thing
			if (area < 0)
				area = -area;
		}

		return area;
	}
};

/* === M E T H O D S ======================================================= */

Layer_Shape::Layer_Shape(const Real &a, const Color::BlendMethod m):
	Layer_Composite      (a,m),
	edge_table	         (new Intersector),
	param_color          (Color::black()),
	param_origin         (Vector(0,0)),
	param_invert         (bool(false)),
	param_antialias      (bool(true)),
	param_blurtype       (int(Blur::FASTGAUSSIAN)),
	param_feather        (Real(0.0)),
	param_winding_style	 (WINDING_NON_ZERO),
	bytestream           (0),
	lastbyteop           (Primitive::NONE),
	lastoppos            (-1)
{
}

Layer_Shape::~Layer_Shape()
{
	delete edge_table;
}

void
Layer_Shape::clear()
{
	edge_table->clear();
	bytestream.clear();
}

bool
Layer_Shape::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_color,
	{
		Color color=param_color.get(Color());
		if (color.get_a() == 0)
		{
			if (converted_blend_)
			{
				set_blend_method(Color::BLEND_ALPHA_OVER);
				color.set_a(1);
			}
			else
			transparent_color_ = true;
		}
		param_color.set(color);
	}
	);
	IMPORT_VALUE(param_origin);
	IMPORT_VALUE(param_invert);
	IMPORT_VALUE(param_antialias);
	IMPORT_VALUE_PLUS(param_feather,
	{
		Real feather=param_feather.get(Real());
		if(feather<0)
		{
			feather=0;
			param_feather.set(feather);
		}
	}
	);

	IMPORT_VALUE(param_blurtype);
	IMPORT_VALUE(param_winding_style);

	if(param=="offset" && param_origin.get_type() == value.get_type())
	{
		param_origin=value;
		return true;
	}
	return Layer_Composite::set_param(param,value);
}

ValueBase
Layer_Shape::get_param(const String &param)const
{
	EXPORT_VALUE(param_color);
	EXPORT_VALUE(param_origin);
	EXPORT_VALUE(param_invert);
	EXPORT_VALUE(param_antialias);
	EXPORT_VALUE(param_feather);
	EXPORT_VALUE(param_blurtype);
	EXPORT_VALUE(param_winding_style);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
Layer_Shape::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("color")
		.set_local_name(_("Color"))
		.set_description(_("Layer_Shape Color"))
	);
	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
	);
	ret.push_back(ParamDesc("invert")
		.set_local_name(_("Invert"))
	);
	ret.push_back(ParamDesc("antialias")
		.set_local_name(_("Antialiasing"))
	);
	ret.push_back(ParamDesc("feather")
		.set_local_name(_("Feather"))
		.set_is_distance()
	);
	ret.push_back(ParamDesc("blurtype")
		.set_local_name(_("Type of Feather"))
		.set_description(_("Type of feathering to use"))
		.set_hint("enum")
		.add_enum_value(Blur::BOX,"box",_("Box Blur"))
		.add_enum_value(Blur::FASTGAUSSIAN,"fastgaussian",_("Fast Gaussian Blur"))
		.add_enum_value(Blur::CROSS,"cross",_("Cross-Hatch Blur"))
		.add_enum_value(Blur::GAUSSIAN,"gaussian",_("Gaussian Blur"))
		.add_enum_value(Blur::DISC,"disc",_("Disc Blur"))
	);
	ret.push_back(ParamDesc("winding_style")
		.set_local_name(_("Winding Style"))
		.set_description(_("Winding style to use"))
		.set_hint("enum")
		.add_enum_value(WINDING_NON_ZERO,"nonzero",_("Non Zero"))
		.add_enum_value(WINDING_EVEN_ODD,"evenodd",_("Even/Odd"))
	);

	return ret;
}

synfig::Layer::Handle
Layer_Shape::hit_check(synfig::Context context, const synfig::Point &p)const
{
	Color color=param_color.get(Color());
	Point origin=param_origin.get(Point());
	bool invert =param_invert.get(bool(true));
	
	Point pos(p-origin);

	int intercepts = edge_table->intersect(pos[0],pos[1]);

	// If we have an odd number of intercepts, we are inside.
	// If we have an even number of intercepts, we are outside.
	bool intersect = ((!!intercepts) ^ invert);

	if(get_amount() == 0 || get_blend_method() == Color::BLEND_ALPHA_OVER)
	{
		intersect = false;
	}

	if(intersect)
	{
		synfig::Layer::Handle tmp;
		if(get_blend_method()==Color::BLEND_BEHIND && (tmp=context.hit_check(p)))
			return tmp;
		if(Color::is_onto(get_blend_method()))
		{
			//if there's something in the lower layer then we're set...
			if(!context.hit_check(p).empty())
				return const_cast<Layer_Shape*>(this);
		}else if(get_blend_method() == Color::BLEND_ALPHA_OVER)
		{
			synfig::info("layer_shape::hit_check - we've got alphaover");
			//if there's something in the lower layer then we're set...
			if(color.get_a() < 0.1 && get_amount() > .9)
			{
				synfig::info("layer_shape::hit_check - can see through us... so nothing");
				return Handle();
			}else return context.hit_check(p);
		}else
			return const_cast<Layer_Shape*>(this);
	}

	return context.hit_check(p);
}

Color
Layer_Shape::get_color(Context context, const Point &p)const
{
	Color color=param_color.get(Color());
	Point origin=param_origin.get(Point());
	bool invert =param_invert.get(bool(true));
	int blurtype=param_blurtype.get(int());
	Real feather=param_feather.get(Real());

	Point pp = p;

	if(feather)
		pp = Blur(feather,feather,blurtype)(p);

	Point pos(pp-origin);

	int intercepts = edge_table->intersect(pos[0],pos[1]);

	// If we have an odd number of intercepts, we are inside.
	// If we have an even number of intercepts, we are outside.
	bool intersect = ((!!intercepts) ^ invert);

	if(!intersect)
		return Color::blend(Color::alpha(),context.get_color(pp),get_amount(),get_blend_method());

	//Ok, we're inside... bummmm ba bum buM...
	if(get_blend_method() == Color::BLEND_STRAIGHT && get_amount() == 1)
		return color;
	else
		return Color::blend(color,context.get_color(p),get_amount(),get_blend_method());
}

//************** SCANLINE RENDERING *********************
void Layer_Shape::PolySpan::line_to(Real x, Real y)
{
	Real n[4] = {0,0,0,0};
	bool afterx = false;

	const Real xin(x), yin(y);

	Real dx = x - cur_x;
	Real dy = y - cur_y;

	//CLIP IT!!!!
	try {
	//outside y - ignore entirely
	if(	 (cur_y >= window.maxy && y >= window.maxy)
	   ||(cur_y <  window.miny && y <  window.miny) )
	{
		cur_x = x;
		cur_y = y;
	}
	else //not degenerate - more complicated
	{
		if(dy > 0) //be sure it's not tooooo small
		{
			// cur_y ... window.miny ... window.maxy ... y

			//initial degenerate - initial clip
			if(cur_y < window.miny)
			{
				//new clipped start point (must also move pen)
				n[2] = cur_x + (window.miny - cur_y) * dx / dy;

				cur_x = n[2];
				cur_y = window.miny;
				move_pen((int)floor(cur_x),window.miny);
			}

			//generate data for the ending clipped info
			if(y > window.maxy)
			{
				//initial line to intersection (and degenerate)
				n[2] = x + (window.maxy - y) * dx / dy;

				//intersect coords
				x = n[2];
				y = window.maxy;
			}
		}
		else
		{
			//initial degenerate - initial clip
			if(cur_y > window.maxy)
			{
				//new clipped start point (must also move pen)
				n[2] = cur_x + (window.maxy - cur_y) * dx / dy;

				cur_x = n[2];
				cur_y = window.maxy;
				move_pen((int)floor(cur_x),window.maxy);
			}

			//generate data for the ending clipped info
			if(y < window.miny)
			{
				//initial line to intersection (and degenerate)
				n[2] = x + (window.miny - y) * dx / dy;

				//intersect coords
				x = n[2];
				y = window.miny;
			}
		}

		//all degenerate - but require bounded clipped values
		if(   (cur_x >= window.maxx && x >= window.maxx)
			||(cur_x <  window.minx && x <  window.minx) )
		{
			//clip both vertices - but only needed in the x direction
			cur_x = max(cur_x,	(Real)window.minx);
			cur_x = min(cur_x,	(Real)window.maxx);

			//clip the dest values - y is already clipped
			x = max(x,(Real)window.minx);
			x = min(x,(Real)window.maxx);

			//must start at new point...
			move_pen((int)floor(cur_x),(int)floor(cur_y));

			draw_line(cur_x,cur_y,x,y);

			cur_x = xin;
			cur_y = yin;
		}
		else
		{
			//clip x
			if(dx > 0)
			{
				//initial degenerate - initial clip
				if(cur_x < window.minx)
				{
					//need to draw an initial segment from clippedx,cur_y to clippedx,intersecty
					n[2] = cur_y + (window.minx - cur_x) * dy / dx;

					move_pen(window.minx,(int)floor(cur_y));
					draw_line(window.minx,cur_y,window.minx,n[2]);

					cur_x = window.minx;
					cur_y = n[2];
				}

				//generate data for the ending clipped info
				if(x > window.maxx)
				{
					//initial line to intersection (and degenerate)
					n[2] = y + (window.maxx - x) * dy / dx;

					n[0] = window.maxx;
					n[1] = y;

					//intersect coords
					x = window.maxx;
					y = n[2];
					afterx = true;
				}
			}else
			{
				//initial degenerate - initial clip
				if(cur_x > window.maxx)
				{
					//need to draw an initial segment from clippedx,cur_y to clippedx,intersecty
					n[2] = cur_y + (window.maxx - cur_x) * dy / dx;

					move_pen(window.maxx,(int)floor(cur_y));
					draw_line(window.maxx,cur_y,window.maxx,n[2]);

					cur_x = window.maxx;
					cur_y = n[2];
				}

				//generate data for the ending clipped info
				if(x < window.minx)
				{
					//initial line to intersection (and degenerate)
					n[2] = y + (window.minx - x) * dy / dx;

					n[0] = window.minx;
					n[1] = y;

					//intersect coords
					x = window.minx;
					y = n[2];
					afterx = true;
				}
			}

			move_pen((int)floor(cur_x),(int)floor(cur_y));
			//draw the relevant line (clipped)
			draw_line(cur_x,cur_y,x,y);

			if(afterx)
			{
				draw_line(x,y,n[0],n[1]);
			}

			cur_x = xin;
			cur_y = yin;
		}
	}
	} catch(...) { synfig::error("line_to: cur_x=%f, cur_y=%f, x=%f, y=%f", cur_x, cur_y, x, y); throw; }

	flags |= NotClosed|NotSorted;
}

static inline bool clip_conic(const Point *const p, const ContextRect &r)
{
	const Real minx = min(min(p[0][0],p[1][0]),p[2][0]);
	const Real miny = min(min(p[0][1],p[1][1]),p[2][1]);
	const Real maxx = max(max(p[0][0],p[1][0]),p[2][0]);
	const Real maxy = max(max(p[0][1],p[1][1]),p[2][1]);

	return 	(minx > r.maxx) ||
			(maxx < r.minx) ||
			(miny > r.maxy) ||
			(maxy < r.miny);
}

static inline bool clip_cubic(const Point *const p, const ContextRect &r)
{
	/*const Real minx = min(min(p[0][0],p[1][0]),min(p[2][0],p[3][0]));
	const Real miny = min(min(p[0][1],p[1][1]),min(p[2][1],p[3][1]));
	const Real maxx = max(max(p[0][0],p[1][0]),max(p[2][0],p[3][1]));
	const Real maxy = max(max(p[0][1],p[1][1]),max(p[2][1],p[3][1]));

	return 	(minx > r.maxx) ||
			(maxx < r.minx) ||
			(miny > r.maxy) ||
			(maxy < r.miny);*/

	return 	((p[0][0] > r.maxx) && (p[1][0] > r.maxx) && (p[2][0] > r.maxx) && (p[3][0] > r.maxx)) ||
			((p[0][0] < r.minx) && (p[1][0] < r.minx) && (p[2][0] < r.minx) && (p[3][0] < r.minx)) ||
			((p[0][1] > r.maxy) && (p[1][1] > r.maxy) && (p[2][1] > r.maxy) && (p[3][1] > r.maxy)) ||
			((p[0][1] < r.miny) && (p[1][1] < r.miny) && (p[2][1] < r.miny) && (p[3][1] < r.miny));
}

static inline Real max_edges_cubic(const Point *const p)
{
	const Real x1 = p[1][0] - p[0][0];
	const Real y1 = p[1][1] - p[0][1];

	const Real x2 = p[2][0] - p[1][0];
	const Real y2 = p[2][1] - p[1][1];

	const Real x3 = p[3][0] - p[2][0];
	const Real y3 = p[3][1] - p[2][1];

	const Real d1 = x1*x1 + y1*y1;
	const Real d2 = x2*x2 + y2*y2;
	const Real d3 = x3*x3 + y3*y3;

	return max(max(d1,d2),d3);
}

static inline Real max_edges_conic(const Point *const p)
{
	const Real x1 = p[1][0] - p[0][0];
	const Real y1 = p[1][1] - p[0][1];

	const Real x2 = p[2][0] - p[1][0];
	const Real y2 = p[2][1] - p[1][1];

	const Real d1 = x1*x1 + y1*y1;
	const Real d2 = x2*x2 + y2*y2;

	return max(d1,d2);
}

void Layer_Shape::PolySpan::conic_to(Real x1, Real y1, Real x, Real y)
{
	Point *current = arc;
	int		level = 0;
	int 	num = 0;
	bool	onsecond = false;

	arc[0] = Point(x,y);
	arc[1] = Point(x1,y1);
	arc[2] = Point(cur_x,cur_y);

	//just draw the line if it's outside
	if(clip_conic(arc,window))
	{
		line_to(x,y);
		return;
	}

	//Ok so it's not super degenerate, subdivide and draw (run through minimum subdivision levels first)
	while(current >= arc)
	{
		if(num >= MAX_SUBDIVISION_SIZE)
		{
			warning("Curve subdivision somehow ran out of space while tessellating!");

			//do something...
			assert(0);
			return;
		}else
		//if the curve is clipping then draw degenerate
		if(clip_conic(current,window))
		{
			line_to(current[0][0],current[0][1]); //backwards so front is destination
			current -= 2;
			if(onsecond) level--;
			onsecond = true;
			num--;
			continue;
		}else
		//if we are not at the level minimum
		if(level < MIN_SUBDIVISION_DRAW_LEVELS)
		{
			Subd_Conic_Stack(current);
			current += 2; 		//cursor on second curve
			level ++;
			num ++;
			onsecond = false;
			continue;
		}else
		//split it again, if it's too big
		if(max_edges_conic(current) > 0.25) //distance of .5 (cover no more than half the pixel)
		{
			Subd_Conic_Stack(current);
			current += 2; 		//cursor on second curve
			level ++;
			num ++;
			onsecond = false;
		}
		else	//NOT TOO BIG? RENDER!!!
		{
			//cur_x,cur_y = current[2], so we need to go 1,0
			line_to(current[1][0],current[1][1]);
			line_to(current[0][0],current[0][1]);

			current -= 2;
			if(onsecond) level--;
			num--;
			onsecond = true;
		}
	}
}

void Layer_Shape::PolySpan::cubic_to(Real x1, Real y1, Real x2, Real y2, Real x, Real y)
{
	Point *current = arc;
	int		num = 0;
	int		level = 0;
	bool	onsecond = false;

	arc[0] = Point(x,y);
	arc[1] = Point(x2,y2);
	arc[2] = Point(x1,y1);
	arc[3] = Point(cur_x,cur_y);

	//just draw the line if it's outside
	if(clip_cubic(arc,window))
	{
		line_to(x,y);
		return;
	}

	//Ok so it's not super degenerate, subdivide and draw (run through minimum subdivision levels first)
	while(current >= arc) //once current goes below arc, there are no more curves left
	{
		if(num >= MAX_SUBDIVISION_SIZE)
		{
			warning("Curve subdivision somehow ran out of space while tessellating!");

			//do something...
			assert(0);
			return;
		}else

		//if we are not at the level minimum
		if(level < MIN_SUBDIVISION_DRAW_LEVELS)
		{
			Subd_Cubic_Stack(current);
			current += 3; 		//cursor on second curve
			level ++;
			num ++;
			onsecond = false;
			continue;
		}else
		//if the curve is clipping then draw degenerate
		if(clip_cubic(current,window))
		{
			line_to(current[0][0],current[0][1]); //backwards so front is destination
			current -= 3;
			if(onsecond) level--;
			onsecond = true;
			num --;
			continue;
		}
		else
		//split it again, if it's too big
		if(max_edges_cubic(current) > 0.25) //could use max_edges<3>
		{
			Subd_Cubic_Stack(current);
			current += 3; 		//cursor on second curve
			level ++;
			num ++;
			onsecond = false;
		}
		else //NOT TOO BIG? RENDER!!!
		{
			//cur_x,cur_y = current[3], so we need to go 2,1,0
			line_to(current[2][0],current[2][1]);
			line_to(current[1][0],current[1][1]);
			line_to(current[0][0],current[0][1]);

			current -= 3;
			if(onsecond) level--;
			num --;
			onsecond = true;
		}
	}
}

//******************** LINE ALGORITHMS ****************************
// THESE CALCULATE THE AREA AND THE COVER FOR THE MARKS, TO THEN SCAN CONVERT
// - BROKEN UP INTO SCANLINES (draw_line - y intersections),
//   THEN THE COVER AND AREA PER TOUCHED PIXEL IS CALCULATED (draw_scanline - x intersections)
void Layer_Shape::PolySpan::draw_scanline(int y, Real x1, Real fy1, Real x2, Real fy2)
{
	int	ix1 = (int)floor(x1);
	int	ix2 = (int)floor(x2);
	Real fx1 = x1 - ix1;
	Real fx2 = x2 - ix2;

	Real dx,dy,dydx,mult;

	dx = x2 - x1;
	dy = fy2 - fy1;

	//case horizontal line
	if(fy1 == fy2)
	{
		move_pen(ix2,y); //pen needs to be at the last coord
		return;
	}

	//case all in same pixel
	if(ix1 == ix2)  //impossible for degenerate case (covered by the previous cases)
	{
		current.addcover(dy,(fx1 + fx2)*dy/2); //horizontal trapezoid area
		return;
	}

	if(dx > 0)
	{
		// ---->	fx1...1  0...1  ...  0...1  0...fx2
		dydx = dy / dx;

		//set initial values
		//Iterate through the covered pixels
		mult = (1 - fx1)*dydx;	//next y intersection diff value (at 1)

		//first pixel
		current.addcover(mult,(1 + fx1)*mult/2);	// fx1,fy1,1,fy@1 - starting trapezoidal area

		//move to the next pixel
		fy1 += mult;
		ix1++;

		move_pen(ix1,y);

		//set up for whole ones
		while(ix1 != ix2)
		{
			//trapezoid(0,y1,1,y1+dydx);
			current.addcover(dydx,dydx/2);	//accumulated area 1/2 the cover

			//move to next pixel (+1)
			ix1++;
			fy1 += dydx;
			move_pen(ix1,y);
		}

		//last pixel
		//final y-pos - last intersect pos
		mult = fx2 * dydx;
		current.addcover(mult,(0+fx2)*mult/2);
	}else
	{
		// fx2...1  0...1  ...  0...1  0...fx1   <----
		//mult = (0 - fx1) * dy / dx;
		//neg sign sucked into dydx
		dydx = -dy / dx;

		//set initial values
		//Iterate through the covered pixels
		mult = fx1*dydx;	//next y intersection diff value

		//first pixel
		current.addcover(mult,fx1*mult/2);	// fx1,fy1,0,fy@0 - starting trapezoidal area

		//move to next pixel
		fy1 += mult;
		ix1--;

		move_pen(ix1,y);

		//set up for whole ones
		while(ix1 != ix2)
		{
			//trapezoid(0,y1,1,y1+dydx);
			current.addcover(dydx,dydx/2);	//accumulated area 1/2 the cover

			//move to next pixel (-1)
			fy1 += dydx;
			ix1--;
			move_pen(ix1,y);
		}

		//last pixel
		mult = fy2 - fy1; //final y-pos - last intersect pos

		current.addcover(mult,(fx2+1)*mult/2);
	}
}

void Layer_Shape::PolySpan::draw_line(Real x1, Real y1, Real x2, Real y2)
{
	int iy1 = (int)floor(y1);
	int iy2 = (int)floor(y2);
	Real fy1 = y1 - iy1;
	Real fy2 = y2 - iy2;

	assert(!isnan(fy1));
	assert(!isnan(fy2));

	Real dx,dy,dxdy,mult,x_from,x_to;

	const Real SLOPE_EPSILON = 1e-10;

	//case all one scanline
	if(iy1 == iy2)
	{
		draw_scanline(iy1,x1,y1,x2,y2);
		return;
	}

	//difference values
	dy = y2 - y1;
	dx = x2 - x1;

	//case vertical line
	if(dx < SLOPE_EPSILON && dx > -SLOPE_EPSILON)
	{
		//calc area and cover on vertical line
		if(dy > 0)
		{
			// ---->	fx1...1  0...1  ...  0...1  0...fx2
			Real sub;

			int	 ix1 = (int)floor(x1);
			Real fx1 = x1 - ix1;

			//current pixel
			sub = 1 - fy1;

			current.addcover(sub,fx1*sub);

			//next pixel
			iy1++;

			//move pen to next pixel
			move_pen(ix1,iy1);

			while(iy1 != iy2)
			{
				//accumulate cover
				current.addcover(1,fx1);

				//next pixel
				iy1++;
				move_pen(ix1,iy1);
			}

			//last pixel
			current.addcover(fy2,fy2*fx1);
		}else
		{
			Real sub;

			int	 ix1 = (int)floor(x1);
			Real fx1 = x1 - ix1;

			//current pixel
			sub = 0 - fy1;

			current.addcover(sub,fx1*sub);

			//next pixel
			iy1--;

			move_pen(ix1,iy1);

			while(iy1 != iy2)
			{
				//accumulate in current pixel
				current.addcover(-1,-fx1);

				//move to next
				iy1--;
				move_pen(ix1,iy1);
			}

			current.addcover(fy2-1,(fy2-1)*fx1);
		}
		return;
	}

	//case normal line - guaranteed dx != 0 && dy != 0

	//calculate the initial intersection with "next" scanline
	if(dy > 0)
	{
		dxdy = dx / dy;

		mult = (1 - fy1) * dxdy;

		//x intersect scanline
		x_from = x1 + mult;
		draw_scanline(iy1,x1,fy1,x_from,1);

		//move to next line
		iy1++;

		move_pen((int)floor(x_from),iy1);

		while(iy1 != iy2)
		{
			//keep up on the x axis, and render the current scanline
			x_to = x_from + dxdy;
			draw_scanline(iy1,x_from,0,x_to,1);
			x_from = x_to;

			//move to next pixel
			iy1++;
			move_pen((int)floor(x_from),iy1);
		}

		//draw the last one, fractional
		draw_scanline(iy2,x_from,0,x2,fy2);

	}else
	{
		dxdy = -dx / dy;

		mult = fy1 * dxdy;

		//x intersect scanline
		x_from = x1 + mult;
		draw_scanline(iy1,x1,fy1,x_from,0);

		//each line after
		iy1--;

		move_pen((int)floor(x_from),iy1);

		while(iy1 != iy2)
		{
			x_to = x_from + dxdy;
			draw_scanline(iy1,x_from,1,x_to,0);
			x_from = x_to;

			iy1--;
			move_pen((int)floor(x_from),iy1);
		}
		//draw the last one, fractional
		draw_scanline(iy2,x_from,1,x2,fy2);
	}
}

//****** LAYER PEN OPERATIONS (move_to, line_to, etc.) ******
void Layer_Shape::move_to(Real x, Real y)
{
	//const int sizeblock = sizeof(Primitive)+sizeof(Point);
	Primitive	op;
	Point		p(x,y);

	op.operation = Primitive::MOVE_TO;
	op.number = 1;	//one point for now

	if(lastbyteop == Primitive::MOVE_TO)
	{
		char *ptr = &bytestream[lastoppos];
		memcpy(ptr,&op,sizeof(op));
		memcpy(ptr+sizeof(op),&p,sizeof(p));
	}
	else //make a new op
	{
		lastbyteop = Primitive::MOVE_TO;
		lastoppos = bytestream.size();

		bytestream.insert(bytestream.end(),(char*)&op,(char*)(&op+1)); 	//insert the bytes for the header
		bytestream.insert(bytestream.end(),(char*)&p,(char*)(&p+1));	//insert the bytes for data
	}

	edge_table->move_to(x,y);
}

void Layer_Shape::close()
{
	Primitive op;

	op.operation = Primitive::CLOSE;
	op.number = 0;

	if(lastbyteop == Primitive::CLOSE)
	{
	}else
	{
		lastbyteop = Primitive::CLOSE;
		lastoppos = bytestream.size();

		bytestream.insert(bytestream.end(),(char*)&op,(char*)(&op+1)); //insert header
	}

	edge_table->close();
	//should not affect the bounding box since it would just be returning to old point...
}

void Layer_Shape::endpath()
{
	Primitive op;

	op.operation = Primitive::END;
	op.number = 0;

	if(lastbyteop == Primitive::END || lastbyteop == Primitive::NONE)
	{
	}else
	{
		bytestream.insert(bytestream.end(),(char*)&op,(char*)(&op+1));
	}
	//should not affect the bounding box since it would just be returning to old point... if at all
}

void Layer_Shape::line_to(Real x, Real y)
{
	assert(!isnan(x));
	assert(!isnan(y));

	//const int sizeblock = sizeof(Primitive)+sizeof(Point);
	Primitive	op;
	Point		p(x,y);

	op.operation = Primitive::LINE_TO;
	op.number = 1;	//one point for now

	if(lastbyteop == Primitive::MOVE_TO || lastbyteop == Primitive::LINE_TO)
	{
		//only need to insert the point
		bytestream.insert(bytestream.end(),(char*)&p,(char*)(&p+1));

		Primitive * prim = (Primitive *)&bytestream[lastoppos];
		prim->number++; //increment number of points in the list
	}else
	{
		lastbyteop = Primitive::LINE_TO;
		lastoppos = bytestream.size();

		bytestream.insert(bytestream.end(),(char*)&op,(char*)(&op+1)); 	//insert the bytes for the header
		bytestream.insert(bytestream.end(),(char*)&p,(char*)(&p+1));	//insert the bytes for data
	}

	edge_table->line_to(x,y);
}

void Layer_Shape::conic_to(Real x1, Real y1, Real x, Real y)
{
	//const int sizeblock = sizeof(Primitive)+sizeof(Point)*2;
	Primitive	op;
	Point		p(x,y);
	Point		p1(x1,y1);

	op.operation = Primitive::CONIC_TO;
	op.number = 2;	//2 points for now

	if(lastbyteop == Primitive::CONIC_TO)
	{
		//only need to insert the new points
		bytestream.insert(bytestream.end(),(char*)&p1,(char*)(&p1+1));
		bytestream.insert(bytestream.end(),(char*)&p,(char*)(&p+1));

		Primitive * prim = (Primitive *)&bytestream[lastoppos];
		prim->number += 2; //increment number of points in the list
	}else
	{
		lastbyteop = Primitive::CONIC_TO;
		lastoppos = bytestream.size();

		bytestream.insert(bytestream.end(),(char*)&op,(char*)(&op+1)); 	//insert the bytes for the header
		bytestream.insert(bytestream.end(),(char*)&p1,(char*)(&p1+1));	//insert the bytes for data
		bytestream.insert(bytestream.end(),(char*)&p,(char*)(&p+1));	//insert the bytes for data
	}

	edge_table->conic_to(x1,y1,x,y);
}

void Layer_Shape::conic_to_smooth(Real x, Real y)				//x1,y1 derived from current tangent
{
	//const int sizeblock = sizeof(Primitive)+sizeof(Point);
	Primitive	op;
	Point		p(x,y);

	op.operation = Primitive::CONIC_TO_SMOOTH;
	op.number = 1;	//2 points for now

	if(lastbyteop == Primitive::CONIC_TO_SMOOTH)
	{
		//only need to insert the new point
		bytestream.insert(bytestream.end(),(char*)&p,(char*)(&p+1));

		Primitive * prim = (Primitive *)&bytestream[lastoppos];
		prim->number += 1; //increment number of points in the list
	}else
	{
		lastbyteop = Primitive::CONIC_TO_SMOOTH;
		lastoppos = bytestream.size();

		bytestream.insert(bytestream.end(),(char*)&op,(char*)(&op+1)); 	//insert the bytes for the header
		bytestream.insert(bytestream.end(),(char*)&p,(char*)(&p+1));	//insert the bytes for data
	}

	edge_table->conic_to_smooth(x,y);
}

void Layer_Shape::curve_to(Real x1, Real y1, Real x2, Real y2, Real x, Real y)
{
	//const int sizeblock = sizeof(Primitive)+sizeof(Point)*3;
	Primitive	op;
	Point		p(x,y);
	Point		p1(x1,y1);
	Point		p2(x2,y2);

	op.operation = Primitive::CUBIC_TO;
	op.number = 3;	//3 points for now

	if(lastbyteop == Primitive::CUBIC_TO)
	{
		//only need to insert the new points
		bytestream.insert(bytestream.end(),(char*)&p1,(char*)(&p1+1));
		bytestream.insert(bytestream.end(),(char*)&p2,(char*)(&p2+1));
		bytestream.insert(bytestream.end(),(char*)&p,(char*)(&p+1));

		Primitive * prim = (Primitive *)&bytestream[lastoppos];
		prim->number += 3; //increment number of points in the list
	}else
	{
		lastbyteop = Primitive::CUBIC_TO;
		lastoppos = bytestream.size();

		bytestream.insert(bytestream.end(),(char*)&op,(char*)(&op+1)); 	//insert the bytes for the header
		bytestream.insert(bytestream.end(),(char*)&p1,(char*)(&p1+1));	//insert the bytes for data
		bytestream.insert(bytestream.end(),(char*)&p2,(char*)(&p2+1));	//insert the bytes for data
		bytestream.insert(bytestream.end(),(char*)&p,(char*)(&p+1));	//insert the bytes for data
	}

	edge_table->curve_to(x1,y1,x2,y2,x,y);
}

void Layer_Shape::curve_to_smooth(Real x2, Real y2, Real x, Real y)		//x1,y1 derived from current tangent
{
	//const int sizeblock = sizeof(Primitive)+sizeof(Point)*3;
	Primitive	op;
	Point		p(x,y);
	Point		p2(x2,y2);

	op.operation = Primitive::CUBIC_TO_SMOOTH;
	op.number = 2;	//3 points for now

	if(lastbyteop == Primitive::CUBIC_TO_SMOOTH)
	{
		//only need to insert the new points
		bytestream.insert(bytestream.end(),(char*)&p2,(char*)(&p2+1));
		bytestream.insert(bytestream.end(),(char*)&p,(char*)(&p+1));

		Primitive * prim = (Primitive *)&bytestream[lastoppos];
		prim->number += 2; //increment number of points in the list
	}else
	{
		lastbyteop = Primitive::CUBIC_TO_SMOOTH;
		lastoppos = bytestream.size();

		bytestream.insert(bytestream.end(),(char*)&op,(char*)(&op+1)); 	//insert the bytes for the header
		bytestream.insert(bytestream.end(),(char*)&p2,(char*)(&p2+1));	//insert the bytes for data
		bytestream.insert(bytestream.end(),(char*)&p,(char*)(&p+1));	//insert the bytes for data
	}
}

// ACCELERATED RENDER FUNCTION - TRANSLATE BYTE CODE INTO FUNCTION CALLS

bool Layer_Shape::render_polyspan(Surface *surface, PolySpan &polyspan,
								Color::BlendMethod got_blend_method, Color::value_type got_amount) const
{
	Color color=param_color.get(Color());
	bool invert =param_invert.get(bool(true));
	bool antialias =param_antialias.get(bool(true));
	WindingStyle winding_style=param_winding_style.get(WINDING_NON_ZERO);

	Surface::alpha_pen p(surface->begin(),got_amount,got_blend_method);
	PolySpan::cover_array::iterator cur_mark = polyspan.covers.begin();
	PolySpan::cover_array::iterator end_mark = polyspan.covers.end();

	Real cover,area,alpha;

	int	y,x;

	p.set_value(color);
	cover = 0;

	if(cur_mark == end_mark)
	{
		//no marks at all
		if(invert)
		{
			p.move_to(polyspan.window.minx,polyspan.window.miny);
			p.put_block(polyspan.window.maxy - polyspan.window.miny,polyspan.window.maxx - polyspan.window.minx);
		}
		return true;
	}

	//fill initial rect / line
	if(invert)
	{
		//fill all the area above the first vertex
		p.move_to(polyspan.window.minx,polyspan.window.miny);
		y = polyspan.window.miny;
		int l = polyspan.window.maxx - polyspan.window.minx;

		p.put_block(cur_mark->y - polyspan.window.miny,l);

		//fill the area to the left of the first vertex on that line
		l = cur_mark->x - polyspan.window.minx;
		p.move_to(polyspan.window.minx,cur_mark->y);
		if(l) p.put_hline(l);
	}

	for(;;)
	{
		y = cur_mark->y;
		x = cur_mark->x;

		p.move_to(x,y);

		area = cur_mark->area;
		cover += cur_mark->cover;

		//accumulate for the current pixel
		while(++cur_mark != polyspan.covers.end())
		{
			if(y != cur_mark->y || x != cur_mark->x)
				break;

			area += cur_mark->area;
			cover += cur_mark->cover;
		}

		//draw pixel - based on covered area
		if(area)	//if we're ok, draw the current pixel
		{
			alpha = polyspan.ExtractAlpha(cover - area, winding_style);
			if(invert) alpha = 1 - alpha;

			if(!antialias)
			{
				if(alpha >= .5) p.put_value();
			}
			else if(alpha) p.put_value_alpha(alpha);

			p.inc_x();
			x++;
		}

		//if we're done, don't use iterator and exit
		if(cur_mark == end_mark) break;

		//if there is no more live pixels on this line, goto next
		if(y != cur_mark->y)
		{
			if(invert)
			{
				//fill the area at the end of the line
				p.put_hline(polyspan.window.maxx - x);

				//fill area at the beginning of the next line
				p.move_to(polyspan.window.minx,cur_mark->y);
				p.put_hline(cur_mark->x - polyspan.window.minx);
			}

			cover = 0;

			continue;
		}

		//draw span to next pixel - based on total amount of pixel cover
		if(x < cur_mark->x)
		{
			alpha = polyspan.ExtractAlpha(cover, winding_style);
			if(invert) alpha = 1 - alpha;

			if(!antialias)
			{
				if(alpha >= .5) p.put_hline(cur_mark->x - x);
			}
			else if(alpha) p.put_hline(cur_mark->x - x,alpha);
		}
	}

	//fill the after stuff
	if(invert)
	{
		//fill the area at the end of the line
		p.put_hline(polyspan.window.maxx - x);

		//fill area at the beginning of the next line
		p.move_to(polyspan.window.minx,y+1);
		p.put_block(polyspan.window.maxy - y - 1,polyspan.window.maxx - polyspan.window.minx);
	}

	return true;
}

bool Layer_Shape::render_polyspan(etl::surface<float> *surface, PolySpan &polyspan) const
{
	bool invert =param_invert.get(bool(true));
	bool antialias =param_antialias.get(bool(true));
	WindingStyle winding_style=param_winding_style.get(WINDING_NON_ZERO);

	etl::surface<float>::pen p(surface->begin());
	PolySpan::cover_array::iterator cur_mark = polyspan.covers.begin();
	PolySpan::cover_array::iterator end_mark = polyspan.covers.end();

	Real cover,area,alpha;

	int	y,x;

	cover = 0;

	//the pen always writes 1 (unless told to do otherwise)
	p.set_value(1);

	if(cur_mark == end_mark)
	{
		//no marks at all
		if(invert)
		{
			p.move_to(polyspan.window.minx,polyspan.window.miny);
			p.put_block(polyspan.window.maxy - polyspan.window.miny,polyspan.window.maxx - polyspan.window.minx);
		}
		return true;
	}

	//fill initial rect / line
	if(invert)
	{
		//fill all the area above the first vertex
		p.move_to(polyspan.window.minx,polyspan.window.miny);
		y = polyspan.window.miny;
		int l = polyspan.window.maxx - polyspan.window.minx;

		p.put_block(cur_mark->y - polyspan.window.miny,l);

		//fill the area to the left of the first vertex on that line
		l = cur_mark->x - polyspan.window.minx;
		p.move_to(polyspan.window.minx,cur_mark->y);
		if(l) p.put_hline(l);

		for(;;)
		{
			y = cur_mark->y;
			x = cur_mark->x;

			p.move_to(x,y);

			area = cur_mark->area;
			cover += cur_mark->cover;

			//accumulate for the current pixel
			while(++cur_mark != polyspan.covers.end())
			{
				if(y != cur_mark->y || x != cur_mark->x)
					break;

				area += cur_mark->area;
				cover += cur_mark->cover;
			}

			//draw pixel - based on covered area
			if(area)	//if we're ok, draw the current pixel
			{
				alpha = 1 - polyspan.ExtractAlpha(cover - area, winding_style);
				if(!antialias)
				{
					if(alpha >= .5) p.put_value();
				}
				else if(alpha) p.put_value(alpha);

				p.inc_x();
				x++;
			}

			//if we're done, don't use iterator and exit
			if(cur_mark == end_mark) break;

			//if there is no more live pixels on this line, goto next
			if(y != cur_mark->y)
			{
				//fill the area at the end of the line
				p.put_hline(polyspan.window.maxx - x);

				//fill area at the beginning of the next line
				p.move_to(polyspan.window.minx,cur_mark->y);
				p.put_hline(cur_mark->x - polyspan.window.minx);

				cover = 0;

				continue;
			}

			//draw span to next pixel - based on total amount of pixel cover
			if(x < cur_mark->x)
			{
				alpha = 1 - polyspan.ExtractAlpha(cover, winding_style);
				if(!antialias)
				{
					if(alpha >= .5) p.put_hline(cur_mark->x - x);
				}
				else if(alpha) p.put_hline(cur_mark->x - x,alpha);
			}
		}

		//fill the area at the end of the line
		p.put_hline(polyspan.window.maxx - x);

		//fill area at the beginning of the next line
		p.move_to(polyspan.window.minx,y+1);
		p.put_block(polyspan.window.maxy - y - 1,polyspan.window.maxx - polyspan.window.minx);
	}else
	{
		for(;;)
		{
			y = cur_mark->y;
			x = cur_mark->x;

			p.move_to(x,y);

			area = cur_mark->area;
			cover += cur_mark->cover;

			//accumulate for the current pixel
			while(++cur_mark != polyspan.covers.end())
			{
				if(y != cur_mark->y || x != cur_mark->x)
					break;

				area += cur_mark->area;
				cover += cur_mark->cover;
			}

			//draw pixel - based on covered area
			if(area)	//if we're ok, draw the current pixel
			{
				alpha = polyspan.ExtractAlpha(cover - area, winding_style);
				if(!antialias)
				{
					if(alpha >= .5) p.put_value();
				}
				else if(alpha) p.put_value(alpha);

				p.inc_x();
				x++;
			}

			//if we're done, don't use iterator and exit
			if(cur_mark == end_mark) break;

			//if there is no more live pixels on this line, goto next
			if(y != cur_mark->y)
			{
				cover = 0;

				continue;
			}

			//draw span to next pixel - based on total amount of pixel cover
			if(x < cur_mark->x)
			{
				alpha = polyspan.ExtractAlpha(cover, winding_style);
				if(!antialias)
				{
					if(alpha >= .5) p.put_hline(cur_mark->x - x);
				}
				else if(alpha) p.put_hline(cur_mark->x - x,alpha);
			}
		}
	}

	return true;
}

bool
Layer_Shape::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	Color color=param_color.get(Color());
	Point origin=param_origin.get(Point());
	bool invert =param_invert.get(bool(true));
	int blurtype=param_blurtype.get(int());
	Real feather=param_feather.get(Real());

	const unsigned int w = renddesc.get_w();
	const unsigned int h = renddesc.get_h();

	const Real pw = abs(renddesc.get_pw());
	const Real ph = abs(renddesc.get_ph());

	//const Real OFFSET_EPSILON = 1e-8;
	SuperCallback stageone(cb,1,10000,15001+renddesc.get_h());
	SuperCallback stagetwo(cb,10000,10001+renddesc.get_h(),15001+renddesc.get_h());
	SuperCallback stagethree(cb,10001+renddesc.get_h(),15001+renddesc.get_h(),15001+renddesc.get_h());

	// Render what is behind us

	//clip if it satisfies the invert solid thing
	if(is_solid_color() && invert)
	{
		Rect aabb = edge_table->aabb;
		Point tl = renddesc.get_tl() - origin;

		Real	pw = renddesc.get_pw(),
				ph = renddesc.get_ph();

		Rect	nrect;

		Real	pixelfeatherx = quality == 10 ? 0 : abs(feather/pw),
				pixelfeathery = quality == 10 ? 0 : abs(feather/ph);

		nrect.set_point((aabb.minx - tl[0])/pw,(aabb.miny - tl[1])/ph);
		nrect.expand((aabb.maxx - tl[0])/pw,(aabb.maxy - tl[1])/ph);

		RendDesc	optdesc(renddesc);

		//make sure to expand so we gain subpixels rather than lose them
		nrect.minx = floor(nrect.minx-pixelfeatherx); nrect.miny = floor(nrect.miny-pixelfeathery);
		nrect.maxx = ceil(nrect.maxx+pixelfeatherx); nrect.maxy = ceil(nrect.maxy+pixelfeathery);

		//make sure the subwindow is clipped with our tile window (minimize useless drawing)
		set_intersect(nrect,nrect,Rect(0,0,renddesc.get_w(),renddesc.get_h()));

		//must resize the surface first
		surface->set_wh(renddesc.get_w(),renddesc.get_h());
		surface->clear();

		//only render anything if it's visible from our current tile
		if(nrect.valid())
		{
			//set the subwindow to the viewable pixels and render it to the subsurface
			optdesc.set_subwindow((int)nrect.minx, (int)nrect.miny,
				(int)(nrect.maxx - nrect.minx), (int)(nrect.maxy - nrect.miny));

			Surface	optimizedbacksurf;
			if(!context.accelerated_render(&optimizedbacksurf,quality,optdesc,&stageone))
				return false;

			//blit that onto the original surface so we can pretend that nothing ever happened
			Surface::pen p = surface->get_pen((int)nrect.minx,(int)nrect.miny);
			optimizedbacksurf.blit_to(p);
		}
	}else
	{
		if(!context.accelerated_render(surface,quality,renddesc,&stageone))
			return false;
	}

	if(cb && !cb->amount_complete(10000,10001+renddesc.get_h())) return false;

	if(feather && quality != 10)
	{
		//we have to blur rather than be crappy

		//so make a separate surface
		RendDesc	workdesc(renddesc);

		etl::surface<float>	shapesurface;

		//the expanded size = 1/2 the size in each direction rounded up
		int	halfsizex = (int) (abs(feather*.5/pw) + 3),
			halfsizey = (int) (abs(feather*.5/ph) + 3);

		//expand by 1/2 size in each direction on either side
		switch(blurtype)
		{
			case Blur::DISC:
			case Blur::BOX:
			case Blur::CROSS:
			{
				workdesc.set_subwindow(-max(1,halfsizex),-max(1,halfsizey),w+2*max(1,halfsizex),h+2*max(1,halfsizey));
				break;
			}
			case Blur::FASTGAUSSIAN:
			{
				if(quality < 4)
				{
					halfsizex*=2;
					halfsizey*=2;
				}
				workdesc.set_subwindow(-max(1,halfsizex),-max(1,halfsizey),w+2*max(1,halfsizex),h+2*max(1,halfsizey));
				break;
			}
			case Blur::GAUSSIAN:
			{
			#define GAUSSIAN_ADJUSTMENT		(0.05)
				Real	pw = (Real)workdesc.get_w()/(workdesc.get_br()[0]-workdesc.get_tl()[0]);
				Real 	ph = (Real)workdesc.get_h()/(workdesc.get_br()[1]-workdesc.get_tl()[1]);

				pw=pw*pw;
				ph=ph*ph;

				halfsizex = (int)(abs(pw)*feather*GAUSSIAN_ADJUSTMENT+0.5);
				halfsizey = (int)(abs(ph)*feather*GAUSSIAN_ADJUSTMENT+0.5);

				halfsizex = (halfsizex + 1)/2;
				halfsizey = (halfsizey + 1)/2;
				workdesc.set_subwindow( -halfsizex, -halfsizey, w+2*halfsizex, h+2*halfsizey );

				break;
			}
		}

		shapesurface.set_wh(workdesc.get_w(),workdesc.get_h());
		shapesurface.clear();

		//render the shape
		if(!render_shape(&shapesurface,quality,workdesc,&stagetwo))return false;

		//blur the image
		Blur(feather,feather,blurtype,&stagethree)(shapesurface,workdesc.get_br()-workdesc.get_tl(),shapesurface);

		//blend with stuff below it...
		unsigned int u = halfsizex, v = halfsizey, x = 0, y = 0;
		for(y = 0; y < h; y++,v++)
		{
			u = halfsizex;
			for(x = 0; x < w; x++,u++)
			{
				float a = shapesurface[v][u];
				if(a)
				{
					//a = floor(a*255+0.5f)/255;
					(*surface)[y][x]=Color::blend(color,(*surface)[y][x],a*get_amount(),get_blend_method());
				}
				//else (*surface)[y][x] = worksurface[v][u];
			}
		}

		//we are done
		if(cb && !cb->amount_complete(100,100))
		{
			synfig::warning("Layer_Shape: could not set amount complete");
			return false;
		}

		return true;
	}else
	{
		//might take out to reduce code size
		return render_shape(surface,true,quality,renddesc,&stagetwo);
	}

}

////
bool
Layer_Shape::accelerated_cairorender(Context context,cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	Color color=param_color.get(Color());
	Point origin=param_origin.get(Point());
	bool invert =param_invert.get(bool(true));
	bool antialias =param_antialias.get(bool(true));
	int blurtype=param_blurtype.get(int());
	Real feather=param_feather.get(Real());
	WindingStyle winding_style=param_winding_style.get(WINDING_NON_ZERO);

	// Grab the rgba values
	const float r(color.get_r());
	const float g(color.get_g());
	const float b(color.get_b());
	const float a(color.get_a());
	
	// First render the context
	if(!is_solid_color())
		if(!context.accelerated_cairorender(cr,quality,renddesc,cb))
		{
			if(cb)
				cb->error(strprintf(__FILE__"%d: Accelerated Cairo Renderer Failure",__LINE__));
			return false;
		}
	
	if(feather && quality != 10)
	{
		cairo_surface_t* subimage;
		RendDesc	workdesc(renddesc);
		
		// Untransform the render desc
		if(!cairo_renddesc_untransform(cr, workdesc))
			return false;
		
		int halfsizex(0), halfsizey(0);
		
		int w=workdesc.get_w(), h=workdesc.get_h();
		const double wpw=(workdesc.get_br()[0]-workdesc.get_tl()[0])/w;
		const double wph=(workdesc.get_br()[1]-workdesc.get_tl()[1])/h;
		//the expanded size = 1/2 the size in each direction rounded up
		halfsizex = (int) (abs(feather*.5/wpw) + 3),
		halfsizey = (int) (abs(feather*.5/wph) + 3);
		
		//expand by 1/2 size in each direction on either side
		switch(blurtype)
		{
			case Blur::DISC:
			case Blur::BOX:
			case Blur::CROSS:
			{
				workdesc.set_subwindow(-max(1,halfsizex),-max(1,halfsizey),w+2*max(1,halfsizex),h+2*max(1,halfsizey));
				break;
			}
			case Blur::FASTGAUSSIAN:
			{
				if(quality < 4)
				{
					halfsizex*=2;
					halfsizey*=2;
				}
				workdesc.set_subwindow(-max(1,halfsizex),-max(1,halfsizey),w+2*max(1,halfsizex),h+2*max(1,halfsizey));
				break;
			}
			case Blur::GAUSSIAN:
			{
#define GAUSSIAN_ADJUSTMENT		(0.05)
				Real	pw = (Real)workdesc.get_w()/(workdesc.get_br()[0]-workdesc.get_tl()[0]);
				Real 	ph = (Real)workdesc.get_h()/(workdesc.get_br()[1]-workdesc.get_tl()[1]);
				
				pw=pw*pw;
				ph=ph*ph;
				
				halfsizex = (int)(abs(pw)*feather*GAUSSIAN_ADJUSTMENT+0.5);
				halfsizey = (int)(abs(ph)*feather*GAUSSIAN_ADJUSTMENT+0.5);
				
				halfsizex = (halfsizex + 1)/2;
				halfsizey = (halfsizey + 1)/2;
				workdesc.set_subwindow( -halfsizex, -halfsizey, w+2*halfsizex, h+2*halfsizey );
				break;
#undef GAUSSIAN_ADJUSTMENT
			}
		}
		
		// New expanded workdesc values
		const int ww=workdesc.get_w();
		const int wh=workdesc.get_h();
		const double wtlx=workdesc.get_tl()[0];
		const double wtly=workdesc.get_tl()[1];
		subimage=cairo_surface_create_similar(cairo_get_target(cr), CAIRO_CONTENT_COLOR_ALPHA, ww, wh);
		cairo_t* subcr=cairo_create(subimage);
		cairo_scale(subcr, 1/wpw, 1/wph);
		cairo_translate(subcr, -wtlx, -wtly);
		cairo_translate(subcr, origin[0], origin[1]);
		switch(winding_style)
		{
			case WINDING_NON_ZERO:
				cairo_set_fill_rule(subcr, CAIRO_FILL_RULE_WINDING);
				break;
			default:
				cairo_set_fill_rule(subcr, CAIRO_FILL_RULE_EVEN_ODD);
				break;
		}
		
		cairo_set_source_rgba(subcr, r, g, b, a);
		if(invert)
			cairo_paint(subcr);
		// Draw the shape
		if(!antialias)
			cairo_set_antialias(subcr, CAIRO_ANTIALIAS_NONE);
		if(invert)
			cairo_set_operator(subcr, CAIRO_OPERATOR_CLEAR);
		else
			cairo_set_operator(subcr, CAIRO_OPERATOR_OVER);
		
		Layer_Shape::shape_to_cairo(subcr);
		cairo_clip(subcr);
		cairo_paint(subcr);
		
		if(!feather_cairo_surface(subimage, workdesc, quality))
		{
			cairo_surface_destroy(subimage);
			cairo_destroy(subcr);
			return false;
		}
		cairo_destroy(subcr);
		
		cairo_save(cr);
		cairo_translate(cr, wtlx, wtly);
		cairo_scale(cr, wpw, wph);
		cairo_set_source_surface(cr, subimage, 0, 0);
		cairo_paint_with_alpha_operator(cr, get_amount(), get_blend_method());
		cairo_restore(cr);
		cairo_surface_destroy(subimage);
		return true;
	}
	cairo_save(cr);
	cairo_translate(cr, origin[0], origin[1]);
	cairo_set_source_rgba(cr, r, g, b, a);
	switch(winding_style)
	{
		case WINDING_NON_ZERO:
			cairo_set_fill_rule(cr, CAIRO_FILL_RULE_WINDING);
			break;
		default:
			cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
			break;
	}
	if(!antialias)
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	if(invert)
	{
		cairo_push_group(cr);
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
		cairo_set_source_rgba(cr, r, g, b, a);
		cairo_paint(cr);
		Layer_Shape::shape_to_cairo(cr);
		cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
		cairo_clip(cr);
		cairo_paint(cr);
		cairo_pop_group_to_source(cr);
		cairo_paint_with_alpha_operator(cr, get_amount(), get_blend_method());
	}
	else
	{
		Layer_Shape::shape_to_cairo(cr);
		cairo_clip(cr);
		cairo_paint_with_alpha_operator(cr, get_amount(), get_blend_method());
	}
	cairo_restore(cr);
	return true;
}
////


//
bool
Layer_Shape::shape_to_cairo(cairo_t *cr)const
{
	int tmp(0);
	//pointers for processing the bytestream
	const char *current 	= &bytestream[0];
	const char *end			= &bytestream[bytestream.size()];
	int	operation 	= Primitive::NONE;
	int number		= 0;
	int curnum;
	
	Primitive 	*curprim;
	Point		*data;
	
	Real x,y/*,x1,y1,x2,y2*/;
	while(current < end)
	{
		tmp++;
		
		try {
			
			//get the op code safely
			curprim = (Primitive *)current;
			
			//advance past indices
			current += sizeof(Primitive);
			if(current > end)
			{
				warning("Layer_Shape::accelerated_render - Error in the byte stream, not enough space for next declaration");
				return false;
			}
			
			//get the relevant data
			operation 	= curprim->operation;
			number 		= curprim->number;
			
			if(operation == Primitive::END)
				break;
			
			if(operation == Primitive::CLOSE)
			{

				cairo_close_path(cr);
				continue;
			}
			
			data = (Point*)current;
			current += sizeof(Point)*number;
			
			//check data positioning
			if(current > end)
			{
				warning("Layer_Shape::accelerated_render - Error in the byte stream, in sufficient data space for declared number of points");
				return false;
			}
			
		} catch(...) { synfig::error("Layer_Shape::render_shape()1: Caught an exception after %d loops, rethrowing...", tmp); throw; }
		
		//transfer all the data - RLE optimized
		for(curnum=0; curnum < number;)
		{
			switch(operation)
			{
				case Primitive::MOVE_TO:
				{
					x = data[curnum][0];
					y = data[curnum][1];
					
					if(curnum == 0)
					{
						cairo_move_to(cr, x, y);
					}
					else
					{
						cairo_line_to(cr, x, y);
					}
					
					curnum++; //only advance one point
					
					break;
				}
					
				case Primitive::LINE_TO:
				{
					x = data[curnum][0];
					y = data[curnum][1];

					cairo_line_to(cr, x, y);
					synfig::info("line to x,y = %f, %f", x, y);

					curnum++;
					break;
				}
//					
//				case Primitive::CONIC_TO:
//				{
//					x = data[curnum+1][0];
//					x = (x - tl[0] + origin[0])*pw;
//					y = data[curnum+1][1];
//					y = (y - tl[1] + origin[1])*ph;
//					
//					x1 = data[curnum][0];
//					x1 = (x1 - tl[0] + origin[0])*pw;
//					y1 = data[curnum][1];
//					y1 = (y1 - tl[1] + origin[1])*ph;
//					
//					tangent[0] = 2*(x - x1);
//					tangent[1] = 2*(y - y1);
//					
//					span.conic_to(x1,y1,x,y);
//					curnum += 2;
//					break;
//				}
//					
//				case Primitive::CONIC_TO_SMOOTH:
//				{
//					x = data[curnum][0];
//					x = (x - tl[0] + origin[0])*pw;
//					y = data[curnum][1];
//					y = (y - tl[1] + origin[1])*ph;
//					
//					x1 = span.cur_x + tangent[0]/2;
//					y1 = span.cur_y + tangent[1]/2;
//					
//					tangent[0] = 2*(x - x1);
//					tangent[1] = 2*(y - y1);
//					
//					span.conic_to(x1,y1,x,y);
//					curnum ++;
//					
//					break;
//				}
//					
//				case Primitive::CUBIC_TO:
//				{
//					x = data[curnum+2][0];
//					x = (x - tl[0] + origin[0])*pw;
//					y = data[curnum+2][1];
//					y = (y - tl[1] + origin[1])*ph;
//					
//					x2 = data[curnum+1][0];
//					x2 = (x2 - tl[0] + origin[0])*pw;
//					y2 = data[curnum+1][1];
//					y2 = (y2 - tl[1] + origin[1])*ph;
//					
//					x1 = data[curnum][0];
//					x1 = (x1 - tl[0] + origin[0])*pw;
//					y1 = data[curnum][1];
//					y1 = (y1 - tl[1] + origin[1])*ph;
//					
//					tangent[0] = 2*(x - x2);
//					tangent[1] = 2*(y - y2);
//					
//					span.cubic_to(x1,y1,x2,y2,x,y);
//					curnum += 3;
//					
//					break;
//				}
//					
//				case Primitive::CUBIC_TO_SMOOTH:
//				{
//					x = data[curnum+1][0];
//					x = (x - tl[0] + origin[0])*pw;
//					y = data[curnum+1][1];
//					y = (y - tl[1] + origin[1])*ph;
//					
//					x2 = data[curnum][0];
//					x2 = (x2 - tl[0] + origin[0])*pw;
//					y2 = data[curnum][1];
//					y2 = (y2 - tl[1] + origin[1])*ph;
//					
//					x1 = span.cur_x + tangent[0]/3.0;
//					y1 = span.cur_y + tangent[1]/3.0;
//					
//					tangent[0] = 2*(x - x2);
//					tangent[1] = 2*(y - y2);
//					
//					span.cubic_to(x1,y1,x2,y2,x,y);
//					curnum += 2;
//					
//					break;
//				}
			} // switch
		} // for
	} // while
	
	return true;
}
//

bool
Layer_Shape::feather_cairo_surface(cairo_surface_t* surface, RendDesc renddesc, int quality)const
{
	Color color=param_color.get(Color());
	int blurtype=param_blurtype.get(int());
	Real feather=param_feather.get(Real());

	if(feather && quality!=10)
	{
		etl::surface<float>	shapesurface;
		shapesurface.set_wh(renddesc.get_w(),renddesc.get_h());
		shapesurface.clear();
		
		CairoSurface cairosurface(surface);
		if(!cairosurface.map_cairo_image())
		{
			synfig::info("map cairo image failed");
			return false;
		}
		// Extract the alpha values:
		int x, y;
		int h(renddesc.get_h()), w(renddesc.get_w());
		float div=1.0/((float)(CairoColor::ceil));
		for(y=0; y<h; y++)
			for(x=0;x<w;x++)
				shapesurface[y][x]=cairosurface[y][x].get_a()*div;
		// Blue the alpha values
		Blur(feather,feather,blurtype)(shapesurface, renddesc.get_br()-renddesc.get_tl(), shapesurface);
		// repaint the cairosurface with the result
		Color ccolor(color);
		for(y=0; y<h; y++)
			for(x=0;x<w;x++)
			{
				float a=shapesurface[y][x];
				ccolor.set_a(a);
				ccolor.clamped();
				cairosurface[y][x]=CairoColor(ccolor).premult_alpha();
			}
		
		cairosurface.unmap_cairo_image();
	}
	return true;
}


bool
Layer_Shape::render_shape(Surface *surface,bool useblend,int /*quality*/,
							const RendDesc &renddesc, ProgressCallback *cb)const
{
	Point origin=param_origin.get(Point());

	int tmp(0);

	SuperCallback	progress(cb,0,renddesc.get_h(),renddesc.get_h());

	// If our amount is set to zero, no need to render anything
	if(!get_amount())
		return true;

	//test new polygon renderer
	// Build edge table
	// Width and Height of a pixel
	const int 	w = renddesc.get_w();
	const int	h = renddesc.get_h();
	const Real	pw = renddesc.get_w()/(renddesc.get_br()[0]-renddesc.get_tl()[0]);
	const Real	ph = renddesc.get_h()/(renddesc.get_br()[1]-renddesc.get_tl()[1]);

	const Point	tl = renddesc.get_tl();

	Vector tangent (0,0);

	PolySpan	span;

	// if the pixels are zero sized then we're too zoomed out to see anything
	if (pw == 0 || ph == 0)
		return true;

	//optimization for tessellating only inside tiles
	span.window.minx = 0;
	span.window.miny = 0;
	span.window.maxx = w;
	span.window.maxy = h;

	//pointers for processing the bytestream
	const char *current 	= &bytestream[0];
	const char *end			= &bytestream[bytestream.size()];

	int	operation 	= Primitive::NONE;
	int number		= 0;
	int curnum;

	Primitive 	*curprim;
	Point		*data;

	Real x,y,x1,y1,x2,y2;


	while(current < end)
	{
		tmp++;

		try {

		//get the op code safely
		curprim = (Primitive *)current;

		//advance past indices
		current += sizeof(Primitive);
		if(current > end)
		{
			warning("Layer_Shape::accelerated_render - Error in the byte stream, not enough space for next declaration");
			return false;
		}

		//get the relevant data
		operation 	= curprim->operation;
		number 		= curprim->number;

		if(operation == Primitive::END)
			break;

		if(operation == Primitive::CLOSE)
		{
			if(span.notclosed())
			{
				tangent[0] = span.close_x - span.cur_x;
				tangent[1] = span.close_y - span.cur_y;
				span.close();
			}
			continue;
		}

		data = (Point*)current;
		current += sizeof(Point)*number;

		//check data positioning
		if(current > end)
		{
			warning("Layer_Shape::accelerated_render - Error in the byte stream, in sufficient data space for declared number of points");
			return false;
		}

		} catch(...) { synfig::error("Layer_Shape::render_shape()1: Caught an exception after %d loops, rethrowing...", tmp); throw; }

		//transfer all the data - RLE optimized
		for(curnum=0; curnum < number;)
		{
			switch(operation)
			{
				case Primitive::MOVE_TO:
				{
					x = data[curnum][0];
					x = (x - tl[0] + origin[0])*pw;
					y = data[curnum][1];
					y = (y - tl[1] + origin[1])*ph;

					if(curnum == 0)
					{
						span.move_to(x,y);

						tangent[0] = 0;
						tangent[1] = 0;
					}
					else
					{
						tangent[0] = x - span.cur_x;
						tangent[1] = y - span.cur_y;

						span.line_to(x,y);
					}

					curnum++; //only advance one point

					break;
				}

				case Primitive::LINE_TO:
				{
					x = data[curnum][0];
					x = (x - tl[0] + origin[0])*pw;
					y = data[curnum][1];
					y = (y - tl[1] + origin[1])*ph;

					tangent[0] = x - span.cur_x;
					tangent[1] = y - span.cur_y;

					span.line_to(x,y);
					curnum++;
					break;
				}

				case Primitive::CONIC_TO:
				{
					x = data[curnum+1][0];
					x = (x - tl[0] + origin[0])*pw;
					y = data[curnum+1][1];
					y = (y - tl[1] + origin[1])*ph;

					x1 = data[curnum][0];
					x1 = (x1 - tl[0] + origin[0])*pw;
					y1 = data[curnum][1];
					y1 = (y1 - tl[1] + origin[1])*ph;

					tangent[0] = 2*(x - x1);
					tangent[1] = 2*(y - y1);

					span.conic_to(x1,y1,x,y);
					curnum += 2;
					break;
				}

				case Primitive::CONIC_TO_SMOOTH:
				{
					x = data[curnum][0];
					x = (x - tl[0] + origin[0])*pw;
					y = data[curnum][1];
					y = (y - tl[1] + origin[1])*ph;

					x1 = span.cur_x + tangent[0]/2;
					y1 = span.cur_y + tangent[1]/2;

					tangent[0] = 2*(x - x1);
					tangent[1] = 2*(y - y1);

					span.conic_to(x1,y1,x,y);
					curnum ++;

					break;
				}

				case Primitive::CUBIC_TO:
				{
					x = data[curnum+2][0];
					x = (x - tl[0] + origin[0])*pw;
					y = data[curnum+2][1];
					y = (y - tl[1] + origin[1])*ph;

					x2 = data[curnum+1][0];
					x2 = (x2 - tl[0] + origin[0])*pw;
					y2 = data[curnum+1][1];
					y2 = (y2 - tl[1] + origin[1])*ph;

					x1 = data[curnum][0];
					x1 = (x1 - tl[0] + origin[0])*pw;
					y1 = data[curnum][1];
					y1 = (y1 - tl[1] + origin[1])*ph;

					tangent[0] = 2*(x - x2);
					tangent[1] = 2*(y - y2);

					span.cubic_to(x1,y1,x2,y2,x,y);
					curnum += 3;

					break;
				}

				case Primitive::CUBIC_TO_SMOOTH:
				{
					x = data[curnum+1][0];
					x = (x - tl[0] + origin[0])*pw;
					y = data[curnum+1][1];
					y = (y - tl[1] + origin[1])*ph;

					x2 = data[curnum][0];
					x2 = (x2 - tl[0] + origin[0])*pw;
					y2 = data[curnum][1];
					y2 = (y2 - tl[1] + origin[1])*ph;

					x1 = span.cur_x + tangent[0]/3.0;
					y1 = span.cur_y + tangent[1]/3.0;

					tangent[0] = 2*(x - x2);
					tangent[1] = 2*(y - y2);

					span.cubic_to(x1,y1,x2,y2,x,y);
					curnum += 2;

					break;
				}
			}
		}
	}

	//sort the bastards so we can render everything
	span.sort_marks();

	return render_polyspan(surface, span,
			useblend?get_blend_method():Color::BLEND_STRAIGHT,
			useblend?get_amount():1.0);
}

bool
Layer_Shape::render_shape(etl::surface<float> *surface,int /*quality*/,
							const RendDesc &renddesc, ProgressCallback */*cb*/)const
{
	Point origin=param_origin.get(Point());
	// If our amount is set to zero, no need to render anything
	if(!get_amount())
		return true;

	//test new polygon renderer
	// Build edge table
	// Width and Height of a pixel
	const int 	w = renddesc.get_w();
	const int	h = renddesc.get_h();
	const Real	pw = renddesc.get_w()/(renddesc.get_br()[0]-renddesc.get_tl()[0]);
	const Real	ph = renddesc.get_h()/(renddesc.get_br()[1]-renddesc.get_tl()[1]);

	const Point	tl = renddesc.get_tl();

	Vector tangent (0,0);

	PolySpan	span;

	//optimization for tessellating only inside tiles
	span.window.minx = 0;
	span.window.miny = 0;
	span.window.maxx = w;
	span.window.maxy = h;

	//pointers for processing the bytestream
	const char *current 	= &bytestream[0];
	const char *end			= &bytestream[bytestream.size()];

	int	operation 	= Primitive::NONE;
	int number		= 0;
	int curnum;

	Primitive 	*curprim;
	Point		*data;

	Real x,y,x1,y1,x2,y2;

	while(current < end)
	{
		//get the op code safely
		curprim = (Primitive *)current;

		//advance past indices
		current += sizeof(Primitive);
		if(current > end)
		{
			warning("Layer_Shape::accelerated_render - Error in the byte stream, not enough space for next declaration");
			return false;
		}

		//get the relevant data
		operation 	= curprim->operation;
		number 		= curprim->number;

		if(operation == Primitive::END)
			break;

		if(operation == Primitive::CLOSE)
		{
			if(span.notclosed())
			{
				tangent[0] = span.close_x - span.cur_x;
				tangent[1] = span.close_y - span.cur_y;
				span.close();
			}
			continue;
		}

		data = (Point*)current;
		current += sizeof(Point)*number;

		//check data positioning
		if(current > end)
		{
			warning("Layer_Shape::accelerated_render - Error in the byte stream, in sufficient data space for declared number of points");
			return false;
		}

		//transfer all the data
		for(curnum=0; curnum < number;)
		{
			switch(operation)
			{
				case Primitive::MOVE_TO:
				{
					x = data[curnum][0];
					x = (x - tl[0] + origin[0])*pw;
					y = data[curnum][1];
					y = (y - tl[1] + origin[1])*ph;

					if(curnum == 0)
					{
						span.move_to(x,y);

						tangent[0] = 0;
						tangent[1] = 0;
					}
					else
					{
						tangent[0] = x - span.cur_x;
						tangent[1] = y - span.cur_y;

						span.line_to(x,y);
					}

					curnum++; //only advance one point

					break;
				}

				case Primitive::LINE_TO:
				{
					x = data[curnum][0];
					x = (x - tl[0] + origin[0])*pw;
					y = data[curnum][1];
					y = (y - tl[1] + origin[1])*ph;

					tangent[0] = x - span.cur_x;
					tangent[1] = y - span.cur_y;

					span.line_to(x,y);
					curnum++;
					break;
				}

				case Primitive::CONIC_TO:
				{
					x = data[curnum+1][0];
					x = (x - tl[0] + origin[0])*pw;
					y = data[curnum+1][1];
					y = (y - tl[1] + origin[1])*ph;

					x1 = data[curnum][0];
					x1 = (x1 - tl[0] + origin[0])*pw;
					y1 = data[curnum][1];
					y1 = (y1 - tl[1] + origin[1])*ph;

					tangent[0] = 2*(x - x1);
					tangent[1] = 2*(y - y1);

					span.conic_to(x1,y1,x,y);
					curnum += 2;
					break;
				}

				case Primitive::CONIC_TO_SMOOTH:
				{
					x = data[curnum][0];
					x = (x - tl[0] + origin[0])*pw;
					y = data[curnum][1];
					y = (y - tl[1] + origin[1])*ph;

					x1 = span.cur_x + tangent[0]/2;
					y1 = span.cur_y + tangent[1]/2;

					tangent[0] = 2*(x - x1);
					tangent[1] = 2*(y - y1);

					span.conic_to(x1,y1,x,y);
					curnum ++;

					break;
				}

				case Primitive::CUBIC_TO:
				{
					x = data[curnum+2][0];
					x = (x - tl[0] + origin[0])*pw;
					y = data[curnum+2][1];
					y = (y - tl[1] + origin[1])*ph;

					x2 = data[curnum+1][0];
					x2 = (x2 - tl[0] + origin[0])*pw;
					y2 = data[curnum+1][1];
					y2 = (y2 - tl[1] + origin[1])*ph;

					x1 = data[curnum][0];
					x1 = (x1 - tl[0] + origin[0])*pw;
					y1 = data[curnum][1];
					y1 = (y1 - tl[1] + origin[1])*ph;

					tangent[0] = 2*(x - x2);
					tangent[1] = 2*(y - y2);

					span.cubic_to(x1,y1,x2,y2,x,y);
					curnum += 3;

					break;
				}

				case Primitive::CUBIC_TO_SMOOTH:
				{
					x = data[curnum+1][0];
					x = (x - tl[0] + origin[0])*pw;
					y = data[curnum+1][1];
					y = (y - tl[1] + origin[1])*ph;

					x2 = data[curnum][0];
					x2 = (x2 - tl[0] + origin[0])*pw;
					y2 = data[curnum][1];
					y2 = (y2 - tl[1] + origin[1])*ph;

					x1 = span.cur_x + tangent[0]/3.0;
					y1 = span.cur_y + tangent[1]/3.0;

					tangent[0] = 2*(x - x2);
					tangent[1] = 2*(y - y2);

					span.cubic_to(x1,y1,x2,y2,x,y);
					curnum += 2;

					break;
				}
			}
		}
	}

	//sort the bastards so we can render everything
	span.sort_marks();

	return render_polyspan(surface, span);
}

Rect
Layer_Shape::get_bounding_rect()const
{
	Point origin=param_origin.get(Point());
	bool invert =param_invert.get(bool(true));
	Real feather=param_feather.get(Real());

	if(invert)
		return Rect::full_plane();

	if (edge_table->initaabb)
		return Rect::zero();

	Rect bounds(edge_table->aabb+origin);
	bounds.expand(max((bounds.get_min() - bounds.get_max()).mag()*0.01,
					  feather));

	return bounds;
}
