/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskblur.cpp
**	\brief TaskBlur
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

#include "taskblur.h"

#include "../../software/function/blur.h"


#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


SYNFIG_EXPORT Task::Token TaskBlur::token(
	DescAbstract<TaskBlur>("Blur") );

Rect
TaskBlur::calc_bounds() const
{
	if (!sub_task()) return Rect::zero();
	Rect bounds = sub_task()->get_bounds();
	Vector size = blur.size * software::Blur::get_extra_size(blur.type);
	size[0] = fabs(size[0]) + 1.0;
	size[1] = fabs(size[1]) + 1.0;
	bounds.minx -= size[0];
	bounds.miny -= size[1];
	bounds.maxx += size[0];
	bounds.maxy += size[1];
	return bounds;
}

void
TaskBlur::set_coords_sub_tasks()
{
	if (!sub_task())
		{ trunc_to_zero(); return; }
	if (!is_valid_coords())
		{ sub_task()->set_coords_zero(); return; }

	Vector ppu = get_pixels_per_unit();
	Vector upp = get_units_per_pixel();

	VectorInt target_extra_size =
		software::Blur::get_extra_size(
			blur.type,
			blur.size.multiply_coords(ppu) );
	VectorInt sub_target_size = target_rect.get_size() + target_extra_size*2;

	Vector source_extra_size(
		target_extra_size[0]*upp[0],
		target_extra_size[1]*upp[1] );
	Rect sub_source_rect = source_rect;
	sub_source_rect.expand_x(source_extra_size[0]);
	sub_source_rect.expand_y(source_extra_size[1]);

	sub_task()->set_coords(sub_source_rect, sub_target_size);
}

/* === E N T R Y P O I N T ================================================= */
