/* === S Y N F I G ========================================================= */
/*!	\file renderer_brush_overlay.cpp
 **	\brief Implementation of the background renderer. Usually a checkerboard
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

    // Trigger a redraw
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
    // Create Cairo surface from synfig::Surface
    int width = overlay_surface.get_w();
    int height = overlay_surface.get_h();

    Cairo::RefPtr<Cairo::ImageSurface> cairo_surface =
        Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, width, height);

    // Convert synfig::Surface to Cairo format
    cairo_surface->flush();
    unsigned char* data = cairo_surface->get_data();
    int stride = cairo_surface->get_stride();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Color pixel = overlay_surface[y][x];
            unsigned char* pixel_data = data + y * stride + x * 4;

            pixel_data[0] = (unsigned char)(pixel.get_b() * pixel.get_a() * 255); // B
            pixel_data[1] = (unsigned char)(pixel.get_g() * pixel.get_a() * 255); // G
            pixel_data[2] = (unsigned char)(pixel.get_r() * pixel.get_a() * 255); // R
            pixel_data[3] = (unsigned char)(pixel.get_a() * 255);                 // A
        }
    }
    cairo_surface->mark_dirty();
    VectorInt offset = get_work_area()->get_windows_offset();
    cr->save();
    cr->set_source(cairo_surface, offset[0], offset[1]);
    cr->paint();
    cr->restore();
}
