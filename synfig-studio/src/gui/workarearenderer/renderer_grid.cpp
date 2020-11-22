/* === S Y N F I G ========================================================= */
/*!	\file renderer_grid.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2011 Nikita Kitaev
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

#include <ETL/misc>
#include <gui/workarea.h>

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
	const Glib::RefPtr<Gdk::Window>& drawable,
	const Gdk::Rectangle& /*expose_area*/
)
{
	assert(get_work_area());
	if(!get_work_area())
		return;

	Cairo::RefPtr<Cairo::Context> cr = drawable->create_cairo_context();

	int drawable_w = drawable->get_width();
	int drawable_h = drawable->get_height();

	synfig::Vector grid_size(get_grid_size());
	if(grid_size[0] < 0) grid_size[0] = -grid_size[0];
	if(grid_size[1] < 0) grid_size[1] = -grid_size[1];

	const synfig::Vector::value_type window_startx(get_work_area()->get_window_tl()[0]);
	const synfig::Vector::value_type window_endx(get_work_area()->get_window_br()[0]);
	const synfig::Vector::value_type window_starty(get_work_area()->get_window_tl()[1]);
	const synfig::Vector::value_type window_endy(get_work_area()->get_window_br()[1]);
	const float pw(get_pw()),ph(get_ph());
	
	synfig::Color grid_color(get_work_area()->get_grid_color());

	// Draw out the grid
	if(grid_size[0]>pw*3.5 && grid_size[1]>ph*3.5)
	{
		synfig::Vector::value_type x,y;

		x=floor(window_startx/grid_size[0])*grid_size[0];
		y=floor(window_starty/grid_size[1])*grid_size[1];

		cr->save();
		cr->set_line_cap(Cairo::LINE_CAP_BUTT);
		cr->set_line_join(Cairo::LINE_JOIN_MITER);
		cr->set_antialias(Cairo::ANTIALIAS_NONE);

		cr->set_line_width(1.0);
		cr->set_source_rgb(grid_color.get_r(),grid_color.get_g(),grid_color.get_b()); 
		std::valarray<double> dashes(2);
		dashes[0]=4.0;
		dashes[1]=4.0;
		cr->set_dash(dashes, 0);

		if(x<window_endx)
			for(;x<window_endx;x+=grid_size[0])
			{
				cr->move_to(
					round_to_int((x-window_startx)/pw),
					0
					);
				cr->line_to(
					round_to_int((x-window_startx)/pw),
					drawable_h
				);
				cr->stroke();
			}
		else
			for(;x>window_endx;x-=grid_size[0])
			{
				cr->move_to(
					round_to_int((x-window_startx)/pw),
					0
					);
				cr->line_to(
					round_to_int((x-window_startx)/pw),
					drawable_h
				);
				cr->stroke();
			}

		if(y<window_endy)
			for(;y<window_endy;y+=grid_size[1])
			{
				cr->move_to(
					0,
					round_to_int((y-window_starty)/ph)
					);
				cr->line_to(
					drawable_w,
					round_to_int((y-window_starty)/ph)
				);
				cr->stroke();
			}
		else
			for(;y>window_endy;y-=grid_size[1])
			{
				cr->move_to(
					0,
					round_to_int((y-window_starty)/ph)
					);
				cr->line_to(
					drawable_w,
					round_to_int((y-window_starty)/ph)
				);
				cr->stroke();
			}
		cr->restore();
	}
}
