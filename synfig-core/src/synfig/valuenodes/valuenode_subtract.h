/* === S Y N F I G ========================================================= */
/*!	\file valuenode_subtract.h
**	\brief Header file for implementation of the "Subtract" valuenode conversion.
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_VALUENODE_SUBTRACT_H
#define __SYNFIG_VALUENODE_SUBTRACT_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_Subtract : public LinkableValueNode
{
	ValueNode::RHandle ref_a;
	ValueNode::RHandle ref_b;
	ValueNode::RHandle scalar;

	ValueNode_Subtract(const ValueBase &value);

public:
	typedef etl::handle<ValueNode_Subtract> Handle;
	typedef etl::handle<const ValueNode_Subtract> ConstHandle;

	static ValueNode_Subtract* create(const ValueBase& value=ValueBase(), etl::loose_handle<Canvas> canvas=nullptr);
	virtual ~ValueNode_Subtract();

	virtual String get_name() const override;
	virtual String get_local_name() const override;
	static bool check_type(Type &type);

	virtual ValueBase operator()(Time t) const override;

protected:
	LinkableValueNode* create_new() const override;

	virtual bool set_link_vfunc(int i,ValueNode::Handle x) override;
	virtual ValueNode::LooseHandle get_link_vfunc(int i) const override;

	virtual Vocab get_children_vocab_vfunc() const override;

public:
	//! Gets the left-hand-side value_node
	ValueNode::Handle get_lhs() const { return ref_a; }

	//! Gets the right-hand-side value_node
	ValueNode::Handle get_rhs() const { return ref_b; }

	//! Gets the scalar value_node
	ValueNode::Handle get_scalar() const { return scalar; }
}; // END of class ValueNode_Subtract

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
