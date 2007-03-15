/* === S Y N F I G ========================================================= */
/*!	\file renderer_grid.cpp
**	\brief Template File
**
**	$Id: renderer_grid.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#include "renderer_grid.h"
#include "workarea.h"
#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer_Grid::~Renderer_Grid()
{
}

bool
Renderer_Grid::get_enabled_vfunc()const
{
	return get_work_area()->grid_status();
}

synfig::Vector
Renderer_Grid::get_grid_size()const
{
	return get_work_area()->get_grid_size();
}

void
Renderer_Grid::render_vfunc(
	const Glib::RefPtr<Gdk::Drawable>& drawable,
	const Gdk::Rectangle& expose_area
)
{
	assert(get_work_area());
	if(!get_work_area())
		return;

//	const synfig::RendDesc &rend_desc(get_work_area()->get_canvas()->rend_desc());

	const synfig::Vector focus_point(get_work_area()->get_focus_point());

//	std::vector< std::pair<Glib::RefPtr<Gdk::Pixbuf>,int> >& tile_book(get_tile_book());

	int drawable_w,drawable_h;
	drawable->get_size(drawable_w,drawable_h);

	// Calculate the window coordinates of the top-left
	// corner of the canvas.
//	const synfig::Vector::value_type
//		x(focus_point[0]/get_pw()+drawable_w/2-get_w()/2),
//		y(focus_point[1]/get_ph()+drawable_h/2-get_h()/2);

	/*const synfig::Vector::value_type window_startx(window_tl[0]);
	const synfig::Vector::value_type window_endx(window_br[0]);
	const synfig::Vector::value_type window_starty(window_tl[1]);
	const synfig::Vector::value_type window_endy(window_br[1]);
	*/
//	const int
//		tile_w(get_work_area()->get_tile_w()),
//		tile_h(get_work_area()->get_tile_h());

//	const int
//		w(get_w()),
//		h(get_h());

	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(drawable));

	const synfig::Vector grid_size(get_grid_size());

	const synfig::Vector::value_type window_startx(get_work_area()->get_window_tl()[0]);
	const synfig::Vector::value_type window_endx(get_work_area()->get_window_br()[0]);
	const synfig::Vector::value_type window_starty(get_work_area()->get_window_tl()[1]);
	const synfig::Vector::value_type window_endy(get_work_area()->get_window_br()[1]);
	const float pw(get_pw()),ph(get_ph());

	// Draw out the grid
	if(grid_size[0]>pw*3.5 && grid_size[1]>ph*3.5)
	{
		synfig::Vector::value_type x,y;

		x=floor(window_startx/grid_size[0])*grid_size[0];
		y=floor(window_starty/grid_size[1])*grid_size[1];

		gc->set_function(Gdk::COPY);
		gc->set_rgb_fg_color(Gdk::Color("#9f9f9f"));
		gc->set_line_attributes(1,Gdk::LINE_ON_OFF_DASH,Gdk::CAP_BUTT,Gdk::JOIN_MITER);

		if(x<window_endx)
			for(;x<window_endx;x+=grid_size[0])
			{
				drawable->draw_line(gc,
					round_to_int((x-window_startx)/pw),
					0,
					round_to_int((x-window_startx)/pw),
					drawable_h
				);
			}
		else
			for(;x>window_endx;x-=grid_size[0])
			{
				drawable->draw_line(gc,
					round_to_int((x-window_startx)/pw),
					0,
					round_to_int((x-window_startx)/pw),
					drawable_h
				);
			}

		if(y<window_endy)
			for(;y<window_endy;y+=grid_size[1])
			{
				drawable->draw_line(gc,
					0,
					round_to_int((y-window_starty)/ph),
					drawable_w,
					round_to_int((y-window_starty)/ph)
				);
			}
		else
			for(;y>window_endy;y-=grid_size[1])
			{
				drawable->draw_line(gc,
					0,
					round_to_int((y-window_starty)/ph),
					drawable_w,
					round_to_int((y-window_starty)/ph)
				);
			}
	}
}
