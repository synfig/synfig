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

#include <algorithm>

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

REGISTER_VALUENODE(ValueNode_Dynamic, RELEASE_VERSION_0_61_06, "dynamic", N_("Dynamic"))


/* === P R O C E D U R E S ================================================= */

struct MathVector : std::vector<Real>
{
	MathVector() = default;
	MathVector(const MathVector&) = default;
	MathVector(std::vector<Real>::iterator begin, std::vector<Real>::iterator end)
		: std::vector<Real>(begin, end)
	{
	}
	MathVector(size_type n)
		: std::vector<Real>(n)
	{
	}

	MathVector& operator=(const MathVector& rhs) = default;

	MathVector& operator+=(const MathVector& rhs)
	{
		std::transform(begin(), end(), rhs.begin(), begin(), std::plus<Real>());
		return *this;
	}

	MathVector operator+(const MathVector& rhs) const
	{
		return MathVector(*this) += rhs;
	}

	MathVector& operator*=(Real rhs)
	{
		for (auto& i: *this)
			i *= rhs;
		return *this;
	}

	MathVector operator*(Real rhs) const
	{
		return MathVector(*this) *= rhs;
	}
};

typedef std::function<void(const double, const std::vector<double>&, std::vector<double>&)> Function;

static void
runge_kutta_cash_karp(Function oscillator, Real& t, MathVector& x, Real& step_size, Real tolerance)
{
	const auto& num_dim = x.size();
	MathVector xf;
	MathVector k1(num_dim), k2(num_dim), k3(num_dim), k4(num_dim), k5(num_dim), k6(num_dim);

	auto max_delta = [] (const MathVector& a, const MathVector& b) -> Real {
		Real max = 0;
		const auto size = a.size();
		for (unsigned int i = 0; i < size; ++i) {
			max = std::max(max, std::fabs(a[i] - b[i]));
		}
		return max;
	};

	typedef std::vector<MathVector> Solution;

	auto VRKF_err = [max_delta](const Solution& y, int order) {
		return std::fabs(std::pow(max_delta(y[order], y[order - 1]), 1./(order + 1)));
	};
	auto VRKF_e = [VRKF_err](const Solution& y, int order, double tolerance) {
		return VRKF_err(y, order) / std::pow(tolerance, 1./(order + 1));
	};
	const Real VRKF_sf = 0.9;

	Real tf(t);

	while (true) {
		tf = t + step_size;
		// k1
		oscillator(t, x, k1);
		k1 *= step_size;
		// k2
		oscillator(t + (1/5.) * step_size, x + k1 * (1/5.), k2);
		k2 *= step_size;
		// k3
		oscillator(t + (3/10.) * step_size, x + k1 * (3/40.) + k2 * (9/40.), k3);
		k3 *= step_size;
		// k4
		oscillator(t + (3/5.) * step_size, x + k1 * (3/10.) + k2 * (-9/10.) + k3 * (6/5.), k4);
		k4 *= step_size;
		// k5
		oscillator(t + /*1. **/ step_size, x + k1 * (-11/54.) + k2 * (5/2.) + k3 * (-70/27.) + k4 * (35/27.), k5);
		k5 *= step_size;
		// k6
		oscillator(t + (7/8.) * step_size, x + k1 * (1631/55296.) + k2 * (175/512.) + k3 * (575/13824.) + k4 * (44275/110592.) + k5 * (253/4096.), k6);
		k6 *= step_size;

		MathVector dy5 = k1 * (37/378.) + k3 * (250/621.) + k4 * (125/594.) + k6 * (512/1771.);
		MathVector dy4 = k1 * (2825/27648.) + k3 * (18575/48384.) + k4 * (13525/55296.) + k5 * (277/14336.) + k6 * (1/4.);

		Solution solution{0, 0, 0, dy4, dy5};
		auto e_4 = VRKF_e(solution, 4, tolerance);
		if (e_4 > 1) {
			// No order less than 5th order is possibly good, so abandon current step
			step_size *= std::max(0.2, VRKF_sf / e_4);
			continue;
		} else {
			// accept 5th order solution

			xf = x + dy5;

			step_size *= std::min(5., VRKF_sf / e_4);
			break;
		}
	}
	x = xf;
	t = tf;
}

static void
integrate(Function oscillator, MathVector& x0, Real to, Real tf, Real step_size, Real tolerance = 1e-5)
{
	Real t = to;
	MathVector x = x0;

	while (t < tf) {
		step_size = std::min(step_size, tf - t);
		runge_kutta_cash_karp(oscillator, t, x, step_size, tolerance);
	}
	x0 = x;
}

/* === M E T H O D S ======================================================= */

ValueNode_Dynamic::ValueNode_Dynamic(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	init_children_vocab();
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
	DEBUG_LOG("SYNFIG_DEBUG_VALUENODE_OPERATORS",
		"%s:%d operator()\n", __FILE__, __LINE__);
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
	MathVector x(state.begin(), state.end());
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

