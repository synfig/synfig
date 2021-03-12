/* === S Y N F I G ========================================================= */
/*!	\file valuenode_add.h
**	\brief Header file for implementation of the "Add" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
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

#ifndef __SYNFIG_VALUENODE_ADD_H
#define __SYNFIG_VALUENODE_ADD_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_Add : public LinkableValueNode
{
public:
	typedef etl::handle<ValueNode_Add> Handle;
	typedef etl::handle<const ValueNode_Add> ConstHandle;

protected:
	ValueNode_Add(const ValueBase &value);

private:
	ValueNode::RHandle ref_a;
	ValueNode::RHandle ref_b;
	ValueNode::RHandle scalar;

public:
	LinkableValueNode* create_new()const;
	static ValueNode_Add* create(const ValueBase &value=ValueBase());
	virtual ~ValueNode_Add();
	virtual ValueBase operator()(Time t)const;
	virtual bool set_link_vfunc(int i,ValueNode::Handle x);
	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;
	virtual String get_name()const;
	virtual String get_local_name()const;
	static bool check_type(Type &type);
	virtual Vocab get_children_vocab_vfunc()const;

	//! Checks if it is possible to call get_inverse() for target_value at time t.
	//! If so, return the link_index related to the return value provided by get_inverse()
	virtual InvertibleStatus is_invertible(const Time& t, const ValueBase& target_value, int* link_index = nullptr) const;
	//! Returns the modified Link to match the target value at time t
	virtual ValueBase get_inverse(const Time& t, const synfig::ValueBase &target_value) const;
	
}; // END of class ValueNode_Add

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
