/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/optimizer/optimizerpixelgammasw.cpp
**	\brief OptimizerPixelGammaSW
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

#include "optimizerpixelgammasw.h"

#include "../task/taskpixelgammasw.h"
#include "../surfacesw.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerPixelGammaSW::run(const RunParams& params) const
{
	TaskPixelGamma::Handle gamma = TaskPixelGamma::Handle::cast_dynamic(params.ref_task);
	if ( gamma
	  && gamma->target_surface
	  && gamma.type_equal<TaskPixelGamma>() )
	{
		TaskPixelGammaSW::Handle gamma_sw;
		init_and_assign_all<rendering::SurfaceSW>(gamma_sw, gamma);

		// TODO: Are we really need to check 'is_temporary' flag?
		if ( gamma_sw->sub_task()->target_surface->is_temporary )
		{
			gamma_sw->sub_task()->target_surface = gamma_sw->target_surface;
			gamma_sw->sub_task()->move_target_rect(
					gamma_sw->get_target_offset() );
		}
		else
		{
			gamma_sw->sub_task()->set_target_origin( VectorInt::zero() );
			gamma_sw->sub_task()->target_surface->set_size(
					gamma_sw->sub_task()->get_target_rect().maxx,
					gamma_sw->sub_task()->get_target_rect().maxy );
		}
		assert( gamma_sw->sub_task()->check() );

		apply(params, gamma_sw);
	}
}

/* === E N T R Y P O I N T ================================================= */
