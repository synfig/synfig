/* === S Y N F I G ========================================================= */
/*!	\file renderer_guides.cpp
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

#include "renderer_guides.h"
#include <gui/workarea.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

#define GUIDE_COLOR_CURRENT     Gdk::Color("#ff6f6f")

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer_Guides::~Renderer_Guides()
{
}

bool
Renderer_Guides::get_enabled_vfunc()const
{
	return get_work_area()->get_show_guides();
}

std::list<float>&
Renderer_Guides::get_guide_list_x()
{
	return get_work_area()->get_guide_list_x();
}

std::list<float>&
Renderer_Guides::get_guide_list_y()
{
	return get_work_area()->get_guide_list_y();
}

bool
Renderer_Guides::event_vfunc(GdkEvent* /*event*/)
{
	// TODO : All the guides stuff done in WorkArea::on_drawing_area_event(GdkEvent *event)
	// could be done here for better code maintenance (or not).
	return false;
}

void
Renderer_Guides::render_vfunc(
	const Glib::RefPtr<Gdk::Window>& drawable,
	const Gdk::Rectangle& /*expose_area*/
)
{
	assert(get_work_area());
	if(!get_work_area())
		return;


	int drawable_w = drawable->get_width();
	int drawable_h = drawable->get_height();

	Cairo::RefPtr<Cairo::Context> cr = drawable->create_cairo_context();

	const synfig::Vector::value_type window_startx(get_work_area()->get_window_tl()[0]);
	const synfig::Vector::value_type window_starty(get_work_area()->get_window_tl()[1]);
	const float pw(get_pw()),ph(get_ph());

	synfig::Color guides_color(get_work_area()->get_guides_color());

	// Draw out the guides
	{
		Duckmatic::GuideList::const_iterator iter;

		cr->save();
		cr->set_line_cap(Cairo::LINE_CAP_BUTT);
		cr->set_line_join(Cairo::LINE_JOIN_MITER);
		cr->set_antialias(Cairo::ANTIALIAS_NONE);

		cr->set_line_width(1.0);
		std::valarray<double> dashes(2);
		dashes[0]=5.0;
		dashes[1]=5.0;
		cr->set_dash(dashes, 0);

		// vertical
		for(iter=get_guide_list_x().begin();iter!=get_guide_list_x().end();++iter)
		{
			const float x((*iter-window_startx)/pw);

			if(iter==get_work_area()->curr_guide)
				cr->set_source_rgb(GDK_COLOR_TO_RGB(GUIDE_COLOR_CURRENT));
			else
				cr->set_source_rgb(guides_color.get_r(),guides_color.get_g(),guides_color.get_b());

			cr->move_to(
				x,
				0
				);
			cr->line_to(
				x,
				drawable_h
			);
			cr->stroke();
		}
		// horizontal
		for(iter=get_guide_list_y().begin();iter!=get_guide_list_y().end();++iter)
		{
			const float y((*iter-window_starty)/ph);
			if(iter==get_work_area()->curr_guide)
				cr->set_source_rgb(GDK_COLOR_TO_RGB(GUIDE_COLOR_CURRENT));
			else
				cr->set_source_rgb(guides_color.get_r(),guides_color.get_g(),guides_color.get_b());

			cr->move_to(
				0,
				y
				);
			cr->line_to(
				drawable_w,
				y
			);
			cr->stroke();
		}

		cr->restore();
	}
}
