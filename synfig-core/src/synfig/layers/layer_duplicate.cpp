/* === S Y N F I G ========================================================= */
/*!	\file layer_duplicate.cpp
**	\brief Implementation of the "Duplicate" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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
using namespace std;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Duplicate);
SYNFIG_LAYER_SET_NAME(Layer_Duplicate,"duplicate");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Duplicate,N_("Duplicate"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Duplicate,N_("Other"));
SYNFIG_LAYER_SET_VERSION(Layer_Duplicate,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Layer_Duplicate,"$Id$");

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

	std::lock_guard<std::mutex> lock(mutex_);
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

bool
Layer_Duplicate::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	if(quality == 10)
		return context.accelerated_render(surface,quality,renddesc,cb);

	if(context->empty())
	{
		surface->set_wh(renddesc.get_w(),renddesc.get_h());
		surface->clear();
		return true;
	}

	SuperCallback subimagecb;
	Surface tmp;
	int i = 0;

	handle<ValueNode_Duplicate> duplicate_param = get_duplicate_param();
	if (!duplicate_param) return context.accelerated_render(surface,quality,renddesc,cb);

	surface->set_wh(renddesc.get_w(),renddesc.get_h());
	surface->clear();

	Color::BlendMethod blend_method(get_blend_method());
	Time time_cur = get_time_mark();
	int steps = duplicate_param->count_steps(time_cur);

	std::lock_guard<std::mutex> lock(mutex_);
	duplicate_param->reset_index(time_cur);
	do
	{
		subimagecb=SuperCallback(cb,i*(5000/steps),(i+1)*(5000/steps),5000);
		context.set_time(time_cur, true);
		if(!context.accelerated_render(&tmp,quality,renddesc,&subimagecb)) return false;

		Surface::alpha_pen apen(surface->begin());
		apen.set_alpha(get_amount());
		// \todo have a checkbox allowing use of 'behind' to reverse the order?
		apen.set_blend_method(i ? blend_method : Color::BLEND_COMPOSITE);
		tmp.blit_to(apen);
		i++;
	} while (duplicate_param->step(time_cur));

	return true;
}

/////
bool
Layer_Duplicate::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	if(quality == 10)
		return context.accelerated_cairorender(cr,quality,renddesc,cb);
	
	if(context->empty())
	{
		cairo_save(cr);
		cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
		cairo_paint(cr);
		cairo_restore(cr);
		return true;
	}
	
	SuperCallback subimagecb;
	
	int i = 0;
	
	handle<ValueNode_Duplicate> duplicate_param = get_duplicate_param();
	if (!duplicate_param) return context.accelerated_cairorender(cr,quality,renddesc,cb);
	
	Color::BlendMethod blend_method(get_blend_method());
	Time time_cur = get_time_mark();
	int steps = duplicate_param->count_steps(time_cur);
	
	std::lock_guard<std::mutex> lock(mutex_);
	duplicate_param->reset_index(time_cur);
	cairo_save(cr);
	do
	{
		subimagecb=SuperCallback(cb,i*(5000/steps),(i+1)*(5000/steps),5000);
		context.set_time(time_cur, true);
		cairo_push_group(cr);
		if(!context.accelerated_cairorender(cr,quality,renddesc,&subimagecb))
		{
			cairo_pop_group(cr);
			return false;
		}
		cairo_pop_group_to_source(cr);
		// \todo have a checkbox allowing use of 'behind' to reverse the order?
		cairo_paint_with_alpha_operator(cr, get_amount(), i ? blend_method : Color::BLEND_COMPOSITE);
		i++;
	} while (duplicate_param->step(time_cur));
	cairo_restore(cr);
	return true;
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

	std::lock_guard<std::mutex> lock(mutex_);
	duplicate_param->reset_index(time_cur);
	do
	{
		context.set_time(time_cur, true);

		rendering::TaskBlend::Handle task_blend(new rendering::TaskBlend());
		task_blend->amount = amount;
		task_blend->blend_method = blend_method;
		task_blend->sub_task_a() = task;
		task_blend->sub_task_b() = context.build_rendering_task();
		task = task_blend;
	}
	while (duplicate_param->step(time_cur));

	return task;
}
