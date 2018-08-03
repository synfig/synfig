/* === S Y N F I G ========================================================= */
/*!	\file renderer_canvas.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Nikita Kitaev
**  ......... ... 2018 Ivan Mahonin
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

#include "renderer_canvas.h"
#include <ETL/misc>
#include <gdkmm/general.h>

#include <gui/localization.h>
#include "app.h"

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

Renderer_Canvas::~Renderer_Canvas()
{
	cancel_render();
#error // TODO: cancel background threads
}

void
Renderer_Canvas::cancel_render()
{
	rendering::Task::List list;
	while(true) {
		{
			Glib::Threads::Mutex::Lock lock(mutex);
			if (events.empty()) return;
			list.clear();
			list.reserve(events.size());
			list.insert(list.end(), events.begin(), events.end());
		}
		rendering::Renderer::cancel(list);
		Glib::usleep(1);
	}
}

void
Renderer_Canvas::enqueue_render(bool after_current_task_complete)
{
	rendering::Task::List tasks_to_cancel;

	{
		Glib::Threads::Mutex::Lock lock(mutex);
		if (after_current_task_complete && !events.empty())
			{ render_after_current_task_complete = true; return; }

		tasks_to_cancel.reserve(events.size());
		tasks_to_cancel.insert(tasks_to_cancel.end(), events.begin(), events.end());

		#error // TODO: build tasks for rendering
	}

	rendering::Renderer::cancel(tasks_to_cancel);
}

void
Renderer_Canvas::wait_render()
{
	EventList list;
	while(true) {
		{
			Glib::Threads::Mutex::Lock lock(mutex);
			if (events.empty()) return;
			list.clear();
			list.reserve(events.size());
			list.insert(list.end(), events.begin(), events.end());
		}
		for(EventList::iterator i = list.begin(); i != list.end(); ++i)
			(*i)->wait();
		Glib::usleep(1);
	}
}


void
Renderer_Canvas::render_vfunc(
	const Glib::RefPtr<Gdk::Window>& drawable,
	const Gdk::Rectangle& /*expose_area*/ )
{
	assert(get_work_area());
	if(!get_work_area())
		return;

	VectorInt window_offset(get_work_area()->get_windows_offset());
	RectInt window_rect(get_work_area()->get_window_rect());

	Cairo::RefPtr<Cairo::Context> cr = drawable->create_cairo_context();
	const WorkAreaTile::List tiles(get_tile_book().get_tiles());

	for(WorkAreaTile::List::const_iterator i = tiles.begin(); i != tiles.end(); ++i)
	{
		if (etl::intersect(window_rect, i->rect) && (i->surface || i->pixbuf))
		{
			cr->save();
			if (i->surface)
			{
				int div = 1;
				if(get_work_area()->get_low_resolution_flag())
				{
					div = get_work_area()->get_low_res_pixel_size();
					cr->scale(div, div);
				}
				cairo_set_source_surface(
					cr->cobj(),
					i->surface,
					(i->rect.minx + window_offset[0])/div,
					(i->rect.miny + window_offset[1])/div );
				cairo_pattern_set_filter(cairo_get_source(cr->cobj()), CAIRO_FILTER_NEAREST);
			}
			else
			if (i->pixbuf)
			{
				Gdk::Cairo::set_source_pixbuf(
					cr,
					i->pixbuf,
					i->rect.minx + window_offset[0],
					i->rect.miny + window_offset[1] );
			}
			cr->paint();
			cr->restore();

			/*
			cr->save();
			cr->set_antialias(Cairo::ANTIALIAS_NONE);
			cr->set_line_width(1.0);
			cr->set_source_rgb(0,0,0);
			cr->rectangle(
				i->rect.minx + window_offset[0],
				i->rect.miny + window_offset[1],
				i->rect.maxx - i->rect.minx,
				i->rect.maxy - i->rect.miny );
			cr->stroke();
			cr->restore();
			*/
		}
	}

	if (!get_canceled() && !get_rendering() && !get_queued())
	{
		std::vector<RectInt> rects;
		get_tile_book().get_dirty_rects(rects, get_refreshes(), window_rect);
		if (!rects.empty())
			get_work_area()->async_update_preview();
	}

	// Draw the border around the rendered region
	{
		cr->save();
		cr->set_line_cap(Cairo::LINE_CAP_BUTT);
		cr->set_line_join(Cairo::LINE_JOIN_MITER);
		cr->set_antialias(Cairo::ANTIALIAS_NONE);
		cr->set_line_width(1.0);
		cr->set_source_rgb(0,0,0);
		cr->rectangle(window_offset[0], window_offset[1], get_w(), get_h());
		cr->stroke();
		cr->restore();
	}
}
