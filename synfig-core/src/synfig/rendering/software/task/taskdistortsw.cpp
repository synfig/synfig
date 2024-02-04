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

#endif

/* === U S I N G =========================================================== */

using namespace synfig::rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool TaskDistortSW::run_task(const rendering::TaskDistort& task) const
{
	if (!task.sub_task())
		return true;
	if (!task.is_valid())
		return true;

	const Vector ppu = task.get_pixels_per_unit();

	rendering::TaskSW::LockWrite la(&task);
	if (!la)
		return false;

	rendering::TaskSW::LockRead lb(task.sub_task());
	if (!lb)
		return false;
	const Vector ppub = task.sub_task()->get_pixels_per_unit();
	const int tw = task.target_rect.get_width();
	const synfig::Surface& b = lb->get_surface();

	synfig::Surface::pen pen(la->get_surface().get_pen(task.target_rect.minx, task.target_rect.miny));

	Matrix bounds_transformation;
	bounds_transformation.m00 = ppu[0];
	bounds_transformation.m11 = ppu[1];
	bounds_transformation.m20 = task.target_rect.minx - ppu[0]*task.source_rect.minx;
	bounds_transformation.m21 = task.target_rect.miny - ppu[1]*task.source_rect.miny;

	Matrix inv_matrix = bounds_transformation.get_inverted();

	Vector dx = inv_matrix.axis_x();
	Vector dy = inv_matrix.axis_y() - dx*(Real)tw;
	Vector p = inv_matrix.get_transformed( Vector((Real)task.target_rect.minx, (Real)task.target_rect.miny) );

	for (int iy = task.target_rect.miny; iy < task.target_rect.maxy; ++iy, p += dy, pen.inc_y(), pen.dec_x(tw)) {
		for (int ix = task.target_rect.minx; ix < task.target_rect.maxx; ++ix, p += dx, pen.inc_x()) {
			Point tmp = point_vfunc(p);

			float u = (tmp[0]-task.required_source_rect.minx)*ppub[0];
			float v = (tmp[1]-task.required_source_rect.miny)*ppub[1];
			if (u<0 || v<0 || u>=b.get_w() || v>=b.get_h() || std::isnan(u) || std::isnan(v)) {
				// problem! It shouldn't happen!!
				pen.put_value(Color::magenta());
				continue;
			} else {
				pen.put_value(b.cubic_sample(u,v));
			}
		}
	}

	return true;
}
