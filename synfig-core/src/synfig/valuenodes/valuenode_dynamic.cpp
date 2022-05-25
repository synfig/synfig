/* === S Y N F I G ========================================================= */
/*!	\file valuenode_dynamic.cpp
**	\brief Implementation of the "Dynamic" valuenode conversion.
**
**	\legal
**	Copyright (c) 2014 Carlos López
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

#include "valuenode_dynamic.h"
#include "valuenode_const.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/vector.h>

#include <ETL/misc>

#include <boost/numeric/odeint/integrate/integrate.hpp>
#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace boost::numeric::odeint;
/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Dynamic, RELEASE_VERSION_0_61_06, "dynamic", N_("Dynamic"))


/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Dynamic::ValueNode_Dynamic(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	set_link("origin",       ValueNode_Const::create(Vector(0,0)));
	set_link("force",        ValueNode_Const::create(Vector(0,0)));
	set_link("torque",       ValueNode_Const::create(Real(0.0)));
	set_link("damping",      ValueNode_Const::create(Real(0.4)));
	set_link("friction",     ValueNode_Const::create(Real(0.4)));
	set_link("spring",       ValueNode_Const::create(Real(30.0)));
	set_link("torsion",      ValueNode_Const::create(Real(30.0)));
	set_link("mass",         ValueNode_Const::create(Real(0.3)));
	set_link("inertia",      ValueNode_Const::create(Real(0.3)));
	set_link("spring_rigid",ValueNode_Const::create(false));
	set_link("torsion_rigid",ValueNode_Const::create(false));
	set_link("origin_drags_tip",ValueNode_Const::create(true));


	if (get_type() == type_vector)
		set_link("tip_static",ValueNode_Const::create(value.get(Vector())));
	else
		throw Exception::BadType(get_type().description.local_name);

	/* Initial values*/
	state.resize(4);
	reset_state(Time(0.0));

	/*Derivative of the base position*/
	origin_d_=ValueNode_Derivative::create(ValueBase(Vector()));
	origin_d_->set_link("order", ValueNode_Const::create((int)(ValueNode_Derivative::SECOND)));

	/* Initialize the last time called to be 0*/
	last_time=Time(0);
}

void
ValueNode_Dynamic::reset_state(Time t)const
{
	state[0]=((*tip_static_)(t).get(Vector())).mag();
	state[1]=0.0; // d/dt(radius) = 0 initially
	state[2]=(double)(Angle::rad(((*tip_static_)(t).get(Vector())).angle()).get());
	state[3]=0.0; // d/dt(angle) = 0 initially
}
LinkableValueNode*
ValueNode_Dynamic::create_new()const
{
	return new ValueNode_Dynamic(get_type());
}

ValueNode_Dynamic*
ValueNode_Dynamic::create(const ValueBase& x, etl::loose_handle<Canvas>)
{
	return new ValueNode_Dynamic(x);
}

ValueNode_Dynamic::~ValueNode_Dynamic()
{
	unlink_all();
}

ValueBase
ValueNode_Dynamic::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);
	double t0=last_time;
	double t1=t;
	double step;
	// If we are at the initial conditions
	if(t1==t0 && t0==0.0)
	{
		reset_state(Time(0.0));
		return (*origin_)(0).get(Vector()) + Vector(state[0], Angle::rad(state[2]));
	}
	// If we are playing backwards then calculate the value from the initial conditions
	if(t1<t0 && t0>0.0)
	{
		reset_state(Time(0.0));
		last_time=Time(0);
		t0=0.0;
	}
	// Prepare the step size based on distance between start and end time
	step=(t1-t0)/4.0;
	// Before call the integrator we need to be sure that the derivative of the
	// origin is properly set. Maybe the user changed the origin
	ValueNode::RHandle value_node(ValueNode::RHandle::cast_dynamic(origin_d_->get_link("link")));
	value_node->replace(origin_);
	Oscillator oscillator(this);
	std::vector<double> x(state.begin(), state.end());
	integrate(oscillator, x, t0, t1, step);
	// Remember time and state for the next call
	last_time=Time(t);
	state.assign(x.begin(), x.end());
	// We need to check if the spring or the torsion are riggid
	bool spring_is_rigid=(*(spring_rigid_))(t).get(bool());
	bool torsion_is_rigid=(*(torsion_rigid_))(t).get(bool());
	Vector tip=(*(tip_static_))(t).get(Vector());
	// Also check if origin drags tip
	bool origin_drags_tip=(*(origin_drags_tip_))(t).get(bool());

	return Vector(origin_drags_tip?(*origin_)(t).get(Vector()):Vector(0,0))
		+
		Vector(spring_is_rigid?tip.mag():state[0], torsion_is_rigid?tip.angle():Angle::rad(state[2]));
}




