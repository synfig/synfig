/* === S Y N F I G ========================================================= */
/*! \file renderer_brush_overlay.cpp
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

/* === H E L P E R S ======================================================= */

static Cairo::RefPtr<Cairo::ImageSurface>to_cairo_surface(const Surface& synfig_surface)
{
	int width = synfig_surface.get_w();
	int height = synfig_surface.get_h();
	if (width <= 0 || height <= 0) {
		return Cairo::RefPtr<Cairo::ImageSurface>();
	}
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
		synfig_surface[0],
		pixel_format,
		0, width, height,
		cairo_surface->get_stride());

	cairo_surface->mark_dirty();
	cairo_surface->flush();
	return cairo_surface;
}


/* === M E T H O D S ======================================================= */

Renderer_BrushOverlay::Renderer_BrushOverlay():
	overlay_enabled(false),
	has_transformation(false)
{
}

Renderer_BrushOverlay::~Renderer_BrushOverlay()
{
}

void
Renderer_BrushOverlay::set_overlay_surface(const Surface& surface, const Point& tl_, const Point& br_, const Matrix& transform)
{
	overlay_surface = surface;
	tl = tl_;
	br = br_;

	if (!transform.is_identity()) {
		transformation_matrix = transform;
		has_transformation = true;
	}

	overlay_enabled = true;
	if (get_work_area())
		get_work_area()->queue_draw();
}

void
Renderer_BrushOverlay::clear_overlay()
{
	overlay_enabled = false;
    has_transformation = false;
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
	if (!get_work_area() || !overlay_enabled || !overlay_surface.is_valid()) {
		return;
	}
	Cairo::RefPtr<Cairo::Context> cr = drawable->create_cairo_context();
	Rect overlay_rect(tl, br);
	Rect world_bounds;
	if (has_transformation) {
		Point p1(overlay_rect.minx, overlay_rect.miny);
		Point p2(overlay_rect.maxx, overlay_rect.miny);
		Point p3(overlay_rect.maxx, overlay_rect.maxy);
		Point p4(overlay_rect.minx, overlay_rect.maxy);
		auto transform_point = [&](const Point& p) {
			return (transformation_matrix * Vector3(p[0], p[1], 1.0)).to_2d();
		};
		// get real (transformed) corners
		Point p1_t = transform_point(p1);
		Point p2_t = transform_point(p2);
		Point p3_t = transform_point(p3);
		Point p4_t = transform_point(p4);

		// get bounding box
		world_bounds.minx = std::min({p1_t[0], p2_t[0], p3_t[0], p4_t[0]});
		world_bounds.miny = std::min({p1_t[1], p2_t[1], p3_t[1], p4_t[1]});
		world_bounds.maxx = std::max({p1_t[0], p2_t[0], p3_t[0], p4_t[0]});
		world_bounds.maxy = std::max({p1_t[1], p2_t[1], p3_t[1], p4_t[1]});
	} else {
		world_bounds = overlay_rect;
	}

	// clip the rendering area to canvas bounds
	const RendDesc& rend_desc = get_work_area()->get_canvas()->rend_desc();
	Point screen_top_left = get_work_area()->comp_to_screen_coords(Point(world_bounds.minx, world_bounds.maxy));
	Point screen_bottom_right = get_work_area()->comp_to_screen_coords(Point(world_bounds.maxx, world_bounds.miny));
	Point canvas_screen_tl = get_work_area()->comp_to_screen_coords(rend_desc.get_tl());
	Point canvas_screen_br = get_work_area()->comp_to_screen_coords(rend_desc.get_br());
	double clip_x = std::max(screen_top_left[0], canvas_screen_tl[0]);
	double clip_y = std::max(screen_top_left[1], canvas_screen_tl[1]);
	double clip_w = std::min(screen_bottom_right[0], canvas_screen_br[0]) - clip_x;
	double clip_h = std::min(screen_bottom_right[1], canvas_screen_br[1]) - clip_y;
	if (clip_w <= 0 || clip_h <= 0) {
	 	return;
	}
	cr->save();
	cr->rectangle(clip_x, clip_y, clip_w, clip_h);
	cr->clip();
	Cairo::RefPtr<Cairo::ImageSurface> cairo_surface = to_cairo_surface(overlay_surface);
	if (!cairo_surface) {
		cr->restore();
		return;
	}

	const Point screen_origin = get_work_area()->comp_to_screen_coords(Point(0,0));
	const Point screen_one_x = get_work_area()->comp_to_screen_coords(Point(1,0));
	const Point screen_one_y = get_work_area()->comp_to_screen_coords(Point(0,1));
	Cairo::Matrix view_matrix(
		screen_one_x[0] - screen_origin[0], screen_one_y[0] - screen_origin[0],
		screen_one_x[1] - screen_origin[1], screen_one_y[1] - screen_origin[1],
		screen_origin[0], screen_origin[1]
	);
	cr->transform(view_matrix);
	if (has_transformation) {
		 Cairo::Matrix model_matrix(
			transformation_matrix.m00, transformation_matrix.m01,
			transformation_matrix.m10, transformation_matrix.m11,
			transformation_matrix.m20, transformation_matrix.m21
		);
	 	cr->transform(model_matrix);
	}
	cr->translate(tl[0], tl[1]);
	Real world_width = br[0] - tl[0];
	Real world_height = br[1] - tl[1];
	cr->scale(world_width / cairo_surface->get_width(), world_height / cairo_surface->get_height());
	cr->set_source(cairo_surface, 0, 0);
	cr->rectangle(0, 0, cairo_surface->get_width(), cairo_surface->get_height());
	cr->paint();
	cr->restore();
}