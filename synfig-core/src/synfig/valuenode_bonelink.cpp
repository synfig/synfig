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
#include "valuenode_staticlist.h"
#include "valuenode_boneweightpair.h"
#include "general.h"
#include "valueoperations.h"
#include "boneweightpair.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define epsilon 1e-6

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_BoneLink::ValueNode_BoneLink(const ValueBase &x, etl::loose_handle<Canvas> canvas):
	LinkableValueNode(x.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);

	ValueNode_StaticList::Handle bone_weight_list(ValueNode_StaticList::create(ValueBase::TYPE_BONE_WEIGHT_PAIR, canvas));
	//bone_weight_list->add(ValueNode_BoneWeightPair::create(BoneWeightPair(Bone(), 1), canvas));

	set_link("bone_weight_list", bone_weight_list);
	set_link("base_value",		 ValueNode_Const::create(x));
	set_link("translate",		 ValueNode_Const::create(true));
	set_link("rotate",  		 ValueNode_Const::create(true));
	set_link("skew", 	 		 ValueNode_Const::create(true));
	set_link("scale_x", 		 ValueNode_Const::create(true));
	set_link("scale_y", 		 ValueNode_Const::create(true));

	if (getenv("SYNFIG_DEBUG_SET_PARENT_CANVAS"))
		printf("%s:%d set parent canvas for bone influence to %lx\n", __FILE__, __LINE__, uintptr_t(canvas.get()));
	set_parent_canvas(canvas);
}

ValueNode_BoneLink*
ValueNode_BoneLink::create(const ValueBase &x, etl::loose_handle<Canvas> canvas)
{
	return new ValueNode_BoneLink(x, canvas);
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
	case 0: CHECK_TYPE_AND_SET_VALUE(bone_weight_list_, ValueBase::TYPE_LIST);
	case 1:
		if (get_type() == ValueBase::TYPE_NIL)
		{
			VALUENODE_SET_VALUE(base_value_);
		}
		else
		{
			CHECK_TYPE_AND_SET_VALUE(base_value_, get_type());
		}
	case 2: CHECK_TYPE_AND_SET_VALUE(translate_, ValueBase::TYPE_BOOL);
	case 3: CHECK_TYPE_AND_SET_VALUE(rotate_,    ValueBase::TYPE_BOOL);
	case 4: CHECK_TYPE_AND_SET_VALUE(skew_,      ValueBase::TYPE_BOOL);
	case 5: CHECK_TYPE_AND_SET_VALUE(scale_x_,   ValueBase::TYPE_BOOL);
	case 6: CHECK_TYPE_AND_SET_VALUE(scale_y_,   ValueBase::TYPE_BOOL);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_BoneLink::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
	case 0: return bone_weight_list_;
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
ValueNode_BoneLink::set_root_canvas(etl::loose_handle<Canvas> x)
{
	LinkableValueNode::set_root_canvas(x);
	bone_weight_list_->set_root_canvas(x);
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
	Matrix matrix;
	matrix *= 0;
	vector<ValueBase> bone_weight_list((*bone_weight_list_)(t).get_list());
	Real total_weight = 0;
	for (vector<ValueBase>::iterator iter = bone_weight_list.begin(); iter != bone_weight_list.end(); iter++)
	{
		Bone bone(iter->get(BoneWeightPair()).get_bone());
		Real weight(iter->get(BoneWeightPair()).get_weight());

		if (getenv("SYNFIG_DEBUG_BONE_TRANSFORM_WEIGHTING"))
		{
			printf("%s  *\n", Matrix().set_scale(bone.get_local_scale()).get_string(15, "local scale").c_str());
			printf("%s  =\n", bone.get_animated_matrix().get_string(15, "animated", strprintf("* %.2f (weight)", weight)).c_str());
		}

		matrix += (Matrix().set_scale(bone.get_local_scale()) *
				  bone.get_animated_matrix() *
				  weight);
		total_weight += weight;
	}

	if (getenv("SYNFIG_DEBUG_BONE_TRANSFORM_WEIGHTING"))
	{
		printf("%s:%d matrix:\n%s\n", __FILE__, __LINE__, matrix.get_string().c_str());
		printf("%s:%d total_weight: %.2f\n", __FILE__, __LINE__, total_weight);;
	}

	if (abs(total_weight) > epsilon) {
		matrix *= (1/total_weight);
		if (getenv("SYNFIG_DEBUG_BONE_TRANSFORM_WEIGHTING"))
			printf("%s:%d final matrix:\n%s\n", __FILE__, __LINE__, matrix.get_string().c_str());

		Transformation transformation(matrix);

		bool translate = (*translate_)(t).get(true);
		bool rotate    = (*rotate_)   (t).get(true);
		bool skew      = (*rotate_)   (t).get(true);
		bool scale_x   = (*scale_x_)  (t).get(true);
		bool scale_y   = (*scale_y_)  (t).get(true);

		if (!translate) transformation.offset = Vector(0.0, 0.0);
		if (!rotate) transformation.angle = Angle::zero();
		if (!skew) transformation.skew_angle = Angle::zero();
		if (!scale_x) transformation.scale[0] = 1.0;
		if (!scale_y) transformation.scale[1] = 1.0;
		return transformation;
	}

	return Transformation();
}

ValueBase
ValueNode_BoneLink::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);
	return ValueTransformation::transform(
		get_bone_transformation(t), (*base_value_)(t) );
}

String
ValueNode_BoneLink::get_name()const
{
	return "bone_link";
}

String
ValueNode_BoneLink::get_local_name()const
{
	return _("Bone Link");
}

bool
ValueNode_BoneLink::check_type(ValueBase::TypeId type)
{
	return ValueTransformation::check_type(type);
}

LinkableValueNode::Vocab
ValueNode_BoneLink::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"bone_weight_list")
		.set_local_name(_("Bone Weight List"))
		.set_description(_("List of bones used to calculate the transformation"))
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
