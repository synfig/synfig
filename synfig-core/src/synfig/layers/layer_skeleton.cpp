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

#include "layer_skeleton.h"

#include <synfig/general.h>
#include <synfig/localization.h>

#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/string.h>
#include <synfig/surface.h>
#include <synfig/time.h>
#include <synfig/value.h>
#include <synfig/valuenodes/valuenode_bone.h>

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
	param_name(ValueBase((const char*)"skeleton"))
{
	std::vector<synfig::Bone> bones;
	int bone_count = 1;
	if (getenv("SYNFIG_NUMBER_OF_BONES_IN_SKELETON"))
		bone_count = atoi(getenv("SYNFIG_NUMBER_OF_BONES_IN_SKELETON"));
	if (bone_count < 1) bone_count = 1;
	else if (bone_count > 10) bone_count = 10;

	while (bone_count--)
		bones.push_back(Bone());

	param_bones.set_list_of(bones);
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();

	set_exclude_from_rendering(true);
	Layer_Shape::set_param("color", ValueBase(Color(0.5, 0.5, 1.0, 1.0)));
	Layer_Composite::set_param("amount", ValueBase(Real(0.5)));
}

#ifdef _DEBUG
Layer_Skeleton::~Layer_Skeleton()
{
	if (getenv("SYNFIG_DEBUG_DESTRUCTORS"))
		printf("%s:%d ~Layer_Skeleton()\n", __FILE__, __LINE__);
}
#endif

bool
Layer_Skeleton::set_param(const String &param, const ValueBase &value)
{
	IMPORT_VALUE(param_name);

	if (param=="bones" && param_bones.get_type()==value.get_type())
	{
		param_bones = value;
		force_sync();
		return true;
	}

	// Skip shape, polygon and composite parameters
	if (param == "amount")
		return Layer_Composite::set_param(param,value);
	return Layer::set_param(param,value);
}

ValueBase
Layer_Skeleton::get_param(const String &param)const
{
	EXPORT_VALUE(param_name);
	EXPORT_VALUE(param_bones);

	EXPORT_NAME();
	EXPORT_VERSION();

	// Skip shape, polygon and composite parameters
	if (param == "amount")
		return Layer_Composite::get_param(param);
	return Layer::get_param(param);
}

Layer::Vocab
Layer_Skeleton::get_param_vocab()const
{
	// Skip shape, polygon and composite parameters
	Layer::Vocab ret(Layer::get_param_vocab());

	ret.push_back(ParamDesc("amount")
		.set_local_name(_("Opacity"))
		.set_description(_("Alpha channel of the layer"))
	);

	// Self params
	ret.push_back(ParamDesc("name")
		.set_local_name(_("Name"))
	);
	ret.push_back(ParamDesc("bones")
		.set_local_name(_("Bones"))
	);

	return ret;
}

void
Layer_Skeleton::sync_vfunc()
{
 	const std::vector<ValueBase> &list = param_bones.get_list();

	static const Real precision = 0.000000001;
	int segments_count = 64;
	Real segment_angle = 2*PI/(Real)segments_count;

	clear();
	for(std::vector<ValueBase>::const_iterator i = list.begin(); i != list.end(); ++i)
 	{
		if (!i->same_type_as(Bone())) continue;
 		const Bone &bone = i->get(Bone());
 		Matrix matrix = bone.get_animated_matrix();
 		Vector origin = matrix.get_transformed(Vector(0.0, 0.0));
 		Vector direction = matrix.get_transformed(Vector(1.0, 0.0), false).norm();
 		Real length = bone.get_length() * bone.get_scalelx();

 		if (length < 0) {
 			length *= -1;
 			direction *= -1;
 		}

 		const Vector &p0 = origin;
 		const Vector p1 = origin + direction * length;

 		Real r0 = fabs(bone.get_width());
 		Real r1 = fabs(bone.get_tipwidth());
 		Real direction_angle = atan2(direction[1], direction[0]);

 		Real angle0_base = length - precision > fabs(r1 - r0)
 				         ? acos((r0 - r1)/length)
 				         : (r0 > r1 ? 0.0 : PI);
 		Real angle1_base = PI - angle0_base;

 		int segments_count0 = (int)round(2*angle1_base / segment_angle);
 		Real segment_angle0 = 2*angle1_base / (Real)segments_count0;

 		int segments_count1 = (int)round(2*angle0_base / segment_angle);
 		Real segment_angle1 = 2*angle0_base / (Real)segments_count1;

		std::vector<Point> list;
		list.reserve(segments_count0 + segments_count1 + 2);

		int j = 0;
		Real angle = direction_angle + angle0_base;
		while(true)
		{
			list.push_back( Point(r0*cos(angle) + p0[0], r0*sin(angle) + p0[1]) );
			if (j++ >= segments_count0) break; else angle += segment_angle0;
		}
		j = 0;
		while(true)
		{
			list.push_back( Point(r1*cos(angle) + p1[0], r1*sin(angle) + p1[1]) );
			if (j++ >= segments_count1) break; else angle += segment_angle1;
		}

 		add_polygon(list);
 	}
}
