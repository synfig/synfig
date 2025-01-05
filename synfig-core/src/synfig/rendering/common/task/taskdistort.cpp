/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskdistort.cpp
**	\brief TaskDistort
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

# include "taskdistort.h"

#endif

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
rendering::TaskDistort::set_coords_sub_tasks()
{
	if (sub_tasks.empty() || !sub_task()) {
		trunc_to_zero();
		return;
	}
	if (!is_valid_coords()) {
		sub_task()->set_coords_zero();
		return;
	}

	const Vector ppu = get_pixels_per_unit();
	const Vector upp = get_units_per_pixel();

	Matrix raster_to_world_transformation;
	{
		raster_to_world_transformation.m00 = upp[0];
		raster_to_world_transformation.m11 = upp[1];
		raster_to_world_transformation.m20 = source_rect.minx - upp[0]*target_rect.minx;
		raster_to_world_transformation.m21 = source_rect.miny - upp[1]*target_rect.miny;
	}

	required_source_rect = compute_required_source_rect(source_rect, raster_to_world_transformation);
	// Add 3 pixels due to possible cubic or cosine sampling
	constexpr int padding = 3;
	required_source_rect.minx -= padding*upp[0];
	required_source_rect.maxx += padding*upp[0];
	required_source_rect.miny -= padding*upp[1];
	required_source_rect.maxy += padding*upp[1];

	Vector new_target_size = required_source_rect.get_size().multiply_coords(ppu);

	sub_task()->set_coords(required_source_rect, VectorInt(std::round(new_target_size[0]), std::round(new_target_size[1])));
}
