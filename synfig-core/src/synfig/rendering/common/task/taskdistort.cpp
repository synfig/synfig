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
	if (!sub_task()) {
		trunc_to_zero();
		return;
	}
	if (!is_valid_coords()) {
		sub_task()->set_coords_zero();
		return;
	}

	const Vector ppu = get_pixels_per_unit();

	Matrix bounds_transformation;
	bounds_transformation.m00 = ppu[0];
	bounds_transformation.m11 = ppu[1];
	bounds_transformation.m20 = target_rect.minx - ppu[0]*source_rect.minx;
	bounds_transformation.m21 = target_rect.miny - ppu[1]*source_rect.miny;

	Matrix inv_matrix = bounds_transformation.get_inverted();

	required_source_rect = compute_required_source_rect(source_rect, inv_matrix);
	sub_task()->set_coords(required_source_rect, target_rect.get_size());
}
