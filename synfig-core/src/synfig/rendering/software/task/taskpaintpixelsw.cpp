/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskpaintpixelsw.cpp
**	\brief TaskPaintPixelSW
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2022 Synfig Contributors
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

#include "taskpaintpixelsw.h"

#include <synfig/general.h>
#include <synfig/localization.h>

#endif

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
rendering::TaskPaintPixelSW::on_target_set_as_source() {
	Task::Handle task = Task::Handle::cast_dynamic(this);

	Task::Handle &subtask = task->sub_task(0);
	if ( subtask
		&& subtask->target_surface == task->target_surface
		&& !Color::is_straight(blend_method) )
	{
		task->trunc_by_bounds();
		subtask->source_rect = task->source_rect;
		subtask->target_rect = task->target_rect;
	}
}

synfig::Color::BlendMethodFlags
synfig::rendering::TaskPaintPixelSW::get_supported_blend_methods() const
{
	return Color::BLEND_METHODS_ALL;
}

bool
synfig::rendering::TaskPaintPixelSW::run_task() const
{
	const Task* task = dynamic_cast<const Task*>(this);
	if (!task)
		return false;
	if (!task->is_valid())
		return true;

	Vector ppu = task->get_pixels_per_unit();

	const synfig::RectInt& target_rect = task->target_rect;

	Matrix bounds_transformation;
	bounds_transformation.m00 = ppu[0];
	bounds_transformation.m11 = ppu[1];
	bounds_transformation.m20 = target_rect.minx - ppu[0]*task->source_rect.minx;
	bounds_transformation.m21 = target_rect.miny - ppu[1]*task->source_rect.miny;

	Matrix matrix = bounds_transformation;
	if (auto interface_transformation = dynamic_cast<const TaskInterfaceTransformation*>(this)) {
		if (auto affine = TransformationAffine::Handle::cast_dynamic(interface_transformation->get_transformation()))
			matrix *= affine->matrix;
		else
			synfig::error(_("Internal Error: unsupported transformation for %s. It should be TransformationAffine."), task->get_token()->name.c_str());
	}
	Matrix inv_matrix = matrix.get_inverted();

	int tw = target_rect.get_width();
	Vector dx = inv_matrix.axis_x();
	Vector dy = inv_matrix.axis_y() - dx*(Real)tw;
	Vector p = inv_matrix.get_transformed( Vector((Real)target_rect.minx, (Real)target_rect.miny) );

	pre_run(matrix, inv_matrix);

	LockWrite la(task);
	if (!la)
		return false;

	synfig::Surface::alpha_pen apen(la->get_surface().get_pen(target_rect.minx, target_rect.miny));
	ColorReal amount = blend ? this->amount : ColorReal(1.0);
	apen.set_blend_method(blend ? blend_method : Color::BLEND_COMPOSITE);

	for(int iy = target_rect.miny; iy < target_rect.maxy; ++iy, p += dy, apen.inc_y(), apen.dec_x(tw)) {
		for(int ix = target_rect.minx; ix < target_rect.maxx; ++ix, p += dx, apen.inc_x()) {
			apen.put_value(get_color(p), amount);
		}
	}

	return true;
}

bool
synfig::rendering::TaskFilterPixelSW::run_task() const
{
	const TaskPixelProcessorBase* task = dynamic_cast<const TaskPixelProcessorBase*>(this);
	if (!task) {
		const Task* task = dynamic_cast<const Task*>(this);
		if (task)
			synfig::error(_("Internal error: Cobra task %s isn't a TaskPixelProcessorBase"), task->get_token()->name.c_str());
		else
			synfig::error(_("Internal error: this TaskFilterPixelSW isn't even a Task"));
		return false;
	}
	if (!task->is_valid())
		return true;
	if (!task->sub_task())
		return true;

	const RectInt r = task->target_rect;

	if (r.valid()) {
		VectorInt offset = task->get_offset();

		RectInt ra = task->sub_task()->target_rect + r.get_min() + offset;
		if (ra.valid()) {
			rect_set_intersect(ra, ra, r);
			if (ra.valid()) {
				LockWrite ldst(task);
				if (!ldst) return false;

				synfig::Surface &c = ldst->get_surface();

				const Vector upp = task->get_units_per_pixel();
				const Rect w = task->source_rect;

				// for raster r to 'world' w conversion: w = upp * r + (w0 * r1 + w1 * r0) / r_size
				Vector constant;
				constant[0] = (w.get_min()[0] * r.get_max()[0] + w.get_max()[0] * r.get_min()[0]) / r.get_size()[0];
				constant[1] = (w.get_min()[1] * r.get_max()[1] + w.get_max()[1] * r.get_min()[1]) / r.get_size()[1];

				Matrix3 raster_to_world;
				raster_to_world.m00 = upp[0];
				raster_to_world.m11 = upp[1];
				raster_to_world.m20 = constant[0];
				raster_to_world.m21 = constant[1];

				pre_run(raster_to_world);

				LockRead lsrc(task->sub_task());
				if (!lsrc) return false;

				const synfig::Surface &a = lsrc->get_surface();

				for (int y = ra.miny; y < ra.maxy; ++y) {
					const Color *ca = &a[y - r.miny + offset[1]][ra.minx - r.minx + offset[0]];
					Color *cc = &c[y][ra.minx];
					Real v = upp[1] * y + constant[1];
					for (int x = ra.minx; x < ra.maxx; ++x, ++ca, ++cc) {
						Real u = upp[0] * x + constant[0];
						*cc = get_color(Vector(u, v), *ca);
					}
				}
			}
		}
	}

	return true;
}
