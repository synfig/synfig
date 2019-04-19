/* === S Y N F I G ========================================================= */
/*!	\file layer_rendering_task.cpp
**	\brief Layer_RenderingTask implementation
**
**	$Id$
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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
#include <synfig/localization.h>

#include "layer_rendering_task.h"

#include <synfig/context.h>
#include <synfig/rendering/software/surfacesw.h>
#include <synfig/rendering/software/function/resample.h>

#include <synfig/debug/debugsurface.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Layer_RenderingTask::Layer_RenderingTask() { }

Rect
Layer_RenderingTask::get_bounding_rect() const
{
	Rect rect = Rect::zero();
	for(rendering::Task::List::const_iterator i = tasks.begin(); i != tasks.end(); ++i)
		if (*i) rect = rect.is_valid() ? rect | (*i)->get_bounds(): (*i)->get_bounds();
	return rect;
}

Color
Layer_RenderingTask::get_color(Context /* context */, const Point &pos)const
{
	for(rendering::Task::List::const_iterator i = tasks.begin(); i != tasks.end(); ++i)
	{
		if (*i && (*i)->is_valid())
		{
			RectInt src_target_rect = (*i)->target_rect;
			Vector src_lt = (*i)->source_rect.get_min();
			Vector src_rb = (*i)->source_rect.get_max();

			Matrix units_to_src_pixels;
			units_to_src_pixels.m00 = (src_target_rect.maxx - src_target_rect.minx)/(src_rb[0] - src_lt[0]);
			units_to_src_pixels.m11 = (src_target_rect.maxy - src_target_rect.miny)/(src_rb[1] - src_lt[1]);
			units_to_src_pixels.m20 = src_target_rect.minx - src_lt[0]*units_to_src_pixels.m00;
			units_to_src_pixels.m21 = src_target_rect.miny - src_lt[1]*units_to_src_pixels.m11;

			Vector p = units_to_src_pixels.get_transformed(pos);
			Rect src_target_rectf(src_target_rect.minx, src_target_rect.miny, src_target_rect.maxx, src_target_rect.maxy);
			if (src_target_rectf.is_inside(p)) {
				rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lock((*i)->target_surface);
				if (lock) return lock->get_surface().linear_sample(p[0], p[1]);
			}
		}
	}
	return Color(0.0, 0.0, 0.0, 0.0);
}

bool
Layer_RenderingTask::accelerated_render(Context /* context */, Surface *surface, int /* quality */, const RendDesc &renddesc, ProgressCallback * /* cb */) const
{
	assert(surface);

	surface->set_wh(renddesc.get_w(), renddesc.get_h());
	surface->clear();

	RectInt dest_target_rect(0, 0, renddesc.get_w(), renddesc.get_h());
	Vector dest_lt = renddesc.get_tl();
	Vector dest_rb = renddesc.get_br();
	if ( dest_target_rect.valid()
	  && approximate_not_equal(dest_lt[0], dest_rb[0])
	  && approximate_not_equal(dest_lt[1], dest_rb[1]) )
	{
		Matrix units_to_dest_pixels;
		units_to_dest_pixels.m00 = (dest_target_rect.maxx - dest_target_rect.minx)/(dest_rb[0] - dest_lt[0]);
		units_to_dest_pixels.m11 = (dest_target_rect.maxy - dest_target_rect.miny)/(dest_rb[1] - dest_lt[1]);
		units_to_dest_pixels.m20 = dest_target_rect.minx - dest_lt[0]*units_to_dest_pixels.m00;
		units_to_dest_pixels.m21 = dest_target_rect.miny - dest_lt[1]*units_to_dest_pixels.m11;

		for(rendering::Task::List::const_reverse_iterator ri = tasks.rbegin(); ri != tasks.rend(); ++ri)
		{
			if (*ri && (*ri)->is_valid())
			{
				RectInt src_target_rect = (*ri)->target_rect;
				Vector src_lt = (*ri)->source_rect.get_min();
				Vector src_rb = (*ri)->source_rect.get_max();

				Matrix src_pixels_to_units;
				src_pixels_to_units.m00 = (src_rb[0] - src_lt[0])/(src_target_rect.maxx - src_target_rect.minx);
				src_pixels_to_units.m11 = (src_rb[1] - src_lt[1])/(src_target_rect.maxy - src_target_rect.miny);
				src_pixels_to_units.m20 = src_lt[0] - src_target_rect.minx*src_pixels_to_units.m00;
				src_pixels_to_units.m21 = src_lt[1] - src_target_rect.miny*src_pixels_to_units.m11;

				Matrix transformation = units_to_dest_pixels * src_pixels_to_units;

				rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lock((*ri)->target_surface);
				if (lock)
					rendering::software::Resample::resample(
						*surface,
						dest_target_rect,
						lock->get_surface(),
						src_target_rect,
						transformation,
						Color::INTERPOLATION_LINEAR,
						false,
						1.f,
						Color::BLEND_COMPOSITE );
			}
		}
	}

	//debug::DebugSurface::save_to_file(*surface, "Layer_RenderingTask");

	return true;
}
