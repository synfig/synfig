/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/tasklayer.cpp
**	\brief TaskLayer
**
**	$Id$
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include "tasklayer.h"

#include <synfig/context.h>

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Rect
TaskLayer::calc_bounds() const
{
	if (!layer) return Rect::zero();

	CanvasBase fake_canvas_base;
	if (sub_layer) fake_canvas_base.push_back(sub_layer);
	fake_canvas_base.push_back(Layer::Handle());
	return layer->get_full_bounding_rect(Context(fake_canvas_base.begin(), ContextParams()));
}

/* === E N T R Y P O I N T ================================================= */
