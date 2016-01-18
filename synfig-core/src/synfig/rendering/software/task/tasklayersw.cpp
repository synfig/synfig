/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/tasklayersw.cpp
**	\brief TaskLayerSW
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

#include "tasklayersw.h"

#include "../surfacesw.h"
#include <synfig/guid.h>
#include <synfig/canvas.h>
#include <synfig/context.h>

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
TaskLayerSW::run(RunParams & /* params */) const
{
	synfig::Surface &target =
		SurfaceSW::Handle::cast_dynamic( target_surface )->get_surface();

	// TODO: target_rect

	RendDesc desc;
	desc.set_tl(get_source_rect_lt());
	desc.set_br(get_source_rect_rb());
	desc.set_wh(target.get_w(), target.get_h());
	desc.set_antialias(1);

	Canvas::Handle canvas = Canvas::create();
	return layer->accelerated_render(canvas->get_context(ContextParams()), &target, 4, desc, NULL);
}

/* === E N T R Y P O I N T ================================================= */
