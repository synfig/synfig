/* === S Y N F I G ========================================================= */
/*!	\file layer_skeleton.cpp
**	\brief Implementation of the "Layer_Skeleton" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2008 Chris Moore
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

#include "string.h"
#include "layer_skeleton.h"
#include "time.h"
#include "context.h"
#include "paramdesc.h"
#include "renddesc.h"
#include "surface.h"
#include "value.h"
#include "valuenode.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Skeleton);
SYNFIG_LAYER_SET_NAME(Layer_Skeleton,"skeleton");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Skeleton,N_("Skeleton"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Skeleton,N_("Other"));
SYNFIG_LAYER_SET_VERSION(Layer_Skeleton,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Layer_Skeleton,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Layer_Skeleton::Layer_Skeleton():
	param_name(ValueBase("skeleton"))
{
	std::vector<synfig::Bone> bones;
	int bone_count = 1;
	if (getenv("SYNFIG_NUMBER_OF_BONES_IN_SKELETON"))
		bone_count = atoi(getenv("SYNFIG_NUMBER_OF_BONES_IN_SKELETON"));
	if (bone_count < 1) bone_count = 1;
	else if (bone_count > 10) bone_count = 10;

	while (bone_count--)
		bones.push_back(Bone());

	param_bones.set(bones);
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

#ifdef _DEBUG
Layer_Skeleton::~Layer_Skeleton()
{
	if (getenv("SYNFIG_DEBUG_DESTRUCTORS"))
		printf("%s:%d ~Layer_Skeleton()\n", __FILE__, __LINE__);
}
#endif

bool
Layer_Skeleton::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_name);

	if (param=="bones" && param_bones.get_type()==value.get_type())
	{
		param_bones = value;
		return true;
	}

	return Layer::set_param(param,value);
}

ValueBase
Layer_Skeleton::get_param(const String &param)const
{
	EXPORT_VALUE(param_name);
	EXPORT_VALUE(param_bones);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer::get_param(param);
}

Layer::Vocab
Layer_Skeleton::get_param_vocab()const
{
	Layer::Vocab ret(Layer::get_param_vocab());

	ret.push_back(ParamDesc("name")
		.set_local_name(_("Name"))
	);

	ret.push_back(ParamDesc("bones")
		.set_local_name(_("Bones"))
	);

	return ret;
}

bool
Layer_Skeleton::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	return context.accelerated_render(surface,quality,renddesc,cb);
}

bool
Layer_Skeleton::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	return context.accelerated_cairorender(cr,quality,renddesc,cb);
}
