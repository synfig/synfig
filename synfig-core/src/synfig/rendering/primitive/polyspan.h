/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/primitive/polyspan.h
**	\brief Polyspan Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
**	......... ... 2015 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_POLYSPAN_H
#define __SYNFIG_RENDERING_POLYSPAN_H

/* === H E A D E R S ======================================================= */

#include <vector>

#include <synfig/rect.h>

#include "contour.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class Polyspan
{
public:
	struct PenMark
	{
		int y, x;
		Real cover, area;

		PenMark(): y(), x(), cover(), area() { }
		PenMark(int xin, int yin, Real c, Real a):
			y(yin), x(xin), cover(c), area(a) { }
		void set(int xin, int yin, Real c, Real a)
			{ y = yin; x = xin; cover = c; area = a; }
		void setcoord(int xin, int yin)
			{ y = yin; x = xin;	}
		void setcover(Real c, Real a)
			{ cover	= c; area = a; }
		void addcover(Real c, Real a)
			{ cover += c; area += a; }
		bool operator < (const PenMark &rhs) const
			{ return y == rhs.y ? x < rhs.x : y < rhs.y; }
	};

	typedef	std::vector<PenMark> cover_array;

	//for assignment to flags value
	enum PolySpanFlags
	{
		NotSorted       = 0x8000,
		NotClosed       = 0x4000,
		NotFinishedLine = 0x2000
	};

	enum {
		MAX_SUBDIVISION_SIZE = 64
	};

private:
	Point			arc[3*MAX_SUBDIVISION_SIZE + 1];

	cover_array		covers;
	PenMark			current;

	int				open_index;

	//ending position of last primitive
	Real			cur_x;
	Real			cur_y;

	//ending position of not-finished line
	Real			cur_line_x;
	Real			cur_line_y;


	//starting position of current primitive list
	Real			close_x;
	Real			close_y;

	//flags for the current segment
	int				flags;

	//the window that will be drawn (used for clipping)
	RectInt		    window;

	//add the current cell, but only if there is information to add
	void addcurrent();

	//move to the next cell (cover values 0 initially), keeping the current if necessary
	void move_pen(int x, int y);

	static Real clamp_coord(Real x);
	static Real clamp_detail(Real x);

	static bool clip_conic(const Point *const p, const RectInt &r);
	static Real max_edges_conic(const Point *const p);
	static void subd_conic_stack(Point *arc);

	static bool clip_cubic(const Point *const p, const RectInt &r);
	static Real max_edges_cubic(const Point *const p);
	static void subd_cubic_stack(Point *arc);

	void finish_line();

public:
	Polyspan();

	const RectInt& get_window() const { return window; }
	const cover_array& get_covers() const { return covers; }

	bool notclosed() const
		{ return (flags & NotClosed) || (cur_x != close_x) || (cur_y != close_y); }

	//0 out all the variables involved in processing
	void clear();
	void init(const RectInt &window)
		{ clear(); this->window = window; }
	void init(int minx, int miny, int maxx, int maxy)
	{
		clear();
		window.minx = minx;
		window.miny = miny;
		window.maxx = maxx;
		window.maxy = maxy;
	}

	//close the primitives with a line (or rendering will not work as expected)
	void close();

	// Not recommended - destroys any separation of spans currently held
	void merge_all();

	//will sort the marks if they are not sorted
	void sort_marks();

	//encapsulate the current sublist of marks (used for drawing)
	void encapsulate_current();

	//move to start a new primitive list (enclose the last primitive if need be)
	void move_to(Real x, Real y);

	//primitive_to functions
	void line_to(Real x, Real y, Real detail = 1.0);
	void conic_to(Real x, Real y, Real x1, Real y1, Real detail = 1.0);
	void cubic_to(Real x, Real y, Real x1, Real y1, Real x2, Real y2, Real detail = 1.0);

	void draw_scanline(int y, Real x1, Real y1, Real x2, Real y2);
	void draw_line(Real x1, Real y1, Real x2, Real y2);

	Real extract_alpha(Real area, Contour::WindingStyle winding_style) const;

	RectInt calc_bounds() const;
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
