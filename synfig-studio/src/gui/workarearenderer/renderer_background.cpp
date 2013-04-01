/* === S Y N F I G ========================================================= */
/*!	\file renderer_background.cpp
 **	\brief Implementation of the background renderer. Usually a checkerboard
 **
 **	$Id$
 **
 **	\legal
 **	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
 **	Copyright (c) 2013 Carlos López
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

#include "renderer_background.h"
#include "workarea.h"
#include <ETL/misc>

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

Renderer_Background::~Renderer_Background()
{
}

bool
Renderer_Background::get_enabled_vfunc()const
{
	return true;
}

void
Renderer_Background::render_vfunc(
							const Glib::RefPtr<Gdk::Drawable>& drawable,
							const Gdk::Rectangle& /*expose_area*/
							)
{
	assert(get_work_area());
	if(!get_work_area())
		return;

	int drawable_w,drawable_h;
	drawable->get_size(drawable_w,drawable_h);
	
	int w=get_w();
	int h=get_h();

	const synfig::Vector focus_point(get_work_area()->get_focus_point());
	// Calculate the window coordinates of the top-left
	// corner of the canvas.
	const float
	x(focus_point[0]/get_pw()+drawable_w/2-w/2),
	y(focus_point[1]/get_ph()+drawable_h/2-h/2);

	cairo_t* cr=gdk_cairo_create(drawable->gobj());
	
	
    cairo_surface_t *check;
	
    check=draw_check(15, 15);
	
    cairo_save(cr);

    cairo_set_source_surface(cr, check, focus_point[0]/get_pw()+drawable_w/2, focus_point[1]/get_ph()+drawable_h/2);
    cairo_surface_destroy(check);
	
    cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_NEAREST);
    cairo_pattern_set_extend(cairo_get_source(cr), CAIRO_EXTEND_REPEAT);
	cairo_rectangle(cr, round_to_int(x), round_to_int(y), w, h);
	cairo_clip(cr);
    cairo_paint (cr);
	
    cairo_restore (cr);
	cairo_destroy(cr);
}

cairo_surface_t *
Renderer_Background::draw_check(int width, int height)
{
    cairo_surface_t *surface;
    cairo_t *cr;
		
    surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, width*2, height*2);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);
	
	//TODO allow custom user colors
    cairo_set_source_rgb (cr, 0.88, 0.88, 0.88); /* light gray */
    cairo_paint (cr);
	
    cairo_set_source_rgb (cr, 0.65, 0.65, 0.65); /* dark gray */
    cairo_rectangle (cr, int(width), 0 , width, height);
    cairo_rectangle (cr, 0, int(height), width , height);
    cairo_fill (cr);
	
    surface = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);
	
    return surface;
}