bool
ValueNode_Dynamic::check_type(Type &type)
{
	return type==type_vector;
}

bool
ValueNode_Dynamic::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(tip_static_,    get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(origin_,        type_vector);
	case 2: CHECK_TYPE_AND_SET_VALUE(force_,         type_vector);
	case 3: CHECK_TYPE_AND_SET_VALUE(torque_,        type_real);
	case 4: CHECK_TYPE_AND_SET_VALUE(damping_coef_,  type_real);
	case 5: CHECK_TYPE_AND_SET_VALUE(friction_coef_, type_real);
	case 6: CHECK_TYPE_AND_SET_VALUE(spring_coef_,   type_real);
	case 7: CHECK_TYPE_AND_SET_VALUE(torsion_coef_,  type_real);
	case 8: CHECK_TYPE_AND_SET_VALUE(mass_,          type_real);
	case 9: CHECK_TYPE_AND_SET_VALUE(inertia_,       type_real);
	case 10: CHECK_TYPE_AND_SET_VALUE(spring_rigid_, type_bool);
	case 11: CHECK_TYPE_AND_SET_VALUE(torsion_rigid_,type_bool);
	case 12: CHECK_TYPE_AND_SET_VALUE(origin_drags_tip_,type_bool);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Dynamic::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return tip_static_;
	case 1: return origin_;
	case 2: return force_;
	case 3: return torque_;
	case 4: return damping_coef_;
	case 5: return friction_coef_;
	case 6: return spring_coef_;
	case 7: return torsion_coef_;
	case 8: return mass_;
	case 9: return inertia_;
	case 10: return spring_rigid_;
	case 11: return torsion_rigid_;
	case 12: return origin_drags_tip_;
	default:
	return 0;
	}
}

LinkableValueNode::Vocab
ValueNode_Dynamic::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;
	ret.push_back(ParamDesc(ValueBase(),"tip_static")
		.set_local_name(_("Tip static"))
		.set_description(_("Equilibrium tip position without external forces"))
	);
	ret.push_back(ParamDesc(ValueBase(),"origin")
		.set_local_name(_("Origin"))
		.set_description(_("Basement of the dynamic system"))
		.set_is_distance()
	);
	ret.push_back(ParamDesc(ValueBase(),"force")
		.set_local_name(_("Force"))
		.set_description(_("External force applied on the mass center of gravity"))
	);
	ret.push_back(ParamDesc(ValueBase(),"torque")
		.set_local_name(_("Torque"))
		.set_description(_("External momentum applied at the center of inertia"))
	);
	ret.push_back(ParamDesc(ValueBase(),"damping")
		.set_local_name(_("Damping coefficient"))
		.set_description(_("Radial damping coefficient of the dynamic system"))
	);
	ret.push_back(ParamDesc(ValueBase(),"friction")
		.set_local_name(_("Friction coefficient"))
		.set_description(_("Rotational friction coefficient of the dynamic system"))
	);
	ret.push_back(ParamDesc(ValueBase(),"spring")
		.set_local_name(_("Spring coefficient"))
		.set_description(_("Radial spring coefficient of the dynamic system"))
	);
	ret.push_back(ParamDesc(ValueBase(),"torsion")
		.set_local_name(_("Torsion coefficient"))
		.set_description(_("Torsion coefficient of the dynamic system"))
	);
	ret.push_back(ParamDesc(ValueBase(),"mass")
		.set_local_name(_("Mass"))
		.set_description(_("Mass of the dynamic system"))
	);
	ret.push_back(ParamDesc(ValueBase(),"inertia")
		.set_local_name(_("Moment of Inertia"))
		.set_description(_("Moment of inertia of the dynamic system"))
	);
	ret.push_back(ParamDesc(ValueBase(),"spring_rigid")
		.set_local_name(_("Spring rigid"))
		.set_description(_("When checked, linear spring is rigid"))
	);
	ret.push_back(ParamDesc(ValueBase(),"torsion_rigid")
		.set_local_name(_("Torsion rigid"))
		.set_description(_("When checked, torsion spring is rigid"))
	);
	ret.push_back(ParamDesc(ValueBase(),"origin_drags_tip")
		.set_local_name(_("Origin drags tip"))
		.set_description(_("When checked, result is origin + tip otherwise result is just tip"))
	);
	return ret;
}

