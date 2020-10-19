/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskcontour.cpp
**	\brief TaskContour
**
**	$Id$
**
**	\legal
**	......... ... 2018 Ivan Mahonin
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

#include "taskcontour.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


SYNFIG_EXPORT Task::Token TaskContour::token(
	DescAbstract<TaskContour>("Contour") );


Rect
TaskContour::calc_bounds() const
{
	return !contour        ? Rect::zero()
	     : contour->invert ? Rect::infinite()
         :                   contour->calc_bounds(transformation->matrix);
}

/* === E N T R Y P O I N T ================================================= */
