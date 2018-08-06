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

#include <synfig/general.h>

#include "renderer_background.h"
#include "workarea.h"
#include <ETL/misc>

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
							const Glib::RefPtr<Gdk::Window>& drawable,
							const Gdk::Rectangle& /*expose_area*/
							)
{
    assert(get_work_area());
    if(!get_work_area())
        return;

    VectorInt offset = get_work_area()->get_windows_offset();
    int w=get_w();
    int h=get_h();

    Cairo::RefPtr<Cairo::Context> cr = drawable->create_cairo_context();

    synfig::Vector grid_size(get_work_area()->get_background_size());
    Cairo::RefPtr<Cairo::Surface> surface_background = draw_check_pattern(grid_size[0], grid_size[1]);

    cr->save();

    cr->set_source(surface_background, offset[0] + w/2, offset[1] - h/2);

    Cairo::RefPtr<Cairo::SurfacePattern> sp_ptr = Cairo::SurfacePattern::create(surface_background);
    sp_ptr->set_filter(Cairo::FILTER_NEAREST);
    sp_ptr->set_extend(Cairo::EXTEND_REPEAT);

    cr->set_source(sp_ptr);

    cr->rectangle(offset[0], offset[1], w, h);
    cr->clip();
    cr->paint();

    cr->restore();
}

Cairo::RefPtr<Cairo::Surface>
Renderer_Background::draw_check_pattern(int width, int height)
{
    Cairo::RefPtr<Cairo::Surface> surface_ptr =  Cairo::ImageSurface::create (Cairo::FORMAT_RGB24, width*2, height*2);
    Cairo::RefPtr<Cairo::Context> cr_ptr = Cairo::Context::create (surface_ptr);

    synfig::Color first_color(get_work_area()->get_background_first_color());
    cr_ptr->set_source_rgb(first_color.get_r(), first_color.get_g(), first_color.get_b());
    cr_ptr->paint();

    synfig::Color second_color(get_work_area()->get_background_second_color());
    cr_ptr->set_source_rgb(second_color.get_r(), second_color.get_g(), second_color.get_b());
    cr_ptr->rectangle(int(width), 0 , width, height);
    cr_ptr->rectangle(0, int(height), width , height);
    cr_ptr->fill();

    return surface_ptr;
}
