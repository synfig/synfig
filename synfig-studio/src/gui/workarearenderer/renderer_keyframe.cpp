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
	WorkArea* workarea=get_work_area();
	assert(workarea);
	if(!workarea)
		return;
	int drawable_w,drawable_h;
	drawable->get_size(drawable_w,drawable_h);


	Cairo::RefPtr<Cairo::Context> cr = drawable->create_cairo_context();

	Canvas::Handle canvas(get_work_area()->get_canvas());
	KeyframeList keyframe_list=canvas->keyframe_list();

	synfig::Time cur_time(canvas->get_time());
	synfig::Time prev_time, next_time;
	// Find out previous and next keyframes
	try{
		KeyframeList::iterator prev;
		prev=keyframe_list.find_prev(cur_time);
		prev_time=prev->get_time();
		}
	catch(synfig::Exception::NotFound)
	{
		prev_time=Time::begin();
	}
	try{
		KeyframeList::iterator next;
		next=keyframe_list.find_next(cur_time);
		next_time=next->get_time();
		}
	catch(synfig::Exception::NotFound)
	{
		next_time=Time::end();
	}
	// Print out the keyframe(s)
	{
		Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_work_area()->get_pango_context()));
		Glib::RefPtr<Pango::Layout> layout_prev(Pango::Layout::create(get_work_area()->get_pango_context()));
		Glib::RefPtr<Pango::Layout> layout_next(Pango::Layout::create(get_work_area()->get_pango_context()));
		try
		{
			int w, h;
			layout->set_text("<<"+keyframe_list.find(cur_time)->get_description()+">>");
			layout->get_size(w, h);
			workarea->keyframe_width = int(w*1.0/Pango::SCALE);
			workarea->keyframe_height = int(h*1.0/Pango::SCALE);
			workarea->keyframe_x=(drawable_w-workarea->keyframe_width)*0.5;
			workarea->keyframe_y=4;
		}
		catch(synfig::Exception::NotFound)
		{
			// central keyframe won't print
			workarea->keyframe_width = get_work_area()->keyframe_height = 0;
			// Lets see if we can print the other keyframes
			if(prev_time!=Time::begin())
			{
				int w, h;
				layout_prev->set_text(keyframe_list.find(prev_time)->get_description());
				layout_prev->get_size(w, h);
				workarea->keyframe_prev_width = int(w*1.0/Pango::SCALE);
				workarea->keyframe_prev_height = int(h*1.0/Pango::SCALE);
				workarea->keyframe_prev_x=4;
				workarea->keyframe_prev_y=4;
			}
			else
				get_work_area()->keyframe_prev_width = get_work_area()->keyframe_prev_height = 0;
			if(next_time!=Time::end())
			{
				int w, h;
				layout_next->set_text(keyframe_list.find(next_time)->get_description());
				layout_next->get_size(w, h);
				workarea->keyframe_next_width = int(w*1.0/Pango::SCALE);
				workarea->keyframe_next_height = int(h*1.0/Pango::SCALE);
				workarea->keyframe_next_x=drawable_w-workarea->keyframe_next_width-4;
				workarea->keyframe_next_y=4;
			}
			else
				workarea->keyframe_next_width = workarea->keyframe_next_height = 0;
			cr->save();

			cr->set_source_rgb(95.0/255.0,0,0);
			// move to center of screen
			cr->move_to(workarea->keyframe_prev_x, workarea->keyframe_prev_y);
			layout_prev->show_in_cairo_context(cr);
			cr->move_to(workarea->keyframe_next_x, workarea->keyframe_next_y);
			layout_next->show_in_cairo_context(cr);

			cr->restore();
			}
		catch(...) {
			assert(0);
		}

		cr->save();

		cr->set_source_rgb(95.0/255.0,0,0);
		// move to center of screen
		cr->move_to(workarea->keyframe_x, workarea->keyframe_y);
		layout->show_in_cairo_context(cr);

		cr->restore();
	}
}
