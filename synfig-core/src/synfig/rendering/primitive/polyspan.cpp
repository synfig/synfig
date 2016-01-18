/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/primitive/polyspan.cpp
**	\brief Polyspan
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
**	......... ... 2015 Ivan Mahonin
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

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include "polyspan.h"

#include <cassert>

#include <synfig/general.h>
#include <synfig/localization.h>

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

//default constructor - 0 everything
Polyspan::Polyspan():
	open_index(0),
	cur_x(0.0),
	cur_y(0.0),
	close_x(0.0),
	close_y(0.0),
	flags(NotSorted)
{ }

//0 out all the variables involved in processing
void
Polyspan::clear()
{
	covers.clear();
	cur_x = cur_y = close_x = close_y = 0;
	open_index = 0;
	current.set(0, 0, 0, 0);
	flags = NotSorted;
}

//add the current cell, but only if there is information to add
void
Polyspan::addcurrent()
{
	if(current.cover || current.area)
	{
		if (covers.size() == covers.capacity())
			covers.reserve(covers.size() + 1024*1024);
		covers.push_back(current);
	}
}

//move to the next cell (cover values 0 initially), keeping the current if necessary
void
Polyspan::move_pen(int x, int y)
{
	if(y != current.y || x != current.x)
	{
		addcurrent();
		current.set(x,y,0,0);
	}
}

//close the primitives with a line (or rendering will not work as expected)
void
Polyspan::close()
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
void
Polyspan::merge_all()
{
	sort(covers.begin(),covers.end());
	open_index = 0;
}

//will sort the marks if they are not sorted
void
Polyspan::sort_marks()
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
void
Polyspan::encapsulate_current()
{
	//sort the current list then reposition the open list section
	sort_marks();
	open_index = covers.size();
}

//move to start a new primitive list (enclose the last primitive if need be)
void
Polyspan::move_to(Real x, Real y)
{
	close();
	if(isnan(x))x=0;
	if(isnan(y))y=0;
	move_pen((int)floor(x),(int)floor(y));
	close_y = cur_y = y;
	close_x = cur_x = x;
}

//primitive_to functions
void
Polyspan::line_to(Real x, Real y)
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
			cur_x = std::max(cur_x,	(Real)window.minx);
			cur_x = std::min(cur_x,	(Real)window.maxx);

			//clip the dest values - y is already clipped
			x = std::max(x,(Real)window.minx);
			x = std::min(x,(Real)window.maxx);

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

bool
Polyspan::clip_conic(const Point *const p, const RectInt &r)
{
	const Real minx = std::min(std::min(p[0][0],p[1][0]),p[2][0]);
	const Real miny = std::min(std::min(p[0][1],p[1][1]),p[2][1]);
	const Real maxx = std::max(std::max(p[0][0],p[1][0]),p[2][0]);
	const Real maxy = std::max(std::max(p[0][1],p[1][1]),p[2][1]);

	return 	(minx > r.maxx) ||
			(maxx < r.minx) ||
			(miny > r.maxy) ||
			(maxy < r.miny);
}

Real
Polyspan::max_edges_conic(const Point *const p)
{
	const Real x1 = p[1][0] - p[0][0];
	const Real y1 = p[1][1] - p[0][1];

	const Real x2 = p[2][0] - p[1][0];
	const Real y2 = p[2][1] - p[1][1];

	const Real d1 = x1*x1 + y1*y1;
	const Real d2 = x2*x2 + y2*y2;

	return std::max(d1,d2);
}

void
Polyspan::subd_conic_stack(Point *arc)
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

void
Polyspan::conic_to(Real x, Real y, Real x1, Real y1)
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
			subd_conic_stack(current);
			current += 2; 		//cursor on second curve
			level ++;
			num ++;
			onsecond = false;
			continue;
		}else
		//split it again, if it's too big
		if(max_edges_conic(current) > 0.25) //distance of .5 (cover no more than half the pixel)
		{
			subd_conic_stack(current);
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

bool
Polyspan::clip_cubic(const Point *const p, const RectInt &r)
{
	return 	((p[0][0] > r.maxx) && (p[1][0] > r.maxx) && (p[2][0] > r.maxx) && (p[3][0] > r.maxx)) ||
			((p[0][0] < r.minx) && (p[1][0] < r.minx) && (p[2][0] < r.minx) && (p[3][0] < r.minx)) ||
			((p[0][1] > r.maxy) && (p[1][1] > r.maxy) && (p[2][1] > r.maxy) && (p[3][1] > r.maxy)) ||
			((p[0][1] < r.miny) && (p[1][1] < r.miny) && (p[2][1] < r.miny) && (p[3][1] < r.miny));
}

Real
Polyspan::max_edges_cubic(const Point *const p)
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

	return std::max(std::max(d1,d2),d3);
}

void
Polyspan::subd_cubic_stack(Point *arc)
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
}


void
Polyspan::cubic_to(Real x, Real y, Real x1, Real y1, Real x2, Real y2)
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
			subd_cubic_stack(current);
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
			subd_cubic_stack(current);
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


void
Polyspan::draw_scanline(int y, Real x1, Real y1, Real x2, Real y2)
{
	int	ix1 = (int)floor(x1);
	int	ix2 = (int)floor(x2);
	Real fx1 = x1 - ix1;
	Real fx2 = x2 - ix2;

	Real dx,dy,dydx,mult;

	dx = x2 - x1;
	dy = y2 - y1;

	//case horizontal line
	if(y1 == y2)
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
		y1 += mult;
		ix1++;

		move_pen(ix1,y);

		//set up for whole ones
		while(ix1 != ix2)
		{
			//trapezoid(0,y1,1,y1+dydx);
			current.addcover(dydx,dydx/2);	//accumulated area 1/2 the cover

			//move to next pixel (+1)
			ix1++;
			y1 += dydx;
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
		y1 += mult;
		ix1--;

		move_pen(ix1,y);

		//set up for whole ones
		while(ix1 != ix2)
		{
			//trapezoid(0,y1,1,y1+dydx);
			current.addcover(dydx,dydx/2);	//accumulated area 1/2 the cover

			//move to next pixel (-1)
			y1 += dydx;
			ix1--;
			move_pen(ix1,y);
		}

		//last pixel
		mult = y2 - y1; //final y-pos - last intersect pos

		current.addcover(mult,(fx2+1)*mult/2);
	}
}

void
Polyspan::draw_line(Real x1, Real y1, Real x2, Real y2)
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

Real
Polyspan::extract_alpha(Real area, Contour::WindingStyle winding_style) const
{
	if (area < 0)
		area = -area;

	if (winding_style == Contour::WINDING_NON_ZERO)
	{
		// non-zero winding style
		if (area > 1)
			return 1;
	}
	else // if (winding_style == Contour::WINDING_EVEN_ODD)
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

RectInt
Polyspan::calc_bounds() const
{
	if (covers.empty()) return RectInt(window.minx, window.miny);
	RectInt bounds(covers.front().x, covers.front().y);
	for(cover_array::const_iterator i = covers.begin() + 1; i != covers.end(); ++i)
		bounds.expand(i->x, i->y);
	set_intersect(bounds, bounds, window);
	return bounds;
}

/* === E N T R Y P O I N T ================================================= */
