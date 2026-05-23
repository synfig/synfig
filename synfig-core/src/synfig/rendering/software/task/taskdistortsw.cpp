/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskdistortsw.cpp
**	\brief TaskDistortSW interface base implementation
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2024 Synfig Contributors
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
# include "pch.h"
#else
# ifdef HAVE_CONFIG_H
#  include <config.h>
# endif

# include "taskdistortsw.h"

# include <synfig/rendering/common/task/tasktransformation.h>
# include <synfig/general.h>
# include <synfig/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig::rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool TaskDistortSW::run_task(const rendering::TaskDistort& task) const
{
	if (!task.is_valid())
		return true;

	if (task.sub_tasks.empty() || !task.sub_tasks.at(0))
		return true;

	const rendering::Task::Handle& sub_task = task.sub_task();

	if (!sub_task->is_valid())
		return true;

	rendering::TaskSW::LockWrite la(&task);
	if (!la)
		return false;

	rendering::TaskSW::LockRead lb(sub_task);
	if (!lb)
		return false;

	const synfig::Surface& b = lb->get_surface();

	Matrix raster_to_world_transformation;
	{
		const Vector upp = task.get_units_per_pixel();
		raster_to_world_transformation.m00 = upp[0];
		raster_to_world_transformation.m11 = upp[1];
		raster_to_world_transformation.m20 = task.source_rect.minx - upp[0]*task.target_rect.minx;
		raster_to_world_transformation.m21 = task.source_rect.miny - upp[1]*task.target_rect.miny;
	}

	Matrix subtask_world_to_raster_transformation;
	{
		const Vector sub_ppu = sub_task->get_pixels_per_unit();
		subtask_world_to_raster_transformation.m00 = sub_ppu[0];
		subtask_world_to_raster_transformation.m11 = sub_ppu[1];
		subtask_world_to_raster_transformation.m20 = sub_task->target_rect.minx - sub_ppu[0]*sub_task->source_rect.minx;
		subtask_world_to_raster_transformation.m21 = sub_task->target_rect.miny - sub_ppu[1]*sub_task->source_rect.miny;
	}

	// Try to apply TransformationAffine if task has it
	Transformation::Handle non_trivial_transformation;
	Transformation::Handle inverted_non_trivial_transformation;
	{
		Transformation::Handle transformation;
		if (auto interface_transformation = dynamic_cast<const TaskInterfaceTransformation*>(this)) {
			transformation = interface_transformation->get_transformation();
		} else if (auto interface_transformation = dynamic_cast<const TaskInterfaceTransformationGetAndPass*>(this)) {
			transformation = interface_transformation->get_transformation();
		}

		if (transformation) {
			if (auto affine = TransformationAffine::Handle::cast_dynamic(transformation)) {
				raster_to_world_transformation = affine->matrix.get_inverted() * raster_to_world_transformation;
				subtask_world_to_raster_transformation *= affine->matrix;
			} else {
				inverted_non_trivial_transformation = transformation->create_inverted();
				if (inverted_non_trivial_transformation) {
					non_trivial_transformation = transformation;
				} else {
					synfig::error(_("Internal Error: unsupported transformation for %s. It should be invertible."), task.get_token()->name.c_str());
					return false;
				}
			}
		}
	}

	const int tw = task.target_rect.get_width();
	const Vector dx = raster_to_world_transformation.axis_x();
	const Vector dy = raster_to_world_transformation.axis_y() - dx*(Real)tw;
	Vector p = raster_to_world_transformation.get_transformed( Vector((Real)task.target_rect.minx, (Real)task.target_rect.miny) );
	synfig::Surface::pen pen(la->get_surface().get_pen(task.target_rect.minx, task.target_rect.miny));

	if (!non_trivial_transformation) {
		for (int iy = task.target_rect.miny; iy < task.target_rect.maxy; ++iy, p += dy, pen.inc_y(), pen.dec_x(tw)) {
			for (int ix = task.target_rect.minx; ix < task.target_rect.maxx; ++ix, p += dx, pen.inc_x()) {
				const Point distorted_point = point_vfunc(p);

				const Point raster_pos = subtask_world_to_raster_transformation.get_transformed(distorted_point);
				if (!raster_pos.is_valid()) {
					// problem! It shouldn't happen!! At least one of the coordinates is NaN
					pen.put_value(Color::cyan());
					continue;
				} else {
					const Real u = raster_pos[0];
					const Real v = raster_pos[1];
					if (u<0 || v<0 || u>=b.get_w() || v>=b.get_h()) {
						// problem! It shouldn't happen!!
						pen.put_value(Color::magenta());
						continue;
					} else {
						pen.put_value(b.cubic_sample(u,v));
					}
				}
			}
		}
	} else {
		// Basically the same loop above, but
		// we have to transform and de-transform each point to be distorted...
		// So, separated loops for performance.
		for (int iy = task.target_rect.miny; iy < task.target_rect.maxy; ++iy, p += dy, pen.inc_y(), pen.dec_x(tw)) {
			for (int ix = task.target_rect.minx; ix < task.target_rect.maxx; ++ix, p += dx, pen.inc_x()) {
				const Point distorted_point = non_trivial_transformation->transform(point_vfunc(inverted_non_trivial_transformation->transform(p)));

				const Point raster_pos = subtask_world_to_raster_transformation.get_transformed(distorted_point);
				if (!raster_pos.is_valid()) {
					// problem! It shouldn't happen!! At least one of the coordinates is NaN
					pen.put_value(Color::cyan());
					continue;
				} else {
					const Real u = raster_pos[0];
					const Real v = raster_pos[1];
					if (u<0 || v<0 || u>=b.get_w() || v>=b.get_h()) {
						// problem! It shouldn't happen!!
						pen.put_value(Color::magenta());
						continue;
					} else {
						pen.put_value(b.cubic_sample(u,v));
					}
				}
			}
		}
	}

	return true;
}
