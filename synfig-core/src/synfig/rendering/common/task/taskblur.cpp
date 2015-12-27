/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskblur.cpp
**	\brief TaskBlur
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
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

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
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

Rect
TaskBlur::calc_bounds() const
{
	Rect bounds = sub_task() ? sub_task()->get_bounds() : Rect::zero();
	Vector size = blur.size * software::Blur::get_extra_size(blur.type) * 1.05;
	bounds.minx -= fabs(size[0]);
	bounds.miny -= fabs(size[1]);
	bounds.maxx += fabs(size[0]);
	bounds.maxy += fabs(size[1]);
	return bounds;
}

/* === E N T R Y P O I N T ================================================= */
