/* === S Y N F I G ========================================================= */
/*!	\file valuenode_derivative.h
**	\brief Header file for implementation of the "Derivative" valuenode conversion.
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

#ifndef __SYNFIG_VALUENODE_DERIVATIVE_H
#define __SYNFIG_VALUENODE_DERIVATIVE_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

// Class ValueNode_Derivative
// Implementation of a derivateive based on finite diferences
// See: http://en.wikipedia.org/wiki/Finite_difference
// and http://en.wikipedia.org/wiki/Finite_difference_coefficients

class ValueNode_Derivative : public LinkableValueNode
{

	ValueNode::RHandle link_;         // Value Node whom is calculated the derivative
	ValueNode::RHandle interval_;     // Size of the interval to calculate the finite differences
	ValueNode::RHandle accuracy_;     // Accuracy order
	ValueNode::RHandle order_;        // First or Second order derivative

	ValueNode_Derivative(const ValueBase &value);

public:

	enum Accuracy
	{
		ROUGH     =0,
		NORMAL    =1,
		FINE      =2,
		EXTREME   =3
	};
	enum Order
	{
		FIRST     =0,
		SECOND    =1
	};
	typedef std::shared_ptr<ValueNode_Derivative> Handle;
	typedef std::shared_ptr<const ValueNode_Derivative> ConstHandle;

	virtual ValueBase operator()(Time t)const;

	virtual ~ValueNode_Derivative();

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
	static ValueNode_Derivative* create(const ValueBase &x);
	virtual Vocab get_children_vocab_vfunc()const;
}; // END of class ValueNode_Derivative

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
