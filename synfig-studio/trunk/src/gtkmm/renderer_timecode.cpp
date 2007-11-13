/* === S Y N F I G ========================================================= */
/*!	\file renderer_timecode.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "renderer_timecode.h"
#include "workarea.h"
#include <pangomm/layout.h>
#include <pangomm/context.h>
#include <pango/pango.h>
#include "app.h"
#include <cassert>

#include "general.h"

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

Renderer_Timecode::~Renderer_Timecode()
{
}

bool
Renderer_Timecode::get_enabled_vfunc()const
{
	Canvas::Handle canvas(get_work_area()->get_canvas());
	return (canvas->rend_desc().get_time_start()!=canvas->rend_desc().get_time_end() ||
		canvas->get_time()!=canvas->rend_desc().get_time_start());
}

synfig::Vector
Renderer_Timecode::get_grid_size()const
{
	return get_work_area()->get_grid_size();
}

void
Renderer_Timecode::render_vfunc(
	const Glib::RefPtr<Gdk::Drawable>& drawable,
	const Gdk::Rectangle& /*expose_area*/
)
{
	assert(get_work_area());
	if(!get_work_area())
		return;

	//const synfig::RendDesc &rend_desc(get_work_area()->get_canvas()->rend_desc());

	const synfig::Vector focus_point(get_work_area()->get_focus_point());

	//std::vector< std::pair<Glib::RefPtr<Gdk::Pixbuf>,int> >& tile_book(get_tile_book());

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

	//const synfig::Vector grid_size(get_grid_size());

//	const synfig::Vector::value_type window_startx(get_work_area()->get_window_tl()[0]);
//	const synfig::Vector::value_type window_endx(get_work_area()->get_window_br()[0]);
//	const synfig::Vector::value_type window_starty(get_work_area()->get_window_tl()[1]);
//	const synfig::Vector::value_type window_endy(get_work_area()->get_window_br()[1]);
//	const float pw(get_pw()),ph(get_ph());

	Canvas::Handle canvas(get_work_area()->get_canvas());
	synfig::Time cur_time(canvas->get_time());

	// Print out the timecode
	{
		Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_work_area()->get_pango_context()));

/*		Glib::ustring timecode(cur_time.get_string(rend_desc.get_frame_rate(),App::get_time_format()));

		try
		{
			timecode+="\n"+canvas->keyframe_list().find(cur_time)->get_description();
			gc->set_rgb_fg_color(Gdk::Color("#FF0000"));
		}
		catch(synfig::Exception::NotFound)
		{
			return;
			gc->set_rgb_fg_color(Gdk::Color("#000000"));
		}

		layout->set_text(timecode);
*/

		gc->set_rgb_fg_color(Gdk::Color("#5f0000"));
		try
		{
			int w, h;
			layout->set_text(canvas->keyframe_list().find(cur_time)->get_description());
			layout->get_size(w, h);
			get_work_area()->timecode_width = int(w*1.0/Pango::SCALE);
			get_work_area()->timecode_height = int(h*1.0/Pango::SCALE);
		}
		catch(synfig::Exception::NotFound)
		{
			get_work_area()->timecode_width = get_work_area()->timecode_height = 0;
			return;
		}
		catch(...) {
			assert(0);
		}

  		drawable->draw_layout(gc, 4, 4, layout);
	}
}
