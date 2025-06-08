/* === S Y N F I G ========================================================= */
/*!	\file ValueNode_IK.cpp
**	\brief Implementation of the "Vector Angle" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
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

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_IK, RELEASE_VERSION_0_61_09, "ik", N_("Ik angle"))

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_IK::ValueNode_IK(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	init_children_vocab();
	if (value.get_type() == type_angle)
		
	{
		set_link("vector",ValueNode_Const::create(Vector(Angle::cos(value.get(Angle())).get(),
														 Angle::sin(value.get(Angle())).get())));
		set_link("target",ValueNode_Const::create(Vector(100, 0)));
		set_link("length_L1",ValueNode_Const::create(Real(3.0)));
		set_link("length_L2",ValueNode_Const::create(Real(2.0)));
		set_link("flip",ValueNode_Const::create(bool(false)));
		set_link("pole",ValueNode_Const::create(bool(true)));
		//set_link("rotasi_elbow",ValueNode_Const::create(Real(0.0))); // Output kedua
	}
	// else
	// {

	// 	assert(0);
	// 	throw std::runtime_error(get_local_name()+_(":Bad type ")+type.description.local_name);
	// }
	

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

	Vector origin = (*vector_)(t).get(Vector());
	Vector target = (*target_)(t).get(Vector());
	Real L1 = (*length_L1_)(t).get(Real());
	Real L2 = (*length_L2_)(t).get(Real());
	bool flip = (*flip_)(t).get(bool());
	bool pole = (*pole_)(t).get(bool());
	//Real rotasi_elbow = (*rotasi_elbow_)(t).get(Real());
	 // Calculate target distance
    Vector delta = target - origin;
    Real dist = delta.mag();
    
    // Clamp distance to avoid overextension
    dist = std::min(dist, L1 + L2);

    // Calculate angles using law of cosines
    Real cos_theta2 = (dist*dist - L1*L1 - L2*L2) / (2 * L1 * L2);
    cos_theta2 = synfig::clamp(cos_theta2, -1.0, 1.0);
    Real theta2 = std::acos(cos_theta2);

    //Real angle_to_target = delta.angle();not work
    Real angle_to_target = std::atan2(delta[1], delta[0]);//

    Real cos_alpha = (dist*dist + L1*L1 - L2*L2) / (2 * dist * L1);
    cos_alpha = synfig::clamp(cos_alpha, -1.0, 1.0);
    Real alpha = std::acos(cos_alpha);

    // Calculate theta1
    Real theta1 = angle_to_target - alpha;

    // Handle flip
    if(flip)
    {
        theta2 = -theta2;
        theta1 = angle_to_target + alpha;
    }

    if(pole)
    {
		return Angle::rad(theta1);
    else
    {
    	return Angle::rad(theta2);
    }

}


bool
ValueNode_IK::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(vector_, type_vector);
	case 1: CHECK_TYPE_AND_SET_VALUE(target_, type_vector);
	case 2: CHECK_TYPE_AND_SET_VALUE(length_L1_, type_real);
	case 3: CHECK_TYPE_AND_SET_VALUE(length_L2_, type_real);
	case 4: CHECK_TYPE_AND_SET_VALUE(flip_, type_bool);
	case 5: CHECK_TYPE_AND_SET_VALUE(pole_, type_bool);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_IK::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return vector_;
	if(i==1) return target_;
	if(i==2) return length_L1_;
	if(i==3) return length_L2_;
	if(i==4) return flip_;
	if(i==5) return pole_;
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

	ret.push_back(ParamDesc("vector")
		.set_local_name(_("Vector"))
		.set_description(_("The vector where the angle is calculated from"))
	);
	ret.push_back(ParamDesc("target")
		.set_local_name(_("Target"))
		.set_description(_("Target position for the IK"))
	);
	ret.push_back(ParamDesc("length_L1")
		.set_local_name(_("Length_L1"))
		.set_description(_("Length of first bone"))
	);
	ret.push_back(ParamDesc("length_L2")
		.set_local_name(_("Length_L2"))
		.set_description(_("Length of second bone"))
	);
	ret.push_back(ParamDesc("flip")
		.set_local_name(_("Flip"))
		.set_description(_("Flip elbow direction"))
	);
	ret.push_back(ParamDesc("pole")
		.set_local_name(_("Pole"))
		.set_description(_("Select type of rotation for pole or elbouw"))
	);

	return ret;
}
