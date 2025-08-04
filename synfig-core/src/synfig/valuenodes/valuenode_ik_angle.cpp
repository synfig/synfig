/* === S Y N F I G ========================================================= */
/*!	\file ValueNode_ik.cpp
**	\brief Implementation of the "Ik angle" valuenode conversion.
**
**	\legal
**	
**  Copyright (c) 2025 ZAINAL AB.
**  Copyright (c) Synfig Contributors
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

#include "valuenode_ik_angle.h"
#include "valuenode_const.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/vector.h>
#include <synfig/canvas.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_IK, RELEASE_VERSION_0_61_09, "ik", N_("IK angle"))

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_IK::ValueNode_IK(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	init_children_vocab();
	if (value.get_type() == type_angle)
		
	{
		set_link("link_pole",ValueNode_Const::create(Vector(Angle::cos(value.get(Angle())).get(),
														 Angle::sin(value.get(Angle())).get())));

		set_link("link_target",ValueNode_Const::create(Vector(100, 0)));
		set_link("length_bone1",ValueNode_Const::create(Real(2.0)));
		set_link("length_bone2",ValueNode_Const::create(Real(2.0)));
		set_link("length_bone3",ValueNode_Const::create(Real(2.0)));
		set_link("flip",ValueNode_Const::create(bool(true))); 
		set_link("joint_bone",ValueNode_Const::create(int(1))); 
		set_link("t_bone",ValueNode_Const::create(int(1))); 
		set_link("f_bone",ValueNode_Const::create(int(1))); // 1 = for bone 1 ,2 for bone 2, 3 for bone 3
		set_link("weight",ValueNode_Const::create(Real(25.0)));
	}
	
}

LinkableValueNode*
ValueNode_IK::create_new()const
{
	return new ValueNode_IK(get_type());
}

ValueNode_IK*
ValueNode_IK::create(const ValueBase& x, etl::loose_handle<Canvas>)
{
	return new ValueNode_IK(x);
}

ValueNode_IK::~ValueNode_IK()
{
	unlink_all();
}

ValueBase
ValueNode_IK::operator()(Time t)const
{
	DEBUG_LOG("SYNFIG_DEBUG_VALUENODE_OPERATORS",
		"%s:%d operator()\n", __FILE__, __LINE__);

    auto hitung_theta2_alpha = [](Real d, Real L1, Real L2, Real p) -> std::pair<Real, Real> {
	    L1 = (L1 > p) ? L1 : p;
	    L2 = (L2 > p) ? L2 : p;
	    Real cos_theta2 = synfig::clamp((d*d - L1*L1 - L2*L2) / (2 * L1 * L2), -1.0, 1.0);
	    Real theta2 = std::acos(cos_theta2);
	    Real cos_alpha = synfig::clamp((d*d + L1*L1 - L2*L2) / (2 * d * L1), -1.0, 1.0);
	    Real alpha = std::acos(cos_alpha);
	    return std::make_pair(theta2, alpha);
	};
    static const Real precision = 0.000000001;
	Vector origin = (*link_pole_)(t).get(Vector());
	Vector target = (*link_target_)(t).get(Vector());
	Real L1 = (*length_bone1_)(Time(0)).get(Real());
	Real L2 = (*length_bone2_)(Time(0)).get(Real());
	Real L3 = (*length_bone3_)(Time(0)).get(Real());
	bool flip = (*flip_)(t).get(bool());
	int bone2 = (*joint_bone_)(Time(0)).get(int());
	int type = (*t_bone_)(Time(0)).get(int());
	int for_bone = (*f_bone_)(Time(0)).get(int());
	Real weight = synfig::clamp((*weight_)(t).get(Real()), 0.0, 100.0);
	Vector delta = target - origin;
	Real dist = delta.mag();
	// ----- 2-BONE MODE -----
	if (bone2==1) {
		dist = std::min(dist, L1 + L2);
		auto [theta2, alpha] = hitung_theta2_alpha(dist, L1, L2, precision);
		Real angle_to_target = std::atan2(delta[1], delta[0]);
		Real theta1 = angle_to_target - alpha;
		if (flip) {
			theta2 = -theta2;
			theta1 = angle_to_target + alpha;
		}
		if (for_bone == 1) return Angle::rad(theta1);
		if (for_bone == 2) return Angle::rad(theta2);
	}
	// ----- 3-BONE MODE -----
	Real x0 = origin[0], y0 = origin[1];
	Real l1l2 = L1 + L2;
	Real fmax = l1l2 + L3;
	Real tomin = l1l2 - (L3 * weight / 100);
	Real L4 = 0.0;
	if (L3 >= l1l2) {
		if (fabs(fmax) > precision){
			L4 = tomin + (l1l2 - tomin) *dist/fmax;
		}
		else L4 = tomin;
	} else {
		tomin = L3 - (L3 * weight / 100);
		Real fmin = (type == 1) ? L3 : 0.0;
		if (fabs(fmax - fmin) > precision){
			L4 = tomin + (l1l2 - tomin) * (dist - fmin) / (fmax - fmin);
		}
		else L4 = tomin;
	}
	Real angle_to_target = std::atan2(delta[1], delta[0]);
	auto [theta2, alpha] = hitung_theta2_alpha(dist, L4, L3, precision);
	Real theta1 = angle_to_target - alpha;
	if ((flip && type !=1) || (!flip && type == 1)) {
		theta2 = -theta2;
		theta1 = angle_to_target + alpha;
	}
	Real tx = x0 + L4 * std::cos(theta1);
	Real ty = y0 + L4 * std::sin(theta1);
	Vector vt(tx, ty);
	Vector base_to_virtual = vt - origin;
	Real distb = L4;
	Real angleb = std::atan2(base_to_virtual[1], base_to_virtual[0]);
	auto [theta2b, alphab] = hitung_theta2_alpha(distb, L1, L2, precision);
	Real theta1b = angleb - alphab;
	if (flip) {
		theta2b = -theta2b;
		theta1b = angleb + alphab;
	}
	if (for_bone == 1) return Angle::rad(theta1b);
	if (for_bone == 2) return Angle::rad(theta2b);
	Real sx = x0 + L1 * std::cos(theta1b);
	Real sy = y0 + L1 * std::sin(theta1b);
	Real x2 = tx + L3 * std::cos(theta1 + theta2);
	Real y2 = ty + L3 * std::sin(theta1 + theta2);
	Real dx3 = (sx - x0) - (x2 - x0);
	Real dy3 = (sy - y0) - (y2 - y0);
	Real dist3 = std::sqrt(dx3 * dx3 + dy3 * dy3);
	auto [theta3, alphax] = hitung_theta2_alpha(dist3, L2, L3, precision);
	if ((flip && type !=1) || (!flip && type == 1)) {
		theta3 = -theta3;
	}
	if (for_bone == 3) return Angle::rad(theta3);
	return Angle::rad(0); // fallback default
}

bool
ValueNode_IK::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link_pole_, type_vector);
	case 1: CHECK_TYPE_AND_SET_VALUE(link_target_, type_vector);
	case 2: CHECK_TYPE_AND_SET_VALUE(length_bone1_, type_real);
	case 3: CHECK_TYPE_AND_SET_VALUE(length_bone2_, type_real);
	case 4: CHECK_TYPE_AND_SET_VALUE(length_bone3_, type_real);
	case 5: CHECK_TYPE_AND_SET_VALUE(flip_, type_bool);
	case 6: CHECK_TYPE_AND_SET_VALUE(joint_bone_, type_integer);
	case 7: CHECK_TYPE_AND_SET_VALUE(t_bone_, type_integer);
	case 8: CHECK_TYPE_AND_SET_VALUE(f_bone_, type_integer);
	case 9: CHECK_TYPE_AND_SET_VALUE(weight_, type_real);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_IK::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());
	if(i==0) return link_pole_;
	if(i==1) return link_target_;
	if(i==2) return length_bone1_;
	if(i==3) return length_bone2_;
	if(i==4) return length_bone3_;
	if(i==5) return flip_;
	if(i==6) return joint_bone_;
	if(i==7) return t_bone_;
	if(i==8) return f_bone_;
	if(i==9) return weight_;
	return 0;
}

bool
ValueNode_IK::check_type(Type &type)
{
	return type==type_angle;
}

LinkableValueNode::Vocab
ValueNode_IK::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;
	ret.push_back(ParamDesc("link_pole")
		.set_local_name(_("Link pole"))
		.set_description(_("Pole position for the IK shoulder"))
	);
	ret.push_back(ParamDesc("link_target")
		.set_local_name(_("Link target"))
		.set_description(_("Target position for the IK target"))
	);
	ret.push_back(ParamDesc("length_bone1")
		.set_local_name(_("Length bone 1"))
		.set_description(_("Length of first bone"))
		.set_static(true)
	);
	ret.push_back(ParamDesc("length_bone2")
		.set_local_name(_("Length bone 2"))
		.set_description(_("Length of second bone"))
		.set_static(true)
	);
	ret.push_back(ParamDesc("length_bone3")
		.set_local_name(_("Length bone 3"))
		.set_description(_("Length of third bone"))
		.set_static(true)
	);
	ret.push_back(ParamDesc("flip")
		.set_local_name(_("Flip"))
		.set_description(_("Flip direction IK."))
	);
	ret.push_back(ParamDesc("joint_bone")
		.set_local_name(_("Joint bone"))
		.set_description(_("Select bone type 2 bones or 3 bones joint bone"))
		.set_hint("enum")
		.set_static(true)
		.add_enum_value(Joint::TWOBONE, "2 bone joint", _("2 Bone joint"))
		.add_enum_value(Joint::THREEBONE, "3 bone joint", _("3 Bone joint"))
	);
	ret.push_back(ParamDesc("t_bone")
		.set_local_name(_("T bone"))
		.set_description(_("Select bone hand/human or foot/animal rig only for 3 joint bone."))
		.set_hint("enum")
		.set_static(true)
		.add_enum_value(Jenisbone::HUMAN, "hand", _("Hand"))
		.add_enum_value(Jenisbone::ANIMAL, "foot", _("Foot"))
	);
	ret.push_back(ParamDesc("f_bone")
		.set_local_name(_("F bone"))
		.set_description(_("Select for bone 1 = up,2 = mid or 3 = down, down only for 3 joint bone"))
		.set_hint("enum")
		.set_static(true)
		.add_enum_value(Forbone::BONE1, "for bone1", _("For bone1"))
		.add_enum_value(Forbone::BONE2, "for bone2", _("For bone2"))
		.add_enum_value(Forbone::BONE3, "for bone3", _("For bone3"))
	);
	ret.push_back(ParamDesc("weight")
		.set_local_name(_("Weight"))
		.set_description(_("Weight percent IK for 3 bone only. Range value 0 to 100"))
	);
	return ret;
}
