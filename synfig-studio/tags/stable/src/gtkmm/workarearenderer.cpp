/* === S I N F G =========================================================== */
/*!	\file template.cpp
**	\brief Template File
**
**	$Id: workarearenderer.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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
#include "workarea.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

WorkAreaRenderer::WorkAreaRenderer():
	enabled_(true),
	priority_(0)
{
}

WorkAreaRenderer::~WorkAreaRenderer()
{
}

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
	const Glib::RefPtr<Gdk::Drawable>& window,
	const Gdk::Rectangle& expose_area
)
{
}

bool
WorkAreaRenderer::event_vfunc(
	GdkEvent* event
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

sinfg::Point
WorkAreaRenderer::screen_to_comp_coords(sinfg::Point pos)const
{
	return get_work_area()->screen_to_comp_coords(pos);
}

sinfg::Point
WorkAreaRenderer::comp_to_screen_coords(sinfg::Point pos)const
{
	return get_work_area()->comp_to_screen_coords(pos);
}
