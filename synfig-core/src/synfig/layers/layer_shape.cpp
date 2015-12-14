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

#include <cfloat>

#include <deque>
#include <vector>

#include "layer_shape.h"

#include <synfig/general.h>
#include <synfig/localization.h>

#include <synfig/blur.h>
#include <synfig/context.h>
#include <synfig/curve_helper.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/string.h>
#include <synfig/surface.h>
#include <synfig/time.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#include <synfig/rendering/common/task/taskblend.h>
#include <synfig/rendering/common/task/taskcontour.h>
#include <synfig/rendering/software/function/contour.h>

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

	int size() const
	{
		return degrees.size();
	}

	void start(Point m)
	{
		reset(m[0],m[0],m[1],m[1]);
		pointlist.push_back(m);
	}

	void add_cubic(Point dest, Point p1, Point p2)
	{
		aabb.expand(p1[0],p1[1]);
		aabb.expand(p2[0],p2[1]);
		aabb.expand(dest[0],dest[1]);

		pointlist.push_back(p1);
		pointlist.push_back(p2);
		pointlist.push_back(dest);

		degrees.push_back(3);
	}

	void add_conic(Point dest, Point p1)
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

	static int quadratic_eqn(Real a, Real b, Real c, Real *t0, Real *t1)
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

		conic_to(x,y,x1,y1);
	}

	void conic_to(Real x, Real y, Real x1, Real y1)
	{
		//if we're not already a curve start one
		if(prim != TYPE_CURVE)
		{
			CurveArray	c;

			c.start(Point(cur_x,cur_y));
			c.add_conic(Point(x,y),Point(x1,y1));

			curves.push_back(c);
		}else
		{
			curves.back().add_conic(Point(x,y),Point(x1,y1));
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

	void curve_to_smooth(Real x, Real y, Real x2, Real y2)
	{
		Real x1 = tangent[0]/3.0 + cur_x;
		Real y1 = tangent[1]/3.0 + cur_y;

		cubic_to(x,y,x1,y1,x2,y2);
	}

	void cubic_to(Real x, Real y,Real x1, Real y1, Real x2, Real y2)
	{
		//if we're not already a curve start one
		if(prim != TYPE_CURVE)
		{
			CurveArray	c;

			c.start(Point(cur_x,cur_y));
			c.add_cubic(Point(x,y),Point(x1,y1),Point(x2,y2));

			curves.push_back(c);
		}else
		{
			curves.back().add_cubic(Point(x,y),Point(x1,y1),Point(x2,y2));
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
	int	intersect(Real x, Real y) const
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

/* === M E T H O D S ======================================================= */

Layer_Shape::Layer_Shape(const Real &a, const Color::BlendMethod m):
	Layer_Composite      (a, m),
	param_color          (Color::black()),
	param_origin         (Vector(0,0)),
	param_invert         (bool(false)),
	param_antialias      (bool(true)),
	param_blurtype       (int(Blur::FASTGAUSSIAN)),
	param_feather        (Real(0.0)),
	param_winding_style	 (int(rendering::Contour::WINDING_NON_ZERO)),
	edge_table	         (new Intersector),
	contour				 (new rendering::Contour)
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
	contour->clear();
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
		.add_enum_value(rendering::Contour::WINDING_NON_ZERO, "nonzero", _("Non Zero"))
		.add_enum_value(rendering::Contour::WINDING_EVEN_ODD, "evenodd", _("Even/Odd"))
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

void Layer_Shape::move_to(Real x, Real y)
{
	contour->move_to(Vector(x, y));
	edge_table->move_to(x, y);
}

void Layer_Shape::close()
{
	contour->close();
	edge_table->close();
}

void Layer_Shape::line_to(Real x, Real y)
{
	contour->line_to(Vector(x, y));
	edge_table->line_to(x,y);
}

void Layer_Shape::conic_to(Real x, Real y, Real x1, Real y1)
{
	contour->conic_to(Vector(x, y), Vector(x1, y1));
	edge_table->conic_to(x,y,x1,y1);
}

void Layer_Shape::cubic_to(Real x, Real y, Real x1, Real y1, Real x2, Real y2)
{
	contour->cubic_to(Vector(x, y), Vector(x1, y1), Vector(x2, y2));
	edge_table->cubic_to(x,y,x1,y1,x2,y2);
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

		Surface	shapesurface;

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
		if(!render_shape(&shapesurface,false,workdesc))return false;

		//blur the image
		Blur(feather,feather,blurtype,&stagethree)(shapesurface,workdesc.get_br()-workdesc.get_tl(),shapesurface);

		//blend with stuff below it...
		unsigned int u = halfsizex, v = halfsizey, x = 0, y = 0;
		for(y = 0; y < h; y++,v++)
		{
			u = halfsizex;
			for(x = 0; x < w; x++,u++)
			{
				Color::value_type a = shapesurface[v][u].get_a();
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
		return render_shape(surface,true,renddesc);
	}

}

bool
Layer_Shape::render_shape(Surface *surface, bool useblend, const RendDesc &renddesc) const
{
	Point origin=param_origin.get(Point());
	Matrix translate;
	translate.set_translate(origin);
	Matrix world_to_pixels_matrix =
		translate
	  * renddesc.get_transformation_matrix()
	  * renddesc.get_world_to_pixels_matrix();

	rendering::software::Contour::render_contour(
		*surface,
		contour->get_chunks(),
		param_invert.get(bool(true)),
		param_antialias.get(bool(true)),
		(rendering::Contour::WindingStyle)param_winding_style.get(int()),
		world_to_pixels_matrix,
		param_color.get(Color()),
		useblend ? get_amount() : 1.0,
		useblend ? get_blend_method() : Color::BLEND_STRAIGHT );

	return true;
}

rendering::Task::Handle
Layer_Shape::build_composite_task_vfunc(ContextParams /*context_params*/)const
{
	// TODO: origin
	// TODO: blurtype
	// TODO: feather

	rendering::TaskContour::Handle task_contour(new rendering::TaskContour());
	// TODO: multithreading without this copying
	task_contour->transformation.set_translate( param_origin.get(Vector()) );
	task_contour->contour = new rendering::Contour();
	task_contour->contour->assign(*contour);
	task_contour->contour->color = param_color.get(Color());
	task_contour->contour->invert = param_invert.get(bool());
	task_contour->contour->antialias = param_antialias.get(bool());
	task_contour->contour->winding_style = (rendering::Contour::WindingStyle)param_winding_style.get(int());
	return task_contour;
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
