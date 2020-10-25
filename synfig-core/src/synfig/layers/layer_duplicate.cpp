/* === S Y N F I G ========================================================= */
/*!	\file layer_duplicate.cpp
**	\brief Implementation of the "Duplicate" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "layer_duplicate.h"

#include <synfig/general.h>
#include <synfig/localization.h>

#include <synfig/canvas.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/string.h>
#include <synfig/surface.h>
#include <synfig/time.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#include <synfig/rendering/common/task/taskblend.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Duplicate);
SYNFIG_LAYER_SET_NAME(Layer_Duplicate,"duplicate");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Duplicate,N_("Duplicate"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Duplicate,N_("Other"));
SYNFIG_LAYER_SET_VERSION(Layer_Duplicate,"0.1");

/* === M E M B E R S ======================================================= */

Layer_Duplicate::Layer_Duplicate():
	Layer_CompositeFork(1.0,Color::BLEND_COMPOSITE)
{
	LinkableValueNode* index_value_node = ValueNode_Duplicate::create(Real(3));
	connect_dynamic_param("index", index_value_node);

	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Layer::Handle
Layer_Duplicate::clone(Canvas::LooseHandle canvas, const GUID& deriv_guid)const
{
	Layer::Handle ret = (Layer::Handle)Layer_Composite::clone(canvas, deriv_guid);

	const DynamicParamList &dpl = dynamic_param_list();
	DynamicParamList::const_iterator iter = dpl.find("index");

	// if we have a dynamic "index" parameter, make a new one in the clone
	// it's not good to have two references to the same index valuenode,
	// or nested duplications cause an infinite loop
	if (iter != dpl.end())
		ret->connect_dynamic_param(iter->first,iter->second->clone(canvas, deriv_guid));

	return ret;
}

bool
Layer_Duplicate::set_param(const String &param, const ValueBase &value)
{
	IMPORT_VALUE(param_index);
	return Layer_Composite::set_param(param,value);
}

ValueBase
Layer_Duplicate::get_param(const String &param)const
{
 	EXPORT_VALUE(param_index);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Color
Layer_Duplicate::get_color(Context context, const Point &pos)const
{
	handle<ValueNode_Duplicate> duplicate_param = get_duplicate_param();
	if (!duplicate_param) return context.get_color(pos);

	Color::BlendMethod blend_method(get_blend_method());
	float amount(get_amount());
	Color color;

	std::lock_guard<std::mutex> lock(mutex);
	Time time_cur = get_time_mark();
	duplicate_param->reset_index(time_cur);
	do
	{
		context.set_time(time_cur+1);
		context.set_time(time_cur);
		color = Color::blend(context.get_color(pos),color,amount,blend_method);
	} while (duplicate_param->step(time_cur));

	return color;
}

Layer::Vocab
Layer_Duplicate::get_param_vocab()const
{
	Layer::Vocab ret;
	ret=Layer_Composite::get_param_vocab();

	ret.push_back(ParamDesc("index")
		.set_local_name(_("Index"))
		.set_description(_("Copy Index"))
	);

	return ret;
}

ValueNode_Duplicate::Handle
Layer_Duplicate::get_duplicate_param()const
{
	const DynamicParamList &dpl = dynamic_param_list();
	DynamicParamList::const_iterator iter = dpl.find("index");
	if (iter == dpl.end()) return NULL;
	etl::rhandle<ValueNode> param(iter->second);
	return ValueNode_Duplicate::Handle::cast_dynamic(param);
}

rendering::Task::Handle
Layer_Duplicate::build_rendering_task_vfunc(Context context) const
{
	handle<ValueNode_Duplicate> duplicate_param = get_duplicate_param();
	if (!duplicate_param)
		return context.build_rendering_task();

	Time time_cur = get_time_mark();

	ColorReal amount = get_amount() * Context::z_depth_visibility(context.get_params(), *this);
	Color::BlendMethod blend_method = get_blend_method();

	rendering::Task::Handle task;

	std::lock_guard<std::mutex> lock(mutex);
	duplicate_param->reset_index(time_cur);
	ContextParams dup_context_params(context.get_params());
	dup_context_params.force_set_time = true;
	Context dup_context(context, dup_context_params);
	do
	{
		rendering::TaskBlend::Handle task_blend(new rendering::TaskBlend());
		task_blend->amount = amount;
		task_blend->blend_method = blend_method;
		task_blend->sub_task_a() = task;
		task_blend->sub_task_b() = dup_context.build_rendering_task();
		task = task_blend;
	}
	while (duplicate_param->step(time_cur));

	return task;
}
