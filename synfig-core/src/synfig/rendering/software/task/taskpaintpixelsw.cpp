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

	if (!is_filter_) {
		synfig::Surface::alpha_pen apen(la->get_surface().get_pen(target_rect.minx, target_rect.miny));
		ColorReal amount = blend ? this->amount : ColorReal(1.0);
		apen.set_blend_method(blend ? blend_method : Color::BLEND_COMPOSITE);
		for (int iy = target_rect.miny; iy < target_rect.maxy; ++iy, p += dy, apen.inc_y(), apen.dec_x(tw)) {
			for (int ix = target_rect.minx; ix < target_rect.maxx; ++ix, p += dx, apen.inc_x()) {
				apen.put_value(get_color(p), amount);
			}
		}
	} else {
		synfig::Surface::pen pen(la->get_surface().get_pen(target_rect.minx, target_rect.miny));
		for (int iy = target_rect.miny; iy < target_rect.maxy; ++iy, p += dy, pen.inc_y(), pen.dec_x(tw)) {
			for (int ix = target_rect.minx; ix < target_rect.maxx; ++ix, p += dx, pen.inc_x()) {
				pen.put_value(get_color(p, pen.get_value()));
			}
		}
	}

	return true;
}
