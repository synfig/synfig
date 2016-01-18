/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskexpandsurfacesw.cpp
**	\brief TaskExpandSurfaceSW
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

#include "taskexpandsurfacesw.h"

#include "../surfacesw.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
TaskExpandSurfaceSW::run(RunParams & /* params */) const
{
	//synfig::Surface &a =
	//	SurfaceSW::Handle::cast_dynamic( target_surface )->get_surface();
	//const synfig::Surface &b =
	//	SurfaceSW::Handle::cast_dynamic( sub_task()->target_surface )->get_surface();

	// TODO:
	return false;
}

/* === E N T R Y P O I N T ================================================= */
