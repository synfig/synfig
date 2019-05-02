/* === S Y N F I G ========================================================= */
/*!	\file valuenode_dynamic.h
**	\brief Header file for implementation of the "Dynamic" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2014 Carlos López
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_VALUENODE_DYNAMIC_H
#define __SYNFIG_VALUENODE_DYNAMIC_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>
#include "valuenode_derivative.h"
#include "valuenode_const.h"

/* === M A C R O S ========================================================= */
#define MASS_INERTIA_MINIMUM 0.0000001
/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Oscillator;

class ValueNode_Dynamic : public LinkableValueNode
{
	friend class Oscillator;
private:

	ValueNode::RHandle tip_static_;    // Equilibrium position without external forces
	ValueNode::RHandle origin_;        // Basement of the dynamic system
	ValueNode::RHandle force_;         // External force applied on the mass center of gravity
	ValueNode::RHandle torque_;        // Momentum applied at the origin
	ValueNode::RHandle damping_coef_;  // Radial Damping coefficient 
	ValueNode::RHandle friction_coef_; // Rotational friction coefficient
	ValueNode::RHandle spring_coef_;   // Spring coefficient 
	ValueNode::RHandle torsion_coef_;  // Torsion coefficient
	ValueNode::RHandle mass_;          // Mass 
	ValueNode::RHandle inertia_;       // Moment of Inertia
	ValueNode::RHandle spring_rigid_;  // True if spring is solid rigid
	ValueNode::RHandle torsion_rigid_; // True if torsion is solid rigid
	ValueNode::RHandle origin_drags_tip_; // If true result=origin+state otherwise result=state


	ValueNode_Derivative::RHandle origin_d_;      // Derivative of the origin along the time
	mutable Time last_time;
	ValueNode_Dynamic(const ValueBase &value);
		/*
		State types (4) for:
		q=radius
		p=d/dt(radius)
		b=angle
		g=d/dt(angle)

		where

		p=dxdt[0]
		p'=dxdt[1]
		g=dxdt[2]
		g'=dxdt[3]
		q=x[0]
		q'=x[1]
		b=x[2]
		b'=x[3]
		*/
	mutable std::vector<double> state;
	void reset_state(Time t)const;
public:

	typedef etl::handle<ValueNode_Dynamic> Handle;
	typedef etl::handle<const ValueNode_Dynamic> ConstHandle;

	virtual ValueBase operator()(Time t)const;

	virtual ~ValueNode_Dynamic();

	virtual String get_name()const;
	virtual String get_local_name()const;

	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;

protected:
	LinkableValueNode* create_new()const;
	virtual bool set_link_vfunc(int i,ValueNode::Handle x);

public:
	using synfig::LinkableValueNode::get_link_vfunc;

	using synfig::LinkableValueNode::set_link_vfunc;
	static bool check_type(Type &type);
	static ValueNode_Dynamic* create(const ValueBase &x);
	virtual Vocab get_children_vocab_vfunc()const;
}; // END of class ValueNode_Dynamic


class Oscillator
{
	etl::handle<const ValueNode_Dynamic> d;
public:
    Oscillator(const ValueNode_Dynamic* x) : d(x) { }
    void operator() ( const std::vector<double> &x , std::vector<double> &dxdt , const double t )
	{
		Vector u(cos(x[2]), sin(x[2]));
		Vector v(-u[1], u[0]);
		Vector sd=(*(d->origin_d_))(t).get(Vector());
		Vector f=(*(d->force_))(t).get(Vector());
		double to=(*(d->torque_))(t).get(double());
		double c=(*(d->damping_coef_))(t).get(double());
		double mu=(*(d->friction_coef_))(t).get(double());
		double k=(*(d->spring_coef_))(t).get(double());
		double tau=(*(d->torsion_coef_))(t).get(double());
		double m=(*(d->mass_))(t).get(double());
		double i=(*(d->inertia_))(t).get(double());
		Vector tip=(*(d->tip_static_))(t).get(Vector());
	
		double fr=f*u;
		double fa=f*v;
		// Those are the second derivatives (speed of origin)
		double srd=sd*u;
		double sad=sd*v;
		// Calculate the steady position in terms of state
		double r0=tip.mag();
		double a0=(double)(Angle::rad(tip.angle()).get());
		double r=x[0]-r0; // effective radius
		double a=x[2]-a0; // effective alpha
		double rd=x[1]; // radius speed
		double ad=x[3]; // alpha speed
		double imr2=i+m*x[0]*x[0]; // effective inertia
		// Check if the spring rigid
		bool spring_is_rigid=(*(d->spring_rigid_))(t).get(bool());
		// Check if the torsion rigid
		bool torsion_is_rigid=(*(d->torsion_rigid_))(t).get(bool());
		// Integration operations
		dxdt[0]=x[1];
		// Disable movement if the spring is rigid 
		// or if the mass is near to zero but animated.
		if(spring_is_rigid || fabs(m)<=MASS_INERTIA_MINIMUM)
			dxdt[1]=0.0;
		else
			dxdt[1]=(fr-c*rd-k*r)/m-srd;
		dxdt[2]=x[3];
		// Disable rotation if the torsion is rigid
		// or if the inertia is near to zero but animated.
		if(torsion_is_rigid || fabs(imr2)<=MASS_INERTIA_MINIMUM)
			dxdt[3]=0.0;
		else
			dxdt[3]=(to+fa*x[0]-mu*ad-tau*a)/imr2-sad;
	}

};
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
