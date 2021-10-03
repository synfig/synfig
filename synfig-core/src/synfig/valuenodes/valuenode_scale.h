/* === S Y N F I G ========================================================= */
/*!	\file valuenode_scale.h
**	\brief Header file for implementation of the "Scale" valuenode conversion.
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

#ifndef __SYNFIG_VALUENODE_SCALE_H
#define __SYNFIG_VALUENODE_SCALE_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_Scale : public LinkableValueNode
{
	ValueNode::RHandle value_node;
	ValueNode::RHandle scalar;

	ValueNode_Scale(const ValueBase &value);

public:
	typedef etl::handle<ValueNode_Scale> Handle;
	typedef etl::handle<const ValueNode_Scale> ConstHandle;

	static ValueNode_Scale* create(const ValueBase &x);
	virtual ~ValueNode_Scale();

	virtual String get_name() const override;
	virtual String get_local_name() const override;
	static bool check_type(Type &type);

	//! Checks if it is possible to call get_inverse() for target_value at time t.
	//! If so, return the link_index related to the return value provided by get_inverse()
	virtual InvertibleStatus is_invertible(const Time& t, const ValueBase& target_value, int* link_index = nullptr) const override;
	//! Returns the modified Link to match the target value at time t
	virtual ValueBase get_inverse(const Time& t, const synfig::ValueBase &target_value) const override;

	virtual ValueBase operator()(Time t) const override;

protected:
	virtual LinkableValueNode* create_new() const override;

	virtual bool set_link_vfunc(int i,ValueNode::Handle x) override;
	virtual ValueNode::LooseHandle get_link_vfunc(int i) const override;

	virtual Vocab get_children_vocab_vfunc() const override;
}; // END of class ValueNode_Scale

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
