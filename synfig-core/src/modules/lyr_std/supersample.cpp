/* === S Y N F I G ========================================================= */
/*!	\file supersample.cpp
**	\brief Implementation of the "Super Sample" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011-2013 Carlos López
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/localization.h>

#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/rendering/common/task/tasktransformation.h>
#include <synfig/string.h>
#include <synfig/value.h>

#include "supersample.h"

#endif

using namespace etl;
using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(SuperSample);
SYNFIG_LAYER_SET_NAME(SuperSample,"super_sample");
SYNFIG_LAYER_SET_LOCAL_NAME(SuperSample,N_("Super Sample"));
SYNFIG_LAYER_SET_CATEGORY(SuperSample,N_("Other"));
SYNFIG_LAYER_SET_VERSION(SuperSample,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

SuperSample::SuperSample():
param_width(ValueBase(int(2))),
param_height(ValueBase(int(2)))
{
	param_scanline=ValueBase(false);
	param_alpha_aware=ValueBase(true);
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();

}

bool
SuperSample::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_width,
		{
			int width=param_width.get(int());
			if(value.get(int()) < 1) width = 1;
			else width=value.get(int());
			param_width.set(width);
			return true;
		}
		);
	IMPORT_VALUE_PLUS(param_height,
		{
			int height=param_height.get(int());
			if(value.get(int()) < 1) height = 1;
			else height=value.get(int());
			param_height.set(height);
			return true;
		}
		);
	IMPORT_VALUE(param_scanline);
	IMPORT_VALUE(param_alpha_aware);

	return false;
}

ValueBase
SuperSample::get_param(const String& param)const
{
	EXPORT_VALUE(param_width);
	EXPORT_VALUE(param_height);
    EXPORT_VALUE(param_scanline);
    EXPORT_VALUE(param_alpha_aware);

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

Layer::Vocab
SuperSample::get_param_vocab(void)const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc("width")
		.set_local_name(_("Width"))
		.set_description(_("Width of the sample area (In pixels)"))
	);
	ret.push_back(ParamDesc("height")
		.set_local_name(_("Height"))
		.set_description(_("Height of the sample area (In pixels)"))
	);
	ret.push_back(ParamDesc("scanline")
		.set_local_name(_("Use Parametric"))
		.set_description(_("When checked, uses the Parametric Renderer"))
	);
	ret.push_back(ParamDesc("alpha_aware")
		.set_local_name(_("Be Alpha Safe"))
		.set_description(_("When checked, avoids alpha artifacts"))
	);

	return ret;
}

Rect
SuperSample::get_bounding_rect(Context context)const
{
	return context.get_full_bounding_rect();
}

rendering::Task::Handle
SuperSample::build_rendering_task_vfunc(Context context)const
{
	int width = param_width.get(int());
	int height = param_height.get(int());
	if (width < 1) width = 1;
	if (height < 1) height = 1;

	rendering::Task::Handle sub_task = context.build_rendering_task();
	if (width == 1 && height == 1)
		return sub_task;

	rendering::TaskTransformationAffine::Handle task_transformation(new rendering::TaskTransformationAffine());
	task_transformation->supersample[0] = width;
	task_transformation->supersample[1] = height;
	task_transformation->sub_task() = sub_task;
	return task_transformation;
}
