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

#ifndef __SYNFIG_VALUENODE_INTERPOLATION_DYNAMIC_H
#define __SYNFIG_VALUENODE_INTERPOLATION_DYNAMIC_H

/* === H E A D E R S ======================================================= */

#include "valuenode.h"

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_Dynamic : public LinkableValueNode
{
public:
	/* The type of container used to hold the state vector */
	typedef std::vector< double > state_type;
	/* The rhs of x' = f(x) */
	void oscilator(const state_type &x , state_type &dxdt , const double t);

private:

	ValueNode::RHandle tip_static_;    // Equilibrium position without external forces
	ValueNode::RHandle origin_;        // Basement of the dynamic system
	ValueNode::RHandle force_;         // External force applied on the mass center of gravity
	ValueNode::RHandle damping_coef_;  // Radial Damping coeficient 
	ValueNode::RHandle friction_coef_; // Rotational friction coeficient
	ValueNode::RHandle spring_coef_;   // Spring coeficient 
	ValueNode::RHandle torsion_coef_;  // Torsion coeficient
	ValueNode::RHandle mass_;          // Mass 
	ValueNode::RHandle inertia_;       // Moment of Inertia

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
	state_type x[4];
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
	static bool check_type(ValueBase::Type type);
	static ValueNode_Dynamic* create(const ValueBase &x);
	virtual Vocab get_children_vocab_vfunc()const;
}; // END of class ValueNode_Dynamic

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
