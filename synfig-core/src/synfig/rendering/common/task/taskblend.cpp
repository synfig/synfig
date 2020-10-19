/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskblend.cpp
**	\brief TaskBlend
**
**	$Id$
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

#include "taskblend.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


SYNFIG_EXPORT Task::Token TaskBlend::token(
	DescAbstract<TaskBlend>("Blend") );

int
TaskBlend::get_pass_subtask_index() const
{
	bool a = (bool)sub_task_a();
	bool b = (bool)sub_task_b();
	if (!a && !b)
		return PASSTO_NO_TASK;
	if (!a && Color::is_onto(blend_method))
		return PASSTO_NO_TASK;
	if (blend_method == Color::BLEND_COMPOSITE) {
		if (!b)
			return 0;
		if (approximate_equal_lp(amount, ColorReal(0.0)))
			return a ? 0 : PASSTO_NO_TASK;
		if (!a && approximate_equal_lp(amount, ColorReal(1.0)))
			return 1;
	}
	return PASSTO_THIS_TASK;
}

Rect
TaskBlend::calc_bounds() const
{
	Rect ra = sub_task_a() ? sub_task_a()->get_bounds() : Rect::zero();
	Rect rb = sub_task_b() ? sub_task_b()->get_bounds() : Rect::zero();
	Rect bounds = ra | rb;
	if (Color::is_onto(blend_method))
		bounds &= ra;
	if (approximate_equal(amount, Color::value_type(1)) && Color::is_straight(blend_method))
		bounds &= rb;
	return bounds;
}

/* === E N T R Y P O I N T ================================================= */
