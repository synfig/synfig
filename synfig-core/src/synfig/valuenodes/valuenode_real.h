/* === S Y N F I G ========================================================= */
/*!	\file valuenode_real.h
**	\brief Header file for implementation of the "Real" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
**  Copyright (c) 2013 Konstantin Dmitriev
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

#ifndef __SYNFIG_VALUENODE_REAL_H
#define __SYNFIG_VALUENODE_REAL_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_Real : public LinkableValueNode
{
	ValueNode::RHandle real_;

public:
	typedef etl::handle<ValueNode_Real> Handle;
	typedef etl::handle<const ValueNode_Real> ConstHandle;

	ValueNode_Real(Type &x);
	ValueNode_Real(const ValueBase &x);

	virtual ValueBase operator()(Time t)const;

	virtual ~ValueNode_Real();

	virtual String get_name()const;
	virtual String get_local_name()const;

	//! Checks if it is possible to call get_inverse() for target_value at time t.
	//! If so, return the link_index related to the return value provided by get_inverse()
	virtual InvertibleStatus is_invertible(const Time& t, const ValueBase& target_value, int* link_index = nullptr) const;
	//! Returns the modified Link to match the target value at time t
	virtual ValueBase get_inverse(const Time& t, const synfig::ValueBase &target_value) const;

	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;

protected:
	LinkableValueNode* create_new()const;
	virtual bool set_link_vfunc(int i,ValueNode::Handle x);

public:
	using synfig::LinkableValueNode::get_link_vfunc;

	using synfig::LinkableValueNode::set_link_vfunc;
	static bool check_type(Type &type);
	static ValueNode_Real* create(const ValueBase &x);
	virtual Vocab get_children_vocab_vfunc()const;
}; // END of class ValueNode_Real

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
