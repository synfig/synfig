/* === S Y N F I G ========================================================= */
/*!	\file layer_composite_fork.cpp
**	\brief Layer_CompositeFork implementation
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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

#include <synfig/localization.h>

#include "layer_composite_fork.h"
#include "layer_rendering_task.h"

#include <synfig/context.h>
#include <synfig/rendering/common/task/tasklayer.h>
#include <synfig/rendering/common/task/taskblend.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Layer_CompositeFork::Layer_CompositeFork(Real amount, Color::BlendMethod blend_method):
	Layer_Composite(amount, blend_method) { }

rendering::Task::Handle
Layer_CompositeFork::build_composite_fork_task_vfunc(ContextParams context_params, rendering::Task::Handle /* sub_task */)const
{
	return build_composite_task_vfunc(context_params);
}

rendering::Task::Handle
Layer_CompositeFork::build_rendering_task_vfunc(Context context)const
{
	rendering::TaskBlend::Handle task_blend(new rendering::TaskBlend());
	task_blend->amount = get_amount() * Context::z_depth_visibility(context.get_params(), *this);
	task_blend->blend_method = get_blend_method();
	task_blend->sub_task_a() = context.build_rendering_task();
	task_blend->sub_task_b() = build_composite_fork_task_vfunc(context.get_params(), task_blend->sub_task_a());
	return task_blend;
}
