/* === S Y N F I G ========================================================= */
/*!	\file valuenode_bonelink.cpp
**	\brief Implementation of the "BoneLink" valuenode conversion.
**
**	$Id$
**
**	\legal
**	......... ... 2013 Ivan Mahonin
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

#include "valuenode_bonelink.h"
#include "valuenode_const.h"
#include "valuenode_bone.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/valueoperations.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_BoneLink, RELEASE_VERSION_1_0, "bone_link", "Bone Link")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_BoneLink::ValueNode_BoneLink(const ValueBase &x):
	LinkableValueNode(x.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);

	set_link("bone",			ValueNode_Const::create(ValueNode_Bone::get_root_bone()));
	set_link("base_value",		ValueNode_Const::create(x));
	set_link("translate",		ValueNode_Const::create(true));
	set_link("rotate",  		ValueNode_Const::create(true));
	set_link("skew", 	 		ValueNode_Const::create(true));
	set_link("scale_x", 		ValueNode_Const::create(true));
	set_link("scale_y", 		ValueNode_Const::create(true));
}

ValueNode_BoneLink*
ValueNode_BoneLink::create(const ValueBase &x)
{
	return new ValueNode_BoneLink(x);
}

LinkableValueNode*
ValueNode_BoneLink::create_new()const
{
	return new ValueNode_BoneLink(get_type());
}

ValueNode_BoneLink::~ValueNode_BoneLink()
{
	unlink_all();
}

bool
ValueNode_BoneLink::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());


	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(bone_,      type_bone_valuenode);
	case 1:
		if (get_type() == type_nil)
		{
			VALUENODE_SET_VALUE(base_value_);
		}
		else
		{
			CHECK_TYPE_AND_SET_VALUE(base_value_, get_type());
		}
	case 2: CHECK_TYPE_AND_SET_VALUE(translate_, type_bool);
	case 3: CHECK_TYPE_AND_SET_VALUE(rotate_,    type_bool);
	case 4: CHECK_TYPE_AND_SET_VALUE(skew_,      type_bool);
	case 5: CHECK_TYPE_AND_SET_VALUE(scale_x_,   type_bool);
	case 6: CHECK_TYPE_AND_SET_VALUE(scale_y_,   type_bool);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_BoneLink::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
	case 0: return bone_;
	case 1: return base_value_;
	case 2: return translate_;
	case 3: return rotate_;
	case 4: return skew_;
	case 5: return scale_x_;
	case 6: return scale_y_;
	}
	return 0;
}

void
ValueNode_BoneLink::set_root_canvas(std::shared_ptr<Canvas> x)
{
	LinkableValueNode::set_root_canvas(x);
	bone_->set_root_canvas(x);
	base_value_->set_root_canvas(x);
	translate_->set_root_canvas(x);
	rotate_->set_root_canvas(x);
	skew_->set_root_canvas(x);
	scale_x_->set_root_canvas(x);
	scale_y_->set_root_canvas(x);
}

Transformation
ValueNode_BoneLink::get_bone_transformation(Time t)const
{
	Transformation transformation;
	ValueNode_Bone::Handle bone_node = (*bone_)(t).get(ValueNode_Bone::Handle());
	if (bone_node)
	{
		Bone bone      = (*bone_node) (t).get(Bone());
		bool translate = (*translate_)(t).get(true);
		bool rotate    = (*rotate_)   (t).get(true);
		bool skew      = (*rotate_)   (t).get(true);
		bool scale_x   = (*scale_x_)  (t).get(true);
		bool scale_y   = (*scale_y_)  (t).get(true);

		transformation.set_matrix(
			bone.get_animated_matrix()
		  * Matrix().set_scale(bone.get_local_scale()) );

		if (!translate) transformation.offset = Vector(0.0, 0.0);
		if (!rotate) transformation.angle = Angle::zero();
		if (!skew) transformation.skew_angle = Angle::zero();
		if (!scale_x) transformation.scale[0] = 1.0;
		if (!scale_y) transformation.scale[1] = 1.0;
	}

	return transformation;
}

ValueBase
ValueNode_BoneLink::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);
	return ValueTransformation::transform(
		get_bone_transformation(t), (*base_value_)(t) );
}



bool
ValueNode_BoneLink::check_type(Type &type)
{
	return ValueTransformation::check_type(type);
}

LinkableValueNode::Vocab
ValueNode_BoneLink::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"bone")
		.set_local_name(_("Bone"))
		.set_description(_("The linked bone"))
	);

	ret.push_back(ParamDesc(ValueBase(),"base_value")
		.set_local_name(_("Base value"))
		.set_description(_("Base value"))
	);

	ret.push_back(ParamDesc(ValueBase(),"translate")
		.set_local_name(_("Translate"))
		.set_description(_("Make translation"))
	);

	ret.push_back(ParamDesc(ValueBase(),"rotate")
		.set_local_name(_("Rotate"))
		.set_description(_("Make rotation"))
	);

	ret.push_back(ParamDesc(ValueBase(),"skew")
		.set_local_name(_("Skew"))
		.set_description(_("Make skew"))
	);

	ret.push_back(ParamDesc(ValueBase(),"scale_x")
		.set_local_name(_("Scale X"))
		.set_description(_("Make scaling by X-axis"))
	);

	ret.push_back(ParamDesc(ValueBase(),"scale_y")
		.set_local_name(_("Scale Y"))
		.set_description(_("Make scaling by Y-axis"))
	);

	return ret;
}
