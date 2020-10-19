/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskpixelprocessor.cpp
**	\brief TaskPixelProcessor
**
**	$Id$
**
**	\legal
**	......... ... 2016-2018 Ivan Mahonin
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

#include "taskpixelprocessor.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


SYNFIG_EXPORT Task::Token TaskPixelProcessor::token(
	DescAbstract<TaskPixelProcessor>("PixelProcessor") );
Task::Token TaskPixelGamma::token(
	DescAbstract<TaskPixelGamma, TaskPixelProcessor>("PixelGamma") );
SYNFIG_EXPORT Task::Token TaskPixelColorMatrix::token(
	DescAbstract<TaskPixelColorMatrix, TaskPixelProcessor>("PixelColorMatrix") );


Rect
TaskPixelProcessor::calc_bounds() const
{
	return is_transparent() ? Rect::zero()
	     : is_affects_transparent() ? Rect::infinite()
	     : sub_task() ? sub_task()->get_bounds()
	     : Rect::zero();
}

VectorInt
TaskPixelProcessor::get_offset() const
{
	if (!sub_task()) return VectorInt::zero();
	Vector offset = (sub_task()->source_rect.get_min() - source_rect.get_min()).multiply_coords(get_pixels_per_unit());
	return VectorInt((int)round(offset[0]), (int)round(offset[1])) - sub_task()->target_rect.get_min();
}

/* === E N T R Y P O I N T ================================================= */
