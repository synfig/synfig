/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/optimizer/optimizerblendsw.cpp
**	\brief OptimizerBlendSW
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

#include "optimizerblendsw.h"

#include "../task/taskblendsw.h"
#include "../surfacesw.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OptimizerBlendSW::run(const RunParams& params) const
{
	TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(params.ref_task);
	if ( blend
	  && blend->target_surface
	  && blend.type_equal<TaskBlend>() )
	{
		TaskBlendSW::Handle blend_sw;
		init_and_assign_all<SurfaceSW>(blend_sw, blend);

		// TODO: Are we really need to check 'is_temporary' flag?
		if ( blend_sw->sub_task_a()->target_surface->is_temporary )
		{
			blend_sw->sub_task_a()->target_surface = blend_sw->target_surface;
			blend_sw->sub_task_a()->move_target_rect(
				blend_sw->get_target_offset() );
			blend_sw->offset_a = -blend_sw->get_target_offset();
			assert( blend_sw->sub_task_a()->check() );
		}
		else
		{
			blend_sw->offset_a = blend_sw->sub_task_a()->get_target_offset();
			blend_sw->sub_task_a()->set_target_origin( VectorInt::zero() );
			blend_sw->sub_task_a()->target_surface->set_size(
				blend_sw->sub_task_a()->get_target_rect().maxx,
				blend_sw->sub_task_a()->get_target_rect().maxy );
			assert( blend_sw->sub_task_a()->check() );
		}

		blend_sw->offset_b = blend_sw->sub_task_b()->get_target_offset();
		blend_sw->sub_task_b()->set_target_origin( VectorInt::zero() );
		blend_sw->sub_task_b()->target_surface->set_size(
			blend_sw->sub_task_b()->get_target_rect().maxx,
			blend_sw->sub_task_b()->get_target_rect().maxy );
		assert( blend_sw->sub_task_b()->check() );

		apply(params, blend_sw);
	}
}

/* === E N T R Y P O I N T ================================================= */
