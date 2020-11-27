/* === S Y N F I G ========================================================= */
/*!	\file workarearenderer.cpp
**	\brief Template File
**
**	$Id$
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

#include "workarearenderer.h"
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

WorkAreaRenderer::WorkAreaRenderer():
	enabled_(true),
	priority_(0),
	work_area_(nullptr)
{ }

WorkAreaRenderer::~WorkAreaRenderer()
{ }

bool
WorkAreaRenderer::get_enabled_vfunc()const
{
	return enabled_;
}

void
WorkAreaRenderer::set_enabled(bool x)
{
	if(x==enabled_)
		return;
	enabled_=x;
	signal_changed()();
}

void
WorkAreaRenderer::set_priority(int x)
{
	if(x==priority_)
		return;
	priority_=x;
	signal_changed()();
}

void
WorkAreaRenderer::set_work_area(WorkArea* x)
{
	work_area_=x;
}

void
WorkAreaRenderer::render_vfunc(
	const Glib::RefPtr<Gdk::Window>& /*window*/,
	const Gdk::Rectangle& /*expose_area*/
)
{
}

bool
WorkAreaRenderer::event_vfunc(
	GdkEvent* /*event*/
)
{
	return false;
}

int
WorkAreaRenderer::get_w()const
{ return get_work_area()->get_w(); }
int
WorkAreaRenderer::get_h()const
{ return get_work_area()->get_h(); }

float
WorkAreaRenderer::get_pw()const
{ return get_work_area()->get_pw(); }
float
WorkAreaRenderer::get_ph()const
{ return get_work_area()->get_ph(); }

synfig::Point
WorkAreaRenderer::screen_to_comp_coords(synfig::Point pos)const
{
	return get_work_area()->screen_to_comp_coords(pos);
}

synfig::Point
WorkAreaRenderer::comp_to_screen_coords(synfig::Point pos)const
{
	return get_work_area()->comp_to_screen_coords(pos);
}
