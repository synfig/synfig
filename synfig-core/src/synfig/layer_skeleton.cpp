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
#include "valuenode_bone.h"

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
	param_name(ValueBase("skeleton")),
	param_bone_shape_width(ValueBase(0.1))
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

	set_exclude_from_rendering(true);
	Layer_Polygon::set_param("color", ValueBase(Color(0.5, 0.5, 1.0, 0.75)));
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
	// lock color param
	if (param == "color") return false;

	IMPORT_VALUE(param_name);

	if (param=="bones" && param_bones.get_type()==value.get_type())
	{
		param_bones = value;
		sync();
		return true;
	}

	if (param=="bone_shape_width" && param_bone_shape_width.get_type()==value.get_type())
	{
		param_bone_shape_width = value;
		sync();
		return true;
	}

	return Layer_Polygon::set_param(param,value);
}

ValueBase
Layer_Skeleton::get_param(const String &param)const
{
	EXPORT_VALUE(param_name);
	EXPORT_VALUE(param_bones);
	EXPORT_VALUE(param_bone_shape_width);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Polygon::get_param(param);
}

Layer::Vocab
Layer_Skeleton::get_param_vocab()const
{
	Layer::Vocab ret(Layer::get_param_vocab());

	// Params from layer Layer_Shape
	//ret.push_back(ParamDesc("color")
	//	.set_local_name(_("Color"))
	//);

	// Self params
	ret.push_back(ParamDesc("name")
		.set_local_name(_("Name"))
	);
	ret.push_back(ParamDesc("bones")
		.set_local_name(_("Bones"))
	);
	ret.push_back(ParamDesc("bone_shape_width")
		.set_local_name(_("Bone Shape Width"))
	);

	return ret;
}

void
Layer_Skeleton::set_time(IndependentContext context, Time time)const
{
	const_cast<Layer_Skeleton*>(this)->sync();
	context.set_time(time);
}

void
Layer_Skeleton::set_time(IndependentContext context, Time time, const Point &pos)const
{
	const_cast<Layer_Skeleton*>(this)->sync();
	context.set_time(time,pos);
}

void
Layer_Skeleton::sync()
{
 	const std::vector<ValueBase> &list = param_bones.get_list();
	Real width = param_bone_shape_width.get(Real());

	clear();
	for(std::vector<ValueBase>::const_iterator i = list.begin(); i != list.end(); ++i)
 	{
		if (!i->same_type_as(Bone())) continue;
 		const Bone &bone = i->get(Bone());
 		Matrix matrix = bone.get_animated_matrix();
 		Vector origin = matrix.get_transformed(Vector(0.0, 0.0));
 		Vector direction = matrix.get_transformed(Vector(1.0, 0.0), false).norm();
 		Real length = bone.get_length() * bone.get_scalelx();

 		Vector &o = origin;
 		Vector dx = direction * length;
 		Vector dy = direction.perp() * width;

		std::vector<Point> vector_list;
		vector_list.reserve(4);
 		vector_list.push_back(o + dy);
 		vector_list.push_back(o + dy + dx);
 		vector_list.push_back(o - dy + dx);
 		vector_list.push_back(o - dy);
		add_polygon(vector_list);
 		upload_polygon(vector_list);
 	}
}
