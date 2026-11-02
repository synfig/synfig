/* === S Y N F I G ========================================================= */
/*!	\file valuenode_bonedriver.cpp
**	\brief Implementation of the "Smart Bone Driver" valuenode conversion.
**
**	\legal
**	Copyright (c) 2026 Synfig Contributors
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

#include "valuenode_bonedriver.h"
#include "valuenode_const.h"
#include "valuenode_bone.h"
#include <cmath>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/misc.h>
#include <synfig/valuenode_registry.h>
#include <synfig/blinepoint.h>
#include <synfig/widthpoint.h>
#include <synfig/dashitem.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/angle.h>
#include <synfig/time.h>
#include <synfig/real.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_BoneDriver, RELEASE_VERSION_CURRENT, "bone_driver", N_("Smart Bone Driver"))

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_BoneDriver::ValueNode_BoneDriver(const ValueBase &x):
	LinkableValueNode(x.get_type())
{
	init_children_vocab();

	set_link("bone",      ValueNode_Const::create(ValueNode_Bone::get_root_bone()));
	set_link("val_min",   ValueNode_Const::create(x));
	set_link("val_max",   ValueNode_Const::create(x));
	set_link("min_angle", ValueNode_Const::create(Angle::deg(0)));
	set_link("max_angle", ValueNode_Const::create(Angle::deg(90)));
	set_link("clamp",     ValueNode_Const::create(true));
}

ValueNode_BoneDriver*
ValueNode_BoneDriver::create(const ValueBase& x, etl::loose_handle<Canvas>)
{
	return new ValueNode_BoneDriver(x);
}

LinkableValueNode*
ValueNode_BoneDriver::create_new()const
{
	return new ValueNode_BoneDriver(get_type());
}

ValueNode_BoneDriver::~ValueNode_BoneDriver()
{
	unlink_all();
}

bool
ValueNode_BoneDriver::set_link_vfunc(int i, ValueNode::Handle value)
{
	assert(i >= 0 && i < link_count());

	switch(i)
	{
	case 0:
		if (value && (value->get_type() == type_bone_valuenode || value->get_type() == type_bone_object))
		{
			VALUENODE_SET_VALUE(bone_);
		}
		return false;
	case 1:
		if (get_type() == type_nil)
		{
			VALUENODE_SET_VALUE(val_min_);
		}
		else
		{
			CHECK_TYPE_AND_SET_VALUE(val_min_, get_type());
		}
		return false;
	case 2:
		if (get_type() == type_nil)
		{
			VALUENODE_SET_VALUE(val_max_);
		}
		else
		{
			CHECK_TYPE_AND_SET_VALUE(val_max_, get_type());
		}
		return false;
	case 3: CHECK_TYPE_AND_SET_VALUE(min_angle_, type_angle);
	case 4: CHECK_TYPE_AND_SET_VALUE(max_angle_, type_angle);
	case 5: CHECK_TYPE_AND_SET_VALUE(clamp_,     type_bool);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_BoneDriver::get_link_vfunc(int i)const
{
	assert(i >= 0 && i < link_count());

	switch(i)
	{
	case 0: return bone_;
	case 1: return val_min_;
	case 2: return val_max_;
	case 3: return min_angle_;
	case 4: return max_angle_;
	case 5: return clamp_;
	}
	return 0;
}

void
ValueNode_BoneDriver::set_root_canvas(etl::loose_handle<Canvas> x)
{
	LinkableValueNode::set_root_canvas(x);
	if (bone_) bone_->set_root_canvas(x);
	if (val_min_) val_min_->set_root_canvas(x);
	if (val_max_) val_max_->set_root_canvas(x);
	if (min_angle_) min_angle_->set_root_canvas(x);
	if (max_angle_) max_angle_->set_root_canvas(x);
	if (clamp_) clamp_->set_root_canvas(x);
}

ValueBase
ValueNode_BoneDriver::operator()(Time t)const
{
	DEBUG_LOG("SYNFIG_DEBUG_VALUENODE_OPERATORS",
		"%s:%d operator()\n", __FILE__, __LINE__);

	if (!bone_ || !val_min_ || !val_max_)
		return ValueBase(get_type());

	Bone bone;
	if (bone_->get_type() == type_bone_object)
	{
		bone = (*bone_)(t).get(Bone());
	}
	else if (bone_->get_type() == type_bone_valuenode)
	{
		ValueNode_Bone::Handle bone_node = (*bone_)(t).get(ValueNode_Bone::Handle());
		if (bone_node)
			bone = (*bone_node)(t).get(Bone());
		else
			return (*val_min_)(t);
	}
	else
	{
		return (*val_min_)(t);
	}

	Angle angle = bone.get_angle();
	Angle min_a = min_angle_ ? (*min_angle_)(t).get(Angle()) : Angle::deg(0);
	Angle max_a = max_angle_ ? (*max_angle_)(t).get(Angle()) : Angle::deg(90);
	bool should_clamp = clamp_ ? (*clamp_)(t).get(true) : true;

	Real delta_a = Angle::rad(max_a - min_a).get();
	Real alpha = 0.0;
	if (std::abs(delta_a) > 1e-6)
		alpha = Angle::rad(angle - min_a).get() / delta_a;

	if (should_clamp)
		alpha = synfig::clamp(alpha, Real(0.0), Real(1.0));

	ValueBase v_min = (*val_min_)(t);
	ValueBase v_max = (*val_max_)(t);

	Type &type(get_type());
	if (type == type_real)
	{
		Real a = v_min.get(Real());
		Real b = v_max.get(Real());
		return a + alpha * (b - a);
	}
	if (type == type_time)
	{
		Real a = v_min.get(Time());
		Real b = v_max.get(Time());
		return Time(a + alpha * (b - a));
	}
	if (type == type_integer)
	{
		int a = v_min.get(int());
		int b = v_max.get(int());
		return (int)std::lrint(a + alpha * (b - a));
	}
	if (type == type_bool)
	{
		return alpha >= 0.5 ? v_max.get(bool()) : v_min.get(bool());
	}
	if (type == type_angle)
	{
		Angle a = v_min.get(Angle());
		Angle b = v_max.get(Angle());
		return a + (b - a) * alpha;
	}
	if (type == type_color)
	{
		Color a = v_min.get(Color());
		Color b = v_max.get(Color());
		return a + (b - a) * alpha;
	}
	if (type == type_vector)
	{
		Vector a = v_min.get(Vector());
		Vector b = v_max.get(Vector());
		return a + (b - a) * alpha;
	}
	if (type == type_bline_point)
	{
		BLinePoint a = v_min.get(BLinePoint());
		const BLinePoint &b = v_max.get(BLinePoint());
		a.set_vertex(a.get_vertex() + (b.get_vertex() - a.get_vertex()) * alpha);
		a.set_width(a.get_width() + (b.get_width() - a.get_width()) * alpha);
		a.set_origin(a.get_origin() + (b.get_origin() - a.get_origin()) * alpha);
		a.set_tangent1(a.get_tangent1() + (b.get_tangent1() - a.get_tangent1()) * alpha);
		a.set_tangent2(a.get_tangent2() + (b.get_tangent2() - a.get_tangent2()) * alpha);
		if (alpha >= 0.5)
		{
			a.set_split_tangent_radius(b.get_split_tangent_radius());
			a.set_split_tangent_angle(b.get_split_tangent_angle());
		}
		return a;
	}
	if (type == type_width_point)
	{
		WidthPoint a = v_min.get(WidthPoint());
		const WidthPoint &b = v_max.get(WidthPoint());
		a.set_position(a.get_position() + (b.get_position() - a.get_position()) * alpha);
		a.set_width(a.get_width() + (b.get_width() - a.get_width()) * alpha);
		if (alpha >= 0.5)
		{
			a.set_side_type_before(b.get_side_type_before());
			a.set_side_type_after(b.get_side_type_after());
			a.set_dash(b.get_dash());
		}
		return a;
	}
	if (type == type_dash_item)
	{
		DashItem a = v_min.get(DashItem());
		const DashItem &b = v_max.get(DashItem());
		a.set_offset(a.get_offset() + (b.get_offset() - a.get_offset()) * alpha);
		a.set_length(a.get_length() + (b.get_length() - a.get_length()) * alpha);
		if (alpha >= 0.5)
		{
			a.set_side_type_before(b.get_side_type_before());
			a.set_side_type_after(b.get_side_type_after());
		}
		return a;
	}

	return v_min;
}

bool
ValueNode_BoneDriver::check_type(Type &type)
{
	return type == type_real
		|| type == type_time
		|| type == type_integer
		|| type == type_bool
		|| type == type_angle
		|| type == type_color
		|| type == type_vector
		|| type == type_bline_point
		|| type == type_width_point
		|| type == type_dash_item;
}

LinkableValueNode::Vocab
ValueNode_BoneDriver::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc("bone")
		.set_local_name(_("Bone"))
		.set_description(_("The controlling bone"))
	);
	ret.push_back(ParamDesc("val_min")
		.set_local_name(_("Value at Min Angle"))
		.set_description(_("Output value when bone is at min angle"))
	);
	ret.push_back(ParamDesc("val_max")
		.set_local_name(_("Value at Max Angle"))
		.set_description(_("Output value when bone is at max angle"))
	);
	ret.push_back(ParamDesc("min_angle")
		.set_local_name(_("Min Angle"))
		.set_description(_("Rotation angle corresponding to min value"))
	);
	ret.push_back(ParamDesc("max_angle")
		.set_local_name(_("Max Angle"))
		.set_description(_("Rotation angle corresponding to max value"))
	);
	ret.push_back(ParamDesc("clamp")
		.set_local_name(_("Clamp"))
		.set_description(_("Whether to clamp rotation between min and max angles"))
	);

	return ret;
}
