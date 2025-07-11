/* === S Y N F I G ========================================================= */
/*!	\file renderer_brush_overlay.cpp
 **	\brief Template File
 **
 **	\legal
 **	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
 **	Copyright (c) 2013 Carlos López
 **
 **	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
 **	it under the terms of the GNU General Public License as published by
 **	the Free Software Foundation, either version 2 of the License, or
 **	(at your option) any later version.
 **
 **	Synfig is distributed in the hope that it will be useful,
 **	but WITHOUT ANY WARRANTY; without even the implied warranty of
 **	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include "renderer_brush_overlay.h"
#include <gui/workarea.h>
#include <synfig/color.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer_BrushOverlay::Renderer_BrushOverlay():
    overlay_enabled(false)
{
}

Renderer_BrushOverlay::~Renderer_BrushOverlay()
{
}

void
Renderer_BrushOverlay::set_overlay_surface(const synfig::Surface& surface, const synfig::Rect& rect)
{
    overlay_surface = surface;
    overlay_rect = rect;
    overlay_enabled = true;
    if (get_work_area())
        get_work_area()->queue_draw();
}

void
Renderer_BrushOverlay::clear_overlay()
{
    overlay_enabled = false;
    if (get_work_area())
        get_work_area()->queue_draw();
}

void
Renderer_BrushOverlay::enable_overlay(bool enabled)
{
    overlay_enabled = enabled;
    if (get_work_area())
        get_work_area()->queue_draw();
}

bool
Renderer_BrushOverlay::get_enabled_vfunc() const
{
    return overlay_enabled && overlay_surface.is_valid();
}

void
Renderer_BrushOverlay::render_vfunc(
    const Glib::RefPtr<Gdk::Window>& drawable,
    const Gdk::Rectangle& /*expose_area*/)
{
    assert(get_work_area());
    if (!get_work_area() || !overlay_enabled || !overlay_surface.is_valid())
        return;

    Cairo::RefPtr<Cairo::Context> cr = drawable->create_cairo_context();

    // Convert Synfig surface to Cairo surface using modern C++ API
    int width = overlay_surface.get_w();
    int height = overlay_surface.get_h();
    if (width <= 0 || height <= 0) return;

    Cairo::RefPtr<Cairo::ImageSurface> cairo_surface =
        Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, width, height);

    cairo_surface->flush();

    union { int i; char c[4]; } checker = {0x01020304};
    bool big_endian = checker.c[0] == 1;
    PixelFormat pixel_format = big_endian
        ? (PF_A_START | PF_RGB | PF_A_PREMULT)
        : (PF_BGR | PF_A | PF_A_PREMULT);

    color_to_pixelformat(
        cairo_surface->get_data(),
        overlay_surface[0],
        pixel_format,
        0,
        cairo_surface->get_width(),
        cairo_surface->get_height(),
        cairo_surface->get_stride());

    cairo_surface->mark_dirty();
    cairo_surface->flush();

    // Compute screen coordinates for proper zoom handling
    synfig::Point world_top_left_layer(overlay_rect.minx, overlay_rect.maxy);
    synfig::Point world_bottom_right_layer(overlay_rect.maxx, overlay_rect.miny);
    synfig::Point screen_top_left = get_work_area()->comp_to_screen_coords(world_top_left_layer);
    synfig::Point screen_bottom_right = get_work_area()->comp_to_screen_coords(world_bottom_right_layer);
    double screen_x = screen_top_left[0];
    double screen_y = screen_top_left[1];
    double screen_w = screen_bottom_right[0] - screen_top_left[0];
    double screen_h = screen_bottom_right[1] - screen_top_left[1];
    if (screen_w <= 0 || screen_h <= 0) return;

    cr->save();
    cr->translate(screen_x, screen_y);
    cr->scale(screen_w / width, screen_h / height);
    cr->set_source(cairo_surface, 0, 0);
    cr->rectangle(0, 0, width, height);
    cr->paint();
    cr->restore();
}