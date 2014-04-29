/* === S Y N F I G ========================================================= */
/*!	\file renderer_keyframe.cpp
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

#include "renderer_keyframe.h"
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

Renderer_Keyframe::~Renderer_Keyframe()
{
}

bool
Renderer_Keyframe::get_enabled_vfunc()const
{
	Canvas::Handle canvas(get_work_area()->get_canvas());
	return (canvas->rend_desc().get_time_start()!=canvas->rend_desc().get_time_end() ||
		canvas->get_time()!=canvas->rend_desc().get_time_start());
}

void
Renderer_Keyframe::render_vfunc(
	const Glib::RefPtr<Gdk::Drawable>& drawable,
	const Gdk::Rectangle& /*expose_area*/
)
{
	assert(get_work_area());
	if(!get_work_area())
		return;

	Cairo::RefPtr<Cairo::Context> cr = drawable->create_cairo_context();

	Canvas::Handle canvas(get_work_area()->get_canvas());
	synfig::Time cur_time(canvas->get_time());

	// Print out the keyframe(s)
	{
		Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_work_area()->get_pango_context()));

		try
		{
			int w, h;
			layout->set_text(canvas->keyframe_list().find(cur_time)->get_description());
			layout->get_size(w, h);
			get_work_area()->keyframe_width = int(w*1.0/Pango::SCALE);
			get_work_area()->keyframe_height = int(h*1.0/Pango::SCALE);
		}
		catch(synfig::Exception::NotFound)
		{
			get_work_area()->keyframe_width = get_work_area()->keyframe_height = 0;
			return;
		}
		catch(...) {
			assert(0);
		}

		cr->save();

		cr->set_source_rgb(95.0/255.0,0,0);
		cr->move_to(4,4);
		layout->show_in_cairo_context(cr);

		cr->restore();
	}
}
