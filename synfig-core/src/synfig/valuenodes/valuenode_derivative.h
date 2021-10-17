/* === S Y N F I G ========================================================= */
/*!	\file valuenode_derivative.h
**	\brief Header file for implementation of the "Derivative" valuenode conversion.
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_VALUENODE_DERIVATIVE_H
#define __SYNFIG_VALUENODE_DERIVATIVE_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

// Class ValueNode_Derivative
// Implementation of a derivative based on finite diferences
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
	typedef etl::handle<ValueNode_Derivative> Handle;
	typedef etl::handle<const ValueNode_Derivative> ConstHandle;

	static ValueNode_Derivative* create(const ValueBase& x, etl::loose_handle<Canvas> canvas=nullptr);
	virtual ~ValueNode_Derivative();

	virtual String get_name() const override;
	virtual String get_local_name() const override;
	static bool check_type(Type &type);

	virtual ValueBase operator()(Time t) const override;

protected:
	LinkableValueNode* create_new() const override;

	virtual bool set_link_vfunc(int i,ValueNode::Handle x) override;
	virtual ValueNode::LooseHandle get_link_vfunc(int i) const override;

	virtual Vocab get_children_vocab_vfunc() const override;
}; // END of class ValueNode_Derivative

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
