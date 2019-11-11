/* === S Y N F I G ========================================================= */
/*!	\file renderer_timecode.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include <synfig/general.h>

#include "renderer_timecode.h"
#include "workarea.h"
#include <pangomm/layout.h>
#include <pangomm/context.h>
#include <pango/pango.h>
#include "app.h"
#include <cassert>

#include <gui/localization.h>

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
	const Glib::RefPtr<Gdk::Window>& drawable,
	const Gdk::Rectangle& /*expose_area*/
)
{
	assert(get_work_area());
	if(!get_work_area())
		return;

	// const synfig::Vector focus_point(get_work_area()->get_focus_point());
	//Warning: Unused variable focus_point
	Cairo::RefPtr<Cairo::Context> cr = drawable->create_cairo_context();

	Canvas::Handle canvas(get_work_area()->get_canvas());
	synfig::Time cur_time(canvas->get_time());

	// Print out the timecode
	{
		Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_work_area()->get_pango_context()));

		int w, h;
		KeyframeList::iterator iter;
		//layout->set_text(canvas->keyframe_list().find(cur_time)->get_description());
		if (canvas->keyframe_list().find(cur_time, iter)) {
			layout->set_text(iter->get_description());
		} else {
			get_work_area()->timecode_width = get_work_area()->timecode_height = 0;
			return;
		}
		layout->get_size(w, h);
		get_work_area()->timecode_width = int(w*1.0/Pango::SCALE);
		get_work_area()->timecode_height = int(h*1.0/Pango::SCALE);

		cr->save();

		cr->set_source_rgb(GDK_COLOR_TO_RGB(TIMECODE_COLOR_TEXT));
		cr->move_to(4,4);
		layout->show_in_cairo_context(cr);

		cr->restore();
	}
}
